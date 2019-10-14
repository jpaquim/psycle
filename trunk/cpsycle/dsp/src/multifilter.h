// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_MULTIFILTER_H)
#define DSP_MULTIFILTER_H

#include "lowpass12e.h"

typedef enum FilterType {
	// This one is kept here because it is used in load/save. Also used in
	// Sampulse instrument filter as "use channel default"
	F_NONE	= 4,
	F_LOWPASS12E = 8,	
	F_NUMFILTERS
} FilterType;

typedef struct {	
	Filter filter;	
	LowPass12E lowpass12E;
	FilterType selected;
	Filter* selectedfilter;
	float samplerate;
	float cutoff;
	float q;
} MultiFilter;

void multifilter_init(MultiFilter*);
void multifilter_settype(MultiFilter*, FilterType type);
FilterType multifilter_type(MultiFilter*);
const char* multifilter_name(MultiFilter*, FilterType);
unsigned int numfilters(MultiFilter*);
void multifilter_inittables(unsigned int samplerate);

#endif
