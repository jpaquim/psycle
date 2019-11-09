// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "buffer.h"
#include <operations.h>
#include <stdlib.h>
#include <string.h>

void buffer_init(Buffer* self, uintptr_t channels)
{
	self->samples = 0;	
	self->offset = 0;
	buffer_resize(self, channels);
}

void buffer_init_shared(Buffer* self, Buffer* src, uintptr_t offset)
{
	uintptr_t channel;

	buffer_init(self, src->numchannels);
	for (channel = 0; channel < src->numchannels; ++channel) {
		self->samples[channel] = src->samples[channel] + offset;
	}
}

void buffer_move(Buffer* self, uintptr_t offset)
{	
	uintptr_t channel;

	for (channel = 0; channel < self->numchannels; ++channel) {
		self->samples[channel] = self->samples[channel] + offset;
	}
}

void buffer_dispose(Buffer* self)
{
	free(self->samples);
}

Buffer* buffer_alloc(void)
{
	return (Buffer*) malloc(sizeof(Buffer));
}

Buffer* buffer_allocinit(uintptr_t channels)
{
	Buffer* rv;

	rv = buffer_alloc();
	if (rv) {
		buffer_init(rv, channels);
	}
	return rv;
}


void buffer_resize(Buffer* self, uintptr_t channels)
{
	free(self->samples);
	self->samples = 0;	
	if (channels > 0) {
		self->samples = (amp_t**) malloc(sizeof(amp_t*)*channels);		
		memset(self->samples, 0, sizeof(amp_t*)*channels);
	}
	self->numchannels = channels;
}

void buffer_setoffset(Buffer* self, uintptr_t offset)
{
	self->offset = offset;
}

uintptr_t buffer_offset(Buffer* self)
{
	return self->offset;
}

amp_t* buffer_at(Buffer* self, uintptr_t channel)
{
	return self->samples[channel] + self->offset;
}

void buffer_clearsamples(Buffer* self, uintptr_t numsamples)
{
	uintptr_t channel;

	for (channel = 0; channel < self->numchannels; ++channel) {
		dsp_clear(self->samples[channel], numsamples);
	}
}

void buffer_addsamples(Buffer* self, Buffer* source, uintptr_t numsamples,
	amp_t vol)
{
	uintptr_t channel;

	if (source) {
		for (channel = 0; channel < source->numchannels && 
			channel < self->numchannels; ++channel) {
				dsp_add(
					source->samples[channel],
					self->samples[channel],
					numsamples,
					vol);
		}
	}
}

void buffer_mulsamples(Buffer* self, uintptr_t numsamples, amp_t mul)
{
	uintptr_t channel;
	
	for (channel = 0; channel < self->numchannels; ++channel) {
		dsp_mul(self->samples[channel], numsamples, mul);
	}	
}

void buffer_pan(Buffer* self, amp_t pan, uintptr_t amount)
{
	uintptr_t channel;
	amp_t vol[2];

	vol[1] = pan * 2.f;
	vol[0] = 2.0f - vol[1];
	if (vol[0] > 1.0f) {
		vol[0] = 1.0f;
	}
	if (vol[1] > 1.0f) {
		vol[1] = 1.0f;
	}
	for (channel = 0; channel < 2 && channel < self->numchannels; ++channel) {
		dsp_mul(self->samples[channel], amount, vol[channel]);
	}
}

uintptr_t buffer_numchannels(Buffer* self)
{
	return self->numchannels;
}

int buffer_mono(Buffer* self)
{
	return self->numchannels == 1;
}
