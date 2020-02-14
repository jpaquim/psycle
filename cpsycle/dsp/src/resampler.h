// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_RESAMPLER_H
#define psy_dsp_RESAMPLER_H

#include "dsptypes.h"

#include "../../detail/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CUBIC_RESOLUTION_LOG 10
#define CUBIC_RESOLUTION 2048 // 1 << CUBIC_RESOLUTION_LOG

struct psy_dsp_Resampler;

typedef void(*psy_dsp_fp_resampler_setspeed)(struct psy_dsp_Resampler*,
	int64_t speed);
typedef psy_dsp_amp_t (*psy_dsp_fp_resampler_work)(struct psy_dsp_Resampler*,
	float const * data,
	uint64_t offset,
	uint32_t res,
	uint64_t length,
	void* /*resampler_data*/ );

typedef struct resampler_vtable {
	psy_dsp_fp_resampler_work work;
	psy_dsp_fp_resampler_setspeed setspeed;
} resampler_vtable;

typedef struct psy_dsp_Resampler {
	struct resampler_vtable* vtable;
} psy_dsp_Resampler;

void psy_dsp_resampler_init(psy_dsp_Resampler*);

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_RESAMPLER_H */
