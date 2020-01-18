// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_FILTERCOEFF_H
#define psy_dsp_FILTERCOEFF_H

#include "dsptypes.h"

extern const double TPI;

typedef struct {	
	psy_dsp_big_amp_t coeff[5];
} psy_dsp_FilterCoeff;

typedef struct {
	psy_dsp_amp_t container[128][128][5];
} psy_dsp_FilterMap;

void psy_dsp_filtercoeff_update(psy_dsp_FilterCoeff*, psy_dsp_FilterMap* map, int cutoff, int q);
void psy_dsp_filtercoeff_setparameter(psy_dsp_FilterCoeff*, float a0, float a1, float a2,
	float b0, float b1, float b2);

typedef void (*psy_dsp_FilterMapCallback)(void*, int freq, int r, psy_dsp_FilterCoeff* coeff);
void psy_dsp_filtermap_compute(psy_dsp_FilterMap*, void* context, psy_dsp_FilterMapCallback);

float psy_dsp_cutoffinternalext(int v);
float psy_dsp_resonanceinternal(float v);

#endif /* psy_dsp_FILTERCOEFF_H */
