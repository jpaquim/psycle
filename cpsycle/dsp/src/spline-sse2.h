// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_SPLINE_SSE2_RESAMPLER_H
#define psy_dsp_SPLINE_SSE2_RESAMPLER_H

#include "resampler.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_dsp_SplineSSE2Resampler {
	psy_dsp_Resampler resampler;
} psy_dsp_SplineSEE2Resampler;

void psy_dsp_spline_sse2_resampler_init(psy_dsp_SplineSEE2Resampler*);


INLINE psy_dsp_Resampler* psy_dsp_spline_sse2_resampler_base(psy_dsp_SplineSEE2Resampler* self)
{
	return &self->resampler;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_SPLINE_SSE2_RESAMPLER_H */
