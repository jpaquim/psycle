// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sample.h"
#include "waveio.h"
#include <string.h>
#include <stdlib.h>
#include <operations.h>
#include <alignedalloc.h>

void psy_audio_vibrato_init(psy_audio_Vibrato* self)
{
	self->attack = 0;
	self->depth = 0;
	self->speed = 0;
	self->type = psy_audio_WAVEFORMS_SINUS;
}

void psy_audio_sample_init(psy_audio_Sample* self, uintptr_t numchannels)
{
	psy_audio_buffer_init(&self->channels, numchannels);
	self->stereo = 1;
	self->numframes = 0;
	self->samplerate = 44100;
	self->defaultvolume = 1.f;
	self->globalvolume = 1.f;
	self->loop.start = 0;
	self->loop.end = 0;
	self->loop.type  = psy_audio_SAMPLE_LOOP_DO_NOT;
	self->sustainloop.start = 0;
	self->sustainloop.end = 0;
	self->sustainloop.type = psy_audio_SAMPLE_LOOP_DO_NOT;
	self->tune = 0;
	self->finetune = 0;
	self->panfactor = 0.5f;
	self->panenabled = 0;
	self->surround = 0;
	self->name = strdup("");	
	psy_audio_vibrato_init(&self->vibrato);
}

void psy_audio_sample_dispose(psy_audio_Sample* self)
{
	uintptr_t channel;

	for (channel = 0; channel < self->channels.numchannels; ++channel) {
		dsp.memory_dealloc(self->channels.samples[channel]);
		self->channels.samples[channel] = 0;
	}
	psy_audio_buffer_dispose(&self->channels);
	self->numframes = 0;
	free(self->name);
}

psy_audio_Sample* psy_audio_sample_alloc(void)
{
	return (psy_audio_Sample*) malloc(sizeof(psy_audio_Sample));
}

psy_audio_Sample* psy_audio_sample_allocinit(uintptr_t numchannels)
{
	psy_audio_Sample* rv;

	rv = psy_audio_sample_alloc();
	if (rv) {
		psy_audio_sample_init(rv, numchannels);
	}
	return rv;
}

psy_audio_Sample* psy_audio_sample_clone(psy_audio_Sample* src)
{
	psy_audio_Sample* rv = 0;
	
	rv = psy_audio_sample_alloc();
	if (rv) {
		uintptr_t channel;

		rv->samplerate = src->samplerate;
		rv->defaultvolume = src->defaultvolume;
		rv->globalvolume = src->globalvolume;
		rv->loop.start = src->loop.start;
		rv->loop.end = src->loop.end;
		rv->loop.type = src->loop.type;
		rv->sustainloop.start = src->sustainloop.start;
		rv->sustainloop.end = src->sustainloop.end;
		rv->sustainloop.type = src->sustainloop.type;
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
		rv->stereo = src->stereo;
		psy_audio_buffer_init(&rv->channels, src->channels.numchannels);
		for (channel = 0; channel < rv->channels.numchannels; ++channel) {
			rv->channels.samples[channel] = dsp.memory_alloc(src->numframes,
				sizeof(float));
		}
		psy_audio_buffer_clearsamples(&rv->channels, src->numframes);
		psy_audio_buffer_addsamples(&rv->channels, &src->channels, src->numframes, 1.0f);
	}
	return rv;
}

void psy_audio_sample_load(psy_audio_Sample* self, const char* path)
{
	char* delim;

	psy_audio_wave_load(self, path);
	delim = strrchr(path, '\\');	
	psy_audio_sample_setname(self, delim ? delim + 1 : path);
}

void psy_audio_sample_save(psy_audio_Sample* self, const char* path)
{
	psy_audio_wave_save(self, path);
}

void psy_audio_sample_setname(psy_audio_Sample* self, const char* name)
{
	free(self->name);
	self->name = strdup(name);
}

const char* psy_audio_sample_name(psy_audio_Sample* self)
{
	return self->name;
}

psy_audio_SampleIterator psy_audio_sample_begin(psy_audio_Sample* self)
{
	psy_audio_SampleIterator rv;

	psy_audio_sampleiterator_init(&rv, self);
	return rv;
}

void psy_audio_sample_setcontloop(psy_audio_Sample* self, psy_audio_SampleLoopType looptype,
	uintptr_t loopstart, uintptr_t loopend)
{
	psy_audio_sampleloop_init_all(&self->loop, looptype, loopstart,
		loopend);
}

void psy_audio_sample_setsustainloop(psy_audio_Sample* self, psy_audio_SampleLoopType looptype,
	uintptr_t loopstart, uintptr_t loopend)
{
	psy_audio_sampleloop_init_all(&self->sustainloop, looptype, loopstart,
		loopend);
}
