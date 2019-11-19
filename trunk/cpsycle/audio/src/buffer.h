// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(BUFFER_H)
#define BUFFER_H

#include <dsptypes.h>
#include "../../detail/stdint.h"

typedef struct {
	uintptr_t numchannels;	
	amp_t** samples;
	uintptr_t offset;
} Buffer;

void buffer_init(Buffer*, uintptr_t channels);
void buffer_init_shared(Buffer*, Buffer* src, uintptr_t offset);
void buffer_dispose(Buffer*);
Buffer* buffer_alloc(void);
Buffer* buffer_allocinit(uintptr_t channels);
void buffer_resize(Buffer*, uintptr_t channels);
void buffer_move(Buffer*, uintptr_t offset);
void buffer_setoffset(Buffer*, uintptr_t offset);
uintptr_t buffer_offset(Buffer*);
amp_t* buffer_at(Buffer*, uintptr_t channel);
void buffer_clearsamples(Buffer*, uintptr_t numsamples);
void buffer_addsamples(Buffer*, Buffer* source, uintptr_t numsamples,
	float vol);
void buffer_mulsamples(Buffer*, uintptr_t numsamples, amp_t mul);
uintptr_t buffer_numchannels(Buffer*);
void buffer_pan(Buffer* self, amp_t pan, uintptr_t amount);
int buffer_mono(Buffer*);

#endif
