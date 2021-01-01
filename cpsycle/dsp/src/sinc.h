// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_SINC_RESAMPLER_H
#define psy_dsp_SINC_RESAMPLER_H

#include "resampler.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sinc_data_t {
	bool enabled;
	double fcpi;
	double fcpidivperiodsize;
} sinc_data_t;

typedef struct psy_dsp_SincResampler {
	psy_dsp_Resampler resampler;
	void* resampler_data;
} psy_dsp_SincResampler;

void psy_dsp_sinc_resampler_init(psy_dsp_SincResampler*);


INLINE psy_dsp_Resampler* psy_dsp_sinc_resampler_base(psy_dsp_SincResampler* self)
{
	return &self->resampler;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_SINC_RESAMPLER_H */
