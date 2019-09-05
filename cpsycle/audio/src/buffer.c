#include "buffer.h"
#include <stdlib.h>
#include <string.h>

void buffer_init(Buffer* self, unsigned int channels)
{
	self->samples = 0;	
	self->pos = 0;
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

real* buffer_at(Buffer* self, unsigned int channel)
{
	return self->samples[channel] + self->pos;
}
