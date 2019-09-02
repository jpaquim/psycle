// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLE_H)
#define SAMPLE_H

#include "buffer.h"

typedef struct {
	Buffer channels;
	unsigned int numframes;
	unsigned int samplerate;
	char* name;
} Sample;

void sample_init(Sample*);
void sample_dispose(Sample*);
void sample_load(Sample*, const char* path);
void sample_setname(Sample*, const char* name);
const char* sample_name(Sample*);

#endif