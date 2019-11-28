// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_FILTER_H)
#define DSP_FILTER_H

#include "filtercoeff.h"

struct Filter;

typedef struct filter_vtable {
	void (*init)(struct Filter*);
	void (*dispose)(struct Filter*);
	amp_t (*work)(struct Filter*, amp_t sample);
	void (*setcutoff)(struct Filter*, float cutoff);	
	float (*cutoff)(struct Filter*);
	void (*setressonance)(struct Filter*, float ressonance);
	float (*ressonance)(struct Filter*);
	void (*setsamplerate)(struct Filter*, float samplerate);
	float (*samplerate)(struct Filter*);
	void (*update)(struct Filter*, int full);
	void (*reset)(struct Filter*);
} filter_vtable;

typedef struct Filter {
	struct filter_vtable* vtable;
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
	amp_t x1, x2, y1, y2;
} FIRWork;

void firwork_init(FIRWork*);
amp_t firwork_work(FIRWork*, FilterCoeff* coeffs, amp_t sample);
void firwork_reset(FIRWork*);


#endif
