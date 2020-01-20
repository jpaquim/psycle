// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_TYPES_H
#define psy_dsp_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_TYPES_H */
