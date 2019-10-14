// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "lowpass12e.h"
#include <math.h>

static FilterMap lowpass12e_coeffmap;
static lowpass12e_coeffmap_initialized;

static void lowpass12e_computecoeffs(LowPass12E* self, int freq, int r, FilterCoeff* coeff);
static void lowpass12e_update(LowPass12E*, int full);
static float lowpass12e_work(LowPass12E*, float sample);
static void lowpass12e_reset(LowPass12E*);

void lowpass12e_init(LowPass12E* self)
{
	customfilter_init(&self->customfilter);	
	self->customfilter.filter.update = lowpass12e_update;
	self->customfilter.filter.work = lowpass12e_work;
	self->customfilter.filter.reset = lowpass12e_reset;
	if (!lowpass12e_coeffmap_initialized) {
		lowpass12e_update(self, 1);
		lowpass12e_coeffmap_initialized = 1;
	} else {
		lowpass12e_update(self, 0);
	}
	firwork_init(&self->firwork);
}

void lowpass12e_update(LowPass12E* self, int full)
{	
	if (full) {
		filtermap_compute(&lowpass12e_coeffmap, self, (FilterMapCallback)
			lowpass12e_computecoeffs);
	}
	filtercoeff_update(&self->coeff, &lowpass12e_coeffmap,
		(int)(((Filter*)self)->cutoff(self) * 127),
		(int) ((Filter*)self)->ressonance(self) * 127);
}

void lowpass12e_computecoeffs(LowPass12E* self, int freq, int r, FilterCoeff* coeff)
{	
	double frequency;
	double samplerate_d;	
	double omega;
	float sn;
	float cs;
	float alpha;
	float a0, a1, a2, b0, b1, b2;		

	frequency = cutoffinternalext(freq);
	samplerate_d = (double)(((Filter*)self)->samplerate(self));
	if (frequency * 2.0 > samplerate_d) { 
		frequency = samplerate_d * 0.5;
	}
	omega = (TPI*frequency) / ((Filter*)self)->samplerate(self);	
	sn = (float)sin(omega);
	cs = (float)cos(omega);
	alpha = (float)(sn * 0.5f / 
		resonanceinternal(r*(freq + 70) / (127.0f + 70.f)));

	b0 =  (1.f - cs)/2.f;
	b1 =   1.f - cs;
	b2 =  (1.f - cs)/2.f;
	a0 =   1.f + alpha;
	a1 =  -2.f * cs;
	a2 =   1.f - alpha;

	filtercoeff_setparameter(coeff, a0, a1, a2, b0, b1, b2);
}

float lowpass12e_work(LowPass12E* self, float sample)
{
	return firwork_work(&self->firwork, &self->coeff, sample);
}

void lowpass12e_reset(LowPass12E* self)
{
	firwork_reset(&self->firwork);
}
