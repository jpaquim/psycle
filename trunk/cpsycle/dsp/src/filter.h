// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_FILTER_H
#define psy_dsp_FILTER_H

#include "filtercoeff.h"
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum psy_dsp_FilterType {
	F_LOWPASS12 = 0,
	F_HIGHPASS12 = 1,
	F_BANDPASS12 = 2,
	F_BANDREJECT12 = 3,
	F_NONE = 4,//This one is kept here because it is used in load/save. Also used in Sampulse instrument filter as "use channel default"
	F_ITLOWPASS = 5,
	F_MPTLOWPASSE = 6,
	F_MPTHIGHPASSE = 7,
	F_LOWPASS12E = 8,
	F_HIGHPASS12E = 9,
	F_BANDPASS12E = 10,
	F_BANDREJECT12E = 11,

	F_NUMFILTERS
} psy_dsp_FilterType;


#define TPI 6.28318530717958647692528676655901

typedef struct FilterCoeff {
	float _coeffs[F_NUMFILTERS - 1][128][128][5];
	float samplerate;
	double _coeff[5];
} FilterCoeff;

void filtercoeff_init(FilterCoeff*);
void filtercoeff_dispose(FilterCoeff*);
float filtercoeff_cutoff(psy_dsp_FilterType ft, int v);
float filtercoeff_resonance(psy_dsp_FilterType ft, int freq, int r);
void filtercoeff_setsamplerate(FilterCoeff* self, float samplerate);
INLINE float filtercoeff_getsamplerate(FilterCoeff* self) { return self->samplerate; };
void filtercoeff_computecoeffs(FilterCoeff* self, psy_dsp_FilterType t, int freq, int r);
float filtercoeff_cutoffinternal(int v);
float filtercoeff_resonanceinternal(float v);
float filtercoeff_bandwidthinternal(int v);
float filtercoeff_cutoffinternalext(int v);
float filtercoeff_resonanceinternalext(float v);
float filtercoeff_bandwidthinternalext(int v);
double filtercoeff_cutoffit(int v);
double filtercoeff_cutoffmptext(int v);
double filtercoeff_resonanceit(int r);
double filtercoeff_resonancempt(int resonance);

extern FilterCoeff filtercoeff;

/// filter.

// virtual inline float Work(float x);
// virtual inline void WorkStereo(float& l, float& r);

struct Filter;

typedef float (*fp_filter_work)(struct Filter*, float x);
typedef void (*fp_filter_workstereo)(struct Filter*, float* l, float* r);


typedef struct FilterVtable {
	fp_filter_work work;
	fp_filter_workstereo workstereo;
} FilterVtable;

typedef struct Filter {
	FilterVtable* vtable;
	psy_dsp_FilterType _type;
	int _cutoff;
	int _q;

	float _coeff0;
	float _coeff1; //coeff[1] reused in ITFilter as highpass coeff
	float _coeff2; //coeff[2] not used in ITFilter
	float _coeff3;
	float _coeff4;
	float _x1, _x2, _y1, _y2;
	float _a1, _a2, _b1, _b2;
} Filter;

void filter_init(Filter*);
void filter_init_samplerate(Filter*, int sampleRate);
void filter_reset(Filter*);//Same as init, without samplerate
void filter_dispose(Filter*);
void filter_update(Filter*);
const char* filter_name(psy_dsp_FilterType);
uintptr_t filter_numfilters(void);

INLINE void filter_setcutoff(Filter* self, int iCutoff)
{
	if (self->_cutoff != iCutoff) {
		self->_cutoff = iCutoff;
		filter_update(self);
	}
}

INLINE int filter_cutoff(Filter* self)
{
	return self->_cutoff;
}

INLINE void filter_setressonance(Filter* self, int iRes)
{
	if (self->_q != iRes) {
		self->_q = iRes;
		filter_update(self);
	}
}

INLINE int filter_ressonance(Filter* self)
{
	return self->_q;
}

INLINE void filter_setsamplerate(Filter* self, int iSampleRate)
{
	if (filtercoeff_getsamplerate(&filtercoeff) != iSampleRate) {
		filtercoeff_setsamplerate(&filtercoeff, (float)iSampleRate);
		filter_update(self);
	}
}

INLINE void filter_settype(Filter* self, psy_dsp_FilterType newftype)
{
	if (newftype != self->_type) {
		self->_type = newftype;
		filter_update(self);
	}
}

INLINE psy_dsp_FilterType filter_type(Filter* self)
{
	return self->_type;
}

typedef struct ITFilter {
	Filter filter;	
} ITFilter;


void itfilter_init(ITFilter* self);

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_FILTER_H */
