// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_OPERATIONS_H)
#define DSP_OPERATIONS_H

void dsp_add(float *src, float *dst, int num, float vol);
void dsp_mul(float *dst, int numSamples, float mul);
void dsp_movmul(float *src, float *dst, int num, float mul);		
void dsp_clear(float *dst, int num);

#endif