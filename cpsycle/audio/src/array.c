// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "array.h"

#include <stdlib.h>

void psy_audio_array_init(psy_audio_Array* self)
{
    self->ptr_ = 0;
	self->base_ = 0;
    self->shared_ = 0;
	self->can_aligned_ = 0;
    self->baselen_ = 0;
	self->len_ = 0;
	self->cap_ = 0;
}

void psy_audio_array_init_shared(psy_audio_Array* self, float* ptr, int len)
{
	self->ptr_ = ptr;
	self->base_ = ptr;
	self->cap_ = 0;
	self->len_ = len;
	self->baselen_ = len;
	self->shared_ = 1;
	self->can_aligned_ = len & 3;	
}

void psy_audio_array_dispose(psy_audio_Array* self)
{
	if (!self->shared_) {
		free(self->ptr_);
	}
	self->ptr_ = 0;
	self->base_ = 0;
    self->shared_ = 0;
	self->can_aligned_ = 0;
    self->baselen_ = 0;
	self->len_ = 0;
	self->cap_ = 0;
}

int psy_audio_array_len(psy_audio_Array* self)
{ 
	return self->len_;
}

float* psy_audio_array_data(psy_audio_Array* self)
{
	return self->ptr_;
}
