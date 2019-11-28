// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "filter.h"

// Filter Interface

static void init(Filter* self) { }
static void dispose(Filter* self) { }
static amp_t work(Filter* self, amp_t sample) { return sample; }
static void setcutoff(Filter* self, float cutoff) { }
static float cutoff(Filter* self) { return 0.f; }
static void setressonance(Filter* self, float ressonance) { }
static float ressonance(Filter* self) { return 0.f; }
static void setsamplerate(Filter* self, float samplerate) { }
static float samplerate(Filter* self) { return 44100.f; }
static void update(Filter* self, int full) { }
static void reset(Filter* self) { }

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

void filter_init(Filter* self)		
{
	vtable_init();
	self->vtable = &vtable;
}

// CustomFilter

static void customfilter_setcutoff(CustomFilter*, float cutoff);
static float customfilter_cutoff(CustomFilter*);
static void customfilter_setressonance(CustomFilter*, float ressonance);
static float customfilter_ressonance(CustomFilter*);
static void customfilter_setsamplerate(CustomFilter*, float samplerate);
static float customfilter_samplerate(CustomFilter*);

static filter_vtable customfilter_vtable;
static int customfilter_vtable_initialized = 0;

void customfilter_vtable_init(Filter* filter)
{
	if (!customfilter_vtable_initialized) {
		customfilter_vtable = *filter->vtable;
		customfilter_vtable.setsamplerate = customfilter_setsamplerate;
		customfilter_vtable.samplerate = customfilter_samplerate;
		customfilter_vtable.setcutoff = customfilter_setcutoff;
		customfilter_vtable.cutoff = customfilter_cutoff;
		customfilter_vtable.setressonance = customfilter_setressonance;
		customfilter_vtable.ressonance = customfilter_ressonance;
	}
}

void customfilter_init(CustomFilter* self)
{
	filter_init(&self->filter);
	customfilter_vtable_init(&self->filter);
	self->filter.vtable = &customfilter_vtable;
	
	self->samplerate = (beat_t) 44100.f;
	self->cutoff = 1.0f;
	self->q = 0.f;
}

void customfilter_setsamplerate(CustomFilter* self, float samplerate)
{
	self->samplerate = samplerate;
	self->filter.vtable->update(self, 1);
}

float customfilter_samplerate(CustomFilter* self)
{
	return self->samplerate;
}

void customfilter_setcutoff(CustomFilter* self, float cutoff)
{
	if (self->cutoff != cutoff) {
		self->cutoff = cutoff;
		self->filter.vtable->update(self, 0);
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
		self->filter.vtable->update(self, 0);
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

amp_t firwork_work(FIRWork* self, FilterCoeff* coeffs, amp_t sample)
{
	amp_t y;

	y = (amp_t) 
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
