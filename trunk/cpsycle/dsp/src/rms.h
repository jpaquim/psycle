// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_RMS_H
#define psy_dsp_RMS_H

#include "dsptypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int count;
	psy_dsp_big_amp_t AccumLeft, AccumRight;
	psy_dsp_amp_t previousLeft, previousRight;
} psy_dsp_RMSData;

void rmsdata_init(psy_dsp_RMSData*);
void rmsdata_accumulate(psy_dsp_RMSData*, const psy_dsp_amp_t* __restrict pSamplesL,
	const psy_dsp_amp_t* __restrict pSamplesR, int count);

typedef struct psy_dsp_RMSVol {
	psy_dsp_RMSData data;
	psy_dsp_amp_t volume;
} psy_dsp_RMSVol;

void psy_dsp_rmsvol_init(psy_dsp_RMSVol*);
psy_dsp_RMSVol* psy_dsp_rmsvol_alloc(void);
psy_dsp_RMSVol* psy_dsp_rmsvol_allocinit(void);
/// Note: Values are accumulated since the standard calculation requires 50ms
/// of data.
void psy_dsp_rmsvol_tick(psy_dsp_RMSVol*, const psy_dsp_amp_t* __restrict pSamplesL,
	const psy_dsp_amp_t* __restrict pSamplesR, int numSamples);
psy_dsp_amp_t psy_dsp_rmsvol_value(psy_dsp_RMSVol*);
void psy_dsp_rmsvol_setsamplerate(unsigned int samplerate);

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_RMS_H */
