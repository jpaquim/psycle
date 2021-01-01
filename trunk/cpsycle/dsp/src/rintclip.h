// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_RINTCLIP_H
#define psy_dsp_RINTCLIP_H

#include "../../detail/psydef.h"
#include "../../detail/os.h"
#include "math.h"
#include "rint.h"


#define psy_dsp_clip(minimum, value, maximum) value < minimum ? minimum : value > maximum ? maximum : value

#ifdef __cplusplus
extern "C" {
#endif

/// clips with min and max values inferred from bits
float inline psy_dsp_clip_bits(float value, unsigned int bits)
{
	int32_t maxv = (int32_t)((1u << (bits - 1u)) - 1u);
	int32_t minv = -maxv - 1;
	return psy_dsp_clip(minv, value, maxv);
}

/// combines float to int32_t conversion with clipping.
int32_t INLINE psy_dsp_rint32_clip_bits(float x, const unsigned int bits)
{
	return psy_dsp_rint32_float(psy_dsp_clip_bits(x, bits));
}

int32_t INLINE psy_dsp_rint32_clip(float x)
{
	return psy_dsp_rint32_float(psy_dsp_clip_bits(x, (sizeof(int32_t) << 3)));
}

/// combines float to uint32_t conversion with clipping.
int32_t INLINE psy_dsp_ruint32_clip_bits(float x, const unsigned int bits)
{
	return psy_dsp_ruint32_float(psy_dsp_clip_bits(x, bits));
}

int32_t INLINE psy_dsp_ruint32_clip(float x)
{
	return psy_dsp_ruint32_float(psy_dsp_clip_bits(x, (sizeof(int32_t) << 3)));
}

/// combines float to int16_t conversion with clipping.
int16_t INLINE psy_dsp_rint16_clip_bits(float x, const unsigned int bits)
{
	return psy_dsp_rint16_float(psy_dsp_clip_bits(x, bits));
}

int16_t INLINE psy_dsp_rint16_clip(float x)
{
	return psy_dsp_rint16_float(psy_dsp_clip_bits(x, (sizeof(int16_t) << 3)));
}

uint16_t INLINE psy_dsp_ruint16_clip_bits(float x, const unsigned int bits)
{
	return psy_dsp_ruint16_float(psy_dsp_clip_bits(x, bits));
}

uint16_t INLINE psy_dsp_ruint16_clip(float x)
{
	return psy_dsp_ruint16_float(psy_dsp_clip_bits(x, (sizeof(uint16_t) << 3)));
}

/// combines float to int8_t conversion with clipping.
int8_t INLINE psy_dsp_rint8_clip_bits(float x, const unsigned int bits)
{
	return (int8_t) psy_dsp_rint16_float(psy_dsp_clip_bits(x, bits));
}

int8_t INLINE psy_dsp_rint8_clip(float x)
{
	return psy_dsp_rint8_float(psy_dsp_clip_bits(x, (sizeof(int8_t) << 3)));
}

uint8_t INLINE psy_dsp_ruint8_clip_bits(float x, const unsigned int bits)
{
	return psy_dsp_ruint8_float(psy_dsp_clip_bits(x, bits));
}

uint8_t INLINE psy_dsp_ruint8_clip(float x)
{
	return psy_dsp_ruint8_float(psy_dsp_clip_bits(x, (sizeof(uint8_t) << 3)));
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_RINTCLIP_H */
