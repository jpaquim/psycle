// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "operations.h"
#include <string.h>
#if defined SSE
#include <xmmintrin.h>
#endif

static void erase_all_nans_infinities_and_denormals(amp_t* sample);

void dsp_add(amp_t *src, amp_t *dst, uintptr_t num, amp_t vol)
{
	for ( ; num != 0; ++dst, ++src, --num) {
		*dst += (*src * vol);
	}	
}
	
void dsp_mul(amp_t *dst, uintptr_t num, amp_t mul)
{	
	for ( ; num != 0; ++dst, --num) {
		*dst *= mul;		
	}	
}
	
void dsp_movmul(amp_t *src, amp_t *dst, uintptr_t num, amp_t mul)
{
	--src;
	--dst;
	do
	{
		*++dst = *++src*mul;
	}
	while (--num);
}
	
void dsp_clear(amp_t *dst, uintptr_t num)
{
#if defined SSE
	const __m128 zeroval = _mm_set_ps1(0.0f);
	while (num > 0)
	{
		_mm_store_ps(dst, zeroval);
		dst += 4;
		num -= 4;
	}
#else
	memset(dst, 0, num * sizeof(amp_t));
#endif
}

void dsp_interleave(amp_t* dst, amp_t* left, amp_t* right, uintptr_t num)
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

void dsp_erase_all_nans_infinities_and_denormals(amp_t* dst,
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

//same method, for a single buffer (allowing to calculate max for each buffer). samples need to be aligned by 16 in optimized paths.
float dsp_maxvol(const float* pSamples, uintptr_t numSamples)
{
#if defined SSE
	__m128 minVol = _mm_set_ps1(0.0f);
	__m128 maxVol = _mm_set_ps1(0.0f);
	const __m128* psrc = (const __m128*)pSamples;
	while (numSamples > 0) {
		maxVol = _mm_max_ps(maxVol, *psrc);
		minVol = _mm_min_ps(minVol, *psrc);
		psrc++;
		numSamples -= 4;
	}
	__m128 highTmp = _mm_movehl_ps(maxVol, maxVol);
	maxVol = _mm_max_ps(maxVol, highTmp);
	highTmp = _mm_move_ss(highTmp, maxVol);
	maxVol = _mm_shuffle_ps(maxVol, highTmp, 0x11);
	maxVol = _mm_max_ps(maxVol, highTmp);

	__m128 lowTmp = _mm_movehl_ps(minVol, minVol);
	minVol = _mm_max_ps(minVol, lowTmp);
	lowTmp = _mm_move_ss(lowTmp, minVol);
	minVol = _mm_shuffle_ps(minVol, lowTmp, 0x11);
	minVol = _mm_max_ps(minVol, lowTmp);

	__m128 minus1 = _mm_set_ps1(-1.0f);
	minVol = _mm_mul_ss(minVol, minus1);
	//beware: -0.0f and 0.0f are supposed to be the same number, -0.0f > 0.0f returns false, just because 0.0f == 0.0f returns true
	maxVol = _mm_max_ps(minVol, maxVol);
	float result;
	_mm_store_ss(&result, maxVol);
	return result;
#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
	// If anyone knows better assembler than me improve this variable utilization:
	float volmax = 0.0f, volmin = 0.0f;
	float* volmaxb = &volmax, * volminb = &volmin;
	__asm
	{
		// we store the max in xmm0 and the min in xmm1
		xorps xmm0, xmm0
		xorps xmm1, xmm1
		mov esi, [pSamples]
		mov eax, [numSamples]
		// Loop does: get the 4 max values and 4 min values in xmm0 and xmm1 respct.
		LOOPSTART:
		cmp eax, 0
			jle END
			maxps xmm0, [esi]
			minps xmm1, [esi]
			add esi, 10H
			sub eax, 4
			jmp LOOPSTART
			END :
		// to finish, get the max and of each of the four values.
		// put 02 and 03 to 20 and 21
		movhlps xmm2, xmm0
			// find max of 00 and 20 (02) and of 01 and 21 (03)
			maxps xmm0, xmm2
			// put 00 (result of max(00,02)) to 20
			movss xmm2, xmm0
			// put 01 (result of max(01,03)) into 00 (that's the only one we care about)
			shufps xmm0, xmm2, 11H
			// and find max of 00 (01) and 20 (00)
			maxps xmm0, xmm2

			movhlps xmm2, xmm1
			minps xmm1, xmm2
			movss xmm2, xmm1
			shufps xmm1, xmm2, 11H
			minps xmm1, xmm2

			mov edi, volmaxb
			movss[edi], xmm0
			mov edi, volminb
			movss[edi], xmm1
	}
	volmin *= -1.0f;
	//beware: -0.0f and 0.0f are supposed to be the same number, -0.0f > 0.0f returns false, just because 0.0f == 0.0f returns true
	return (volmin > volmax) ? volmin : volmax;
#else
	float vol = 0.0f;
	--pSamples;	
	do { /// not all waves are symmetrical
		const float nvol = (float) fabs(*++pSamples);
		if (nvol > vol) vol = nvol;
	} while (--numSamples);
	return vol;
#endif
}