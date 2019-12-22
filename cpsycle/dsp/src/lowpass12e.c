// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "lowpass12e.h"
#include <math.h>

static psy_dsp_FilterMap lowpass12e_coeffmap;
static lowpass12e_coeffmap_initialized;

static void lowpass12e_computecoeffs(psy_dsp_LowPass12E* self, int freq, int r, psy_dsp_FilterCoeff* coeff);
static void lowpass12e_update(psy_dsp_LowPass12E*, int full);
static psy_dsp_amp_t lowpass12e_work(psy_dsp_LowPass12E*, psy_dsp_amp_t sample);
static void lowpass12e_reset(psy_dsp_LowPass12E*);

static filter_vtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_dsp_CustomFilter* customfilter)
{
	if (!vtable_initialized) {
		vtable = *customfilter->filter.vtable;
		vtable.update = (psy_dsp_fp_filter_update) lowpass12e_update;
		vtable.work = (psy_dsp_fp_filter_work) lowpass12e_work;
		vtable.reset = (psy_dsp_fp_filter_reset) lowpass12e_reset;
		vtable_initialized = 1;
	}	
}

void psy_dsp_lowpass12e_init(psy_dsp_LowPass12E* self)
{
	psy_dsp_customfilter_init(&self->customfilter);	
	vtable_init(&self->customfilter);
	self->customfilter.filter.vtable = &vtable;

	if (!lowpass12e_coeffmap_initialized) {
		lowpass12e_update(self, 1);
		lowpass12e_coeffmap_initialized = 1;
	} else {
		lowpass12e_update(self, 0);
	}
	psy_dsp_firwork_init(&self->firwork);
}

void lowpass12e_update(psy_dsp_LowPass12E* self, int full)
{	
	if (full) {
		psy_dsp_filtermap_compute(&lowpass12e_coeffmap, self, (psy_dsp_FilterMapCallback)
			lowpass12e_computecoeffs);
	}
	psy_dsp_filtercoeff_update(&self->coeff, &lowpass12e_coeffmap,
		(int)(((psy_dsp_Filter*)self)->vtable->cutoff(&self->customfilter.filter) * 127),
		(int) ((psy_dsp_Filter*)self)->vtable->ressonance(&self->customfilter.filter) * 127);
}

void lowpass12e_computecoeffs(psy_dsp_LowPass12E* self, int freq, int r, psy_dsp_FilterCoeff* coeff)
{	
	double frequency;
	double samplerate_d;	
	double omega;
	float sn;
	float cs;
	float alpha;
	float a0, a1, a2, b0, b1, b2;		

	frequency = psy_dsp_cutoffinternalext(freq);
	samplerate_d = (double)(((psy_dsp_Filter*)self)->vtable->samplerate(
		&self->customfilter.filter));
	if (frequency * 2.0 > samplerate_d) { 
		frequency = samplerate_d * 0.5;
	}
	omega = (TPI*frequency) / ((psy_dsp_Filter*)self)->vtable->samplerate(
		&self->customfilter.filter);	
	sn = (float)sin(omega);
	cs = (float)cos(omega);
	alpha = (float)(sn * 0.5f / 
		psy_dsp_resonanceinternal(r*(freq + 70) / (127.0f + 70.f)));

	b0 =  (1.f - cs)/2.f;
	b1 =   1.f - cs;
	b2 =  (1.f - cs)/2.f;
	a0 =   1.f + alpha;
	a1 =  -2.f * cs;
	a2 =   1.f - alpha;

	psy_dsp_filtercoeff_setparameter(coeff, a0, a1, a2, b0, b1, b2);
}

psy_dsp_amp_t lowpass12e_work(psy_dsp_LowPass12E* self, psy_dsp_amp_t sample)
{
	return psy_dsp_firwork_work(&self->firwork, &self->coeff, sample);
}

void lowpass12e_reset(psy_dsp_LowPass12E* self)
{
	psy_dsp_firwork_reset(&self->firwork);
}
