// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_TYPES_H
#define psy_dsp_TYPES_H

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define psy_dsp_PI 3.14159265358979323846
#define psy_dsp_PI_F 3.14159265358979323846f
#define psy_dsp_e 2.71828182845904523536028747135266249775724709369995

typedef float psy_dsp_amp_t;
typedef float psy_dsp_beat_t;
typedef float psy_dsp_seconds_t;
typedef float psy_dsp_percent_t;

typedef double psy_dsp_big_amp_t;
typedef double psy_dsp_big_beat_t;
typedef double psy_dsp_big_seconds_t;
typedef double psy_dsp_big_percent_t;

typedef enum {
	PSY_DSP_AMP_RANGE_VST,
	PSY_DSP_AMP_RANGE_NATIVE,
	PSY_DSP_AMP_RANGE_IGNORE
} psy_dsp_amp_range_t;

#define psy_dsp_epsilon 0.001

INLINE int psy_dsp_testrange(psy_dsp_big_beat_t position, psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t width)
{
	return position >= offset && position < offset + width;
}

INLINE bool psy_dsp_testrange_e(psy_dsp_big_beat_t position, psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t width)
{
	return position + psy_dsp_epsilon * 2 >= offset &&
		position < offset + width - psy_dsp_epsilon;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_TYPES_H */
