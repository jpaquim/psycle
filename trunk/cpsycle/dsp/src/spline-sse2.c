// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "spline-sse2.h"

#include <math.h>

#if DIVERSALIS__CPU__X86__SSE >= 2 && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS

#include <xmmintrin.h>
#include <emmintrin.h>

extern float cubic_table[CUBIC_RESOLUTION * 4];
extern float l_table[CUBIC_RESOLUTION];

static psy_dsp_amp_t work(psy_dsp_SplineSEE2Resampler*,
	const int16_t* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length);
static psy_dsp_amp_t work_unchecked(psy_dsp_SplineSEE2Resampler*,
	const int16_t* data,
	uint32_t res);
static float work_float(psy_dsp_SplineSEE2Resampler*,
	const float* data,
	float offset,
	uint64_t length,
	float const* loopBeg,
	float const* loopEnd);
static float work_float_unchecked(psy_dsp_SplineSEE2Resampler*,
	const float* data, uint32_t res);

static resampler_vtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_dsp_SplineSEE2Resampler* self)
{
	if (!vtable_initialized) {
		vtable = *self->resampler.vtable;
		vtable.work = (psy_dsp_fp_resampler_work)work;
		vtable.work_unchecked = (psy_dsp_fp_resampler_work_unchecked)
			work_unchecked;
		vtable.work_float = (psy_dsp_fp_resampler_work_float)work_float;
		vtable.work_float_unchecked =
			(psy_dsp_fp_resampler_work_float_unchecked)work_float_unchecked;
		vtable_initialized = 1;
	}
}

void psy_dsp_spline_sse2_resampler_init(psy_dsp_SplineSEE2Resampler* self)
{
	psy_dsp_resampler_init(&self->resampler);
	vtable_init(self);
	self->resampler.vtable = &vtable;	
}

psy_dsp_amp_t work(psy_dsp_SplineSEE2Resampler* self,
	const int16_t* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length)
{		
	res >>= 32 - CUBIC_RESOLUTION_LOG;
	res <<= 2;//Since we have four floats in the table, the position is 16byte aligned.
	const short d0 = (offset == 0) ? 0 : *(data - 1);
	__m128i data128;
	if (offset < length - 2) { data128 = _mm_set_epi32(data[2], data[1], *data, d0); } else if (offset < length - 1) { data128 = _mm_set_epi32(0, data[1], *data, d0); } else { data128 = _mm_set_epi32(0, 0, *data, d0); }
	register __m128 y = _mm_cvtepi32_ps(data128);
	register __m128 result = _mm_mul_ps(y, _mm_load_ps(&cubic_table[res]));
	result = _mm_add_ps(result, _mm_movehl_ps(result, result));
	result = _mm_add_ss(result, _mm_shuffle_ps(result, result, 0x11));
	float newval;
	_mm_store_ss(&newval, result);
	return newval;
}

psy_dsp_amp_t work_unchecked(psy_dsp_SplineSEE2Resampler* self,
	const int16_t* data,	
	uint32_t res)
{
	res >>= 32 - CUBIC_RESOLUTION_LOG;
	res <<= 2;//Since we have four floats in the table, the position is 16byte aligned.
	register __m128 y = _mm_cvtepi32_ps(_mm_set_epi32(data[2], data[1], *data, *(data - 1)));
	register __m128 result = _mm_mul_ps(y, _mm_load_ps(&cubic_table[res]));
	result = _mm_add_ps(result, _mm_movehl_ps(result, result));
	result = _mm_add_ss(result, _mm_shuffle_ps(result, result, 0x11));
	float newval;
	_mm_store_ss(&newval, result);
	return newval;
}

float work_float(psy_dsp_SplineSEE2Resampler* self,
	const float* data,
	float offset,
	uint64_t length,
	const float* loopBeg,
	const float* loopEnd)
{
	const float foffset = (float)floor(offset);
	const unsigned int ioffset = (unsigned int)(foffset);
	uint32_t res = (uint32_t)((offset - foffset) * CUBIC_RESOLUTION);
	res <<= 2;//Since we have four floats in the table, the position is 16byte aligned.
	data += ioffset;
	const float d0 = (ioffset == 0) ? *loopEnd : *(data - 1);
	__m128 y;
	if (ioffset < length - 2) { y = _mm_set_ps(data[2], data[1], *data, d0); } else if (ioffset < length - 1) { y = _mm_set_ps(*loopBeg, data[1], *data, d0); } else { y = _mm_set_ps(*(loopBeg + 1), *loopBeg, *data, d0); }
	register __m128 result = _mm_mul_ps(y, _mm_load_ps(&cubic_table[res]));
	result = _mm_add_ps(result, _mm_movehl_ps(result, result));
	result = _mm_add_ss(result, _mm_shuffle_ps(result, result, 0x11));
	float newval;
	_mm_store_ss(&newval, result);
	return newval;
}

// todo not optimized!
float work_float_unchecked(psy_dsp_SplineSEE2Resampler* self,
	const float* data, uint32_t res)
{
	res >>= 32 - CUBIC_RESOLUTION_LOG;
	res <<= 2;//Since we have four floats in the table, the position is 16byte aligned.
	__m128 y = _mm_set_ps(data[2], data[1], *data, *(data - 1));
	register __m128 result = _mm_mul_ps(y, _mm_load_ps(&cubic_table[res]));
	result = _mm_add_ps(result, _mm_movehl_ps(result, result));
	result = _mm_add_ss(result, _mm_shuffle_ps(result, result, 0x11));
	float newval;
	_mm_store_ss(&newval, result);
	return newval;
}

#else
#include <string.h>

void psy_dsp_spline_sse2_resampler_init(psy_dsp_SplineSEE2Resampler* self)
{
	memset(self, 0, sizeof(psy_dsp_SplineSEE2Resampler));
}
#endif
