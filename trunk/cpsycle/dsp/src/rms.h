// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(RMS_H)
#define RMS_H

#include "dsptypes.h"

typedef struct {
	int count;
	psy_dsp_big_amp_t AccumLeft, AccumRight;
	psy_dsp_amp_t previousLeft, previousRight;
} RMSData;

void rmsdata_init(RMSData*);
void rmsdata_accumulate(RMSData*, const psy_dsp_amp_t* __restrict pSamplesL,
	const psy_dsp_amp_t* __restrict pSamplesR, int count);

typedef struct RMSVol {
	RMSData data;
	psy_dsp_amp_t volume;
} RMSVol;

void rmsvol_init(RMSVol*);
RMSVol* rmsvol_alloc(void);
RMSVol* rmsvol_allocinit(void);
/// Note: Values are accumulated since the standard calculation requires 50ms
/// of data.
void rmsvol_tick(RMSVol*, const psy_dsp_amp_t* __restrict pSamplesL,
	const psy_dsp_amp_t* __restrict pSamplesR, int numSamples);
psy_dsp_amp_t rmsvol_value(RMSVol*);
void rmsvol_setsamplerate(unsigned int samplerate);

#endif
