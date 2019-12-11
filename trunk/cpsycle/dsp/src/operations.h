// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_OPERATIONS_H)
#define DSP_OPERATIONS_H

#include "dsptypes.h"
#include "stddef.h"
#include "../../detail/stdint.h"

typedef struct {
	void* (*memory_alloc)(size_t count, size_t size);
	void (*memory_dealloc)(void* address);
	void (*add)(psy_dsp_amp_t *src, psy_dsp_amp_t *dst, uintptr_t num,
		psy_dsp_amp_t vol);
	void (*mul)(psy_dsp_amp_t *dst, uintptr_t numSamples, psy_dsp_amp_t mul);
	void (*movmul)(psy_dsp_amp_t *src, psy_dsp_amp_t *dst, uintptr_t num,
		psy_dsp_amp_t mul);
	void (*clear)(psy_dsp_amp_t *dst, uintptr_t num);
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
} psy_dsp_Operations;

extern psy_dsp_Operations dsp;

void psy_dsp_noopt_init(psy_dsp_Operations*);
void psy_dsp_sse2_init(psy_dsp_Operations*);

#endif
