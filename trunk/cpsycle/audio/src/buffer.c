// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "buffer.h"
#include <operations.h>
#include <stdlib.h>
#include <string.h>

static psy_dsp_amp_t psy_audio_buffer_rangefactor(psy_audio_Buffer*,
	psy_dsp_amp_range_t);

void psy_audio_buffer_init(psy_audio_Buffer* self, uintptr_t channels)
{
	self->samples = 0;	
	self->offset = 0;
	self->numsamples = 0;
	self->range = PSY_DSP_AMP_RANGE_NATIVE;
	psy_audio_buffer_resize(self, channels);
}

void psy_audio_buffer_init_shared(psy_audio_Buffer* self, psy_audio_Buffer* src,
	uintptr_t offset)
{
	uintptr_t channel;

	psy_audio_buffer_init(self, src->numchannels);
	for (channel = 0; channel < src->numchannels; ++channel) {
		self->samples[channel] = src->samples[channel] + offset;
	}
}

void psy_audio_buffer_move(psy_audio_Buffer* self, uintptr_t offset)
{	
	uintptr_t channel;

	for (channel = 0; channel < self->numchannels; ++channel) {
		self->samples[channel] = self->samples[channel] + offset;
	}
}

void psy_audio_buffer_dispose(psy_audio_Buffer* self)
{
	free(self->samples);
}

psy_audio_Buffer* psy_audio_buffer_alloc(void)
{
	return (psy_audio_Buffer*) malloc(sizeof(psy_audio_Buffer));
}

psy_audio_Buffer* psy_audio_buffer_allocinit(uintptr_t channels)
{
	psy_audio_Buffer* rv;

	rv = psy_audio_buffer_alloc();
	if (rv) {
		psy_audio_buffer_init(rv, channels);
	}
	return rv;
}


void psy_audio_buffer_resize(psy_audio_Buffer* self, uintptr_t channels)
{
	free(self->samples);
	self->samples = 0;	
	if (channels > 0) {
		self->samples = (psy_dsp_amp_t**) malloc(sizeof(psy_dsp_amp_t*)*channels);		
		memset(self->samples, 0, sizeof(psy_dsp_amp_t*)*channels);
	}
	self->numchannels = channels;
}

void psy_audio_buffer_setoffset(psy_audio_Buffer* self, uintptr_t offset)
{
	self->offset = offset;
}

uintptr_t psy_audio_buffer_offset(psy_audio_Buffer* self)
{
	return self->offset;
}

psy_dsp_amp_t* psy_audio_buffer_at(psy_audio_Buffer* self, uintptr_t channel)
{
	return self->samples[channel] + self->offset;
}

void psy_audio_buffer_clearsamples(psy_audio_Buffer* self, uintptr_t numsamples)
{
	uintptr_t channel;

	for (channel = 0; channel < self->numchannels; ++channel) {
		dsp.clear(self->samples[channel], numsamples);
	}
}

void psy_audio_buffer_addsamples(psy_audio_Buffer* self, psy_audio_Buffer* source,
	uintptr_t numsamples,
	psy_dsp_amp_t vol)
{
	if (source) {
		uintptr_t channel;
		psy_dsp_amp_t factor;

		factor = psy_audio_buffer_rangefactor(source, self->range) * vol;	
		for (channel = 0; channel < source->numchannels && 
			channel < self->numchannels; ++channel) {
				dsp.add(
					source->samples[channel],
					self->samples[channel],
					numsamples,
					factor);
		}
	}
}

void psy_audio_buffer_mulsamples(psy_audio_Buffer* self, uintptr_t numsamples, psy_dsp_amp_t mul)
{
	uintptr_t channel;
	
	for (channel = 0; channel < self->numchannels; ++channel) {
		dsp.mul(self->samples[channel], numsamples, mul);
	}	
}

void psy_audio_buffer_pan(psy_audio_Buffer* self, psy_dsp_amp_t pan, uintptr_t amount)
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

uintptr_t psy_audio_buffer_numchannels(psy_audio_Buffer* self)
{
	return self->numchannels;
}

int psy_audio_buffer_mono(psy_audio_Buffer* self)
{
	return self->numchannels == 1;
}

void psy_audio_buffer_insertsamples(psy_audio_Buffer* self, psy_audio_Buffer* source, uintptr_t numsamples,
	uintptr_t numsourcesamples)
{	
	psy_dsp_amp_t rangefactor;

	rangefactor = psy_audio_buffer_rangefactor(source, self->range);
	if (numsourcesamples < numsamples) {		
		uintptr_t diff;		
		uintptr_t c;
		
		
		diff = numsamples - numsourcesamples;
		for (c = 0; c < self->numchannels; ++c) {
			dsp.clear(self->samples[c] + numsourcesamples, diff);			
			dsp.add(self->samples[c], self->samples[c] + numsourcesamples, diff, 
				rangefactor);
			dsp.clear(self->samples[c], numsourcesamples);
			if (c < source->numchannels) {
				dsp.add(source->samples[c], self->samples[c], numsourcesamples, 
				rangefactor);
			}
		}
	} else {
		uintptr_t c;

		for (c = 0; c < self->numchannels; ++c) {
			dsp.clear(self->samples[c], numsamples);
			if (c < source->numchannels) {
				dsp.add(source->samples[c], self->samples[c], numsamples,
					rangefactor);
			}
		}
	}
}

void psy_audio_buffer_scale(psy_audio_Buffer* self, psy_dsp_amp_range_t range,
	uintptr_t numsamples)
{	
	if (self->range != range && range != PSY_DSP_AMP_RANGE_IGNORE) {
		uintptr_t c;
		psy_dsp_amp_t rangefactor;

		rangefactor = psy_audio_buffer_rangefactor(self, range);
		for (c = 0; c < self->numchannels; ++c) {
			dsp.mul(self->samples[c], numsamples, rangefactor);
		}
		self->range = range;
	}
}

psy_dsp_amp_t psy_audio_buffer_rangefactor(psy_audio_Buffer* self, psy_dsp_amp_range_t range)
{
	psy_dsp_amp_t rv;

	if (self->range != range && range != PSY_DSP_AMP_RANGE_IGNORE) {
		if (range == PSY_DSP_AMP_RANGE_NATIVE) {
			rv = 32768.f;		
		} else 
		if (range == PSY_DSP_AMP_RANGE_VST) {
			rv = 1 / 32768.f;
		} else {
			rv = 1.f;
		}
	} else {
		rv = 1.f;
	}
	return rv;
}
