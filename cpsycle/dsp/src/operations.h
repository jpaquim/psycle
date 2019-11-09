// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_OPERATIONS_H)
#define DSP_OPERATIONS_H

#include "dsptypes.h"
#include "../../detail/stdint.h"

void dsp_add(amp_t *src, amp_t *dst, uintptr_t num, amp_t vol);
void dsp_mul(amp_t *dst, uintptr_t numSamples, amp_t mul);
void dsp_movmul(amp_t *src, amp_t *dst, uintptr_t num, amp_t mul);
void dsp_clear(amp_t *dst, uintptr_t num);
void dsp_interleave(amp_t* dst, amp_t* left, amp_t* right, uintptr_t num);
void dsp_erase_all_nans_infinities_and_denormals(amp_t* dst, uintptr_t num);

#endif