/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "operations.h"
/* std */
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* prototypes */
static void* dsp_memory_alloc(size_t count, size_t size);
static void dsp_memory_dealloc(void* address);
static void dsp_add(psy_dsp_amp_t *src, psy_dsp_amp_t *dst, uintptr_t num, psy_dsp_amp_t vol);
static void dsp_mul(psy_dsp_amp_t *dst, uintptr_t num, psy_dsp_amp_t mul);
static void dsp_movmul(psy_dsp_amp_t *src, psy_dsp_amp_t *dst, uintptr_t num, psy_dsp_amp_t mul);
static void dsp_clear(psy_dsp_amp_t *dst, uintptr_t num);
static psy_dsp_amp_t* dsp_crop(psy_dsp_amp_t* src, uintptr_t offset, uintptr_t num);
static void dsp_interleave(psy_dsp_amp_t* dst, psy_dsp_amp_t* left, psy_dsp_amp_t* right, uintptr_t num);
static void dsp_erase_all_nans_infinities_and_denormals(psy_dsp_amp_t* dst,
		uintptr_t num);
static void erase_all_nans_infinities_and_denormals(psy_dsp_amp_t* sample);
static float dsp_maxvol(const float* pSamples, uintptr_t numSamples);
static void dsp_accumulate(psy_dsp_big_amp_t* accumleft, 
					psy_dsp_big_amp_t* accumright, 
					const psy_dsp_amp_t* __restrict pSamplesL,
					const psy_dsp_amp_t* __restrict pSamplesR, int count);
static void dsp_reverse(psy_dsp_amp_t* dst, uintptr_t num);

void psy_dsp_noopt_init(psy_dsp_Operations* self)
{	
	self->memory_alloc = dsp_memory_alloc;
	self->memory_dealloc = dsp_memory_dealloc;
	self->add = dsp_add;
	self->mul = dsp_mul;
	self->movmul = dsp_movmul;
	self->clear = dsp_clear;
	self->crop = dsp_crop;
	self->interleave = dsp_interleave;
	self->erase_all_nans_infinities_and_denormals = dsp_erase_all_nans_infinities_and_denormals;
	self->maxvol = dsp_maxvol;
	self->accumulate = dsp_accumulate;
	self->reverse = dsp_reverse;
}

void* dsp_memory_alloc(size_t count, size_t size)
{
	return malloc(count * size);
}

void dsp_memory_dealloc(void* address)
{
	free(address);
}

void dsp_add(psy_dsp_amp_t *src, psy_dsp_amp_t *dst, uintptr_t num, psy_dsp_amp_t vol)
{
	for ( ; num != 0; ++dst, ++src, --num) {
		*dst += (*src * vol);
	}	
}

void dsp_mul(psy_dsp_amp_t *dst, uintptr_t num, psy_dsp_amp_t mul)
{	
	for ( ; num != 0; ++dst, --num) {
		*dst *= mul;		
	}	
}

void dsp_movmul(psy_dsp_amp_t *src, psy_dsp_amp_t *dst, uintptr_t num, psy_dsp_amp_t mul)
{
	--src;
	--dst;
	do
	{
		*++dst = *++src*mul;
	}
	while (--num);
}

void dsp_clear(psy_dsp_amp_t *dst, uintptr_t num)
{
	memset(dst, 0, num * sizeof(psy_dsp_amp_t));
}

psy_dsp_amp_t* dsp_crop(psy_dsp_amp_t* src, uintptr_t offset, uintptr_t num)
{
	psy_dsp_amp_t* rv;

	if (num > 0) {
		uintptr_t i;
		psy_dsp_amp_t* frame;
		psy_dsp_amp_t* dst;

		dst = rv = dsp_memory_alloc(num, sizeof(psy_dsp_amp_t));
		for (frame = src + offset, i = 0; i < num; ++i) {
			*(dst++) = *(frame++);
		}
	} else {
		rv = NULL;
	}
	dsp.memory_dealloc(src);
	return rv;
}

void dsp_interleave(psy_dsp_amp_t* dst, psy_dsp_amp_t* left, psy_dsp_amp_t* right, uintptr_t num)
{
	uintptr_t i;
	--dst;
	--left;
	--right;	
	i = num;
	do {
		++dst;
		++left;
		++right;
		*dst = *left;
		++dst;
		*dst = *right;
	}
	while (--i);
}

float dsp_maxvol(const float* pSamples, uintptr_t numSamples)
{
	float vol = 0.0f;
	--pSamples;	
	do { /// not all waves are symmetrical
		const float nvol = (float) fabs(*++pSamples);
		if (nvol > vol) vol = nvol;
	} while (--numSamples);
	return vol;
}

void dsp_erase_all_nans_infinities_and_denormals(psy_dsp_amp_t* dst,
		uintptr_t num) {
	uintptr_t i;

	for(i = 0; i < num; ++i) {
		erase_all_nans_infinities_and_denormals(&dst[i]);
	}
}

/// Cure for malicious samples
/// Type : Filters Denormals, NaNs, Infinities
/// References : Posted by urs[AT]u-he[DOT]com
void erase_all_nans_infinities_and_denormals(float* sample) {
	#if !defined DIVERSALIS__CPU__X86
		// just do nothing.. not crucial for other archs ?
	#else
		uint32_t const exponent_mask = 0x7f800000;
		uint32_t exponent;
		// exponent < exponent_mask is 0 if NaN or Infinity, otherwise 1
		uint32_t not_nan_nor_infinity;

		// exponent > 0 is 0 if denormalized, otherwise 1
		uint32_t not_denormal;
		union {
			float sample;
			uint32_t bits;
		} u;
		u.sample = *sample;		
		exponent = u.bits & exponent_mask;
		// exponent < exponent_mask is 0 if NaN or Infinity, otherwise 1
		not_nan_nor_infinity = exponent < exponent_mask;
		// exponent > 0 is 0 if denormalized, otherwise 1
		not_denormal = exponent > 0;
		u.bits *= not_nan_nor_infinity & not_denormal;
		*sample = u.sample;
	#endif
}

void dsp_accumulate(psy_dsp_big_amp_t* accumleft, 
					psy_dsp_big_amp_t* accumright, 
					const psy_dsp_amp_t* __restrict pSamplesL,
					const psy_dsp_amp_t* __restrict pSamplesR,
					int count)
{
	psy_dsp_big_amp_t acleft = *accumleft;
	psy_dsp_big_amp_t acright = *accumright;
	--pSamplesL; --pSamplesR;
	while (count--) {
		++pSamplesL; acleft  += *pSamplesL * *pSamplesL;
		++pSamplesR; acright += *pSamplesR * *pSamplesR;
	}
	*accumleft = acleft;
	*accumright = acright;
}

void dsp_reverse(psy_dsp_amp_t* dst, uintptr_t num)
{
	if (num > 0) {
		uintptr_t j;
		uintptr_t halved;

		halved = (uintptr_t)floor(num / 2.0f);

		for (j = 0; j < halved; ++j) {
			psy_dsp_amp_t temp;

			temp = dst[j];
			dst[j] = dst[num - 1 - j];
			dst[num - 1 - j] = temp;
		}
	}
}
