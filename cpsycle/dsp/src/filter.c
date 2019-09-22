// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "filter.h"

static void init(Filter* self) { }
static void dispose(Filter* self) { }
static float work(Filter* self, float sample) { return 0.f; }
void setcutoff(Filter* self, float cutoff) { }
float cutoff(Filter* self) { return 0.f; }
void setressonance(Filter* self, float ressonance) { }
float ressonance(Filter* self) { return 0.f; }
void setsamplerate(Filter* self, float samplerate) { }
float samplerate(Filter* self) { return 44100.f; }

void filter_init(Filter* self)		
{
	self->init = init;
	self->dispose = dispose;
	self->work = work;
}

