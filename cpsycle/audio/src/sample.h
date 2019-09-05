// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLE_H)
#define SAMPLE_H

#include "buffer.h"

/// Wave Form Types
typedef struct {
	enum {
		SINUS = 0x0,
		SQUARE = 0x1,
		SAWUP = 0x2,
		SAWDOWN = 0x3,
		RANDOM = 0x4
	} Type;	
} WaveForms;

typedef struct {
	Buffer channels;
	unsigned int numframes;
	unsigned int samplerate;
	char* name;
	float defaultvolume;
	float globalvolume;
	/// Tuning for the center note (value that is added to the note received).
	/// values from -60 to 59. 0 = C-5 (middle C, i.e. play at original speed
	/// with note C-5);
	short tune;
} Sample;

void sample_init(Sample*);
void sample_dispose(Sample*);
void sample_load(Sample*, const char* path);
void sample_setname(Sample*, const char* name);
const char* sample_name(Sample*);

#endif