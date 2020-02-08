// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_VALUEMAPPER_H
#define psy_dsp_VALUEMAPPER_H

#include "dsptypes.h"
#include "../../detail/psydef.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/// maps a byte (0 to 128) to a float (0 to 1).
float INLINE psy_dsp_map_128_1(intptr_t value)
{
	return (float)(value) * 0.0078125f;
}
/// maps a byte (0 to 256) to a float (0 to 1).
float INLINE psy_dsp_map_256_1(intptr_t value)
{
	return (float)(value) * 0.00390625f;
}
/// maps a byte (0 to 255) to a float (0 to 1).
float INLINE psy_dsp_map_255_1(intptr_t value)
{
	return (float)(value) * 0.00392156f;
}
/// maps a byte (0 to 256) to a float (0 to 100).
float INLINE psy_dsp_map_256_100(intptr_t value)
{
	return (float)(value) * 0.390625f;
}
/// maps a byte (0 to 32768) to a float (0 to 1).
float INLINE psy_dsp_map_32768_1(intptr_t value)
{
	return (float)((double)(value) * 0.000030517578125);
}
/// maps a byte (0 to 65536) to a float (0 to 1).
float INLINE psy_dsp_map_65536_1(intptr_t value)
{
	return (float)((double)(value) * 0.0000152587890625);
}
/// maps a float (0 to 1) to a byte (0 to 128).
intptr_t INLINE psy_dsp_map_1_128(float value)
{
	return (intptr_t)(value * 128.f);
}
/// maps a float (0 to 1) to a byte (0 to 256).
intptr_t INLINE psy_dsp_map_1_256(float value)
{
	return (intptr_t)(value * 256.f);
}
/// maps a float (0 to 1) to a int (0 to 32768).
intptr_t INLINE psy_dsp_map_1_32768(float value)
{
	return (intptr_t)(value * 32768.f);
}
/// maps a float (0 to 1) to a int (0 to 65536).
intptr_t INLINE psy_dsp_map_1_65536(intptr_t value)
{
	return (intptr_t)(value * 65536.f);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_CONVERT_H */
