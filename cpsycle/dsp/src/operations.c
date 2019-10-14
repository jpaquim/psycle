// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "operations.h"
#include <string.h>

static void erase_all_nans_infinities_and_denormals(float* sample);

void dsp_add(float *src, float *dst, int num, float vol)
{
	for ( ; num != 0; ++dst, ++src, --num) {
		*dst += (*src * vol);
	}	
}
	
void dsp_mul(float *dst, int num, float mul)
{	
	for ( ; num != 0; ++dst, --num) {
		*dst *= mul;		
	}	
}
	
void dsp_movmul(float *src, float *dst, int num, float mul)
{
	--src;
	--dst;
	do
	{
		*++dst = *++src*mul;
	}
	while (--num);
}
	
void dsp_clear(float *dst, int num)
{
	memset(dst, 0, num * sizeof(float));
}

void dsp_interleave(float* dst, float* left, float* right, int num)
{
	int i;		
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

void dsp_erase_all_nans_infinities_and_denormals(float* dst,
		unsigned int num) {
	unsigned int i;

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
		union {
			float sample;
			uint32_t bits;
		} u;
		u.sample = sample;

		uint32_t const exponent_mask(0x7f800000);
		uint32_t const exponent(u.bits & exponent_mask);

		// exponent < exponent_mask is 0 if NaN or Infinity, otherwise 1
		uint32_t const not_nan_nor_infinity(exponent < exponent_mask);

		// exponent > 0 is 0 if denormalized, otherwise 1
		uint32_t const not_denormal(exponent > 0);

		u.bits *= not_nan_nor_infinity & not_denormal;
		*sample = u.sample;
	#endif
}