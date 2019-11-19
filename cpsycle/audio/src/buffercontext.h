// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(BUFFERCONTEXT_H)
#define BUFFERCONTEXT_H

#include "buffer.h"
#include <list.h>

typedef struct {
	Buffer* input;
	Buffer* output;
	List* events;
	unsigned int numsamples;
	unsigned int numtracks;
	struct RMSVol* rmsvol;
} BufferContext;

void buffercontext_init(BufferContext*, List* events, Buffer* input,
	Buffer* output, unsigned int numsamples, unsigned int numtracks,
	struct RMSVol* rmsvol);
void buffercontext_dispose(BufferContext*);
unsigned int buffercontext_numsamples(BufferContext*);
unsigned int buffercontext_numtracks(BufferContext*);
float buffercontext_rmsvolume(BufferContext*);


#endif
