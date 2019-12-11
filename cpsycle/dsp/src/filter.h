// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_FILTER_H)
#define DSP_FILTER_H

#include "filtercoeff.h"

struct Filter;


typedef void (*fp_filter_init)(struct Filter*);
typedef void (*fp_filter_dispose)(struct Filter*);
typedef psy_dsp_amp_t (*fp_filter_work)(struct Filter*, psy_dsp_amp_t sample);
typedef void (*fp_filter_setcutoff)(struct Filter*, float cutoff);	
typedef float (*fp_filter_cutoff)(struct Filter*);
typedef void (*fp_filter_setressonance)(struct Filter*, float ressonance);
typedef float (*fp_filter_ressonance)(struct Filter*);
typedef void (*fp_filter_setsamplerate)(struct Filter*, float samplerate);
typedef float (*fp_filter_samplerate)(struct Filter*);
typedef void (*fp_filter_update)(struct Filter*, int full);
typedef void (*fp_filter_reset)(struct Filter*);

typedef struct filter_vtable {
	fp_filter_init init;
	fp_filter_dispose dispose;
	fp_filter_work work;
	fp_filter_setcutoff setcutoff;
	fp_filter_cutoff cutoff;
	fp_filter_setressonance setressonance;
	fp_filter_ressonance ressonance;
	fp_filter_setsamplerate setsamplerate;
	fp_filter_samplerate samplerate;
	fp_filter_update update;
	fp_filter_reset reset;
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
	psy_dsp_amp_t x1;
	psy_dsp_amp_t x2;
	psy_dsp_amp_t y1;
	psy_dsp_amp_t y2;
} FIRWork;

void firwork_init(FIRWork*);
psy_dsp_amp_t firwork_work(FIRWork*, FilterCoeff* coeffs,
	psy_dsp_amp_t sample);
void firwork_reset(FIRWork*);


#endif
