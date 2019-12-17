// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sample.h"
#include "waveio.h"
#include <string.h>
#include <stdlib.h>
#include <operations.h>
#include <alignedalloc.h>

void double_setvalue(Double* self, double value)
{
	self->QuadPart = (uint64_t)(value * 4294967296.0f);
}

void sampleiterator_init(SampleIterator* self, Sample* sample)
{
	self->sample = sample;
	self->forward = 1;
	double_setvalue(&self->pos, 0.0);
	double_setvalue(&self->speed, 1.0);
}

int sampleiterator_inc(SampleIterator* self)
{			
	if (self->sample->looptype == LOOP_DO_NOT) {
		self->pos.QuadPart += self->speed.QuadPart;
		if (self->pos.HighPart >= self->sample->numframes) {
			self->pos.LowPart = 0;
			self->pos.HighPart = 0;			
			return 0;			
		}
	} else
	if (self->sample->looptype == LOOP_NORMAL) {
		self->pos.QuadPart += self->speed.QuadPart;
		if (self->pos.HighPart >= self->sample->loopend) {
			self->pos.HighPart = self->sample->loopstart +
				self->sample->loopend - self->pos.HighPart;
		}
	} else
	if (self->sample->looptype == LOOP_BIDI) {
		if (self->forward) {
			self->pos.QuadPart += self->speed.QuadPart;
			if (self->pos.HighPart >= self->sample->loopend) {
				Double loopend;
				Double delta;
				
				loopend.LowPart = 0;
				loopend.HighPart = self->sample->loopend;
				delta.QuadPart = self->pos.QuadPart - loopend.QuadPart;
				self->pos.QuadPart = loopend.QuadPart - delta.QuadPart;
				self->forward = 0;
			}
		} else {
			self->pos.QuadPart -= self->speed.QuadPart;
			if (self->pos.HighPart <= self->sample->loopstart) {
				Double loopstart;
				Double delta;
				
				loopstart.LowPart = 0;
				loopstart.HighPart = self->sample->loopstart;
				delta.QuadPart = loopstart.QuadPart - self->pos.QuadPart;
				self->pos.QuadPart = loopstart.QuadPart + delta.QuadPart;
				self->forward = 1;
			}
		}
	}	
	return 1;
}

unsigned int sampleiterator_frameposition(SampleIterator* self)
{
	return self->pos.HighPart;
}

void vibrato_init(Vibrato* self)
{
	self->attack = 0;
	self->depth = 0;
	self->speed = 0;
	self->type = WAVEFORMS_SINUS;
}

void sample_init(Sample* self)
{
	buffer_init(&self->channels, 2);
	self->numframes = 0;
	self->samplerate = 44100;
	self->defaultvolume = 1.f;
	self->globalvolume = 1.f;
	self->loopstart = 0;
	self->loopend = 0;
	self->looptype  = LOOP_DO_NOT;
	self->sustainloopstart = 0;
	self->sustainloopend = 0;
	self->sustainlooptype = LOOP_DO_NOT;
	self->tune = 0;
	self->finetune = 0;
	self->panfactor = 0.5f;
	self->panenabled = 0;
	self->surround = 0;
	self->name = _strdup("");
	vibrato_init(&self->vibrato);
}

void sample_dispose(Sample* self)
{
	unsigned int channel;
	for (channel = 0; channel < self->channels.numchannels; ++channel) {
		free(self->channels.samples[channel]);
		self->channels.samples[channel] = 0;
	}
	buffer_dispose(&self->channels);
	self->numframes = 0;
	free(self->name);
}

Sample* sample_alloc(void)
{
	return (Sample*) malloc(sizeof(Sample));
}

Sample* sample_allocinit(void)
{
	Sample* rv;

	rv = sample_alloc();
	if (rv) {
		sample_init(rv);
	}
	return rv;
}

Sample* sample_clone(Sample* src)
{
	Sample* rv = 0;
	
	rv = sample_alloc();
	if (rv) {
		uintptr_t c;

		rv->samplerate = src->samplerate;
		rv->defaultvolume = src->defaultvolume;
		rv->globalvolume = src->globalvolume;
		rv->loopstart = src->loopstart;
		rv->loopend = src->loopend;
		rv->looptype = src->looptype;
		rv->sustainloopstart = src->sustainloopstart;
		rv->sustainloopend = src->sustainloopend;
		rv->sustainlooptype = src->sustainlooptype;
		rv->tune = src->tune;
		rv->finetune = src->finetune;
		rv->panfactor = src->panfactor;
		rv->panenabled = src->panenabled;
		rv->surround = src->surround;
		rv->name = strdup(src->name);
		rv->vibrato.attack = src->vibrato.attack;
		rv->vibrato.depth = src->vibrato.depth;
		rv->vibrato.speed = src->vibrato.speed;
		rv->vibrato.type = src->vibrato.type;
		rv->numframes = src->numframes;
		buffer_init(&rv->channels, src->channels.numchannels);
		for (c = 0; c < rv->channels.numchannels; ++c) {
			rv->channels.samples[c] = dsp.memory_alloc(src->numframes,
				sizeof(float));
		}
		buffer_clearsamples(&rv->channels, src->numframes);
		buffer_addsamples(&rv->channels, &src->channels, src->numframes, 1.0f);
	}
	return rv;
}

void sample_load(Sample* self, const char* path)
{
	char* delim;
	wave_load(self, path);
	delim = strrchr(path, '\\');	
	sample_setname(self, delim ? delim + 1 : path);	
}

void sample_save(Sample* self, const char* path)
{
	wave_save(self, path);
}

void sample_setname(Sample* self, const char* name)
{
	free(self->name);
	self->name = _strdup(name);
}

const char* sample_name(Sample* self)
{
	return self->name;
}

SampleIterator sample_begin(Sample* self)
{
	SampleIterator rv;

	sampleiterator_init(&rv, self);
	return rv;
}
