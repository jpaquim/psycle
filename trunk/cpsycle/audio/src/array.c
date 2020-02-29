// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "array.h"

#include <operations.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "../../detail/psydef.h"

void psy_audio_array_init(psy_audio_Array* self)
{
    self->ptr_ = 0;
	self->base_ = 0;
    self->shared_ = 0;
    self->baselen_ = 0;
	self->len_ = 0;
	self->cap_ = 0;
}

void psy_audio_array_init_len(psy_audio_Array* self, int len, float value)
{
    self->len_ = len;
    self->baselen_ = len;
    self->shared_ = 0;
	self->cap_ = len - (len % 4) + 4;
	self->ptr_ = dsp.memory_alloc(self->cap_, sizeof(float));
	self->base_ = self->ptr_;
	dsp.clear(self->ptr_, len);	
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

void psy_audio_array_init_arange(psy_audio_Array* self, float start, float stop, float step)
{	
	int count = 0;
	double i;

	self->len_ = (stop - start + 0.5) / step;
	self->baselen_ = self->len_;
	self->cap_ = self->len_ - (self->len_ % 4) + 4;
	self->ptr_ = dsp.memory_alloc(self->cap_, sizeof(float)); // reserve
	if (self->ptr_) {
		self->base_ = self->ptr_;
		for (i = start; i < stop; i += step, ++count) {
			self->ptr_[count] = i;
		}
	} else {
		self->cap_ = 0;		
		self->base_ = 0;
		self->baselen_ = 0;
		self->len_ = 0;
	}
}

void psy_audio_array_dispose(psy_audio_Array* self)
{
	if (!self->shared_) {
		dsp.memory_dealloc(self->ptr_);		
	}
	self->ptr_ = 0;
	self->base_ = 0;
    self->shared_ = 0;
    self->baselen_ = 0;
	self->len_ = 0;
	self->cap_ = 0;
}

psy_audio_Array* psy_audio_array_alloc(void)
{
	return malloc(sizeof(psy_audio_Array));
}

uintptr_t psy_audio_array_len(psy_audio_Array* self)
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

void psy_audio_array_margin(psy_audio_Array* self, int start, int end)
{
  self->ptr_ = self->base_ + start;
  assert(end >= start && start >= 0 && end <= self->baselen_);
  self->len_ = end-start;  
}

void psy_audio_array_offset(psy_audio_Array* self, int offset)
{
  self->base_ += offset;
  self->ptr_ += offset;  
}

void psy_audio_array_clearmargin(psy_audio_Array* self)
{
  self->ptr_ = self->base_;
  self->len_ = self->baselen_;  
}

void psy_audio_array_fill(psy_audio_Array* self, float val)
{
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = val;
	}
}

void psy_audio_array_fillzero(psy_audio_Array* self)
{
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = 0;
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

void psy_audio_array_div_constant(psy_audio_Array* self, float factor)
{
	dsp.mul(self->ptr_, self->len_, 1.f / factor);
}

void psy_audio_array_div_array(psy_audio_Array* self, psy_audio_Array* other)
{
	float* source;
	uintptr_t i;
	uintptr_t num;

	num = (self->len_ < other->len_) ? self->len_ : other->len_;	  
	source = psy_audio_array_data(other);
	for (i = 0; i < num; ++i) {
		self->ptr_[i] /= source[i];
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

void psy_audio_array_add_array(psy_audio_Array* self, psy_audio_Array* other)
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

void psy_audio_array_add_constant(psy_audio_Array* self, float value)
{	
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] += value;
	}
}

void psy_audio_array_sub_array(psy_audio_Array* self, psy_audio_Array* other)
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

void psy_audio_array_sub_constant(psy_audio_Array* self, float value)
{	
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] -= value;
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
void psy_audio_array_pow(psy_audio_Array* self)
{
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float)sin(self->ptr_[i]);
	}
}

void psy_audio_array_random(psy_audio_Array* self)
{  
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {		
		self->ptr_[i] = (float)(rand()%RAND_MAX) / (float)RAND_MAX;
	}
}

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

void psy_audio_array_sgn(psy_audio_Array* self)
{
	uintptr_t i;

	for (i = 0; i < self->len_; ++i) {		
		self->ptr_[i] = (self->ptr_[i] > 0) ? 1.f : ((self->ptr_[i] < 0) ? -1.f : 0.f);
	}
}

void psy_audio_array_max_array(psy_audio_Array* self, psy_audio_Array* other)
{
	float* source;
	uintptr_t i;
	uintptr_t num;

	num = (self->len_ < other->len_) ? self->len_ : other->len_;	  
	source = psy_audio_array_data(other);
	for (i = 0; i < num; ++i) {
		self->ptr_[i] = max(self->ptr_[i], source[i]);
	}
}

void psy_audio_array_max_constant(psy_audio_Array* self, float value)
{		
	uintptr_t i;	
			
	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = max(self->ptr_[i], value);
	}
}

void psy_audio_array_min_array(psy_audio_Array* self, psy_audio_Array* other)
{
	float* source;
	uintptr_t i;
	uintptr_t num;

	num = (self->len_ < other->len_) ? self->len_ : other->len_;	  
	source = psy_audio_array_data(other);
	for (i = 0; i < num; ++i) {
		self->ptr_[i] = min(self->ptr_[i], source[i]);		
	}
}

void psy_audio_array_min_constant(psy_audio_Array* self, float value)
{	
	uintptr_t i;	
			
	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = min(self->ptr_[i], value);
	}
}

void psy_audio_array_band_array(psy_audio_Array* self, psy_audio_Array* other)
{
	float* source;
	uintptr_t i;
	uintptr_t num;

	num = (self->len_ < other->len_) ? self->len_ : other->len_;	  
	source = psy_audio_array_data(other);
	for (i = 0; i < num; ++i) {
		int v = (int)(self->ptr_[i]) & (int)(source[i]);
		self->ptr_[i] = (float)(v);
	}
}

void psy_audio_array_band_constant(psy_audio_Array* self, float value)
{	
	uintptr_t i;	
			
	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float)((int)(self->ptr_[i]) & (int) value);
	}
}

void psy_audio_array_bor_array(psy_audio_Array* self, psy_audio_Array* other)
{
	float* source;
	uintptr_t i;
	uintptr_t num;

	num = (self->len_ < other->len_) ? self->len_ : other->len_;	  
	source = psy_audio_array_data(other);
	for (i = 0; i < num; ++i) {
		int v = (int)(self->ptr_[i]) | (int)(source[i]);
		self->ptr_[i] = (float)(v);
	}
}

void psy_audio_array_bor_constant(psy_audio_Array* self, float value)
{	
	uintptr_t i;	
			
	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float)((int)(self->ptr_[i]) | (int) value);
	}
}

void psy_audio_array_bxor_array(psy_audio_Array* self, psy_audio_Array* other)
{
	float* source;
	uintptr_t i;
	uintptr_t num;

	num = (self->len_ < other->len_) ? self->len_ : other->len_;	  
	source = psy_audio_array_data(other);
	for (i = 0; i < num; ++i) {
		int v = (int)(self->ptr_[i]) ^ (int)(source[i]);
		self->ptr_[i] = (float)(v);
	}
}

void psy_audio_array_bxor_constant(psy_audio_Array* self, float value)
{	
	uintptr_t i;	
			
	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float)((int)(self->ptr_[i]) ^ (int) value);
	}
}

void psy_audio_array_sleft_array(psy_audio_Array* self, psy_audio_Array* other)
{
	float* source;
	uintptr_t i;
	uintptr_t num;

	num = (self->len_ < other->len_) ? self->len_ : other->len_;	  
	source = psy_audio_array_data(other);
	for (i = 0; i < num; ++i) {
		int v = (int)(self->ptr_[i]) << (int)(source[i]);
		self->ptr_[i] = (float)(v);
	}
}

void psy_audio_array_sleft_constant(psy_audio_Array* self, float value)
{	
	uintptr_t i;	
			
	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float)((int)(self->ptr_[i]) << (int) value);
	}
}

void psy_audio_array_sright_array(psy_audio_Array* self, psy_audio_Array* other)
{
	float* source;
	uintptr_t i;
	uintptr_t num;

	num = (self->len_ < other->len_) ? self->len_ : other->len_;	  
	source = psy_audio_array_data(other);
	for (i = 0; i < num; ++i) {
		int v = (int)(self->ptr_[i]) >> (int)(source[i]);
		self->ptr_[i] = (float)(v);
	}
}

void psy_audio_array_sright_constant(psy_audio_Array* self, float value)
{	
	uintptr_t i;	
			
	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float)((int)(self->ptr_[i]) >> (int) value);
	}
}

void psy_audio_array_bnot(psy_audio_Array* self)
{	
	uintptr_t i;	
			
	for (i = 0; i < self->len_; ++i) {
		self->ptr_[i] = (float)(~(int)(self->ptr_[i]));
	}
}

int psy_audio_array_copy(psy_audio_Array* self, psy_audio_Array* source)
{
	if (psy_audio_array_len(source)  != self->len_) {
		return 0;
	}  
	dsp.movmul(psy_audio_array_data(source), self->ptr_, self->len_, 1.f);
	return 1;
}

void psy_audio_array_resize(psy_audio_Array* self, uintptr_t newsize)
{
	if (!self->shared_ && self->cap_ < newsize) {
		float* buf;
		uintptr_t i;

		self->cap_ = newsize - (newsize % 4) + 4;
		buf = dsp.memory_alloc(self->cap_, sizeof(float));
		for (i = 0; i < self->baselen_; ++i) {
			buf[i] = self->ptr_[i];
		}
		dsp.memory_dealloc(self->base_);
		self->base_ = buf;
		self->ptr_ = buf;	
		self->baselen_ = self->len_ = newsize;
	}
}

