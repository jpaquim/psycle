// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_DSP_LINEAR_RESAMPLER_H)
#define PSY_DSP_LINEAR_RESAMPLER_H

#include "resampler.h"

typedef struct psy_dsp_LinearResampler {
	psy_dsp_Resampler resampler;
} psy_dsp_LinearResampler;

void psy_dsp_linearresampler_init(psy_dsp_LinearResampler*);

#endif
