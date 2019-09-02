#include "buffer.h"
#include <stdlib.h>
#include <string.h>

void buffer_init(Buffer* self, unsigned int channels)
{
	self->samples = 0;	
	buffer_resize(self, channels);
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
