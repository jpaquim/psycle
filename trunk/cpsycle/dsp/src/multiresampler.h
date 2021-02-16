// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_MULTIRESAMPLER_H
#define psy_dsp_MULTIRESAMPLER_H

#include "linear.h"

#ifdef PSYCLE_USE_SSE
#include "spline-sse2.h"
#include "sinc-sse2.h"
#endif
#include "spline.h"
#include "sinc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum psy_dsp_ResamplerQuality {
	psy_dsp_RESAMPLERQUALITY_ZERO_ORDER,
	psy_dsp_RESAMPLERQUALITY_LINEAR,
	psy_dsp_RESAMPLERQUALITY_SPLINE,
	psy_dsp_RESAMPLERQUALITY_SINC,
	// psy_dsp_RESAMPLERQUALITY_SOXR,
	psy_dsp_RESAMPLERQUALITY_NUMRESAMPLERS
} psy_dsp_ResamplerQuality;

typedef struct psy_dsp_MultiResampler {
	psy_dsp_Resampler resampler;
	psy_dsp_LinearResampler linear;
#ifdef PSYCLE_USE_SSE
	psy_dsp_SplineSEE2Resampler spline_sse2;	
	psy_dsp_SincSSE2Resampler sinc_sse2;
#endif
	psy_dsp_SplineResampler spline;
	psy_dsp_SincResampler sinc;	
	psy_dsp_ResamplerQuality selected;
	psy_dsp_Resampler* selectedresampler;
	bool use_sse2_if_available;
} psy_dsp_MultiResampler;

void psy_dsp_multiresampler_init(psy_dsp_MultiResampler*, psy_dsp_ResamplerQuality);
void psy_dsp_multiresampler_setquality(psy_dsp_MultiResampler*, psy_dsp_ResamplerQuality);
psy_dsp_ResamplerQuality psy_dsp_multiresampler_quality(psy_dsp_MultiResampler*);
const char* psy_dsp_multiresampler_name(psy_dsp_ResamplerQuality);
uintptr_t psy_dsp_multiresampler_count(void);

INLINE psy_dsp_Resampler* psy_dsp_multiresampler_base(psy_dsp_MultiResampler* self)
{
	return &self->resampler;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_MULTIRESAMPLER_H */
