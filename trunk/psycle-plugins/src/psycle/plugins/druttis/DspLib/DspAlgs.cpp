//////////////////////////////////////////////////////////////////////
//
//				DspAlgs.cpp
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#include "DspAlgs.h"

//////////////////////////////////////////////////////////////////////
//
//				generate_wave
//
//////////////////////////////////////////////////////////////////////

void generate_wave(float *psamples, int nsamples, float *pcoeffs, int ncoeffs)
{
	int i;
	int j;
	float t;
	float y;
	float ofs;
	float amp;
	float range;
	float min = 0.0f;
	float max = 0.0f;

	for (i = 0; i < nsamples; i++) {
		y = 0.0f;
		t = (float)i * 6.283185307179586476925286766559f / (float)nsamples;
		for (j = 0; j < ncoeffs; j++) {
			y += (float) sin(t * (float) (j + 1));
		}
		psamples[i] = y;
		if (y < min)
			min = y;
		if (y > max)
			max = y;
	}

	range = max - min;

	if (range > 0.0f) {

		amp = 2.0f / range;
		ofs = (min - max) / 2.0f;

		for (i = 0; i < nsamples; i++) {
//												psamples[i] += ofs;
			psamples[i] *= amp;
		}
	}

}
