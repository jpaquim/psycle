// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_FILTERCOEFF_H)
#define DSP_FILTERCOEFF_H

extern const double TPI;

typedef struct {	
	double coeff[5];
} FilterCoeff;

typedef struct {
	float container[128][128][5];
} FilterMap;


void filtercoeff_update(FilterCoeff*, FilterMap* map, int cutoff, int q);
void filtercoeff_setparameter(FilterCoeff*, float a0, float a1, float a2,
	float b0, float b1, float b2);

typedef void (*FilterMapCallback)(void*, int freq, int r, FilterCoeff* coeff);
void filtermap_compute(void*, void* context, FilterMapCallback);

float cutoffinternalext(int v);
float resonanceinternal(float v);

#endif
