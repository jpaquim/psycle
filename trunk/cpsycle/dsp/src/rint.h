// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_RINT_H
#define psy_dsp_RINT_H

#include "../../detail/psydef.h"
#include "../../detail/os.h"
#include "math.h"

#ifdef __cplusplus
extern "C" {
#endif

// int32_t
int32_t INLINE psy_dsp_rint32_double(double d)
{
	union result_union
	{
		double d;
		int32_t i;
	} result;
	result.d = d + 6755399441055744.0; // 2^51 + 2^52
	return result.i;
}

int32_t INLINE psy_dsp_rint32_float(float f)
{
#if defined DIVERSALIS__CPU__X86 && defined DIVERSALIS__COMPILER__MICROSOFT && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL// also intel's compiler?
///\todo not always the fastest when using sse(2)
///\todo the double "2^51 + 2^52" version might be faster.
int32_t i;
__asm
{
	fld f;
	fistp i;
}
return i;
#else
return static_cast<int32_t>(f);
#endif
}

// uint32_t
uint32_t INLINE psy_dsp_ruint32_double(double d)
{
	return (uint32_t)psy_dsp_rint32_double(d);
}

uint32_t INLINE psy_dsp_ruint32_float(float f)
{
	return (uint32_t) psy_dsp_rint32_float(f);
}

// int16_t
int16_t INLINE psy_dsp_rint16_double(double d)
{
	return (int16_t) psy_dsp_rint32_double(d);
}

int16_t INLINE psy_dsp_rint16_float(float f)
{
	return (uint16_t) psy_dsp_rint32_float(f);
}

int16_t INLINE psy_dsp_ruint16_double(double d)
{
	return (uint16_t) psy_dsp_rint32_double(d);
}

int16_t INLINE psy_dsp_ruint16_float(float f)
{
	return (uint16_t) psy_dsp_rint32_float(f);
}

// int8_t
int8_t INLINE psy_dsp_rint8_double(double d)
{
	return (int8_t) psy_dsp_rint32_double(d);
}

int8_t INLINE psy_dsp_rint8_float(float f)
{
	return (int8_t) psy_dsp_rint32_float(f);
}

// uint8_t
uint8_t INLINE psy_dsp_ruint8_double(double d)
{
	return (uint8_t) psy_dsp_rint32_double(d);
}

uint8_t INLINE psy_dsp_ruint8_float(float f)
{
	return (uint8_t) psy_dsp_rint32_float(f);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_RINT_H */
