// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "buffer.h"
#include <operations.h>
#include <stdlib.h>
#include <string.h>

void buffer_init(psy_audio_Buffer* self, uintptr_t channels)
{
	self->samples = 0;	
	self->offset = 0;
	buffer_resize(self, channels);
}

void buffer_init_shared(psy_audio_Buffer* self, psy_audio_Buffer* src, uintptr_t offset)
{
	uintptr_t channel;

	buffer_init(self, src->numchannels);
	for (channel = 0; channel < src->numchannels; ++channel) {
		self->samples[channel] = src->samples[channel] + offset;
	}
}

void buffer_move(psy_audio_Buffer* self, uintptr_t offset)
{	
	uintptr_t channel;

	for (channel = 0; channel < self->numchannels; ++channel) {
		self->samples[channel] = self->samples[channel] + offset;
	}
}

void buffer_dispose(psy_audio_Buffer* self)
{
	free(self->samples);
}

psy_audio_Buffer* buffer_alloc(void)
{
	return (psy_audio_Buffer*) malloc(sizeof(psy_audio_Buffer));
}

psy_audio_Buffer* buffer_allocinit(uintptr_t channels)
{
	psy_audio_Buffer* rv;

	rv = buffer_alloc();
	if (rv) {
		buffer_init(rv, channels);
	}
	return rv;
}


void buffer_resize(psy_audio_Buffer* self, uintptr_t channels)
{
	free(self->samples);
	self->samples = 0;	
	if (channels > 0) {
		self->samples = (psy_dsp_amp_t**) malloc(sizeof(psy_dsp_amp_t*)*channels);		
		memset(self->samples, 0, sizeof(psy_dsp_amp_t*)*channels);
	}
	self->numchannels = channels;
}

void buffer_setoffset(psy_audio_Buffer* self, uintptr_t offset)
{
	self->offset = offset;
}

uintptr_t buffer_offset(psy_audio_Buffer* self)
{
	return self->offset;
}

psy_dsp_amp_t* buffer_at(psy_audio_Buffer* self, uintptr_t channel)
{
	return self->samples[channel] + self->offset;
}

void buffer_clearsamples(psy_audio_Buffer* self, uintptr_t numsamples)
{
	uintptr_t channel;

	for (channel = 0; channel < self->numchannels; ++channel) {
		dsp.clear(self->samples[channel], numsamples);
	}
}

void buffer_addsamples(psy_audio_Buffer* self, psy_audio_Buffer* source, uintptr_t numsamples,
	psy_dsp_amp_t vol)
{
	uintptr_t channel;

	if (source) {
		for (channel = 0; channel < source->numchannels && 
			channel < self->numchannels; ++channel) {
				dsp.add(
					source->samples[channel],
					self->samples[channel],
					numsamples,
					vol);
		}
	}
}

void buffer_mulsamples(psy_audio_Buffer* self, uintptr_t numsamples, psy_dsp_amp_t mul)
{
	uintptr_t channel;
	
	for (channel = 0; channel < self->numchannels; ++channel) {
		dsp.mul(self->samples[channel], numsamples, mul);
	}	
}

void buffer_pan(psy_audio_Buffer* self, psy_dsp_amp_t pan, uintptr_t amount)
{
	uintptr_t channel;
	psy_dsp_amp_t vol[2];

	vol[1] = pan * 2.f;
	vol[0] = 2.0f - vol[1];
	if (vol[0] > 1.0f) {
		vol[0] = 1.0f;
	}
	if (vol[1] > 1.0f) {
		vol[1] = 1.0f;
	}
	for (channel = 0; channel < 2 && channel < self->numchannels; ++channel) {
		dsp.mul(self->samples[channel], amount, vol[channel]);
	}
}

uintptr_t buffer_numchannels(psy_audio_Buffer* self)
{
	return self->numchannels;
}

int buffer_mono(psy_audio_Buffer* self)
{
	return self->numchannels == 1;
}

void buffer_insertsamples(psy_audio_Buffer* self, psy_audio_Buffer* source, uintptr_t numsamples,
	uintptr_t numsourcesamples)
{	
	if (numsourcesamples < numsamples) {		
		uintptr_t diff;		
		uintptr_t c;

		diff = numsamples - numsourcesamples;		
		for (c = 0; c < self->numchannels; ++c) {
			dsp.clear(self->samples[c] + numsourcesamples, diff);			
			dsp.add(self->samples[c], self->samples[c] + numsourcesamples, diff, 1.f);
			dsp.clear(self->samples[c], numsourcesamples);
			dsp.add(source->samples[c], self->samples[c], numsourcesamples, 1.f);
		}
	} else {
		uintptr_t c;

		for (c = 0; c < self->numchannels; ++c) {
			dsp.clear(self->samples[c], numsamples);			
			dsp.add(source->samples[c], self->samples[c], numsamples, 1.f);	
		}
	}
}
