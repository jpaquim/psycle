// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "rms.h"
#include "operations.h"
#include <math.h>
#include <stdlib.h>

static int numRMSSamples = 1;

#if defined SSE
#define is_aligned(POINTER, BYTE_COUNT) \
	(((uintptr_t)(const void*)(POINTER)) % (BYTE_COUNT) == 0)
#endif

// psy_dsp_RMSData

void rmsdata_init(psy_dsp_RMSData* self)
{
	self->AccumLeft = 0;
	self->AccumRight = 0;
	self->count = 0;
	self->previousLeft = 0;
	self->previousRight = 0;
}

void rmsdata_accumulate(psy_dsp_RMSData* self,
	const psy_dsp_amp_t* __restrict pSamplesL,
	const psy_dsp_amp_t* __restrict pSamplesR,
	int count)
{	
	dsp.accumulate(&self->AccumLeft, &self->AccumRight, pSamplesL, pSamplesR,
		count);
}

// psy_dsp_RMSVol

void psy_dsp_rmsvol_init(psy_dsp_RMSVol* self)
{
	rmsdata_init(&self->data);
	self->volume = 0.f;
}

psy_dsp_RMSVol* psy_dsp_rmsvol_alloc(void)
{
	return (psy_dsp_RMSVol*) malloc(sizeof(psy_dsp_RMSVol));
}

psy_dsp_RMSVol* psy_dsp_rmsvol_allocinit(void)
{
	psy_dsp_RMSVol* rv;

	rv = psy_dsp_rmsvol_alloc();
	if (rv) {
		psy_dsp_rmsvol_init(rv);
	}
	return rv;
}

/// Note: Values are accumulated since the standard calculation requires 50ms of data.
void psy_dsp_rmsvol_tick(psy_dsp_RMSVol* self, const psy_dsp_amp_t * __restrict pSamplesL,
	const psy_dsp_amp_t * __restrict pSamplesR, int numSamples)
{
	const float * pL = pSamplesL;
	const float * pR = pSamplesR;

	int ns = numSamples;
	int count = numRMSSamples - self->data.count;
	if (ns >= count) {
		// count can be negative when changing the samplerate.
		if (count >= 0) {
			rmsdata_accumulate(&self->data, pSamplesL, pSamplesR, count);				
#if defined SSE
			//small workaround for 16byte boundary (it makes it slightly incorrect, but hopefully just a bit).
			if (!is_aligned(pSamplesL, 16)) {
				ns -= count & 0x3;
				count = count & ~0x3;
			}
#endif
			ns -= count;
			pL+=count; pR+=count;
		}
		self->data.previousLeft  = (float) sqrt(self->data.AccumLeft  / numRMSSamples);
		self->data.previousRight = (float) sqrt(self->data.AccumRight / numRMSSamples);
		self->data.AccumLeft = 0;
		self->data.AccumRight = 0;
		self->data.count = 0;		
	}
	self->data.count += ns;
	rmsdata_accumulate(&self->data, pL, pR, ns);
	self->volume = self->data.previousLeft > self->data.previousRight
			? self->data.previousLeft
			: self->data.previousRight;
}

psy_dsp_amp_t psy_dsp_rmsvol_value(psy_dsp_RMSVol* self)
{
	return self->volume;
}

void psy_dsp_rmsvol_setsamplerate(unsigned int samplerate)
{
	/// standard calculation requires 50ms of data.
	numRMSSamples = (int) (samplerate * 0.05f);
}
