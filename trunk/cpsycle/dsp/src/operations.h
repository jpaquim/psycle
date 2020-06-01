// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_OPERATIONS_H
#define psy_dsp_OPERATIONS_H

#include "../../detail/psydef.h"

#include "dsptypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void* (*memory_alloc)(size_t count, size_t size);
	void (*memory_dealloc)(void* address);
	void (*add)(psy_dsp_amp_t *src, psy_dsp_amp_t *dst, uintptr_t num,
		psy_dsp_amp_t vol);
	void (*mul)(psy_dsp_amp_t *dst, uintptr_t numSamples, psy_dsp_amp_t mul);
	void (*movmul)(psy_dsp_amp_t *src, psy_dsp_amp_t *dst, uintptr_t num,
		psy_dsp_amp_t mul);
	void (*clear)(psy_dsp_amp_t *dst, uintptr_t num);
	psy_dsp_amp_t* (*crop)(psy_dsp_amp_t* src, uintptr_t offset, uintptr_t num);	
	void (*interleave)(psy_dsp_amp_t* dst, psy_dsp_amp_t* left,
		psy_dsp_amp_t* right, uintptr_t num);
	void (*erase_all_nans_infinities_and_denormals)(psy_dsp_amp_t* dst,
		uintptr_t num);
	float (*maxvol)(const float* src, uintptr_t num);
	void (*accumulate)(psy_dsp_big_amp_t* accumleft, 
		psy_dsp_big_amp_t* accumright, 
		const psy_dsp_amp_t* __restrict pSamplesL,
		const psy_dsp_amp_t* __restrict pSamplesR,
		int count);
	void (*reverse)(psy_dsp_amp_t* dst, uintptr_t num);
} psy_dsp_Operations;

extern psy_dsp_Operations dsp;

void psy_dsp_noopt_init(psy_dsp_Operations*);
void psy_dsp_sse2_init(psy_dsp_Operations*);

// (Fideloop's)
INLINE void psy_dsp_normalize(psy_dsp_Operations* self, psy_dsp_amp_t* dst,
	uintptr_t numframes)
{
	if (numframes > 0) {
		psy_dsp_amp_t maxL = 0;
		double ratio = 0;

		maxL = self->maxvol(dst, numframes);
		if (maxL > 0.0) {
			ratio = (double)32767 / maxL;
		}
		if (ratio != 1) {
			self->mul(dst, numframes, (psy_dsp_amp_t)ratio);
		}
	}
}

INLINE void psy_dsp_fade(psy_dsp_Operations* self, psy_dsp_amp_t* dst,
	uintptr_t numframes, psy_dsp_amp_t startvol, psy_dsp_amp_t endvol)
{
	uintptr_t j;
	double slope;

	slope = ((double)endvol - (double)startvol) / (double)(numframes);
	for (j = 0; j < numframes; ++j) {
		dst[j] *= (psy_dsp_amp_t)(startvol + j * slope);
	}
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_OPERATIONS_H */
