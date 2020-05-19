// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_SPLINE_RESAMPLER_H
#define psy_dsp_SPLINE_RESAMPLER_H

#include "resampler.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_dsp_SplineResampler {
	psy_dsp_Resampler resampler;
} psy_dsp_SplineResampler;

void psy_dsp_splineresampler_init(psy_dsp_SplineResampler*);


INLINE psy_dsp_Resampler* psy_dsp_splineresampler_base(psy_dsp_SplineResampler* self)
{
	return &self->resampler;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_SPLINE_RESAMPLER_H */
