// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "multifilter.h"

static const char* names[] = {
	"LowPass 12E",
	0
};

static void init(MultiFilter* self) { }
static void dispose(MultiFilter* self) { }
static void reset(MultiFilter*);
static float work(MultiFilter*, float sample);
static void setcutoff(MultiFilter*, float cutoff);
static float cutoff(MultiFilter*);
static void setressonance(MultiFilter*, float ressonance);
static float ressonance(MultiFilter*);
static void setsamplerate(MultiFilter*, float samplerate);
static float samplerate(MultiFilter*);
static void update(MultiFilter*, int full);

static filter_vtable vtable;
static int vtable_initialized = 0;

static void vtable_init(Filter* filter)
{
	if (!vtable_initialized) {
		vtable = *filter->vtable;
		vtable.reset = (fp_filter_reset) reset;
		vtable.work = (fp_filter_work) work;
		vtable.update = (fp_filter_update) update;
		vtable.setcutoff = (fp_filter_setcutoff) setcutoff;
		vtable.cutoff = (fp_filter_cutoff) cutoff;
		vtable.setressonance = (fp_filter_setressonance) setressonance;
		vtable.ressonance = (fp_filter_ressonance) ressonance;
		vtable.setsamplerate = (fp_filter_setsamplerate) setsamplerate;
		vtable.samplerate = (fp_filter_samplerate) samplerate;
		vtable.work = (fp_filter_work) work;
		vtable_initialized = 1;
	}	
}

void multifilter_init(MultiFilter* self)
{
	filter_init(&self->filter);
	vtable_init(&self->filter);
	self->filter.vtable = &vtable;
	self->samplerate = 44100.f;
	self->cutoff = 1.f;
	self->q = 0.f;	
	lowpass12e_init(&self->lowpass12E);
	self->selected = F_NONE;
	multifilter_settype(self, self->selected);
}

void multifilter_settype(MultiFilter* self, FilterType type)
{
	Filter* filter;

	self->selected = type;	
	switch (self->selected) {
		case F_LOWPASS12E:
			filter = (Filter*)(&self->lowpass12E);
			filter->vtable->setcutoff(filter, self->cutoff);
			filter->vtable->setressonance(filter, self->q);
		break;
		default:
			filter = 0;			
		break;
	}	
	self->selectedfilter = filter;
}

FilterType multifilter_type(MultiFilter* self)
{
	return self->selected;
}

const char* multifilter_name(MultiFilter* self, FilterType type)
{
	return names[(int)type];
}

unsigned int numfilters(MultiFilter* self)
{
	return F_NUMFILTERS;
}

void multifilter_inittables(unsigned int samplerate)
{
	{
		LowPass12E lp12e;
	
		lowpass12e_init(&lp12e); // forces static stable to initialize
		lp12e.customfilter.filter.vtable->setsamplerate(
			&lp12e.customfilter.filter, (float)samplerate);
	}
	
}

void reset(MultiFilter* self)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->vtable->reset(self->selectedfilter);
	}
}

amp_t work(MultiFilter* self, amp_t sample)
{
	return self->selectedfilter 
		? self->selectedfilter->vtable->work(self->selectedfilter, sample)
		: sample;
}

static void setcutoff(MultiFilter* self, float cutoff)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->vtable->setcutoff(self->selectedfilter, cutoff);
	}
	self->cutoff = cutoff;
}

static float cutoff(MultiFilter* self)
{
	return self->selectedfilter 
		? self->selectedfilter->vtable->cutoff(self->selectedfilter)
		: self->cutoff;
}

void setressonance(MultiFilter* self, float ressonance)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->vtable->setressonance(self->selectedfilter,
			ressonance);
	}
	self->q = ressonance;
}

float ressonance(MultiFilter* self)
{
	return self->selectedfilter 
		? self->selectedfilter->vtable->ressonance(self->selectedfilter)
		: self->q;
}

void setsamplerate(MultiFilter* self, float samplerate)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->vtable->setsamplerate(self->selectedfilter,
			samplerate);
	}
	self->samplerate = samplerate;
}

float samplerate(MultiFilter* self)
{
	return self->selectedfilter 
		? self->selectedfilter->vtable->samplerate(self->selectedfilter)
		: self->samplerate;
}

void update(MultiFilter* self, int full)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->vtable->update(self->selectedfilter, full);
	}
}
