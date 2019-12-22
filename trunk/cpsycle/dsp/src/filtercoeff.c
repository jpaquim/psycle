// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "filtercoeff.h"
#include <math.h>

const double TPI = 6.28318530717958647692528676655901;

static void filtermap_setcoeffs(psy_dsp_FilterMap*, int cutoff, int q,
	psy_dsp_FilterCoeff*);

void psy_dsp_filtermap_compute(psy_dsp_FilterMap* self, void* context, 
	psy_dsp_FilterMapCallback callback)
{	
	int cutoff;
	int q;
	psy_dsp_FilterCoeff coeffs;

	for (cutoff = 0; cutoff < 128; ++cutoff) {
		for (q = 0; q < 128; ++q) {
			callback(context, cutoff, q, &coeffs);
			filtermap_setcoeffs(self, cutoff, q, &coeffs);
		}
	}		
}

void filtermap_setcoeffs(psy_dsp_FilterMap* self, int cutoff, int q,  psy_dsp_FilterCoeff* coeffs)
{
	int pole;

	for (pole = 0; pole < 5; ++ pole) {
		self->container[cutoff][q][pole] = (psy_dsp_amp_t) coeffs->coeff[pole];
	}			
}

float psy_dsp_resonanceinternal(float v)
{
	return (float)((pow(v / 127.0, 4) * 150 + 0.1) * 0.5);
}

float psy_dsp_cutoffinternalext(int v)
{
	float factor = 8.f/132.f;
	float factor2 = 20000.f/256.f;

	return (float)(((pow(2.0f,(v+5)*factor)-1.f)*factor2));
}

void psy_dsp_filtercoeff_update(psy_dsp_FilterCoeff* self, psy_dsp_FilterMap* map, int cutoff, int q)
{
	int pole;

	for (pole = 0; pole < 5; ++pole) {
		self->coeff[pole] =  map->container[cutoff][q][pole];
	}
}

void psy_dsp_filtercoeff_setparameter(psy_dsp_FilterCoeff* self, float a0, float a1, float a2,
	float b0, float b1, float b2)
{
	self->coeff[0] = b0/a0;
	self->coeff[1] = b1/a0;
	self->coeff[2] = b2/a0;
	self->coeff[3] = -a1/a0;
	self->coeff[4] = -a2/a0;
}
