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

static void generateaudio(psy_audio_Sampler*, psy_audio_BufferContext*);
static void seqtick(psy_audio_Sampler*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void sequencerlinetick(psy_audio_Sampler*);
static psy_List* sequencerinsert(psy_audio_Sampler*, psy_List* events);
static void stop(psy_audio_Sampler*);
static const psy_audio_MachineInfo* info(psy_audio_Sampler*);
static unsigned int numparametercols(psy_audio_Sampler*);
static uintptr_t numparameters(psy_audio_Sampler*);
static int parametertype(psy_audio_Sampler* self, uintptr_t param);
static void parameterrange(psy_audio_Sampler*, uintptr_t param, int* minval,
	int* maxval);
static int parameterlabel(psy_audio_Sampler*, char* txt, uintptr_t param);
static int parametername(psy_audio_Sampler*, char* txt, uintptr_t param);
static void parametertweak(psy_audio_Sampler*, uintptr_t param, float val);
static int describevalue(psy_audio_Sampler*, char* txt, uintptr_t param,
	int value);
static float parametervalue(psy_audio_Sampler*, uintptr_t param);
static void dispose(psy_audio_Sampler*);
static int alloc_voice(psy_audio_Sampler*);
static void releaseallvoices(psy_audio_Sampler*);
static Voice* activevoice(psy_audio_Sampler*, uintptr_t channel);
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
static int currslot(psy_audio_Sampler*, uintptr_t channel,
	const psy_audio_PatternEvent*);

static void voice_init(Voice*, psy_audio_Sampler*, psy_audio_Instrument*,
	uintptr_t channel, unsigned int samplerate);
static void voice_dispose(Voice*);
Voice* voice_alloc(void);
Voice* voice_allocinit(psy_audio_Sampler*, psy_audio_Instrument*,
	uintptr_t channel, unsigned int samplerate);
static void voice_seqtick(Voice*, const psy_audio_PatternEvent*);
static void voice_nna(Voice*);
static void voice_noteon(Voice*, const psy_audio_PatternEvent*);
static void voice_noteoff(Voice*);
static void voice_fastnoteoff(Voice*);
static void voice_work(Voice*, psy_audio_Buffer*, uintptr_t numsamples);
static void voice_release(Voice*);
static void voice_fastrelease(Voice*);
static void voice_clearpositions(Voice*);

static const uint32_t SAMPLERVERSION = 0x00000002;

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

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_Sampler* self)
{
	if (!vtable_initialized) {
		vtable = *(psy_audio_sampler_base(self)->vtable);
		vtable.generateaudio = (fp_machine_generateaudio) generateaudio;
		vtable.seqtick = (fp_machine_seqtick) seqtick;
		vtable.sequencerlinetick = (fp_machine_sequencerlinetick) sequencerlinetick;
		vtable.sequencerinsert = (fp_machine_sequencerinsert) sequencerinsert;
		vtable.stop = (fp_machine_stop) stop;
		vtable.info = (fp_machine_info) info;
		vtable.dispose = (fp_machine_dispose) dispose;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable.loadspecific = (fp_machine_loadspecific) loadspecific;
		vtable.savespecific = (fp_machine_savespecific) savespecific;
		vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		vtable.numparameters = (fp_machine_numparameters) numparameters;
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;
		vtable.describevalue = (fp_machine_describevalue) describevalue;	
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;		
		vtable.parameterrange = (fp_machine_parameterrange) parameterrange;
		vtable.parametertype = (fp_machine_parametertype) parametertype;
		vtable.parametername = (fp_machine_parametername) parametername;
		vtable.parameterlabel = (fp_machine_parameterlabel) parameterlabel;			
		vtable_initialized = 1;
	}
}

void psy_audio_sampler_init(psy_audio_Sampler* self, MachineCallback callback)
{	
	custommachine_init(&self->custommachine, callback);	
	vtable_init(self);
	psy_audio_sampler_base(self)->vtable = &vtable;
	psy_audio_machine_seteditname(psy_audio_sampler_base(self), "Sampler");
	self->numvoices = SAMPLER_DEFAULT_POLYPHONY;	
	self->voices = 0;	
	self->resamplingmethod = RESAMPLERTYPE_LINEAR;
	self->defaultspeed = 1;	
	self->maxvolume = 0xFF;
	self->xmsamplerload = 0;
	psy_table_init(&self->lastinst);
}

void dispose(psy_audio_Sampler* self)
{
	psy_List* p;
	
	for (p = self->voices; p != 0; p = p->next) {
		Voice* voice;

		voice = (Voice*) p->entry;
		voice_dispose(voice);		
		free(voice);
	}
	psy_list_free(self->voices);
	self->voices = 0;
	custommachine_dispose(&self->custommachine);
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

void generateaudio(psy_audio_Sampler* self, psy_audio_BufferContext* bc)
{	
	psy_List* p;
	uintptr_t c = 0;
	
	removeunusedvoices(self);
	for (p = self->voices; p != 0 && c < self->numvoices; p = p->next, ++c) {
		Voice* voice;

		voice = (Voice*) p->entry;		
		voice_work(voice, bc->output, bc->numsamples);
	}	
}

// seqtick
void seqtick(psy_audio_Sampler* self, uintptr_t channel,
	const psy_audio_PatternEvent* event)
{		
	Voice* voice = 0;	
		
	if (event->note == NOTECOMMANDS_RELEASE) {
		releasevoices(self, channel);
		return;
	}
	if (event->note < NOTECOMMANDS_RELEASE) {
		nnavoices(self, channel);
	} else {
		voice = activevoice(self, channel);		
	}
	if (!voice) {		
		psy_audio_Instrument* instrument;
		
		instrument = instruments_at(psy_audio_machine_instruments(
			psy_audio_sampler_base(self)),
			currslot(self, channel, event));
		if (instrument) {
			voice = voice_allocinit(self, instrument, channel,
				psy_audio_machine_samplerate(psy_audio_sampler_base(self)));
			psy_list_append(&self->voices, voice);
		}
	}
	if (voice) {
		voice_seqtick(voice, event);
	}
}

void sequencerlinetick(psy_audio_Sampler* self)
{
	psy_List* p;
	
	for (p = self->voices; p != 0; p = p->next) {
		Voice* voice;

		voice = (Voice*) p->entry;		
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
		psy_table_insert(&self->lastinst, channel, (void*)event->inst);
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
	
	for (p = self->voices; p != 0; p = p->next) {
		Voice* voice;

		voice = (Voice*) p->entry;		
		voice_release(voice);		
	}
}

void releasevoices(psy_audio_Sampler* self, uintptr_t channel)
{
	psy_List* p;
	
	for (p = self->voices; p != 0; p = p->next) {
		Voice* voice;

		voice = (Voice*) p->entry;
		if (voice->channel == channel) {			
			voice_release(voice);
		}
	}
}

void nnavoices(psy_audio_Sampler* self, uintptr_t channel)
{
	psy_List* p;
	
	for (p = self->voices; p != 0; p = p->next) {
		Voice* voice;

		voice = (Voice*) p->entry;
		if (voice->channel == channel) {			
			voice_nna(voice);
		}
	}
}

Voice* activevoice(psy_audio_Sampler* self, uintptr_t channel)
{
	Voice* rv = 0;	
	psy_List* p = 0;
	
	for (p = self->voices; p != 0; p = p->next) {
		Voice* voice;

		voice = (Voice*) p->entry;
		if (voice->channel == channel && voice->env.stage != ENV_RELEASE
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
		
	for (p = self->voices; p != 0; p = q) {
		Voice* voice;

		q = p->next;
		voice = (Voice*) p->entry;				
		if (voice->env.stage == ENV_OFF) {
			voice_dispose(voice);
			free(voice);
			psy_list_remove(&self->voices, p);
		}			
	}
}

const psy_audio_MachineInfo* info(psy_audio_Sampler* self)
{	
	return &macinfo;
}

void parametertweak(psy_audio_Sampler* self, uintptr_t param, float value)
{	
	switch (param) {
		case 0: self->numvoices = machine_parametervalue_scaled(psy_audio_sampler_base(self), param, value); break;
		case 1: self->resamplingmethod = (ResamplerType) machine_parametervalue_scaled(psy_audio_sampler_base(self), param, value); break;
		case 2: self->defaultspeed = machine_parametervalue_scaled(psy_audio_sampler_base(self), param, value); break;
		case 3: self->maxvolume = machine_parametervalue_scaled(psy_audio_sampler_base(self), param, value); break;
		default:
		break;
	}
}

int describevalue(psy_audio_Sampler* self, char* txt, uintptr_t param, int value)
{ 
	if (param == 1) {
		sprintf(txt, psy_dsp_multiresampler_name((ResamplerType) value));
		return 1;		
	} else
	if (param == 2) {
		switch(value)
		{
			case 0:sprintf(txt,"played by C3");return 1;break;
			case 1:sprintf(txt,"played by C4");return 1;break;		
		}
	} else
	if (param == 3) {
		sprintf(txt,"%0X", value);
		return 1;
	}
	return 0;
}

float parametervalue(psy_audio_Sampler* self, uintptr_t param)
{	
	switch (param) {
		case 0:
			return machine_parametervalue_normed(psy_audio_sampler_base(self), param,
				self->numvoices);
		break;
		case 1:
			return machine_parametervalue_normed(psy_audio_sampler_base(self), param,
				self->resamplingmethod);
		break;
		case 2:
			return machine_parametervalue_normed(psy_audio_sampler_base(self), param,
				self->defaultspeed);
		case 3:
			return machine_parametervalue_normed(psy_audio_sampler_base(self), param,
				self->maxvolume);
		default:
		break;
	}
	return 0;
}

uintptr_t numparameters(psy_audio_Sampler* self)
{
	return 4;
}

unsigned int numparametercols(psy_audio_Sampler* self)
{
	return 4;
}

int parametertype(psy_audio_Sampler* self, uintptr_t param)
{
	return MPF_STATE;
}

void parameterrange(psy_audio_Sampler* self, uintptr_t param, int* minval, int* maxval)
{
	switch (param) {
	case 0:
		*minval = 1;
		*maxval = 64;
		break;
	case 1:
		*minval = 0;
		*maxval = 3;
		break;
	case 2:
		*minval = 0;
		*maxval = 1;
		break;
	case 3:
		*minval = 0;
		*maxval = 255;
		break;
	default:
		*minval = 0;
		*maxval = 0;
		break;
	}
}

int parameterlabel(psy_audio_Sampler* self, char* txt, uintptr_t param)
{
	int rv = 1;
	switch (param) {
	case 0:
		psy_snprintf(txt, 128, "%s", "Polyphony Voices");
		break;
	case 1:
		psy_snprintf(txt, 128, "%s", "Resampling method");
		break;
	case 2:
		psy_snprintf(txt, 128, "%s", "Default speed");
		break;
	case 3:
		psy_snprintf(txt, 128, "%s", "Max volume");
		break;
	default:
		txt[0] = '\0';
		rv = 0;
		break;
	}
	return rv;
}

int parametername(psy_audio_Sampler* self, char* txt, uintptr_t param)
{
	int rv = 1;
	switch (param) {
	case 0:
		psy_snprintf(txt, 128, "%s", "Polyphony");
		break;
	case 1:
		psy_snprintf(txt, 128, "%s", "Resampling");
		break;
	case 2:
		psy_snprintf(txt, 128, "%s", "Default speed");
		break;
	case 3:
		psy_snprintf(txt, 128, "%s", "Max volume");
		break;
	default:
		txt[0] = '\0';
		rv = 0;
		break;
	}
	return rv;
}

uintptr_t numinputs(psy_audio_Sampler* self)
{
	return 0;
}

uintptr_t numoutputs(psy_audio_Sampler* self)
{
	return 2;
}

// Voice
void voice_init(Voice* self, psy_audio_Sampler* sampler,
	psy_audio_Instrument* instrument, uintptr_t channel,
	unsigned int samplerate) 
{	
	self->sampler = sampler;
	self->samples = psy_audio_machine_samples(psy_audio_sampler_base(sampler));
	self->instrument = instrument;
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
		sampler->resamplingmethod);
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

void voice_reset(Voice* self)
{
	self->effcmd = SAMPLER_CMD_NONE;
	self->portaspeed = 1.0;	
	psy_dsp_adsr_reset(&self->env);
	psy_dsp_adsr_reset(&self->filterenv);
	psy_dsp_filter_reset(psy_dsp_multifilter_base(&self->filter_l));
	psy_dsp_filter_reset(psy_dsp_multifilter_base(&self->filter_r));
}

void voice_dispose(Voice* self)
{
	voice_clearpositions(self);
	self->positions = 0;	
}

Voice* voice_alloc(void)
{
	return (Voice*) malloc(sizeof(Voice));
}

Voice* voice_allocinit(psy_audio_Sampler* sampler,
	psy_audio_Instrument* instrument, uintptr_t channel,
	unsigned int samplerate)
{
	Voice* rv;

	rv = voice_alloc();
	if (rv) {
		voice_init(rv, sampler, instrument, channel, samplerate);
	}
	return rv;
}

void voice_seqtick(Voice* self, const psy_audio_PatternEvent* event)
{
	self->dopan = 0;
	if (event->cmd == SAMPLER_CMD_VOLUME) {
		 self->usedefaultvolume = 0;
		 self->vol = event->parameter / 
			 (psy_dsp_amp_t) self->sampler->maxvolume;
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
		double samplesprobeat;

		self->effval = event->parameter;
		self->effcmd = event->cmd;				
		samplesprobeat = 1 / psy_audio_machine_beatspersample(
			psy_audio_sampler_base(self->sampler));
		self->portanumframes = (uintptr_t) samplesprobeat;
		self->portacurrframe = 0;
		self->portaspeed = pow(2.0f, event->parameter / 12.0 *
			1.0 / samplesprobeat);
	} else
	if (event->cmd == SAMPLER_CMD_PORTADOWN) {
		double samplesprobeat;

		self->effval = event->parameter;
		self->effcmd = event->cmd;				
		samplesprobeat = 1 / psy_audio_machine_beatspersample(
			psy_audio_sampler_base(self->sampler));
		self->portanumframes = (uintptr_t) samplesprobeat;
		self->portacurrframe = 0;
		self->portaspeed = pow(2.0f, -event->parameter / 12.0 * 1.0 / samplesprobeat);
	}	
	if (event->note < NOTECOMMANDS_RELEASE) {		
		voice_noteon(self, event);
	}
}

void voice_noteon(Voice* self, const psy_audio_PatternEvent* event)
{	
	psy_audio_Sample* sample;		
	int baseC = 48;
	psy_List* entries;
	psy_List* p;
						
	voice_clearpositions(self);
	entries = instrument_entriesintersect(self->instrument,
		event->note, 127);
	for (p = entries; p != 0; p = p->next) {
		psy_audio_InstrumentEntry* entry;
		
		entry = (psy_audio_InstrumentEntry*) p->entry;
		sample = psy_audio_samples_at(self->samples, entry->sampleindex);
		if (sample) {
			SampleIterator* iterator;			
			
			iterator = sampleiterator_alloc();
			*iterator = sample_begin(sample);			
			psy_list_append(&self->positions, iterator);
			iterator->speed = (int64_t)(4294967296.0f *
				pow(2.0f,
					(event->note + sample->tune - baseC + 
						((psy_dsp_amp_t)sample->finetune * 0.01f)) / 12.0f) *
					((psy_dsp_beat_t)sample->samplerate / 44100));
			self->resampler.resampler.vtable->setspeed(&
				self->resampler.resampler, iterator->speed);
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

void voice_clearpositions(Voice* self)
{
	psy_List* p;

	for (p = self->positions; p != 0; p = p->next) {
		free(p->entry);
	}
	psy_list_free(self->positions);
	self->positions = 0;
}

void voice_nna(Voice* self)
{
	if (self->instrument) {
		switch (instrument_nna(self->instrument)) {
			case psy_audio_NNA_STOP:
				voice_fastnoteoff(self);
			break;
			case psy_audio_NNA_NOTEOFF:
				voice_noteoff(self);
			break;
			case psy_audio_NNA_CONTINUE:				
			break;
			default:
				// note cut
				voice_fastnoteoff(self);
			break;
		}
	}
}

void voice_noteoff(Voice* self)
{
	psy_dsp_adsr_release(&self->env);
	psy_dsp_adsr_release(&self->filterenv);	
}

void voice_fastnoteoff(Voice* self)
{
	psy_dsp_adsr_fastrelease(&self->env);
	psy_dsp_adsr_fastrelease(&self->filterenv);
}

void voice_work(Voice* self, psy_audio_Buffer* output, uintptr_t numsamples)
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
		
		for (p = self->positions; p != 0; p = p->next) {
			SampleIterator* position;
			uintptr_t i;
			psy_dsp_amp_t panning;
			psy_dsp_amp_t svol;
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
			panning = self->dopan ? self->pan : position->sample->panfactor;
			rvol = panning * svol;
			lvol = ((psy_dsp_amp_t) 1 - panning) * svol;
			svol *= ((psy_dsp_amp_t) 0.5f);
			//FT2 Style (Two slides) mode, but with max amp = 0.5.
			if (rvol > 0.5f) { rvol = (psy_dsp_amp_t) 0.5f; }
			if (lvol > 0.5f) { lvol = (psy_dsp_amp_t) 0.5f; }			
			if (svol > 0.5f) { svol = (psy_dsp_amp_t) 0.5f; }

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
					val = self->resampler.resampler.vtable->work(
						&self->resampler.resampler,
						&src[frame], position->pos.HighPart,
						position->pos.LowPart, position->sample->numframes,
						0);
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
						self->resampler.resampler.vtable->setspeed(
							&self->resampler.resampler,
							position->speed);
					}
				}
				if (!sampleiterator_inc(position)) {			
					voice_reset(self);					
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

void voice_release(Voice* self)
{
	self->portaspeed = 1.0;
	self->effcmd = SAMPLER_CMD_NONE;
	psy_dsp_adsr_release(&self->env);	
	psy_dsp_adsr_release(&self->filterenv);
}

void voice_fastrelease(Voice* self)
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

	for (p = events; p != 0; p = p->next) {
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
				noteoff->delta += entry->offset + (event->parameter & 0x0f) / 6.f *
					psy_audio_machine_currbeatsperline(
						psy_audio_sampler_base(self));
				psy_list_append(&insert, noteoff);
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
		// Old version had default C4 as false
		// DefaultC4(false);
		// LinearSlide(false);
		uint32_t size = 0;

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
					unsigned char defaultC4;
					psyfile_read(songfile->file, &defaultC4, sizeof(defaultC4)); // correct A4 frequency.
					// DefaultC4(defaultC4);
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
