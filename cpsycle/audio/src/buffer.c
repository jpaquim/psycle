#include "buffer.h"
#include <operations.h>
#include <stdlib.h>
#include <string.h>

void buffer_init(Buffer* self, unsigned int channels)
{
	self->samples = 0;	
	self->offset = 0;
	buffer_resize(self, channels);
}

void buffer_init_shared(Buffer* self, Buffer* src, unsigned int offset)
{
	unsigned int channel;

	buffer_init(self, src->numchannels);
	for (channel = 0; channel < src->numchannels; ++channel) {
		self->samples[channel] = src->samples[channel] + offset;
	}
}

void buffer_move(Buffer* self, unsigned int offset)
{	
	unsigned int channel;

	for (channel = 0; channel < self->numchannels; ++channel) {
		self->samples[channel] = self->samples[channel] + offset;
	}
}

void buffer_dispose(Buffer* self)
{
	free(self->samples);
}

void buffer_resize(Buffer* self, unsigned int channels)
{
	free(self->samples);
	self->samples = 0;
	if (channels > 0) {
		self->samples = (real**) malloc(sizeof(real*)*channels);
		self->numchannels = channels;
		memset(self->samples, 0, sizeof(real*)*channels);
	}	
}

void buffer_setoffset(Buffer* self, unsigned int offset)
{
	self->offset = offset;
}

unsigned int buffer_offset(Buffer* self)
{
	return self->offset;
}

real* buffer_at(Buffer* self, unsigned int channel)
{
	return self->samples[channel] + self->offset;
}

void buffer_clearsamples(Buffer* self, unsigned int numsamples)
{
	unsigned int channel;

	for (channel = 0; channel < self->numchannels; ++channel) {
		dsp_clear(self->samples[channel], numsamples);
	}
}

void buffer_addsamples(Buffer* self, Buffer* source, unsigned int numsamples,
	float vol)
{
	unsigned int channel;

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

void buffer_mulsamples(Buffer* self, unsigned int numsamples, float mul)
{
	unsigned int channel;
	
	for (channel = 0; channel < self->numchannels; ++channel) {
		dsp_mul(self->samples[channel], numsamples, mul);
	}	
}
