// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_DSP_MULTIRESAMPLER_H)
#define PSY_DSP_MULTIRESAMPLER_H

#include "linear.h"

typedef enum ResamplerType {
	RESAMPLERTYPE_ZERO_ORDER,
	RESAMPLERTYPE_LINEAR,
	RESAMPLERTYPE_SPLINE,
	RESAMPLERTYPE_SINC,
	RESAMPLERTYPE_SOXR,
	RESAMPLERTYPE_NUMRESAMPLERS
} ResamplerType;

typedef struct {	
	psy_dsp_Resampler resampler;
	psy_dsp_LinearResampler linear;
	ResamplerType selected;
	psy_dsp_Resampler* selectedresampler;	
} psy_dsp_MultiResampler;

void psy_dsp_multiresampler_init(psy_dsp_MultiResampler*);
void psy_dsp_multiresampler_settype(psy_dsp_MultiResampler*, ResamplerType type);
ResamplerType psy_dsp_multiresampler_type(psy_dsp_MultiResampler*);
const char* psy_dsp_multiresampler_name(ResamplerType);
uintptr_t psy_dsp_multiresampler_count(void);

#endif
