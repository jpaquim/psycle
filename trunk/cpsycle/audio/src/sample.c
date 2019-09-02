// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "sample.h"
#include "waveio.h"
#include <string.h>
#include <stdlib.h>

void sample_init(Sample* self)
{
	buffer_init(&self->channels, 2);
	self->numframes = 0;
	self->samplerate = 44100;
	self->name = strdup("");
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
