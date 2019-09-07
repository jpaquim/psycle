// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "sample.h"
#include "waveio.h"
#include <string.h>
#include <stdlib.h>

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
	self->name = strdup("");
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

void sample_load(Sample* self, const char* path)
{
	char* delim;
	wave_load(self, path);
	delim = strrchr(path, '\\');	
	sample_setname(self, delim ? delim + 1 : path);	
}

void sample_setname(Sample* self, const char* name)
{
	free(self->name);
	self->name = strdup(name);
}

const char* sample_name(Sample* self)
{
	return self->name;
}
