// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(BUFFER_H)
#define BUFFER_H

typedef float real;

typedef struct {
	unsigned int numchannels;	
	real** samples;
	unsigned int offset;	
} Buffer;

void buffer_init(Buffer*, unsigned int channels);
void buffer_init_shared(Buffer*, Buffer* src, unsigned int offset);
void buffer_dispose(Buffer*);
void buffer_resize(Buffer*, unsigned int channels);
void buffer_move(Buffer*, unsigned int offset);
void buffer_setoffset(Buffer*, unsigned int offset);
unsigned int buffer_offset(Buffer*);
real* buffer_at(Buffer*, unsigned int channel);
void buffer_clearsamples(Buffer*, unsigned int numsamples);
void buffer_addsamples(Buffer*, Buffer* source, unsigned int numsamples,
	float vol);
void buffer_mulsamples(Buffer*, unsigned int numsamples, float mul);
unsigned int buffer_numchannels(Buffer*);
int buffer_mono(Buffer*);

#endif