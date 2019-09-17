// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "buffercontext.h"

void buffercontext_init(BufferContext* self, List* events, Buffer* input, 
	Buffer* output, unsigned int numsamples, unsigned int numtracks)
{	
	self->events = events;
	self->input = input;
	self->output = output;
	self->numsamples = numsamples;
	self->numtracks = numtracks;	
}

void buffercontext_dispose(BufferContext* self)
{	
}

unsigned int buffercontext_numsamples(BufferContext* self)
{
	return self->numsamples;
}

unsigned int buffercontext_numtracks(BufferContext* self)
{
	return self->numtracks;
}