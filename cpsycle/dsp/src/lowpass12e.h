// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_LOWPASS12E_H)
#define DSP_LOWPASS12E_H

#include "filter.h"

typedef struct {
	psy_dsp_CustomFilter customfilter;	
	psy_dsp_FilterCoeff coeff;
	psy_dsp_FIRWork firwork;
} psy_dsp_LowPass12E;

void psy_dsp_lowpass12e_init(psy_dsp_LowPass12E*);

#endif
