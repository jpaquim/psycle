// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sinc-sse2.h"
#include "sinc.h"

#include "operations.h"

#include <assert.h>
#include <math.h>

#if DIVERSALIS__CPU__X86__SSE >= 2 && defined DIVERSALIS__COMPILER__MICROSOFT && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS

#include "sse_mathfun.h"
#include <xmmintrin.h>
#include <emmintrin.h>

typedef __declspec(align(16))
	union {
		float f[4];
		int i[4];
		v4sf  v;
	} V4SF;

extern float cubic_table[CUBIC_RESOLUTION * 4];

// either or both of resolution and zeros can be fine-tuned to find a tolerable compromise between quality and memory/cpu usage
// resolution lowers the aliasing noise above the filter point. Zeros make the filter cutoff sharper (and determine the amount of pre and post samples needed). 
/// sinc_sse2_ table values per zero crossing
#define SINC_RESOLUTION_LOG 14
/// sinc_sse2_ table zero crossings (per side) -- too low and it aliases, too high uses lots of cpu. Recommended to be even.
//  Note: If increasing this value, also adapt requiredPreSamples(), requiredPostSamples() ,
//  XMSampler::WaveDataController::RefillBuffer() and XMSampler::WaveDataController::PreWork()
#define SINC_ZEROS 16
/// IF SINC_ZEROS is power of two, define OPTIMIZED_RES_SHIFT to allow the algorithm to do shift instead of multiply
#define OPTIMIZED_RES_SHIFT 4
#define USE_SINC_DELTA SINC_RESOLUTION_LOG < 10
#define SINC_RESOLUTION (1 << SINC_RESOLUTION_LOG)
#define SINC_TABLESIZE (SINC_RESOLUTION * SINC_ZEROS)
#define windowtype 2
#define cutoffoffset 0.95

static float sinc_sse2_windowed_table[SINC_TABLESIZE];
#if USE_SINC_DELTA
static float sinc_sse2__delta_[SINC_TABLESIZE];
#endif
static float window_div_x[SINC_TABLESIZE];
static int sinc_sse2_tables_initialized = 0;

static void psy_dsp_sinc_sse2_resampler_initstatictables(void);
static float sinc_sse2_filtered(int16_t const* data, uint32_t res, int leftExtent, int rightExtent, sinc_data_t* resampler_data);
static float sinc_sse2_internal(int16_t const* data, uint32_t res, int leftExtent, int rightExtent);
static float sinc_sse2_float_filtered(float const* data, uint32_t res, int leftExtent, int rightExtent, sinc_data_t* resampler_data);
static float sinc_sse2_float_internal(float const* data, uint32_t res, int leftExtent, int rightExtent);


static void dispose(psy_dsp_SincSSE2Resampler*);
static psy_dsp_amp_t work(psy_dsp_SincSSE2Resampler*,
	int16_t const* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length);
static psy_dsp_amp_t work_unchecked(psy_dsp_SincSSE2Resampler*,
	const int16_t* data,
	uint32_t res);
static float work_float(psy_dsp_SincSSE2Resampler*,
	float const* data,
	float offset,
	uint64_t length,
	float const* loopBeg,
	float const* loopEnd);
static float work_float_unchecked(psy_dsp_SincSSE2Resampler*,
	float const* data, uint32_t res);

static void* getresamplerdata(psy_dsp_SincSSE2Resampler* self)
{
	return (sinc_data_t*) malloc(sizeof(sinc_data_t));
}

static void disposeresamplerdata(psy_dsp_SincSSE2Resampler* self, void* resampler_data)
{
	free(resampler_data);
}

static void setspeed(psy_dsp_SincSSE2Resampler* self, double speed)
{
	sinc_data_t* t = (sinc_data_t*)(self->resampler_data);
	if (speed > 1.0) {
		t->enabled = TRUE;
		t->fcpi = cutoffoffset * psy_dsp_PI / speed;
		t->fcpidivperiodsize = t->fcpi / SINC_RESOLUTION;
	} else {
		t->enabled = FALSE;
	}
}

static resampler_vtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_dsp_SincSSE2Resampler* self)
{
	if (!vtable_initialized) {
		vtable = *self->resampler.vtable;
		vtable.dispose = (psy_dsp_fp_resampler_dispose)dispose;
		vtable.setspeed =
			(psy_dsp_fp_resampler_setspeed)setspeed;		
		vtable.work = (psy_dsp_fp_resampler_work)work;
		vtable.work_unchecked = (psy_dsp_fp_resampler_work_unchecked)
			work_unchecked;
		vtable.work_float = (psy_dsp_fp_resampler_work_float)work_float;
		vtable.work_float_unchecked =
			(psy_dsp_fp_resampler_work_float_unchecked)work_float_unchecked;
		vtable_initialized = 1;
	}
}

void psy_dsp_sinc_sse2_resampler_init(psy_dsp_SincSSE2Resampler* self)
{
	psy_dsp_resampler_init(&self->resampler);
	vtable_init(self);
	self->resampler.vtable = &vtable;
	psy_dsp_sinc_sse2_resampler_initstatictables();
	self->resampler_data = getresamplerdata(self);
}

void psy_dsp_sinc_sse2_resampler_initstatictables(void)
{
	if (!sinc_sse2_tables_initialized) {
		int i;
		static const double ONEDIVSINC_TBSIZEMINONE = 1.0 / (double)(SINC_TABLESIZE);
		static const double ONEDIVSINC_RESOLUTION = 1.0 / (double)(SINC_RESOLUTION);
		// Windowed SincSSE2 Resampling {
		// http://en.wikipedia.org/wiki/Window_function
		// one-sided-- the function is symmetrical, one wing of the sinc_sse2_ will be sufficient.
		// we only apply half window (from pi..2pi instead of 0..2pi) because we only have half sinc_sse2_.
		// The functions are adapted to this fact!!

		sinc_sse2_windowed_table[0] = 1.f; // save the trouble of evaluating 0/0.
		window_div_x[0] = 1.f;
		for (i = 1; i < SINC_TABLESIZE; ++i) {
			double valx = psy_dsp_PI * (double)(i)*ONEDIVSINC_TBSIZEMINONE;
			double tempval;
			//Higher bandwidths means longer stopgap (bad), but also faster sidelobe attenuation (good).
			if (windowtype == 0) {
				// nuttal window. Bandwidth = 2.0212
				tempval = 0.355768 - 0.487396 * cos(psy_dsp_PI + valx) + 0.144232 * cos(2.0 * valx) - 0.012604 * cos(psy_dsp_PI + 3.0 * valx);
				// 0.3635819 , 0.4891775 , 0.1365995 , 0.0106411
			} else if (windowtype == 1) {
				// kaiser-bessel, alpha~7.5  . From OpenMPT, WindowedFIR
				tempval = 0.40243 - 0.49804 * cos(psy_dsp_PI + valx) + 0.09831 * cos(2.0 * valx) - 0.00122 * cos(psy_dsp_PI + 3.0 * valx);
			} else if (windowtype == 2) {
				// blackman window. Bandwidth = 1.73
				tempval = 0.42659 - 0.49656 * cos(psy_dsp_PI + valx) + 0.076849 * cos(2.0 * valx);
			} else if (windowtype == 3) {
				// C.H.Helmrich on Hydrogenaudio: http://www.hydrogenaud.io/forums/index.php?showtopic=105090
				tempval = 0.79445 * cos(0.5 * valx) + 0.20555 * cos(1.5 * valx);
			} else if (windowtype == 4) {
				// hann(ing) window. Bandwidth = 1.5
				tempval = 0.5 * (1.0 - cos(psy_dsp_PI + valx));
			} else if (windowtype == 5) {
				// hamming window. Bandwidth = 1.37
				tempval = 0.53836 - 0.46164 * cos(psy_dsp_PI + valx);
			} else if (windowtype == 6) {
				//lanczos (sinc_sse2_) window. Bandwidth = 1.30
				tempval = sin(valx) / valx;
			} else {
				//rectangular
				tempval = 1.0;
			}
			{
#if USE_SINC_DELTA
				int write_pos = i;
#else
				//Optimized version. Instead of putting the sinc_sse2_ linearly in the table, we allocate the zeroes of a specific offset consecutively.
				int idxzero = (i / SINC_RESOLUTION);
				int write_pos = (i % SINC_RESOLUTION) * SINC_ZEROS + idxzero;
#endif

				tempval /= (double)(i)*psy_dsp_PI * ONEDIVSINC_RESOLUTION;
				window_div_x[write_pos] = (float)tempval;
				//sinc_sse2_ runs at half speed of SINC_RESOLUTION (i.e. two zero crossing points per period).
				sinc_sse2_windowed_table[write_pos] = (float)(sin((double)(i)*cutoffoffset * psy_dsp_PI * ONEDIVSINC_RESOLUTION) * tempval);
			}
		}
		dsp.erase_all_nans_infinities_and_denormals(sinc_sse2_windowed_table, SINC_TABLESIZE);
#if USE_SINC_DELTA		
		for (i = 0; i < SINC_TABLESIZE - 1; ++i) {
			sinc_sse2__delta_[i] = sinc_sse2__windowed_table[i + 1] - sinc_sse2__windowed_table[i];
		}
		sinc_sse2__delta_[SINC_TABLESIZE - 1] = 0 - sinc_sse2__windowed_table[SINC_TABLESIZE - 1];
		dsp.erase_all_nans_infinities_and_denormals(sinc_sse2__delta_, SINC_TABLESIZE);		
#endif //USE_SINC_DELTA
		// }
		sinc_sse2_tables_initialized = 1;
	}
}

void dispose(psy_dsp_SincSSE2Resampler* self)
{
	disposeresamplerdata(self, self->resampler_data);
}

psy_dsp_amp_t work(psy_dsp_SincSSE2Resampler* self,
	int16_t const* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length)
{		
	const int leftExtent = (offset < SINC_ZEROS) ? offset + 1 : SINC_ZEROS;
	const int rightExtent = (length - offset - 1 < SINC_ZEROS) ? length - offset - 1 : SINC_ZEROS;
	sinc_data_t* t = (sinc_data_t*)(self->resampler_data);
	if (t->enabled) {
		return sinc_sse2_filtered(data, res, leftExtent, rightExtent, t);
	}
	return sinc_sse2_internal(data, res, leftExtent, rightExtent);
}

psy_dsp_amp_t work_unchecked(psy_dsp_SincSSE2Resampler* self,
	const int16_t* data,
	uint32_t res)
{
	sinc_data_t* t = (sinc_data_t*)(self->resampler_data);
	if (t->enabled) {
		return sinc_sse2_filtered(data, res, SINC_ZEROS, SINC_ZEROS, t);
	}
	return sinc_sse2_internal(data, res, SINC_ZEROS, SINC_ZEROS);
}

float work_float(psy_dsp_SincSSE2Resampler* self,
	float const* data,
	float offset,
	uint64_t length,
	float const* loopBeg,
	float const* loopEnd)
{
	sinc_data_t* t;
	//todo: maybe implement loopBeg and loopEnd
	const float foffset = (float)floor(offset);
	const int ioffset = (int)(foffset);
	uint32_t res = (uint32_t)((offset - foffset) * SINC_RESOLUTION);
	const int leftExtent = (ioffset < SINC_ZEROS) ? ioffset + 1 : SINC_ZEROS;
	const int rightExtent = (int)((length - ioffset - 1 < SINC_ZEROS) ? length - ioffset - 1 : SINC_ZEROS);
	data += ioffset;
	t = (sinc_data_t*)(self->resampler_data);
	if (t->enabled) {
		return sinc_sse2_float_filtered(data, res, leftExtent, rightExtent, t);
	}
	return sinc_sse2_float_internal(data, res, leftExtent, rightExtent);
}

float work_float_unchecked(psy_dsp_SincSSE2Resampler* self,
	float const* data, uint32_t res)
{
	sinc_data_t* t = (sinc_data_t*)(self->resampler_data);
	if (t->enabled) {
		return sinc_sse2_float_filtered(data, res, SINC_ZEROS, SINC_ZEROS, t);
	}
	return sinc_sse2_float_internal(data, res, SINC_ZEROS, SINC_ZEROS);
}


float sinc_sse2_filtered(int16_t const* data, uint32_t res, int leftExtent, int rightExtent, sinc_data_t* resampler_data)
{
	int i;
	int16_t const* pdata;
	float newval = 0.0f;
	float* ppretab;
	double w;
	double fcpi;

	res >>= (32 - SINC_RESOLUTION_LOG);
	//Avoid evaluating position zero. It would need special treatment on the sinc_sse2__table of future points.
	//On the filtered version, we just use position 1 instead. (sinc_sse2_e we're upsampling, the filter needs to happen anyway)
	if (res == 0) res++;

	pdata = data;

	//Current and Past points. They increase from 0 because they are leaving us.
	// (they would decrease from zero if it wasn't a mirrored sinc_sse2_)
#if defined OPTIMIZED_RES_SHIFT
	ppretab = &window_div_x[res << OPTIMIZED_RES_SHIFT];
#else
	ppretab = &window_div_x[res * SINC_ZEROS];
#endif
	w = res * resampler_data->fcpidivperiodsize;
	fcpi = resampler_data->fcpi;
	for (i =0; i < leftExtent; ++i, w += fcpi) {
		double sinc_sse2_ = sin(w) * *ppretab;
		newval += sinc_sse2_ * (*pdata);
		pdata--;
		ppretab++;
	}

	//Future points. They decrease from SINC_RESOLUTION sinc_sse2_e they are reaching the "Now" point.
#if defined OPTIMIZED_RES_SHIFT
	ppretab = &window_div_x[SINC_TABLESIZE - (res << OPTIMIZED_RES_SHIFT)];
#else
	ppretab = &window_div_x[SINC_TABLESIZE - (res * SINC_ZEROS)];
#endif
	w = (SINC_RESOLUTION - res) * resampler_data->fcpidivperiodsize;
	pdata = data + 1;
	for (i = 0; i < rightExtent; ++i, w += fcpi) {
		double sinc_sse2_ = sin(w) * *ppretab;
		newval += sinc_sse2_ * (*pdata);
		pdata++;
		ppretab++;
	}

	return newval;
}

float sinc_sse2_internal(int16_t const* data, uint32_t res, int leftExtent, int rightExtent)
{
	res >>= (32 - SINC_RESOLUTION_LOG);
#if defined OPTIMIZED_RES_SHIFT
	res <<= OPTIMIZED_RES_SHIFT;
#else
	res *= SINC_ZEROS;
#endif
	//Avoid evaluating position zero. It would need special treatment on the sinc_table of future points.
	//On the non-filtered version, we can just return the data
	if (res == 0) return *data;

	float newval = 0.0f;
	register __m128 result = _mm_setzero_ps();
	int16_t const* pdata = data;

	float* psinc = &sinc_sse2_windowed_table[res];
	while (leftExtent > 3) {
		register __m128i data128 = _mm_set_epi32(pdata[-3], pdata[-2], pdata[-1], pdata[0]);
		register __m128 datafloat = _mm_cvtepi32_ps(data128);
#if OPTIMIZED_RES_SHIFT > 1
		register __m128 sincfloat = _mm_load_ps(psinc);
#else
		register __m128 sincfloat = _mm_loadu_ps(psinc);
#endif
		result = _mm_add_ps(result, _mm_mul_ps(datafloat, sincfloat));
		pdata -= 4;
		psinc += 4;
		leftExtent -= 4;
	}
	while (leftExtent > 0) {
		newval += *psinc * *pdata;
		pdata--;
		psinc++;
		leftExtent--;
	}
	pdata = data + 1;
	psinc = &sinc_sse2_windowed_table[SINC_TABLESIZE - res];
	while (rightExtent > 3) {
		register __m128i data128 = _mm_set_epi32(pdata[3], pdata[2], pdata[1], pdata[0]);
		register __m128 datafloat = _mm_cvtepi32_ps(data128);
#if OPTIMIZED_RES_SHIFT > 1
		register __m128 sincfloat = _mm_load_ps(psinc);
#else
		register __m128 sincfloat = _mm_loadu_ps(psinc);
#endif
		result = _mm_add_ps(result, _mm_mul_ps(datafloat, sincfloat));
		pdata += 4;
		psinc += 4;
		rightExtent -= 4;
	}
	while (rightExtent > 0) {
		newval += *psinc * *pdata;
		pdata++;
		psinc++;
		rightExtent--;
	}
	result = _mm_add_ps(result, _mm_movehl_ps(result, result));
	result = _mm_add_ss(result, _mm_shuffle_ps(result, result, 0x11));
	float newval2;
	_mm_store_ss(&newval2, result);
	return newval + newval2;
}

//TODO: Optimize
float sinc_sse2_float_filtered(float const* data, uint32_t res, int leftExtent, int rightExtent, sinc_data_t* resampler_data)
{
	res >>= (32 - SINC_RESOLUTION_LOG);
	//Avoid evaluating position zero. It would need special treatment on the sinc_table of future points.
	//On the filtered version, we just use position 1 instead. (since we're upsampling, the filter needs to happen anyway)
	if (res == 0) res++;

	float const* pdata = data;
	float newval = 0.0f;

	//Current and Past points. They increase from 0 because they are leaving us.
	// (they would decrease from zero if it wasn't a mirrored sinc)
#if defined OPTIMIZED_RES_SHIFT
	float* ppretab = &window_div_x[res << OPTIMIZED_RES_SHIFT];
#else
	float* ppretab = &window_div_x[res * SINC_ZEROS];
#endif
	double w = res * resampler_data->fcpidivperiodsize;
	const double fcpi = resampler_data->fcpi;
	for (int i = 0; i < leftExtent; ++i, w += fcpi) {
		double sinc = sin(w) * *ppretab;
		newval += sinc * *pdata;
		pdata--;
		ppretab++;
	}

	//Future points. They decrease from SINC_RESOLUTION since they are reaching the "Now" point.
#if defined OPTIMIZED_RES_SHIFT
	ppretab = &window_div_x[SINC_TABLESIZE - (res << OPTIMIZED_RES_SHIFT)];
#else
	ppretab = &window_div_x[SINC_TABLESIZE - (res * SINC_ZEROS)];
#endif
	w = (SINC_RESOLUTION - res) * resampler_data->fcpidivperiodsize;
	pdata = data + 1;
	for (int i = 0; i < rightExtent; ++i, w += fcpi) {
		double sinc = sin(w) * *ppretab;
		newval += sinc * *pdata;
		pdata++;
		ppretab++;
	}

	return newval;
}

float sinc_sse2_float_internal(float const* data, uint32_t res, int leftExtent, int rightExtent)
{
	res >>= (32 - SINC_RESOLUTION_LOG);
#if defined OPTIMIZED_RES_SHIFT
	res <<= OPTIMIZED_RES_SHIFT;
#else
	res *= SINC_ZEROS;
#endif

	//Avoid evaluating position zero. It would need special treatment on the sinc_table of future points.
	//On the non-filtered version, we can just return the data
	if (res == 0) return *data;
		
	float newval = 0.0f;
	register __m128 result = _mm_setzero_ps();
	float const* pdata = data;
	float* psinc = &sinc_sse2_windowed_table[res];
	while (leftExtent > 3) {
		register __m128 datafloat = _mm_set_ps(pdata[-3], pdata[-2], pdata[-1], pdata[0]);
#if OPTIMIZED_RES_SHIFT > 1
		register __m128 sincfloat = _mm_load_ps(psinc);
#else
		register __m128 sincfloat = _mm_loadu_ps(psinc);
#endif
		result = _mm_add_ps(result, _mm_mul_ps(datafloat, sincfloat));
		pdata -= 4;
		psinc += 4;		
		leftExtent -= 4;
	}
	while (leftExtent > 0) {
		newval += *psinc * *pdata;
		pdata--;
		psinc++;
		leftExtent--;
	}
	pdata = data + 1;
	psinc = &sinc_sse2_windowed_table[SINC_TABLESIZE - res];
	while (rightExtent > 3) {		
		register __m128 datafloat = _mm_loadu_ps(pdata);
#if OPTIMIZED_RES_SHIFT > 1
		register __m128 sincfloat = _mm_load_ps(psinc);
#else
		register __m128 sincfloat = _mm_loadu_ps(psinc);
#endif
		result = _mm_add_ps(result, _mm_mul_ps(datafloat, sincfloat));
		pdata += 4;
		psinc += 4;
		rightExtent -= 4;
}
	while (rightExtent > 0) {
		newval += *psinc * *pdata;
		pdata++;
		psinc++;
		rightExtent--;
	}
	result = _mm_add_ps(result, _mm_movehl_ps(result, result));
	result = _mm_add_ss(result, _mm_shuffle_ps(result, result, 0x11));
	float newval2;
	_mm_store_ss(&newval2, result);
	return newval + newval2;	
}

#else
#include <string.h>

// void psy_dsp_sinc_sse2_resampler_init(psy_dsp_SincSSE2Resampler* self)
// {
//	memset(self, 0, sizeof(psy_dsp_SincSEE2Resampler));
// }
#endif
