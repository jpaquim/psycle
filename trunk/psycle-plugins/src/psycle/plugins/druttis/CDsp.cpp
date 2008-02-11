// -*- mode:c++; indent-tabs-mode:t -*-
//============================================================================
//				CDsp.cpp
//				------
//				Digital Signal Processing Library using Floating Point
//				by Joakim Dahlström a.k.a. Druttis
//============================================================================
#include <packageneric/pre-compiled.private.hpp>
#include "CDsp.h"
//============================================================================
//				CreateWavetable
//============================================================================
void CDsp::CreateWavetable(float *coeffs, int ncoeffs, float *psamples, int nsamples)
{
	int i;
	float min = 1.0f;
	float max = -1.0f;
	for (i = 0; i < nsamples; i++) {
		float ph = (float) i / (float) nsamples;
		float out = 0.0f;
		for (int j = 1; j <= ncoeffs; j++) {
			out += (float) sin((float) j * ph * PI2) * coeffs[j - 1];
			if (out < min)
				min = out;
			if (out > max)
				max = out;
		}
		psamples[i] = out;
	}
	float range = max - min;
	if (range) {
		for (i = 0; i < nsamples; i++)
			psamples[i] = psamples[i] * 2.0f / range;
	}
}