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
static int unused_voice(Sampler*);
static void release_voices(Sampler*, int channel);
static unsigned int numinputs(Sampler*);
static unsigned int numoutputs(Sampler*);
static void loadspecific(Sampler*, struct SongFile*, unsigned int slot);
static void savespecific(Sampler*, struct SongFile*, unsigned int slot);
static const char* editname(Sampler* self);
static void seteditname(Sampler* self, const char* name);

static int currslot(Sampler*, unsigned int channel, const PatternEvent*);

static void voice_init(Voice*, Instrument*, Sample*, int channel, unsigned int samplerate);
static void voice_dispose(Voice*);
static void voice_seqtick(Voice*, const PatternEvent*);
static void voice_noteon(Voice*, const PatternEvent*);
static void voice_noteoff(Voice*, const PatternEvent*);
static void voice_work(Voice*, Buffer*, int numsamples);
static void voice_release(Voice*);
static void voice_fastrelease(Voice*);

static int songtracks = 16;
static const uint32_t SAMPLERVERSION = 0x00000002;

static MachineInfo const MacInfo = {
	MI_VERSION,
	0x0250,
	GENERATOR | 32 | 64,
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

void sampler_init(Sampler* self, MachineCallback callback)
{
	int voice;
	
	machine_init(&self->machine, callback);	
	self->machine.generateaudio = generateaudio;
	self->machine.seqtick = seqtick;
	self->machine.info = info;
	self->machine.dispose = dispose;
	self->machine.numinputs = numinputs;
	self->machine.numoutputs = numoutputs;
	self->machine.loadspecific = loadspecific;
	self->machine.savespecific = savespecific;
	self->machine.numparametercols = numparametercols;
	self->machine.numparameters = numparameters;
	self->machine.parametertweak = parametertweak;
	self->machine.describevalue = describevalue;	
	self->machine.parametervalue = parametervalue;		
	self->machine.parameterrange = parameterrange;
	self->machine.parametertype = parametertype;
	self->machine.parametername = parametername;
	self->machine.parameterlabel = parameterlabel;
	self->machine.editname = editname;
	self->machine.seteditname = seteditname;
	self->numvoices = SAMPLER_MAX_POLYPHONY;	
	for (voice = 0; voice < self->numvoices; ++voice) {
		voice_init(&self->voices[voice], 0, 0, 0, 44100);
	}
	self->resamplingmethod = 2;
	self->defaultspeed = 1;
	self->pan = 0.5f;
	self->editname = strdup("Sampler");
	table_init(&self->lastinst);
}

void dispose(Sampler* self)
{
	int voice;

	for (voice=0; voice < self->numvoices; ++voice) {
		voice_dispose(&self->voices[voice]);
	}
	machine_dispose(&self->machine);
	free(self->editname);
	self->editname = 0;
	table_dispose(&self->lastinst);
}

void generateaudio(Sampler* self, BufferContext* bc)
{	
	int voice;

	for (voice = 0; voice < self->numvoices; ++voice) {
		voice_work(&self->voices[voice], bc->output, bc->numsamples);
	}
//	if (sample-buffer_numchannels(bc->output)
}

void seqtick(Sampler* self, int channel, const PatternEvent* event)
{	
	Sample* sample;
	int slot;
		
	slot = currslot(self, channel, event);
	if (slot == NOTECOMMANDS_EMPTY) {
		return;
	}	
	sample = samples_at(self->machine.samples(self), slot);	
	if (sample) {
		Instrument* instrument;

		release_voices(self, channel);
		instrument = instruments_at(self->machine.instruments(self), slot);		
		if (instrument) {
			int voice;
			
			voice = unused_voice(self);
			if (voice != -1) {
				voice_init(&self->voices[voice], instrument, sample, channel,
					self->machine.callback.samplerate(self->machine.callback.context));
				voice_seqtick(&self->voices[voice], event);			
			}
		}
	}
}

int unused_voice(Sampler* self)
{
	int voice ;

	for (voice = 0; voice < self->numvoices; ++voice) {
		if (self->voices[voice].env.stage == ENV_OFF) {
			return voice;
		}
	}
	return -1;
}

int currslot(Sampler* self, unsigned int channel, const PatternEvent* event)
{
	int rv;

	if (event->inst != NOTECOMMANDS_EMPTY) {
		table_insert(&self->lastinst, channel, (void*)event->inst);
		rv = event->inst;
	} else
	if (table_exists(&self->lastinst, channel)) {
		rv = (int)(ptrdiff_t) table_at(&self->lastinst, channel);
	} else { 
		rv = NOTECOMMANDS_EMPTY;
	}
	return rv;
}

void release_voices(Sampler* self, int channel)
{
	int voice;

	for (voice = 0; voice < self->numvoices; ++voice) {
		if (self->voices[voice].channel == channel) {
			voice_release(&self->voices[voice]);
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
	}
	return 0;
}

int parametervalue(Sampler* self, int param)
{	
	switch (param) {
		case 0: return self->numvoices; break;
		case 1: return self->resamplingmethod; break;
		case 2: return self->defaultspeed; break;
		default:
		break;
	}
	return 0;
}

unsigned int numparameters(Sampler* self)
{
	return 3;
}

unsigned int numparametercols(Sampler* self)
{
	return 3;
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
		_snprintf(txt, 128, "%s", "Polyphony Voices");
		break;
	case 1:
		_snprintf(txt, 128, "%s", "Resampling method");
		break;
	case 2:
		_snprintf(txt, 128, "%s", "Default speed");
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
		_snprintf(txt, 128, "%s", "Polyphony");
		break;
	case 1:
		_snprintf(txt, 128, "%s", "Resampling");
		break;
	case 2:
		_snprintf(txt, 128, "%s", "Default speed");
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

void voice_init(Voice* self, Instrument* instrument, Sample* sample,
	int channel, unsigned int samplerate) 
{
	self->instrument = instrument;
	self->sample = sample;
	self->channel = channel;
	self->vol = 1.f;
	if (sample) {
		self->position = sample_begin(self->sample);
	}	
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
		((Filter*)&self->filter_l)->setcutoff(&self->filter_l, 
			self->instrument->filtercutoff);
		((Filter*)&self->filter_r)->setcutoff(&self->filter_r,
			self->instrument->filtercutoff);	
		((Filter*)&self->filter_l)->setressonance(&self->filter_l, 
			self->instrument->filterres);
		((Filter*)&self->filter_r)->setressonance(&self->filter_r,
			self->instrument->filterres);
		multifilter_settype(&self->filter_l, instrument->filtertype);
		multifilter_settype(&self->filter_r, instrument->filtertype);
	}
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

const char* editname(Sampler* self)
{
	return self->editname;
}

void seteditname(Sampler* self, const char* name)
{
	free(self->editname);
	self->editname = strdup(name);
}

void voice_reset(Voice* self)
{				
	adsr_reset(&self->env);
	adsr_reset(&self->filterenv);
	((Filter*)(&self->filter_r))->reset(&self->filter_l);
	((Filter*)(&self->filter_r))->reset(&self->filter_r);
}

void voice_dispose(Voice* self)
{
}

void voice_seqtick(Voice* self, const PatternEvent* event)
{
	if (event->cmd == SAMPLER_CMD_VOLUME) {
		 self->vol = event->parameter / (amp_t)255;
	} else
	if (event->note == NOTECOMMANDS_RELEASE) {
		voice_noteoff(self, event);
	} else
	if (event->note < NOTECOMMANDS_RELEASE) {
		voice_noteon(self, event);
	}
}

void voice_noteon(Voice* self, const PatternEvent* event)
{
	int baseC;	
	
	baseC = 60;
	self->position = sample_begin(self->sample);
	double_setvalue(&self->position.speed,
		pow(2.0f,
			(event->note + self->sample->tune - baseC + 
				((amp_t)self->sample->finetune * 0.01f)) / 12.0f) *
			((beat_t)self->sample->samplerate / 44100));
	adsr_start(&self->env);
	adsr_start(&self->filterenv);
}

void voice_noteoff(Voice* self, const PatternEvent* event)
{
	adsr_release(&self->env);
	adsr_release(&self->filterenv);
}

void voice_work(Voice* self, Buffer* output, int numsamples)
{	
	if (self->sample && self->env.stage != ENV_OFF) {		
		int i;
		amp_t vol;		
				
		vol = self->vol * self->sample->globalvolume * self->sample->defaultvolume;
		if (vol > 0.5) {
			vol = 0.5;
		}
		for (i = 0; i < numsamples; ++i) {
			unsigned int c;			
							
			for (c = 0; c < buffer_numchannels(&self->sample->channels); ++c) {
				amp_t* src;
				amp_t* dst;
				amp_t val;				
				unsigned int frame;

				src = buffer_at(&self->sample->channels, c);
				if (c >= buffer_numchannels(output)) {
					break;
				}
				dst = buffer_at(output, c);
				frame = sampleiterator_frameposition(&self->position);
				val = src[frame];
				if (c == 0) {
					if (multifilter_type(&self->filter_l) != F_NONE) {
						((Filter*)&self->filter_l)->setcutoff(&self->filter_l,
							self->filterenv.value);
						val = ((Filter*)&self->filter_l)->work(&self->filter_l,
							val);
					}
				} else
				if (c == 1) {
					if (multifilter_type(&self->filter_r) != F_NONE) {
						((Filter*)&self->filter_r)->setcutoff(&self->filter_r,
							self->filterenv.value);
						val = ((Filter*)&self->filter_r)->work(&self->filter_r,
							val);
					}
				}								
				dst[i] += val * vol * self->env.value;
			}				
			adsr_tick(&self->env);			
			if (multifilter_type(&self->filter_l) != F_NONE) {
				adsr_tick(&self->filterenv);
			}			
			if (!sampleiterator_inc(&self->position)) {			
				voice_reset(self);					
				break;				
			}
		}
		if (buffer_mono(&self->sample->channels) &&
			buffer_numchannels(output) > 1) {
			dsp_add(
				buffer_at(output, 0),
				buffer_at(output, 1),
				numsamples,
				1.f);
		}
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

