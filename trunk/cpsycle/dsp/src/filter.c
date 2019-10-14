// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "filter.h"

// Filter Interface

static void init(Filter* self) { }
static void dispose(Filter* self) { }
static float work(Filter* self, float sample) { return sample; }
static void setcutoff(Filter* self, float cutoff) { }
static float cutoff(Filter* self) { return 0.f; }
static void setressonance(Filter* self, float ressonance) { }
static float ressonance(Filter* self) { return 0.f; }
static void setsamplerate(Filter* self, float samplerate) { }
static float samplerate(Filter* self) { return 44100.f; }
static void update(Filter* self, int full) { }
static void reset(Filter* self) { }

void filter_init(Filter* self)		
{
	self->init = init;
	self->dispose = dispose;
	self->work = work;
	self->setsamplerate = setsamplerate;
	self->samplerate = samplerate;
	self->setcutoff = setcutoff;
	self->cutoff = cutoff;
	self->setressonance = setressonance;
	self->ressonance = ressonance;
	self->reset = reset;
}

// CustomFilter

static void customfilter_setcutoff(CustomFilter*, float cutoff);
static float customfilter_cutoff(CustomFilter*);
static void customfilter_setressonance(CustomFilter*, float ressonance);
static float customfilter_ressonance(CustomFilter*);
static void customfilter_setsamplerate(CustomFilter*, float samplerate);
static float customfilter_samplerate(CustomFilter*);

void customfilter_init(CustomFilter* self)
{
	filter_init(&self->filter);
	self->filter.setsamplerate = customfilter_setsamplerate;
	self->filter.samplerate = customfilter_samplerate;
	self->filter.setcutoff = customfilter_setcutoff;
	self->filter.cutoff = customfilter_cutoff;
	self->filter.setressonance = customfilter_setressonance;
	self->filter.ressonance = customfilter_ressonance;

	self->samplerate = 44100.f;
	self->cutoff = 1.0f;
	self->q = 0.f;
}

void customfilter_setsamplerate(CustomFilter* self, float samplerate)
{
	self->samplerate = samplerate;
	self->filter.update(self, 1);
}

float customfilter_samplerate(CustomFilter* self)
{
	return self->samplerate;
}

void customfilter_setcutoff(CustomFilter* self, float cutoff)
{
	if (self->cutoff != cutoff) {
		self->cutoff = cutoff;
		self->filter.update(self, 0);
	}
}

float customfilter_cutoff(CustomFilter* self)
{
	return self->cutoff;
}

void customfilter_setressonance(CustomFilter* self, float q)
{
	if (self->q != q) {
		self->q = q;
		self->filter.update(self, 0);
	}
}

float customfilter_ressonance(CustomFilter* self)
{
	return self->q;
}

void firwork_init(FIRWork* self)
{
	self->x1 = 0;
	self->x2 = 0;
	self->y1 = 0;
	self->y2 = 0;	
}

void firwork_reset(FIRWork* self)
{
	firwork_init(self);
}

float firwork_work(FIRWork* self, FilterCoeff* coeffs, float sample)
{
	float y;

	y = (float) 
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
