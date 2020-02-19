// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "array.h"

#include <operations.h>
#include <stdlib.h>
#include <math.h>

void psy_audio_array_init(psy_audio_Array* self)
{
    self->ptr_ = 0;
	self->base_ = 0;
    self->shared_ = 0;
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
}

void psy_audio_array_dispose(psy_audio_Array* self)
{
	if (!self->shared_) {
		free(self->ptr_);
	}
	self->ptr_ = 0;
	self->base_ = 0;
    self->shared_ = 0;
    self->baselen_ = 0;
	self->len_ = 0;
	self->cap_ = 0;
}

int psy_audio_array_len(psy_audio_Array* self)
{ 
	return self->len_;
}

void psy_audio_array_set(psy_audio_Array* self, uintptr_t index, float value)
{
	self->ptr_[index] = value;    
}

float psy_audio_array_at(psy_audio_Array* self, uintptr_t index)
{
	return self->ptr_[index];
}

float* psy_audio_array_data(psy_audio_Array* self)
{
	return self->ptr_;
}

void psy_audio_array_clear(psy_audio_Array* self)
{
	dsp.clear(self->ptr_, self->len_);
}

void psy_audio_array_fill(psy_audio_Array* self, float val)
{
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = val;
	}
}

void psy_audio_array_fillfrom(psy_audio_Array* self, float val, uintptr_t pos)
{
	uintptr_t num;
	uintptr_t i;

	if (self->len_ > pos) {
		num = self->len_ - pos;
		for (i = pos; i < num; ++i) {
			self->ptr_[i] = val;
		}
	}
}

void psy_audio_array_mul_constant(psy_audio_Array* self, float factor)
{
	dsp.mul(self->ptr_, self->len_, factor);
}

void psy_audio_array_mul_array(psy_audio_Array* self, psy_audio_Array* other)
{
	float* source;
	uintptr_t i;
	uintptr_t num;

	num = (self->len_ < other->len_) ? self->len_ : other->len_;	  
	source = psy_audio_array_data(other);
	for (i = 0; i < num; ++i) {
		self->ptr_[i] *= source[i];
	}
}

void psy_audio_array_mix(psy_audio_Array* self, psy_audio_Array* other,
	float factor)
{
	uintptr_t num;

	num = (self->len_ < other->len_) ? self->len_ : other->len_;	
	dsp.add(psy_audio_array_data(self), psy_audio_array_data(other), num,
		factor);
}

void psy_audio_array_add(psy_audio_Array* self, psy_audio_Array* other)
{
	float* source;
	uintptr_t i;
	uintptr_t num;

	num = (self->len_ < other->len_) ? self->len_ : other->len_;	  
	source = psy_audio_array_data(other);
	for (i = 0; i < num; ++i) {
		self->ptr_[i] += source[i];
	}
}

void psy_audio_array_addconstant(psy_audio_Array* self, float value)
{	
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] += value;
	}
}

void psy_audio_array_sub(psy_audio_Array* self, psy_audio_Array* other)
{	
	uintptr_t i;
	uintptr_t num;
	float* source;

	num = (self->len_ < other->len_) ? self->len_ : other->len_;	  
	source = psy_audio_array_data(other);
	for (i = 0; i < num; ++i) {
		self->ptr_[i] -= source[i];
	}
}

void psy_audio_array_rsum(psy_audio_Array* self, double lv)
{	
	uintptr_t i;
	double sum;
	
	sum = lv;
	for (i = 0; i < self->len_; ++i) {		
		sum += psy_audio_array_at(self, i);
		psy_audio_array_set(self, i, (float) sum);
	}
}

int psy_audio_array_copyfrom(psy_audio_Array* self, psy_audio_Array* other,
		uintptr_t pos) {
	uintptr_t i;
	uintptr_t num;
	float* source;

	if (pos < 0) {
		return 0;
	}
	num = (self->len_ < other->len_) ? self->len_ : other->len_;
	if (pos > num) {
		return 0;
	}
	source = psy_audio_array_data(other);
	num -= pos;
	for (i = 0; i < num; ++i) {
		self->ptr_[i + pos] = source[i];
	}
	return 1;
}

// array methods
void psy_audio_array_sqrt(psy_audio_Array* self)
{
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float) sqrt(self->ptr_[i]);
	}
}

void psy_audio_array_sin(psy_audio_Array* self)
{
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float) sin(self->ptr_[i]);
	}
}

void psy_audio_array_cos(psy_audio_Array* self)
{
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float) cos(self->ptr_[i]);
	}
}

void psy_audio_array_tan(psy_audio_Array* self)
{
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float) tan(self->ptr_[i]);
	}
}

void psy_audio_array_ceil(psy_audio_Array* self)
{
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float) ceil(self->ptr_[i]);
	}
}

void psy_audio_array_floor(psy_audio_Array* self)
{
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float) floor(self->ptr_[i]);
	}
}

void psy_audio_array_fabs(psy_audio_Array* self)
{
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float) fabs(self->ptr_[i]);
	}
}
