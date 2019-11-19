// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_OPERATIONS_H)
#define DSP_OPERATIONS_H

#include "dsptypes.h"
#include "../../detail/stdint.h"

typedef struct {
	void* (*memory_alloc)(size_t count, size_t size);
	void (*memory_dealloc)(void* address);
	void (*add)(amp_t *src, amp_t *dst, uintptr_t num, amp_t vol);
	void (*mul)(amp_t *dst, uintptr_t numSamples, amp_t mul);
	void (*movmul)(amp_t *src, amp_t *dst, uintptr_t num, amp_t mul);
	void (*clear)(amp_t *dst, uintptr_t num);
	void (*interleave)(amp_t* dst, amp_t* left, amp_t* right, uintptr_t num);
	void (*erase_all_nans_infinities_and_denormals)(amp_t* dst, uintptr_t num);
	float (*maxvol)(const float* src, uintptr_t num);
	void (*accumulate)(big_amp_t* accumleft, 
					big_amp_t* accumright, 
					const amp_t* __restrict pSamplesL,
					const amp_t* __restrict pSamplesR,
					int count);
} Dsp;

extern Dsp dsp;

void dsp_noopt_init(Dsp*);
void dsp_sse2_init(Dsp*);

#endif
