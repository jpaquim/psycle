// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_FILTER_H)
#define DSP_FILTER_H

#include "filtercoeff.h"

typedef struct {
	void (*init)(void*);
	void (*dispose)(void*);
	float (*work)(void*, float sample);
	void (*setcutoff)(void*, float cutoff);	
	float (*cutoff)(void*);
	void (*setressonance)(void*, float ressonance);
	float (*ressonance)(void*);
	void (*setsamplerate)(void*, float samplerate);
	float (*samplerate)(void*);
	void (*update)(void*, int full);
	void (*reset)(void*);
} Filter;

void filter_init(Filter*);

typedef struct {
	Filter filter;	
	float samplerate;
	float cutoff;
	float q;
} CustomFilter;

void customfilter_init(CustomFilter*);

typedef struct {
	float x1, x2, y1, y2;
} FIRWork;

void firwork_init(FIRWork*);
float firwork_work(FIRWork*, FilterCoeff* coeffs, float sample);
void firwork_reset(FIRWork*);


#endif
