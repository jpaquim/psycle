// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_DSP_MULTIFILTER_H)
#define PSY_DSP_MULTIFILTER_H

#include "lowpass12e.h"

typedef enum FilterType {
	// This one is kept here because it is used in load/save. Also used in
	// Sampulse instrument filter as "use channel default"
	F_NONE	= 4,
	F_LOWPASS12E = 8,	
	F_NUMFILTERS
} FilterType;

typedef struct {	
	psy_dsp_Filter filter;	
	psy_dsp_LowPass12E lowpass12E;
	FilterType selected;
	psy_dsp_Filter* selectedfilter;
	float samplerate;
	float cutoff;
	float q;
} psy_dsp_MultiFilter;

void psy_dsp_multifilter_init(psy_dsp_MultiFilter*);
void psy_dsp_multifilter_settype(psy_dsp_MultiFilter*, FilterType type);
FilterType psy_dsp_multifilter_type(psy_dsp_MultiFilter*);
const char* psy_dsp_multifilter_name(psy_dsp_MultiFilter*, FilterType);
unsigned int psy_dsp_multifilter_numfilters(psy_dsp_MultiFilter*);
void psy_dsp_multifilter_inittables(unsigned int samplerate);

#endif
