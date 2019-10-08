// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(RMS_H)
#define RMS_H

typedef struct {
	int count;
	double AccumLeft, AccumRight;
	float previousLeft, previousRight;
} RMSData;

void rmsdata_init(RMSData*);
void rmsdata_accumulate(RMSData*, const float* __restrict pSamplesL,
	const float* __restrict pSamplesR, int count);

typedef struct RMSVol {
	RMSData data;
	float volume;
} RMSVol;

void rmsvol_init(RMSVol*);
/// Note: Values are accumulated since the standard calculation requires 50ms
/// of data.
void rmsvol_tick(RMSVol*, const float* __restrict pSamplesL,
	const float* __restrict pSamplesR, int numSamples);
float rmsvol_value(RMSVol*);
void rmsvol_setsamplerate(unsigned int samplerate);

#endif
