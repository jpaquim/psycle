// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_MULTIRESAMPLER_H
#define psy_dsp_MULTIRESAMPLER_H

#include "linear.h"

#ifdef PSYCLE_USE_SSE
#include "spline-sse2.h"
#include "sinc-sse2.h"
#else
#include "spline.h"
#include "sinc.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef enum ResamplerType {
	RESAMPLERTYPE_ZERO_ORDER,
	RESAMPLERTYPE_LINEAR,
	RESAMPLERTYPE_SPLINE,
	RESAMPLERTYPE_SINC,
	// RESAMPLERTYPE_SOXR,
	RESAMPLERTYPE_NUMRESAMPLERS
} ResamplerType;

typedef struct {	
	psy_dsp_Resampler resampler;
	psy_dsp_LinearResampler linear;
#ifdef PSYCLE_USE_SSE
	psy_dsp_SplineSEE2Resampler spline;	
	psy_dsp_SincSSE2Resampler sinc;
#else
	psy_dsp_SplineResampler spline;
	psy_dsp_SincResampler sinc;
#endif	
	ResamplerType selected;
	psy_dsp_Resampler* selectedresampler;	
} psy_dsp_MultiResampler;

void psy_dsp_multiresampler_init(psy_dsp_MultiResampler*);
void psy_dsp_multiresampler_settype(psy_dsp_MultiResampler*, ResamplerType type);
ResamplerType psy_dsp_multiresampler_type(psy_dsp_MultiResampler*);
const char* psy_dsp_multiresampler_name(ResamplerType);
uintptr_t psy_dsp_multiresampler_count(void);

INLINE psy_dsp_Resampler* psy_dsp_multiresampler_base(psy_dsp_MultiResampler* self)
{
	return &self->resampler;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_MULTIRESAMPLER_H */
