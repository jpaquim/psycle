// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_QUANTIZE_H
#define psy_dsp_QUANTIZE_H

#include "../../detail/psydef.h"
#include "math.h"

#ifdef __cplusplus
extern "C" {
#endif

//in -> -32768.0..32768.0 , out -32768..32767
void psy_dsp_quantize16(float *pin, int *piout, int c);
//in -> -32768.0..32768.0 , out -32768..32767
void psy_dsp_quantize16withdither(float *pin, int *piout, int c);
//in -> -8388608.0..8388608.0, out  -2147483648.0 to 2147483648.0
void psy_dsp_quantize24in32bit(float *pin, int *piout, int c);
//in -> -8388608.0..8388608.0 in 4 bytes, out -8388608..8388608, aligned to 3 bytes, big endian
void psy_dsp_quantize24be(float *pin, int *piout, int c);
//in -> -8388608.0..8388608.0 in 4 bytes, out -8388608..8388608, aligned to 3 bytes, little endian
void psy_dsp_quantize24le(float *pin, int *piout, int c);
//in -> -32768..32767 stereo interlaced, out -32768.0..32767.0 stereo deinterlaced
void psy_dsp_dequantize16anddeinterlace(short int *pin, float *poutleft,float *poutright,int c);
//in -> -2147483648..2147483647 stereo interlaced, out -32768.0..32767.0 stereo deinterlaced
void psy_dsp_dequantize32anddeinterlace(int *pin, float *poutleft,float *poutright,int c);
//in -> -1.0..1.0 stereo interlaced, out -32768.0..32767.0 stereo deinterlaced
void psy_dsp_deinterlacefloat(float *pin, float *poutleft,float *poutright,int c);

INLINE int32_t psy_dsp_rint(float flt) { return flt; }

INLINE int32_t psy_dsp_fround(float flt)
{
#if defined(_WIN64)
    return flt > 0 ? floor(flt + 0.5) : ceil(flt - 0.5);
#else
    int i;
    static const double half = 0.5f;
    _asm
    {
        fld flt
        fsub half
        fistp i
    }
    return i;
#endif
}	

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_QUANTIZE_H */
