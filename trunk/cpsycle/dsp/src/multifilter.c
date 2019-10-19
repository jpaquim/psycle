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

void multifilter_init(MultiFilter* self)
{
	self->samplerate = 44100.f;
	self->cutoff = 1.f;
	self->q = 0.f;
	self->filter.reset = reset;
	self->filter.work = work;
	self->filter.update = update;
	self->filter.setcutoff = setcutoff;
	self->filter.cutoff = cutoff;
	self->filter.setressonance = setressonance;
	self->filter.ressonance = ressonance;
	self->filter.setsamplerate = setsamplerate;
	self->filter.samplerate = samplerate;
	self->filter.work = work;
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
			filter->setcutoff(filter, self->cutoff);
			filter->setressonance(filter, self->q);
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
		lp12e.customfilter.filter.setsamplerate(&lp12e, (float)samplerate);
	}
	
}

void reset(MultiFilter* self)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->reset(self->selectedfilter);
	}
}

amp_t work(MultiFilter* self, amp_t sample)
{
	return self->selectedfilter 
		? self->selectedfilter->work(self->selectedfilter, sample)
		: sample;
}

static void setcutoff(MultiFilter* self, float cutoff)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->setcutoff(self->selectedfilter, cutoff);
	}
	self->cutoff = cutoff;
}

static float cutoff(MultiFilter* self)
{
	return self->selectedfilter 
		? self->selectedfilter->cutoff(self->selectedfilter)
		: self->cutoff;
}

void setressonance(MultiFilter* self, float ressonance)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->setressonance(self->selectedfilter, ressonance);
	}
	self->q = ressonance;
}

float ressonance(MultiFilter* self)
{
	return self->selectedfilter 
		? self->selectedfilter->ressonance(self->selectedfilter)
		: self->q;
}

void setsamplerate(MultiFilter* self, float samplerate)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->setsamplerate(self->selectedfilter, samplerate);
	}
	self->samplerate = samplerate;
}

float samplerate(MultiFilter* self)
{
	return self->selectedfilter 
		? self->selectedfilter->samplerate(self->selectedfilter)
		: self->samplerate;
}

void update(MultiFilter* self, int full)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->update(self->selectedfilter, full);
	}
}
