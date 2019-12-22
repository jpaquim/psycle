// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "filter.h"

// psy_dsp_Filter Interface

static void init(psy_dsp_Filter* self) { }
static void dispose(psy_dsp_Filter* self) { }
static psy_dsp_amp_t work(psy_dsp_Filter* self, psy_dsp_amp_t sample) { 
	return sample; }
static void setcutoff(psy_dsp_Filter* self, float cutoff) { }
static float cutoff(psy_dsp_Filter* self) { return 0.f; }
static void setressonance(psy_dsp_Filter* self, float ressonance) { }
static float ressonance(psy_dsp_Filter* self) { return 0.f; }
static void setsamplerate(psy_dsp_Filter* self, float samplerate) { }
static float samplerate(psy_dsp_Filter* self) { return 44100.f; }
static void update(psy_dsp_Filter* self, int full) { }
static void reset(psy_dsp_Filter* self) { }

static filter_vtable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.init = init;
		vtable.dispose = dispose;
		vtable.work = work;
		vtable.setsamplerate = setsamplerate;
		vtable.samplerate = samplerate;
		vtable.setcutoff = setcutoff;
		vtable.cutoff = cutoff;
		vtable.setressonance = setressonance;
		vtable.ressonance = ressonance;
		vtable.reset = reset;
		vtable_initialized = 1;
	}	
}

void psy_dsp_filter_init(psy_dsp_Filter* self)		
{
	vtable_init();
	self->vtable = &vtable;
}

// psy_dsp_CustomFilter

static void customfilter_setcutoff(psy_dsp_CustomFilter*, float cutoff);
static float customfilter_cutoff(psy_dsp_CustomFilter*);
static void customfilter_setressonance(psy_dsp_CustomFilter*, float ressonance);
static float customfilter_ressonance(psy_dsp_CustomFilter*);
static void customfilter_setsamplerate(psy_dsp_CustomFilter*, float samplerate);
static float customfilter_samplerate(psy_dsp_CustomFilter*);

static filter_vtable customfilter_vtable;
static int customfilter_vtable_initialized = 0;

void customfilter_vtable_init(psy_dsp_Filter* filter)
{
	if (!customfilter_vtable_initialized) {
		customfilter_vtable = *filter->vtable;
		customfilter_vtable.setsamplerate = (psy_dsp_fp_filter_setsamplerate) 
			customfilter_setsamplerate;
		customfilter_vtable.samplerate = (psy_dsp_fp_filter_samplerate) 
			customfilter_samplerate;
		customfilter_vtable.setcutoff = (psy_dsp_fp_filter_setcutoff)
			customfilter_setcutoff;
		customfilter_vtable.cutoff = (psy_dsp_fp_filter_cutoff)
			customfilter_cutoff;
		customfilter_vtable.setressonance = (psy_dsp_fp_filter_setressonance)
			customfilter_setressonance;
		customfilter_vtable.ressonance = (psy_dsp_fp_filter_ressonance)
			customfilter_ressonance;
		customfilter_vtable_initialized = 1;
	}
}

void psy_dsp_customfilter_init(psy_dsp_CustomFilter* self)
{
	psy_dsp_filter_init(&self->filter);
	customfilter_vtable_init(&self->filter);
	self->filter.vtable = &customfilter_vtable;
	
	self->samplerate = (psy_dsp_beat_t) 44100.f;
	self->cutoff = 1.0f;
	self->q = 0.f;
}

void customfilter_setsamplerate(psy_dsp_CustomFilter* self, float samplerate)
{
	self->samplerate = samplerate;
	self->filter.vtable->update(&self->filter, 1);
}

float customfilter_samplerate(psy_dsp_CustomFilter* self)
{
	return self->samplerate;
}

void customfilter_setcutoff(psy_dsp_CustomFilter* self, float cutoff)
{
	if (self->cutoff != cutoff) {
		self->cutoff = cutoff;
		self->filter.vtable->update(&self->filter, 0);
	}
}

float customfilter_cutoff(psy_dsp_CustomFilter* self)
{
	return self->cutoff;
}

void customfilter_setressonance(psy_dsp_CustomFilter* self, float q)
{
	if (self->q != q) {
		self->q = q;
		self->filter.vtable->update(&self->filter, 0);
	}
}

float customfilter_ressonance(psy_dsp_CustomFilter* self)
{
	return self->q;
}

void psy_dsp_firwork_init(psy_dsp_FIRWork* self)
{
	self->x1 = 0;
	self->x2 = 0;
	self->y1 = 0;
	self->y2 = 0;	
}

void psy_dsp_firwork_reset(psy_dsp_FIRWork* self)
{
	psy_dsp_firwork_init(self);
}

psy_dsp_amp_t psy_dsp_firwork_work(psy_dsp_FIRWork* self, psy_dsp_FilterCoeff* coeffs,
	psy_dsp_amp_t sample)
{
	psy_dsp_amp_t y;

	y = (psy_dsp_amp_t) 
		 (sample * coeffs->coeff[0] +  
		self->x1 * coeffs->coeff[1] + 
		self->x2 * coeffs->coeff[2] + 
		self->y1 * coeffs->coeff[3] + 
		self->y2 * coeffs->coeff[4]);
	self->y2 = self->y1;
	self->y1 = y;
	self->x2 = self->x1; 
	self->x1 = sample;
	return y;
}
