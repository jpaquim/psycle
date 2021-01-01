// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "buffercontext.h"
#include <rms.h>
#include <math.h>

void psy_audio_buffercontext_init(psy_audio_BufferContext* self,
	psy_List* events,
	psy_audio_Buffer* input,
	psy_audio_Buffer* output,
	uintptr_t numsamples,
	uintptr_t numtracks)
{	
	self->events = events;
	self->input = input;
	self->output = output;
	self->numsamples = numsamples;
	self->numtracks = numtracks;
}

void psy_audio_buffercontext_dispose(psy_audio_BufferContext* self)
{
}

void psy_audio_buffercontext_setoffset(psy_audio_BufferContext* self, uintptr_t offset)
{
	if (self->input) {
		psy_audio_buffer_setoffset(self->input, offset);
	}
	if (self->output) {
		psy_audio_buffer_setoffset(self->output, offset);
	}
}
