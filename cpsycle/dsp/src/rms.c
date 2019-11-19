// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "rms.h"
#include "operations.h"
#include <math.h>
#include <stdlib.h>

static int numRMSSamples = 1;

// RMSData

void rmsdata_init(RMSData* self)
{
	self->AccumLeft = 0;
	self->AccumRight = 0;
	self->count = 0;
	self->previousLeft = 0;
	self->previousRight = 0;
}

void rmsdata_accumulate(RMSData* self, const amp_t* __restrict pSamplesL,
	const amp_t* __restrict pSamplesR, int count)
{	
	dsp.accumulate(&self->AccumLeft, &self->AccumRight, pSamplesL, pSamplesR, count);
}

// RMSVol

void rmsvol_init(RMSVol* self)
{
	rmsdata_init(&self->data);
	self->volume = 0.f;
}

RMSVol* rmsvol_alloc(void)
{
	return (RMSVol*) malloc(sizeof(RMSVol));
}

RMSVol* rmsvol_allocinit(void)
{
	RMSVol* rv;

	rv = rmsvol_alloc();
	if (rv) {
		rmsvol_init(rv);
	}
	return rv;
}

/// Note: Values are accumulated since the standard calculation requires 50ms of data.
void rmsvol_tick(RMSVol* self, const amp_t * __restrict pSamplesL,
	const amp_t * __restrict pSamplesR, int numSamples)
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
			ns -= count&0x3;
			count = count&~0x3;
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

amp_t rmsvol_value(RMSVol* self)
{
	return self->volume;
}

void rmsvol_setsamplerate(unsigned int samplerate)
{
	/// standard calculation requires 50ms of data.
	numRMSSamples = (int) (samplerate * 0.05f);
}
