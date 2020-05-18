// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_DITHER_H
#define psy_dsp_DITHER_H

#include "../../detail/psydef.h"

#include "mersennetwister.h"

typedef	enum {
	psy_dsp_DITHER_PDF_TRIANGULAR = 0,
	psy_dsp_DITHER_PDF_RECTANGULAR,
	psy_dsp_DITHER_PDF_GAUSSIAN
} psy_dsp_DitherPdf;

typedef	enum {
	psy_dsp_DITHER_NOISESHAPE_NONE = 0,
	psy_dsp_DITHER_NOISESHAPE_HIGHPASS,
} psy_dsp_DitherNoiseShape;

// Psycle uses -32768.0..32768.0 range for insamples. This Process is done
// with that in mind. Modify the bdMultiplier and bdQ values in the switch
// case for other use cases.

typedef struct psy_dsp_DitherSettings {
	uintptr_t bitdepth;
	psy_dsp_DitherPdf pdf;
	psy_dsp_DitherNoiseShape noiseshape;
} psy_dsp_DitherSettings;

typedef struct psy_dsp_Dither {
	psy_dsp_DitherSettings settings;
	psy_dsp_MersenneTwister mt;
} psy_dsp_Dither;

void psy_dsp_dither_init(psy_dsp_Dither*);
void psy_dsp_dither_dispose(psy_dsp_Dither*);
void psy_dsp_dither_process(psy_dsp_Dither*, float* inSamps, uintptr_t length);

INLINE void  psy_dsp_dither_setsettings(psy_dsp_Dither* self,
	psy_dsp_DitherSettings settings)
{
	self->settings = settings;
}

INLINE psy_dsp_DitherSettings psy_dsp_dither_settings(psy_dsp_Dither* self)
{
	return self->settings;
}

INLINE void  psy_dsp_dither_setbitdepth(psy_dsp_Dither* self,
	uintptr_t newdepth)
{
	self->settings.bitdepth = newdepth;
}

INLINE void psy_dsp_dither_setpdf(psy_dsp_Dither* self,
	psy_dsp_DitherPdf newpdf)
{
	self->settings.pdf = newpdf;
}

INLINE void psy_dsp_dither_setnoiseshaping(psy_dsp_Dither* self,
	psy_dsp_DitherNoiseShape newns)
{
	self->settings.noiseshape = newns;
}

#endif /* psy_dsp_DITHER_H */
