// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_CONVERT_H
#define psy_dsp_CONVERT_H

#include "dsptypes.h"
#include "../../detail/psydef.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

INLINE static psy_dsp_amp_t psy_dsp_convert_db_to_amp(psy_dsp_amp_t db)
{
	return (psy_dsp_amp_t) pow(10.0f, db / 20.0f);
}
/// compares two floating point numbers for rough equality (difference less than epsilon by default).
bool INLINE roughly_equals(float a, float b)
{
	return fabs(a - b) < psy_dsp_epsilon;
}
/// compares two floating point numbers for rough equality
bool INLINE roughly_equals_e(float a, float b, float tolerance)
{
	return fabs(a - b) < tolerance;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_CONVERT_H */
