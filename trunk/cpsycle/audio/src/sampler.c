// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sampler.h"
#include "pattern.h"
#include "plugin_interface.h"
#include "instruments.h"
#include "samples.h"
#include "songio.h"
#include <operations.h>
#include <linear.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "../../detail/portable.h"
#include "constants.h"

// Version for the sampler machine data.
// The instruments and sample bank versions are saved with the song chunk
// versioning
#define XMSAMPLER_VERSION 0x00010001
// Version zero was the development version (no format freeze). Version one is
// the published one.
#define XMSAMPLER_VERSION_ONE 0x00010000
// Version Sampler PS1
#define SAMPLERVERSION 0x00000002

// SamplerChannel
static void psy_audio_samplerchannel_init(psy_audio_SamplerChannel*);
static void psy_audio_samplerchannel_dispose(psy_audio_SamplerChannel*);
static void psy_audio_samplerchannel_seteffect(psy_audio_SamplerChannel*,
	const psy_audio_PatternEvent*);

void psy_audio_samplerchannel_init(psy_audio_SamplerChannel* self)
{
	self->volume = 1.f;
	self->panfactor = (psy_dsp_amp_t) 0.5f;
}

void psy_audio_samplerchannel_dispose(psy_audio_SamplerChannel* self)
{
}

void psy_audio_samplerchannel_seteffect(psy_audio_SamplerChannel* self,
	const psy_audio_PatternEvent* ev)
{
	switch (ev->cmd) {
		case SET_CHANNEL_VOLUME:
			self->volume = (psy_dsp_amp_t) ((ev->parameter < 64)
				? (ev->parameter / 64.0f)
				: 1.0f);
		break;
		case SAMPLER_CMD_PANNING:
			self->panfactor = ev->parameter / (psy_dsp_amp_t) 255;
		break;
		default:
		;
	}
}

// Sampler
static void generateaudio(psy_audio_Sampler*, psy_audio_BufferContext*);
static void seqtick(psy_audio_Sampler*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void sequencerlinetick(psy_audio_Sampler*);
static psy_List* sequencerinsert(psy_audio_Sampler*, psy_List* events);
static void stop(psy_audio_Sampler*);
static const psy_audio_MachineInfo* info(psy_audio_Sampler*);
// Parameters
static unsigned int numparametercols(psy_audio_Sampler*);
static uintptr_t numparameters(psy_audio_Sampler*);
static psy_audio_MachineParam* parameter(psy_audio_Sampler* self,
	uintptr_t param);
static void dispose(psy_audio_Sampler*);
static int alloc_voice(psy_audio_Sampler*);
static void releaseallvoices(psy_audio_Sampler*);
static psy_audio_SamplerVoice* activevoice(psy_audio_Sampler*, uintptr_t channel);
static void releasevoices(psy_audio_Sampler*, uintptr_t channel);
static void nnavoices(psy_audio_Sampler*, uintptr_t channel);
static void removeunusedvoices(psy_audio_Sampler* self);
static uintptr_t numinputs(psy_audio_Sampler*);
static uintptr_t numoutputs(psy_audio_Sampler*);
static void loadspecific(psy_audio_Sampler*, psy_audio_SongFile*,
	uintptr_t slot);
static bool loadxmsamplerchannel(psy_audio_Sampler*, psy_audio_SongFile*);
static void savespecific(psy_audio_Sampler*, psy_audio_SongFile*,
	uintptr_t slot);

static void disposechannels(psy_audio_Sampler*);
static psy_audio_SamplerChannel* sampler_channel(psy_audio_Sampler*, uintptr_t channelnum);
static int currslot(psy_audio_Sampler*, uintptr_t channel,
	const psy_audio_PatternEvent*);

static psy_audio_MachineInfo const macinfo = {
	MI_VERSION,
	0x0250,
	GENERATOR | 32 | 64 | MACH_SUPPORTS_INSTRUMENTS,
	MACHMODE_GENERATOR,
	"Sampler"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Sampler",
	"Psycledelics",
	"help",	
	MACH_SAMPLER,
	0,
	0
};

const psy_audio_MachineInfo* psy_audio_sampler_info(void)
{
	return &macinfo;
}

static MachineVtable sampler_vtable;
static int sampler_vtable_initialized = 0;

static void sampler_vtable_init(psy_audio_Sampler* self)
{
	if (!sampler_vtable_initialized) {
		sampler_vtable = *(psy_audio_sampler_base(self)->vtable);
		sampler_vtable.generateaudio = (fp_machine_generateaudio) generateaudio;
		sampler_vtable.seqtick = (fp_machine_seqtick) seqtick;
		sampler_vtable.sequencerlinetick = (fp_machine_sequencerlinetick) sequencerlinetick;
		sampler_vtable.sequencerinsert = (fp_machine_sequencerinsert) sequencerinsert;
		sampler_vtable.stop = (fp_machine_stop) stop;
		sampler_vtable.info = (fp_machine_info) info;
		sampler_vtable.dispose = (fp_machine_dispose) dispose;
		sampler_vtable.numinputs = (fp_machine_numinputs) numinputs;
		sampler_vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		sampler_vtable.loadspecific = (fp_machine_loadspecific) loadspecific;
		sampler_vtable.savespecific = (fp_machine_savespecific) savespecific;
		sampler_vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		sampler_vtable.numparameters = (fp_machine_numparameters) numparameters;
		sampler_vtable.parameter = (fp_machine_parameter) parameter;
		sampler_vtable_initialized = 1;
	}
}

void psy_audio_sampler_init(psy_audio_Sampler* self, MachineCallback callback)
{	
	uintptr_t i;

	custommachine_init(&self->custommachine, callback);	
	sampler_vtable_init(self);
	psy_audio_sampler_base(self)->vtable = &sampler_vtable;
	psy_audio_machine_seteditname(psy_audio_sampler_base(self), "Sampler");
	self->numvoices = SAMPLER_DEFAULT_POLYPHONY;	
	self->voices = 0;	
	self->resamplingmethod = RESAMPLERTYPE_LINEAR;
	self->defaultspeed = 1;	
	self->maxvolume = 0xFF;
	self->panpersistent = 0;
	self->xmsamplerload = 0;
	self->basec = 48;
	psy_table_init(&self->channels);
	psy_table_init(&self->lastinst);
	psy_audio_intmachineparam_init(&self->param_numvoices,
		"Polyphony", "Polyphony", MPF_STATE,
		(int32_t*)&self->numvoices,
		1, 64);
	psy_audio_choicemachineparam_init(&self->param_resamplingmethod,
		"Resampling", "Resampling", MPF_STATE,
		(int32_t*)&self->resamplingmethod,
		0, 3);
	for (i = 0; i < RESAMPLERTYPE_NUMRESAMPLERS; ++i) {
		psy_audio_choicemachineparam_setdescription(&self->param_resamplingmethod, i,
			psy_dsp_multiresampler_name((ResamplerType)i));
	}
	psy_audio_choicemachineparam_init(&self->param_defaultspeed,
		"Default speed", "Default speed", MPF_STATE,
		(int32_t*)&self->defaultspeed,
		0, 1);	
	psy_audio_choicemachineparam_setdescription(&self->param_defaultspeed, 0,
		"played by C3"); 
	psy_audio_choicemachineparam_setdescription(&self->param_defaultspeed, 1,
		"played by C4");
	psy_audio_intmachineparam_init(&self->param_maxvolume,
		"Max volume", "Max volume", MPF_STATE,
		(int32_t*)&self->maxvolume,
		0, 255);
	psy_audio_intmachineparam_setmask(&self->param_maxvolume, "%0X");
	psy_audio_choicemachineparam_init(&self->param_panpersistent,
		"Pan Persistence", "Pan Persistence", MPF_STATE,
		(int32_t*)&self->panpersistent,
		0, 1);
	psy_audio_choicemachineparam_setdescription(&self->param_panpersistent, 0,
		"reset on new note");
	psy_audio_choicemachineparam_setdescription(&self->param_panpersistent, 1,
		"keep on channel");
}

void dispose(psy_audio_Sampler* self)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; p = p->next) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;
		psy_audio_samplervoice_dispose(voice);		
		free(voice);
	}
	psy_list_free(self->voices);
	self->voices = 0;
	psy_audio_intmachineparam_dispose(&self->param_numvoices);
	psy_audio_choicemachineparam_dispose(&self->param_resamplingmethod);
	psy_audio_choicemachineparam_dispose(&self->param_defaultspeed);
	psy_audio_intmachineparam_dispose(&self->param_maxvolume);
	disposechannels(self);
	custommachine_dispose(&self->custommachine);	
}

void disposechannels(psy_audio_Sampler* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->channels);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_SamplerChannel* channel;

		channel = (psy_audio_SamplerChannel*) psy_tableiterator_value(&it);
		psy_audio_samplerchannel_dispose(channel);
		free(channel);
	}
	psy_table_dispose(&self->channels);
}

psy_audio_Sampler* psy_audio_sampler_alloc(void)
{
	return (psy_audio_Sampler*) malloc(sizeof(psy_audio_Sampler));
}

psy_audio_Sampler* psy_audio_sampler_allocinit(MachineCallback callback)
{
	psy_audio_Sampler* rv;

	rv = psy_audio_sampler_alloc();
	if (rv) {
		psy_audio_sampler_init(rv, callback);
	}
	return rv;
}

psy_audio_SamplerChannel* sampler_channel(psy_audio_Sampler* self, uintptr_t track)
{
	psy_audio_SamplerChannel* rv;

	rv = psy_table_at(&self->channels, track);
	if (rv == NULL) {
		rv = malloc(sizeof(psy_audio_SamplerChannel));
		psy_audio_samplerchannel_init(rv);
		psy_table_insert(&self->channels, track, (void*) rv);
	}
	return rv;
}

void generateaudio(psy_audio_Sampler* self, psy_audio_BufferContext* bc)
{	
	psy_List* p;
	uintptr_t c = 0;
	
	removeunusedvoices(self);
	for (p = self->voices; p != NULL && c < self->numvoices; p = p->next, ++c) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;		
		psy_audio_samplervoice_work(voice, bc->output, bc->numsamples);
	}	
}

void seqtick(psy_audio_Sampler* self, uintptr_t channelnum,
	const psy_audio_PatternEvent* event)
{		
	psy_audio_SamplerVoice* voice = 0;
	psy_audio_SamplerChannel* channel = 0;

	if (event->cmd == SAMPLER_CMD_EXTENDED) {
		if ((event->parameter & 0xF0) == SAMPLER_CMD_EXT_NOTEDELAY) {
			// skip for now and reinsert in sequencerinsert
			// with delayed offset
			return;
		}
	}

	channel = sampler_channel(self, channelnum);
	if (channel) {
		psy_audio_samplerchannel_seteffect(channel, event);
	}
	if (event->note == NOTECOMMANDS_RELEASE) {
		releasevoices(self, channelnum);
		return;
	}
	if (event->note < NOTECOMMANDS_RELEASE) {
		nnavoices(self, channelnum);
	} else {
		voice = activevoice(self, channelnum);
	}
	if (!voice) {		
		psy_audio_Instrument* instrument;
		
		instrument = instruments_at(psy_audio_machine_instruments(
			psy_audio_sampler_base(self)),
			currslot(self, channelnum, event));
		if (instrument) {
			voice = psy_audio_samplervoice_allocinit(self, instrument,
				channel,
				channelnum,
				psy_audio_machine_samplerate(psy_audio_sampler_base(self)));
			psy_list_append(&self->voices, voice);
		}
	}	
	if (voice) {
		psy_audio_samplervoice_seqtick(voice, event,
			1 / psy_audio_machine_beatspersample(
				psy_audio_sampler_base(self)));
	}
}

void sequencerlinetick(psy_audio_Sampler* self)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; p = p->next) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;		
		// voice->portaspeed = 1.0;
	}
}

void stop(psy_audio_Sampler* self)
{
	releaseallvoices(self);
}

int currslot(psy_audio_Sampler* self, uintptr_t channel,
	const psy_audio_PatternEvent* event)
{
	int rv;

	if (event->inst != NOTECOMMANDS_EMPTY) {
		psy_table_insert(&self->lastinst, channel, (void*)(uintptr_t)event->inst);
		rv = event->inst;
	} else
	if (psy_table_exists(&self->lastinst, channel)) {
		rv = (int)(uintptr_t) psy_table_at(&self->lastinst, channel);
	} else { 
		rv = NOTECOMMANDS_EMPTY;
	}
	return rv;
}

void releaseallvoices(psy_audio_Sampler* self)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; p = p->next) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;		
		psy_audio_samplervoice_release(voice);		
	}
}

void releasevoices(psy_audio_Sampler* self, uintptr_t channel)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; p = p->next) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;
		if (voice->channelnum == channel) {			
			psy_audio_samplervoice_release(voice);
		}
	}
}

void nnavoices(psy_audio_Sampler* self, uintptr_t channel)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; p = p->next) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;
		if (voice->channelnum == channel) {			
			psy_audio_samplervoice_nna(voice);
		}
	}
}

psy_audio_SamplerVoice* activevoice(psy_audio_Sampler* self, uintptr_t channel)
{
	psy_audio_SamplerVoice* rv = 0;	
	psy_List* p = 0;
	
	for (p = self->voices; p != NULL; p = p->next) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;
		if (voice->channelnum == channel && voice->env.stage != ENV_RELEASE
				&& voice->env.stage != ENV_OFF) {
			rv = voice;
			break;
		}
	}
	return rv;
}

void removeunusedvoices(psy_audio_Sampler* self)
{
	psy_List* p;
	psy_List* q;
		
	for (p = self->voices; p != NULL; p = q) {
		psy_audio_SamplerVoice* voice;

		q = p->next;
		voice = (psy_audio_SamplerVoice*) p->entry;				
		if (voice->env.stage == ENV_OFF) {
			psy_audio_samplervoice_dispose(voice);
			free(voice);
			psy_list_remove(&self->voices, p);
		}			
	}
}

const psy_audio_MachineInfo* info(psy_audio_Sampler* self)
{	
	return &macinfo;
}

uintptr_t numparameters(psy_audio_Sampler* self)
{
	return 5;
}

unsigned int numparametercols(psy_audio_Sampler* self)
{
	return 5;
}

uintptr_t numinputs(psy_audio_Sampler* self)
{
	return 0;
}

uintptr_t numoutputs(psy_audio_Sampler* self)
{
	return 2;
}

// psy_audio_SamplerVoice
void psy_audio_samplervoice_init(psy_audio_SamplerVoice* self,
	psy_audio_Sampler* sampler,
	psy_audio_Samples* samples,
	psy_audio_Instrument* instrument,
	psy_audio_SamplerChannel* channel,
	uintptr_t channelnum,
	unsigned int samplerate,
	int resamplingmethod,
	int maxvolume) 
{	
	self->sampler = sampler;
	self->samples = samples;
	self->instrument = instrument;
	self->channelnum = channelnum;
	self->channel = channel;
	self->usedefaultvolume = 1;
	self->vol = 1.f;
	self->pan = 0.5f;
	self->dopan = 0;
	self->positions = 0;
	self->portaspeed = 1.0;	
	self->effcmd = SAMPLER_CMD_NONE;
	self->effval = 0;
	self->dooffset = 0;
	self->maxvolume = maxvolume;
	self->positions = 0;
	if (instrument) {
		psy_dsp_adsr_init(&self->env, &instrument->volumeenvelope, samplerate);
		psy_dsp_adsr_init(&self->filterenv, &instrument->filterenvelope, samplerate);	
	} else {
		psy_dsp_adsr_initdefault(&self->env, samplerate);
		psy_dsp_adsr_initdefault(&self->filterenv, samplerate);
	}	
	psy_dsp_multifilter_init(&self->filter_l);
	psy_dsp_multifilter_init(&self->filter_r);			
	psy_dsp_multiresampler_init(&self->resampler);
	psy_dsp_multiresampler_settype(&self->resampler,
		resamplingmethod);
	if (instrument) {
		psy_dsp_filter_setcutoff(psy_dsp_multifilter_base(&self->filter_l),
			self->instrument->filtercutoff);
		psy_dsp_filter_setcutoff(psy_dsp_multifilter_base(&self->filter_r),
			self->instrument->filtercutoff);	
		psy_dsp_filter_setressonance(psy_dsp_multifilter_base(
			&self->filter_l), self->instrument->filterres);
		psy_dsp_filter_setressonance(psy_dsp_multifilter_base(
			&self->filter_r), self->instrument->filterres);
		psy_dsp_multifilter_settype(&self->filter_l, instrument->filtertype);
		psy_dsp_multifilter_settype(&self->filter_r, instrument->filtertype);
	}
}

void psy_audio_samplervoice_reset(psy_audio_SamplerVoice* self)
{
	self->effcmd = SAMPLER_CMD_NONE;
	self->portaspeed = 1.0;	
	psy_dsp_adsr_reset(&self->env);
	psy_dsp_adsr_reset(&self->filterenv);
	psy_dsp_filter_reset(psy_dsp_multifilter_base(&self->filter_l));
	psy_dsp_filter_reset(psy_dsp_multifilter_base(&self->filter_r));
}

void psy_audio_samplervoice_dispose(psy_audio_SamplerVoice* self)
{
	psy_audio_samplervoice_clearpositions(self);
	self->positions = 0;	
}

psy_audio_SamplerVoice* psy_audio_samplervoice_alloc(void)
{
	return (psy_audio_SamplerVoice*) malloc(sizeof(psy_audio_SamplerVoice));
}

psy_audio_SamplerVoice* psy_audio_samplervoice_allocinit(psy_audio_Sampler* sampler,
	psy_audio_Instrument* instrument,
	psy_audio_SamplerChannel* channel,
	uintptr_t channelnum,
	unsigned int samplerate)
{
	psy_audio_SamplerVoice* rv;

	rv = psy_audio_samplervoice_alloc();
	if (rv) {
		psy_audio_samplervoice_init(rv,
			sampler,
			psy_audio_machine_samples(psy_audio_sampler_base(sampler)),
			instrument,
			channel,
			channelnum,
			samplerate,
			sampler->resamplingmethod,
			sampler->maxvolume);
	}
	return rv;
}

void psy_audio_samplervoice_seqtick(psy_audio_SamplerVoice* self, const psy_audio_PatternEvent* event, double samplesprobeat)
{
	self->dopan = 0;	
	if (event->cmd == SAMPLER_CMD_VOLUME) {
		 self->usedefaultvolume = 0;
		 self->vol = event->parameter / 
			 (psy_dsp_amp_t) self->maxvolume;
	} else
	if (event->cmd == SAMPLER_CMD_PANNING) {
		self->dopan = 1;
		self->pan = event->parameter / (psy_dsp_amp_t) 255;
	} else
	if (event->cmd == SAMPLER_CMD_OFFSET) {
		self->dooffset = 1;
		self->offset = event->parameter;
	} else
	if (event->cmd == SAMPLER_CMD_PORTAUP) {
		self->effval = event->parameter;
		self->effcmd = event->cmd;
		self->portanumframes = (uintptr_t) samplesprobeat;
		self->portacurrframe = 0;
		self->portaspeed = pow(2.0f, event->parameter / 12.0 *
			1.0 / samplesprobeat);
	} else
	if (event->cmd == SAMPLER_CMD_PORTADOWN) {
		self->effval = event->parameter;
		self->effcmd = event->cmd;
		self->portanumframes = (uintptr_t) samplesprobeat;
		self->portacurrframe = 0;
		self->portaspeed = pow(2.0f, -event->parameter / 12.0 * 1.0 / samplesprobeat);
	}	
	if (event->note < NOTECOMMANDS_RELEASE) {		
		psy_audio_samplervoice_noteon(self, event, samplesprobeat);
	}
}

void psy_audio_samplervoice_noteon(psy_audio_SamplerVoice* self, const psy_audio_PatternEvent* event, double samplesprobeat)
{	
	psy_audio_Sample* sample;
	psy_List* entries;
	psy_List* p;
						
	psy_audio_samplervoice_clearpositions(self);
	entries = instrument_entriesintersect(self->instrument,
		event->note, 127, 0);
	for (p = entries; p != NULL; p = p->next) {
		psy_audio_InstrumentEntry* entry;
		
		entry = (psy_audio_InstrumentEntry*) p->entry;
		sample = psy_audio_samples_at(self->samples, entry->sampleindex);
		if (sample) {
			SampleIterator* iterator;			
			
			iterator = sampleiterator_alloc();
			*iterator = sample_begin(sample);
			iterator->resampler_data =
				psy_dsp_multiresampler_base(&self->resampler)->vtable->getresamplerdata(
					psy_dsp_multiresampler_base(&self->resampler));
			psy_list_append(&self->positions, iterator);
			if (self->instrument->loop && self->sampler) {
				psy_dsp_beat_t bpl;
				double totalsamples;
				
				bpl = psy_audio_machine_currbeatsperline(
					psy_audio_sampler_base(self->sampler));		
				totalsamples = samplesprobeat * bpl * self->instrument->lines;
				iterator->speed = (int64_t)(4294967296.0f *
					(sample->numframes / (double)totalsamples));
			} else {
				iterator->speed = (int64_t)(4294967296.0f *
					pow(2.0f,
						(event->note + sample->tune - self->sampler->basec +
							((psy_dsp_amp_t)sample->finetune * 0.01f)) / 12.0f) *
					((psy_dsp_beat_t)sample->samplerate / 44100));
			}
			psy_dsp_resampler_setspeed(psy_dsp_multiresampler_base(
				&self->resampler),
				iterator->resampler_data,
				iterator->speed * 1/ 4294967296.0f);
		}
	}	
	psy_list_free(entries);	
	if (self->positions) {		
		psy_dsp_adsr_start(&self->env);		
		psy_dsp_adsr_start(&self->filterenv);
	}
	if (!self->dopan && self->instrument->randompan) {
		self->dopan = 1; 
		self->pan = rand() / (psy_dsp_amp_t) 32768.f;
	}
}

void psy_audio_samplervoice_noteon_frequency(psy_audio_SamplerVoice* self, double frequency)
{
	psy_audio_Sample* sample;
	psy_List* entries;
	psy_List* p;

	psy_audio_samplervoice_clearpositions(self);
	entries = instrument_entriesintersect(self->instrument,
		0, 0, frequency);
	for (p = entries; p != NULL; p = p->next) {
		psy_audio_InstrumentEntry* entry;

		entry = (psy_audio_InstrumentEntry*)p->entry;
		sample = psy_audio_samples_at(self->samples, entry->sampleindex);
		if (sample) {
			SampleIterator* iterator;

			iterator = sampleiterator_alloc();
			*iterator = sample_begin(sample);
			iterator->resampler_data =
				psy_dsp_multiresampler_base(&self->resampler)->vtable->getresamplerdata(
					psy_dsp_multiresampler_base(&self->resampler));
			psy_list_append(&self->positions, iterator);
			iterator->speed = (int64_t)(4294967296.0f *
				frequency / 440);
			psy_dsp_resampler_setspeed(psy_dsp_multiresampler_base(
				&self->resampler),
				iterator->resampler_data,
				iterator->speed * 1 / 4294967296.0f);
		}
	}
	psy_list_free(entries);
	if (self->positions) {
		psy_dsp_adsr_start(&self->env);
		psy_dsp_adsr_start(&self->filterenv);
	}
	if (!self->dopan && self->instrument->randompan) {
		self->dopan = 1;
		self->pan = rand() / (psy_dsp_amp_t)32768.f;
	}
}

void psy_audio_samplervoice_clearpositions(psy_audio_SamplerVoice* self)
{
	psy_List* p;

	for (p = self->positions; p != NULL; p = p->next) {
		SampleIterator* iterator;

		iterator = (SampleIterator*)p->entry;
		psy_dsp_multiresampler_base(&self->resampler)->vtable->disposeresamplerdata(
			psy_dsp_multiresampler_base(&self->resampler),
			iterator->resampler_data);
		free(iterator);
	}
	psy_list_free(self->positions);
	self->positions = 0;
}

void psy_audio_samplervoice_nna(psy_audio_SamplerVoice* self)
{
	if (self->instrument) {
		switch (instrument_nna(self->instrument)) {
			case psy_audio_NNA_STOP:
				psy_audio_samplervoice_fastnoteoff(self);
			break;
			case psy_audio_NNA_NOTEOFF:
				psy_audio_samplervoice_noteoff(self);
			break;
			case psy_audio_NNA_CONTINUE:				
			break;
			default:
				// note cut
				psy_audio_samplervoice_fastnoteoff(self);
			break;
		}
	}
}

void psy_audio_samplervoice_noteoff(psy_audio_SamplerVoice* self)
{
	psy_dsp_adsr_release(&self->env);
	psy_dsp_adsr_release(&self->filterenv);	
}

void psy_audio_samplervoice_fastnoteoff(psy_audio_SamplerVoice* self)
{
	psy_dsp_adsr_fastrelease(&self->env);
	psy_dsp_adsr_fastrelease(&self->filterenv);
}

void psy_audio_samplervoice_work(psy_audio_SamplerVoice* self, psy_audio_Buffer* output, uintptr_t numsamples)
{		
	if (self->positions && self->env.stage != ENV_OFF) {
		psy_List* p;
		psy_dsp_amp_t* env;
		uintptr_t i;
		uintptr_t portaframe = self->portacurrframe;
		double portaspeed = 1.0;
		int portadone = 0;

		env = malloc(numsamples * sizeof(psy_dsp_amp_t));
		for (i = 0; i < numsamples; ++i) {
			psy_dsp_adsr_tick(&self->env);
			env[i] = self->env.value;
		}
		
		for (p = self->positions; p != NULL; p = p->next) {
			SampleIterator* position;
			uintptr_t i;
			psy_dsp_amp_t panning;
			psy_dsp_amp_t svol;
			psy_dsp_amp_t cvol;
			psy_dsp_amp_t rvol;
			psy_dsp_amp_t lvol;
		
			position = (SampleIterator*) p->entry;
			if (self->dooffset) {
				uint64_t w_offset;

				w_offset = self->offset * position->sample->numframes;
				w_offset = w_offset << 24;
				position->pos.QuadPart += w_offset;
				if (position->pos.HighPart > position->sample->numframes) {
					double_setvalue(&position->pos, 0.0);
				}
			}
			svol = position->sample->globalvolume *
						(self->usedefaultvolume
							? position->sample->defaultvolume
							: self->vol);
			cvol = self->channel ? self->channel->volume : (psy_dsp_amp_t) 1.f;
			if (self->channel && self->sampler->panpersistent) {
				panning = self->channel->panfactor;
			} else {
				panning = self->dopan ? self->pan : position->sample->panfactor;
			}
			rvol = panning * svol * cvol;
			lvol = ((psy_dsp_amp_t) 1 - panning) * svol * cvol;
			
			//FT2 Style (Two slides) mode, but with max amp = 0.5.
			if (rvol > 0.5f) { rvol = (psy_dsp_amp_t) 0.5f; }
			if (lvol > 0.5f) { lvol = (psy_dsp_amp_t) 0.5f; }			

			if (self->effcmd == SAMPLER_CMD_PORTAUP ||
					self->effcmd == SAMPLER_CMD_PORTADOWN) {
				portaframe = self->portacurrframe;
				portaspeed = self->portaspeed;
			}			
			for (i = 0; i < numsamples; ++i) {
				unsigned int c;			
								
				for (c = 0; c < psy_audio_buffer_numchannels(
						&position->sample->channels); ++c) {
					psy_dsp_amp_t* src;
					psy_dsp_amp_t* dst;
					psy_dsp_amp_t val;				
					unsigned int frame;

					src = psy_audio_buffer_at(&position->sample->channels, c);
					if (c >= psy_audio_buffer_numchannels(output)) {
						break;
					}
					dst = psy_audio_buffer_at(output, c);
					frame = sampleiterator_frameposition(position);					
					val = psy_dsp_resampler_work_float(
						psy_dsp_multiresampler_base(&self->resampler),
						src,
						(float)double_real(&position->pos),
						position->sample->numframes,
						position->resampler_data,
						src,
						src + (position->sample->numframes - 1));
					if (c == 0) {
						if (psy_dsp_multifilter_type(&self->filter_l) != F_NONE) {
							psy_dsp_filter_setcutoff(
								psy_dsp_multifilter_base(&self->filter_l),
								self->filterenv.value);
							val = psy_dsp_filter_work(
								psy_dsp_multifilter_base(&self->filter_l),
								val);
						}
					} else
					if (c == 1) {
						if (psy_dsp_multifilter_type(&self->filter_r) != F_NONE) {
							psy_dsp_filter_setcutoff(
								psy_dsp_multifilter_base(&self->filter_r),
								self->filterenv.value);
							val = psy_dsp_filter_work(
								psy_dsp_multifilter_base(&self->filter_r),
								val);
						}
					}								
					dst[i] += val * env[i];					
				}				
				if (psy_dsp_multifilter_type(&self->filter_l) != F_NONE) {
					psy_dsp_adsr_tick(&self->filterenv);
				}
				if (self->effcmd == SAMPLER_CMD_PORTAUP ||
						self->effcmd == SAMPLER_CMD_PORTADOWN) {
					++portaframe;
					if (portaframe >= self->portanumframes) {
						portaspeed = 1.0;
						portadone = 1;
					} else {
						position->speed = 
							(int64_t) (position->speed * portaspeed);
						psy_dsp_resampler_setspeed(psy_dsp_multiresampler_base(
							&self->resampler),
							position->resampler_data,
							position->speed * 1 / 4294967296.0f);
					}
				}
				if (!sampleiterator_inc(position)) {			
					psy_audio_samplervoice_reset(self);					
					break;				
				}								
			}			
			if (psy_audio_buffer_mono(&position->sample->channels) &&
				psy_audio_buffer_numchannels(output) > 1) {
				dsp.add(
					psy_audio_buffer_at(output, 0),
					psy_audio_buffer_at(output, 1),
					numsamples,
					self->instrument->globalvolume);
			}
			if (psy_audio_buffer_numchannels(output) > 1) {
				dsp.mul(psy_audio_buffer_at(output, 0), numsamples,
					lvol * self->instrument->globalvolume);
				dsp.mul(psy_audio_buffer_at(output, 1), numsamples,
					rvol * self->instrument->globalvolume);
			}			
		}
		if (self->effcmd == SAMPLER_CMD_PORTAUP ||
				self->effcmd == SAMPLER_CMD_PORTADOWN) {
			self->portacurrframe += numsamples;
			if (portadone) {
				self->effcmd = SAMPLER_CMD_NONE;
				self->portaspeed = 1.0;
				self->portacurrframe = 0;
			}
		}
		free(env);
	}
	self->dooffset = 0;
}

void psy_audio_samplervoice_release(psy_audio_SamplerVoice* self)
{
	self->portaspeed = 1.0;
	self->effcmd = SAMPLER_CMD_NONE;
	psy_dsp_adsr_release(&self->env);	
	psy_dsp_adsr_release(&self->filterenv);
}

void psy_audio_samplervoice_fastrelease(psy_audio_SamplerVoice* self)
{
	self->portaspeed = 1.0;
	self->effcmd = SAMPLER_CMD_NONE;
	psy_dsp_adsr_fastrelease(&self->env);	
	psy_dsp_adsr_fastrelease(&self->filterenv);
}

psy_List* sequencerinsert(psy_audio_Sampler* self, psy_List* events)
{
	psy_List* p;
	psy_List* insert = 0;

	for (p = events; p != NULL; p = p->next) {
		psy_audio_PatternEntry* entry;
		psy_audio_PatternEvent* event;

		entry = p->entry;
		event = patternentry_front(entry);
		if (event->cmd == SAMPLER_CMD_EXTENDED) {
			if ((event->parameter & 0xf0) == SAMPLER_CMD_EXT_NOTEOFF) {
				psy_audio_PatternEntry* noteoff;

				// This means there is always 6 ticks per row whatever number of rows.
				//_triggerNoteOff = (Global::player().SamplesPerRow()/6.f)*(ite->_parameter & 0x0f);
				noteoff = patternentry_allocinit();
				patternentry_front(noteoff)->note = NOTECOMMANDS_RELEASE;
				patternentry_front(noteoff)->mach = patternentry_front(entry)->mach;
				noteoff->delta += /*entry->offset*/ + (event->parameter & 0x0f) / 6.f *
					psy_audio_machine_currbeatsperline(
						psy_audio_sampler_base(self));
				psy_list_append(&insert, noteoff);
			} else 
			if ((event->parameter & 0xF0) == SAMPLER_CMD_EXT_NOTEDELAY) {
				psy_audio_PatternEntry* newentry;
				psy_audio_PatternEvent* ev;
				int numticks;

				newentry = patternentry_clone(entry);
				ev = patternentry_front(newentry);
				numticks = event->parameter & 0x0f;
				ev->cmd = 0;
				ev->parameter = 0;
				newentry->delta += numticks * psy_audio_machine_beatspertick(
					psy_audio_sampler_base(self));				
				psy_list_append(&insert, newentry);
			}
		}
	}
	return insert;
}

// fileio
void loadspecific(psy_audio_Sampler* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	if (self->xmsamplerload) {
		int32_t temp;
		bool wrongState = FALSE;
		uint32_t filevers;
		size_t filepos;
		uint32_t size=0;

		psy_audio_sampler_defaultC4(self, TRUE);
		self->panpersistent = TRUE;
		psyfile_read(songfile->file, &size,sizeof(size));
		filepos = psyfile_getpos(songfile->file);
		psyfile_read(songfile->file, &filevers, sizeof(filevers));
			
		// Check higher bits of version (AAAABBBB). 
		// different A, incompatible, different B, compatible
 		if ( (filevers&0xFFFF0000) == XMSAMPLER_VERSION_ONE )
		{
			ZxxMacro zxxMap[128];
			int i;
			bool m_bAmigaSlides;// Using Linear or Amiga Slides.
			bool m_UseFilters;
			int32_t m_GlobalVolume;
			int32_t m_PanningMode;

			// numSubtracks
			psyfile_read(songfile->file, &temp, sizeof(temp));
			self->numvoices = temp;
			// quality
			psyfile_read(songfile->file, &temp, sizeof(temp));
			/*switch (temp)
			{
				case 2:	_resampler.quality(helpers::dsp::resampler::quality::spline); break;
				case 3:	_resampler.quality(helpers::dsp::resampler::quality::sinc); break;
				case 0:	_resampler.quality(helpers::dsp::resampler::quality::zero_order); break;
				case 1:
				default: _resampler.quality(helpers::dsp::resampler::quality::linear);
			}*/

			for (i = 0; i < 128; ++i) {
				psyfile_read(songfile->file, &zxxMap[i].mode, sizeof(zxxMap[i].mode));
				psyfile_read(songfile->file, &zxxMap[i].value, sizeof(zxxMap[i].value));
			}

			psyfile_read(songfile->file, &m_bAmigaSlides, sizeof(m_bAmigaSlides));
			psyfile_read(songfile->file, &m_UseFilters, sizeof(m_UseFilters));
			psyfile_read(songfile->file, &m_GlobalVolume, sizeof(m_GlobalVolume));
			psyfile_read(songfile->file, &m_PanningMode, sizeof(m_PanningMode));

			for (i = 0; i < MAX_TRACKS; ++i) {
				loadxmsamplerchannel(self, songfile);
				// m_Channel[i].Load(*riffFile);
			}
		} else {
			wrongState = TRUE;
		}
		if (!wrongState) {
			return;
		} else {
			psyfile_seek(songfile->file, filepos + size);
			return; // FALSE;
		}
	} else {
		uint32_t size;

		// Old version had default C4 as false
		psy_audio_sampler_defaultC4(self, FALSE);
		// LinearSlide(false);
		size = 0;
		psyfile_read(songfile->file, &size, sizeof(size));
		if (size)
		{
			/// Version 0
			int32_t temp;
			psyfile_read(songfile->file, &temp, sizeof(temp)); // numSubtracks
			self->numvoices = temp;
			psyfile_read(songfile->file, &temp, sizeof(temp)); // quality		
			/* switch (temp)
			{
			case 2:	_resampler.quality(helpers::dsp::resampler::quality::spline); break;
			case 3:	_resampler.quality(helpers::dsp::resampler::quality::sinc); break;
			case 0:	_resampler.quality(helpers::dsp::resampler::quality::zero_order); break;
			case 1:
			default: _resampler.quality(helpers::dsp::resampler::quality::linear);
			} */

			if (size > 3 * sizeof(unsigned int))
			{
				unsigned int internalversion;
				psyfile_read(songfile->file, &internalversion, sizeof(internalversion));
				if (internalversion >= 1) {
					uint8_t defaultC4;

					psyfile_read(songfile->file, &defaultC4, sizeof(defaultC4)); // correct A4 frequency.
					psy_audio_sampler_defaultC4(self, defaultC4 != 0);
				}
				if (internalversion >= 2) {
					unsigned char slidemode;
					psyfile_read(songfile->file, &slidemode, sizeof(slidemode)); // correct slide.
					// LinearSlide(slidemode);
				}
			}
		}
	}
}

bool loadxmsamplerchannel(psy_audio_Sampler* self, psy_audio_SongFile* songfile)
{
	char temp[8];
	int32_t size = 0;
	///< (0..200)   &0x100 = Mute. // value used for Storage and reset
	int32_t m_ChannelDefVolume;
	//  0..200 .  &0x100 == Surround. // value used for Storage and reset
	int32_t m_DefaultPanFactor;
	int32_t m_DefaultCutoff;
	int32_t m_DefaultRessonance;

	psyfile_read(songfile->file, &temp, 4);
	temp[4]='\0';
	psyfile_read(songfile->file, &size, sizeof(size));
	if (strcmp(temp,"CHAN")) return FALSE;
	///< (0..200)   &0x100 = Mute.
	psyfile_read(songfile->file, &m_ChannelDefVolume,
		sizeof(m_ChannelDefVolume));
	//<  0..200 .  &0x100 = Surround.
	psyfile_read(songfile->file, &m_DefaultPanFactor,
		sizeof(m_DefaultPanFactor));
	psyfile_read(songfile->file, &m_DefaultCutoff, sizeof(m_DefaultCutoff));
	psyfile_read(songfile->file, &m_DefaultRessonance,
		sizeof(m_DefaultRessonance));
	{ 
		uint32_t i = 0;

		psyfile_read(songfile->file, &i, sizeof(i));
		// m_DefaultFilterType = static_cast<dsp::FilterType>(i);
	}
	return TRUE;
}

void savespecific(psy_audio_Sampler* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t temp;
	uint32_t size = 3 * sizeof(temp) + 2 * sizeof(unsigned char);
	unsigned char defaultC4;
	unsigned char slidemode;

	psyfile_write(songfile->file, &size, sizeof(size));
	temp = self->numvoices;
	psyfile_write(songfile->file, &temp, sizeof(temp)); // numSubtracks
	/* switch (_resampler.quality())
	{
	case helpers::dsp::resampler::quality::zero_order: temp = 0; break;
	case helpers::dsp::resampler::quality::spline: temp = 2; break;
	case helpers::dsp::resampler::quality::sinc: temp = 3; break;
	case helpers::dsp::resampler::quality::linear: //fallthrough
	default: temp = 1;
	} */
	temp = 1;
	psyfile_write(songfile->file, &temp, sizeof(temp)); // quality
	temp = SAMPLERVERSION;
	psyfile_write(songfile->file, &temp, sizeof(temp));
	defaultC4 = 1; // isDefaultC4();
	psyfile_write(songfile->file, &defaultC4, sizeof(defaultC4)); // correct A4
	slidemode = 1;
	psyfile_write(songfile->file, &slidemode, sizeof(slidemode)); // correct slide
}

psy_audio_MachineParam* parameter(psy_audio_Sampler* self,
	uintptr_t param)
{
	switch (param) {
		case 0: return &self->param_numvoices.machineparam; break;
		case 1: return &self->param_resamplingmethod.machineparam; break;
		case 2: return &self->param_defaultspeed.machineparam;  break;
		case 3: return &self->param_maxvolume.machineparam; break;
		case 4: return &self->param_panpersistent.machineparam; break;
		default:;
	}
	return 0;
}
