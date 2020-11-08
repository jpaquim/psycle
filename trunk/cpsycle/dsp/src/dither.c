// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "dither.h"

void psy_dsp_dither_init(psy_dsp_Dither* self)
{
	uint32_t init[4] = { 0x123, 0x234, 0x345, 0x456 };

	self->settings.bitdepth = 16;
	self->settings.pdf = psy_dsp_DITHER_PDF_TRIANGULAR;
	self->settings.noiseshape = psy_dsp_DITHER_NOISESHAPE_NONE;

	psy_dsp_mersennetwisterinit_by_array(&self->mt, init, sizeof(init));
}

void psy_dsp_dither_dispose(psy_dsp_Dither* self)
{
}

void psy_dsp_dither_process(psy_dsp_Dither* self, float * inSamps,
	uintptr_t length)
{
	// gaussian rand returns two values, this tells us which we're on
	bool newgauss = TRUE;
	// our random number, and an extra variable for the gaussian distribution
	double gauss = 0.0;
	double randval = 0.0;
	// quantization error of the last sample, used for noise shaping
	float prevError = 0.0f;
	// the amount the sample will eventually be multiplied by for integer quantization
	float bdMultiplier;
	// the inverse.. i.e., the number that will eventually become the quantization interval
	float bdQ;
	uintptr_t i;

	switch(self->settings.bitdepth) {
		case 8:
			bdMultiplier = 1 / 256.0f;
			bdQ = 256.0f;
			break;
		default:
		case 16:
			bdMultiplier = 1.0f;
			bdQ = 1.0f;
			break;
		case 24:
			bdMultiplier = 256.0f;
			bdQ = 1 / 256.0f;
			break;
		case 32:
			bdMultiplier = 65536.0f;
			bdQ = 1 / 65536.0f;
			break;
	}

	for(i = 0; i < length; ++i) {
		switch(self->settings.pdf) {
			case psy_dsp_DITHER_PDF_RECTANGULAR:
				randval = psy_dsp_mersennetwister_genrand_real1(&self->mt) - 0.5;
			break;
			case psy_dsp_DITHER_PDF_GAUSSIAN:
				if (newgauss) {
					psy_dsp_mersennetwister_genrand_gaussian(&self->mt,
						&randval, &gauss);
				} else {
					randval = gauss;
				}
				newgauss = !newgauss; //genrand_gaussian() has a standard deviation (rms) of 1..
				randval *= 0.5; // we need it to be one-half the quantizing interval (which is 1), so we just halve it
			break;
			case psy_dsp_DITHER_PDF_TRIANGULAR:
				randval =
					(psy_dsp_mersennetwister_genrand_real1(&self->mt) - 0.5) +
					(psy_dsp_mersennetwister_genrand_real1(&self->mt) - 0.5);
			break;
			default:
				randval = 0;
			break;
		}

		*(inSamps + i) += randval * bdQ;

		///\todo this seems inefficient.. we're essentially quantizing twice, once for practice to get the error, and again
		/// for real when we write to the wave file.
		if(self->settings.noiseshape == psy_dsp_DITHER_NOISESHAPE_HIGHPASS) {
			*(inSamps + i) += prevError;
			// The only way to determine the quantization error of rounding to int after scaling by a given factor
			// is to do the actual scaling, and then divide the resulting error by the same factor to keep it in
			// the correct scale (until it's re-multiplied on wave-writing (this is pretty ridiculous))
			prevError = (*(inSamps + i) * bdMultiplier - (int)(*(inSamps + i) * bdMultiplier)) * bdQ;
		}
	}
}
