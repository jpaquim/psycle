// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sampler.h"
#include "pattern.h"
#include "plugin_interface.h"
#include "instruments.h"
#include "samples.h"
#include "songio.h"
#include <operations.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <portable.h>

static void generateaudio(Sampler*, BufferContext*);
static void seqtick(Sampler*, int channel, const PatternEvent*);
static const MachineInfo* info(Sampler*);
static unsigned int numparametercols(Sampler*);
static unsigned int numparameters(Sampler*);
static int parametertype(Sampler* self, int par);
static void parameterrange(Sampler*, int numparam, int* minval, int* maxval);
static int parameterlabel(Sampler*, char* txt, int param);
static int parametername(Sampler*, char* txt, int param);
static void parametertweak(Sampler*, int par, int val);
static int describevalue(Sampler*, char* txt, int param, int value);
static int parametervalue(Sampler*, int param);
static void dispose(Sampler*);
static int alloc_voice(Sampler*);
static void releasevoices(Sampler*, uintptr_t channel);
static void removeunusedvoices(Sampler* self);
static unsigned int numinputs(Sampler*);
static unsigned int numoutputs(Sampler*);
static void loadspecific(Sampler*, struct SongFile*, unsigned int slot);
static void savespecific(Sampler*, struct SongFile*, unsigned int slot);

static int currslot(Sampler*, unsigned int channel, const PatternEvent*);

static void voice_init(Voice*, Sampler*, Instrument*, uintptr_t channel,
	unsigned int samplerate, Samples* samples);
static void voice_dispose(Voice*);
static void voice_seqtick(Voice*, const PatternEvent*);
static void voice_noteon(Voice*, const PatternEvent*);
static void voice_noteoff(Voice*, const PatternEvent*);
static void voice_work(Voice*, Buffer*, int numsamples);
static void voice_release(Voice*);
static void voice_fastrelease(Voice*);
static void voice_clearpositions(Voice*);

static int songtracks = 16;
static const uint32_t SAMPLERVERSION = 0x00000002;

static MachineInfo const MacInfo = {
	MI_VERSION,
	0x0250,
	GENERATOR | 32 | 64,
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

const MachineInfo* sampler_info(void)
{
	return &MacInfo;
}

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(Sampler* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;		
		vtable.generateaudio = (fp_machine_generateaudio) generateaudio;
		vtable.seqtick = (fp_machine_seqtick) seqtick;
		vtable.info = (fp_machine_info) info;
		vtable.dispose = (fp_machine_dispose) dispose;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable.loadspecific = (fp_machine_loadspecific) loadspecific;
		vtable.savespecific = (fp_machine_savespecific) savespecific;
		vtable.numparametercols = (fp_machine_numparametercols) numparametercols;
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

void sampler_init(Sampler* self, MachineCallback callback)
{
	Machine* base = &self->custommachine.machine;
	
	custommachine_init(&self->custommachine, callback);	
	vtable_init(self);
	self->custommachine.machine.vtable = &vtable;
	self->numvoices = SAMPLER_DEFAULT_POLYPHONY;
	self->voices = 0;
	self->resamplingmethod = 2;
	self->defaultspeed = 1;	
	self->maxvolume = 0xFF;
	base->vtable->seteditname(base, "Sampler");
	table_init(&self->lastinst);
}

void dispose(Sampler* self)
{
	List* p;
	
	for (p = self->voices; p != 0; p = p->next) {
		Voice* voice;

		voice = (Voice*) p->entry;
		voice_dispose(voice);		
		free(voice);
	}
	list_free(self->voices);
	self->voices = 0;
	custommachine_dispose(&self->custommachine);
}

void generateaudio(Sampler* self, BufferContext* bc)
{	
	List* p;
	uintptr_t c = 0;

	removeunusedvoices(self);
	for (p = self->voices; p != 0 && c < self->numvoices; p = p->next, ++c) {
		Voice* voice;

		voice = (Voice*) p->entry;		
		voice_work(voice, bc->output, bc->numsamples);
	}
}

void seqtick(Sampler* self, int channel, const PatternEvent* event)
{	
	Machine* base = (Machine*)self;
	Instrument* instrument;
	int slot;
		
	slot = currslot(self, channel, event);
	if (slot == NOTECOMMANDS_EMPTY) {
		return;
	}	
	instrument = instruments_at(base->vtable->instruments(
		&self->custommachine.machine), slot);
	if (instrument) {
		releasevoices(self, channel);
		{
			Voice* voice;

			voice = malloc(sizeof(Voice));
			voice_init(voice, self, instrument, channel,
				base->vtable->samplerate(base),
				self->custommachine.machine.vtable->samples(
				&self->custommachine.machine));
			list_append(&self->voices, voice);
			voice_seqtick(voice, event);		
		}
	}
}

int currslot(Sampler* self, unsigned int channel, const PatternEvent* event)
{
	int rv;

	if (event->inst != NOTECOMMANDS_EMPTY) {
		table_insert(&self->lastinst, channel, (void*)event->inst);
		rv = event->inst;
	} else
	if (table_exists(&self->lastinst, channel)) {
		rv = (int)(uintptr_t) table_at(&self->lastinst, channel);
	} else { 
		rv = NOTECOMMANDS_EMPTY;
	}
	return rv;
}

void releasevoices(Sampler* self, uintptr_t channel)
{
	List* p;
	
	for (p = self->voices; p != 0; p = p->next) {
		Voice* voice;

		voice = (Voice*) p->entry;
		if (voice->channel == channel) {
			voice_release(voice);
		}
	}
}

void removeunusedvoices(Sampler* self)
{
	List* p;
	List* q;
		
	for (p = self->voices; p != 0; p = q) {
		Voice* voice;

		q = p->next;
		voice = (Voice*) p->entry;				
		if (voice->env.stage == ENV_OFF) {
			voice_dispose(voice);
			free(voice);
			list_remove(&self->voices, p);
		}			
	}
}

const MachineInfo* info(Sampler* self)
{	
	return &MacInfo;
}

void parametertweak(Sampler* self, int param, int value)
{	
	switch (param) {
		case 0: self->numvoices = value; break;
		case 1: self->resamplingmethod = value; break;
		case 2: self->defaultspeed = value; break;
		case 3: self->maxvolume = value; break;
		default:
		break;
	}
}

int describevalue(Sampler* self, char* txt, int param, int value)
{ 
	if (param == 1) {
		switch(value)
		{
			case 0:sprintf(txt,"Hold/Chip [Lowest quality]");return 1;break;
			case 1:sprintf(txt,"Linear [Low quality]");return 1;break;
			case 2:sprintf(txt,"Spline [Medium quality]");return 1;break;
			case 3:sprintf(txt,"32Tap Sinc [High Quality]");return 1;break;			
		}		
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

int parametervalue(Sampler* self, int param)
{	
	switch (param) {
		case 0: return self->numvoices; break;
		case 1: return self->resamplingmethod; break;
		case 2: return self->defaultspeed; break;
		case 3: return self->maxvolume; break;
		default:
		break;
	}
	return 0;
}

unsigned int numparameters(Sampler* self)
{
	return 4;
}

unsigned int numparametercols(Sampler* self)
{
	return 4;
}

int parametertype(Sampler* self, int par)
{
	return MPF_STATE;
}

void parameterrange(Sampler* self, int param, int* minval, int* maxval)
{
	switch (param) {
	case 0:
		*minval = 1;
		*maxval = 16;
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

int parameterlabel(Sampler* self, char* txt, int param)
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

int parametername(Sampler* self, char* txt, int param)
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

unsigned int numinputs(Sampler* self)
{
	return 0;
}

unsigned int numoutputs(Sampler* self)
{
	return 2;
}

static void loadspecific(Sampler* self, struct SongFile* songfile, unsigned int slot)
{
	//Old version had default C4 as false
	// DefaultC4(false);
	// LinearSlide(false);
	unsigned int size = 0;
	psyfile_read(songfile->file, &size, sizeof(size));
	if (size)
	{
		/// Version 0
		int temp;
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

void savespecific(Sampler* self, struct SongFile* songfile, unsigned int slot)
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

void voice_init(Voice* self, Sampler* sampler, Instrument* instrument,
	uintptr_t channel, unsigned int samplerate, Samples* samples) 
{	
	self->sampler = sampler;
	self->samples = samples;
	self->instrument = instrument;
	self->channel = channel;
	self->usedefaultvolume = 1;
	self->vol = 1.f;
	self->pan = 0.5f;
	self->positions = 0;
	if (instrument) {
		adsr_init(&self->env, &instrument->volumeenvelope, samplerate);
		adsr_init(&self->filterenv, &instrument->filterenvelope, samplerate);	
	} else {
		adsr_initdefault(&self->env, samplerate);
		adsr_initdefault(&self->filterenv, samplerate);
	}	
	multifilter_init(&self->filter_l);
	multifilter_init(&self->filter_r);
	if (instrument) {
		((Filter*)&self->filter_l)->vtable->setcutoff(&self->filter_l.filter, 
			self->instrument->filtercutoff);
		((Filter*)&self->filter_r)->vtable->setcutoff(&self->filter_r.filter,
			self->instrument->filtercutoff);	
		((Filter*)&self->filter_l)->vtable->setressonance(&self->filter_l.filter, 
			self->instrument->filterres);
		((Filter*)&self->filter_r)->vtable->setressonance(&self->filter_r.filter,
			self->instrument->filterres);
		multifilter_settype(&self->filter_l, instrument->filtertype);
		multifilter_settype(&self->filter_r, instrument->filtertype);
	}
}

void voice_reset(Voice* self)
{
	adsr_reset(&self->env);
	adsr_reset(&self->filterenv);
	((Filter*)(&self->filter_r))->vtable->reset(&self->filter_l.filter);
	((Filter*)(&self->filter_r))->vtable->reset(&self->filter_r.filter);	
}

void voice_dispose(Voice* self)
{
	voice_clearpositions(self);
	self->positions = 0;	
}

void voice_seqtick(Voice* self, const PatternEvent* event)
{	
	if (event->cmd == SAMPLER_CMD_VOLUME) {
		 self->usedefaultvolume = 0;
		 self->vol = event->parameter / 
			 (psy_dsp_amp_t) self->sampler->maxvolume;
	} else
	if (event->cmd == SAMPLER_CMD_PANNING) {
		self->pan = event->parameter / (psy_dsp_amp_t) 255;
	}
	if (event->note == NOTECOMMANDS_RELEASE) {
		voice_noteoff(self, event);
	} else
	if (event->note < NOTECOMMANDS_RELEASE) {
		voice_noteon(self, event);
	}
}

void voice_noteon(Voice* self, const PatternEvent* event)
{	
	Sample* sample;		
	int baseC = 48;
	List* entries;
	List* p;
						
	voice_clearpositions(self);
	entries = instrument_entriesintersect(self->instrument,
		event->note, 127);
	for (p = entries; p != 0; p = p->next) {
		InstrumentEntry* entry;
		
		entry = (InstrumentEntry*) p->entry;
		sample = samples_at(self->samples, entry->sampleindex);
		if (sample) {
			SampleIterator* iterator;

			iterator = malloc(sizeof(SampleIterator));
			*iterator = sample_begin(sample);
			list_append(&self->positions, iterator);

			double_setvalue(&iterator->speed,
				pow(2.0f,
					(event->note + sample->tune - baseC + 
						((psy_dsp_amp_t)sample->finetune * 0.01f)) / 12.0f) *
					((psy_dsp_beat_t)sample->samplerate / 44100));
		}
	}		
	list_free(entries);	
	if (self->positions) {
		adsr_start(&self->env);
		adsr_start(&self->filterenv);
	}
}

void voice_clearpositions(Voice* self)
{
	List* p;

	for (p = self->positions; p != 0; p = p->next) {
		free(p->entry);
	}
	list_free(self->positions);
	self->positions = 0;
}


void voice_noteoff(Voice* self, const PatternEvent* event)
{
	adsr_release(&self->env);
	adsr_release(&self->filterenv);
}

void voice_work(Voice* self, Buffer* output, int numsamples)
{		
	if (self->positions && self->env.stage != ENV_OFF) {
		List* p;
		psy_dsp_amp_t* env;
		int i;

		env = malloc(numsamples * sizeof(psy_dsp_amp_t));
		for (i = 0; i < numsamples; ++i) {
			adsr_tick(&self->env);
			env[i] = self->env.value;
		}
		for (p = self->positions; p != 0; p = p->next) {
			SampleIterator* position;
			int i;			
			psy_dsp_amp_t svol;
			psy_dsp_amp_t rvol;
			psy_dsp_amp_t lvol;

			position = (SampleIterator*) p->entry;
			svol = position->sample->globalvolume *
						(self->usedefaultvolume
							? position->sample->defaultvolume
							: self->vol);			
			rvol = position->sample->panfactor * svol;
			lvol = (1.f - position->sample->panfactor) * svol;
			svol *= 0.5f;
			//FT2 Style (Two slides) mode, but with max amp = 0.5.
			if (rvol > 0.5f) { rvol = 0.5f; }
			if (lvol > 0.5f) { lvol = 0.5f; }			
			if (svol > 0.5f) { svol = 0.5f; }
			
			for (i = 0; i < numsamples; ++i) {
				unsigned int c;			
								
				for (c = 0; c < buffer_numchannels(&position->sample->channels);
						++c) {
					psy_dsp_amp_t* src;
					psy_dsp_amp_t* dst;
					psy_dsp_amp_t val;				
					unsigned int frame;

					src = buffer_at(&position->sample->channels, c);
					if (c >= buffer_numchannels(output)) {
						break;
					}
					dst = buffer_at(output, c);
					frame = sampleiterator_frameposition(position);
					val = src[frame];
					if (c == 0) {
						if (multifilter_type(&self->filter_l) != F_NONE) {
							((Filter*)&self->filter_l)->vtable->setcutoff(
								&self->filter_l.filter,
								self->filterenv.value);
							val = ((Filter*)&self->filter_l)->vtable->work(
								&self->filter_l.filter,
								val);
						}
					} else
					if (c == 1) {
						if (multifilter_type(&self->filter_r) != F_NONE) {
							((Filter*)&self->filter_r)->vtable->setcutoff(
								&self->filter_r.filter,
								self->filterenv.value);
							val = ((Filter*)&self->filter_r)->vtable->work(
								&self->filter_r.filter,
								val);
						}
					}								
					dst[i] += val * env[i];					
				}				
				if (multifilter_type(&self->filter_l) != F_NONE) {
					adsr_tick(&self->filterenv);
				}			
				if (!sampleiterator_inc(position)) {			
					voice_reset(self);					
					break;				
				}
			}
			if (buffer_mono(&position->sample->channels) &&
				buffer_numchannels(output) > 1) {
				dsp.add(
					buffer_at(output, 0),
					buffer_at(output, 1),
					numsamples,
					1.f);
			}
			if (buffer_numchannels(output) > 1) {
				dsp.mul(buffer_at(output, 0), numsamples, lvol);
				dsp.mul(buffer_at(output, 1), numsamples, rvol);
			}
		}
		free(env);
	}
}

void voice_release(Voice* self)
{
	adsr_release(&self->env);	
	adsr_release(&self->filterenv);
}

void voice_fastrelease(Voice* self)
{
	adsr_release(&self->env);	
	adsr_release(&self->filterenv);
}
