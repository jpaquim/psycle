// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "quantize.h"
#include "math.h"

#define SHORT_MIN	-32768
#define SHORT_MAX	32767

//In -> -32768.0..32768.0 , out -32768..32767
void psy_dsp_quantize16withdither(float *pin, int *piout, int c)
{
	double const d2i = (1.5 * (1 << 26) * (1 << 26));
	int l;
	do
	{
		double res = ((double)pin[1] /*+ frand()*/) + d2i;
		int r = *(int *)&res;

		if (r < SHORT_MIN)
		{
			r = SHORT_MIN;
		}
		else if (r > SHORT_MAX)
		{
			r = SHORT_MAX;
		}
		res = ((double)pin[0] /*+ frand() */) + d2i;
		l = *(int *)&res;

		if (l < SHORT_MIN)
		{
			l = SHORT_MIN;
		}
		else if (l > SHORT_MAX)
		{
			l = SHORT_MAX;
		}
		*piout++ = (r << 16) | (uint16_t)(l);
		pin += 2;
	}
	while(--c);
}

// In -> -32768.0..32768.0 , out -32768..32767
void psy_dsp_quantize16(float *pin, int *piout, int c)
{
	int l;
	do
	{
		int r = psy_dsp_fround(pin[1]);
		if (r < SHORT_MIN)
		{
			r = SHORT_MIN;
		}
		else if (r > SHORT_MAX)
		{
			r = SHORT_MAX;
		}

		l = psy_dsp_fround(pin[0]);
		if (l < SHORT_MIN)
		{
			l = SHORT_MIN;
		}
		else if (l > SHORT_MAX)
		{
			l = SHORT_MAX;
		}
		*piout++ = (r << 16) | (uint16_t)(l);
		pin += 2;
	}
	while(--c);
}

//In -> -8388608.0..8388608.0, out  -2147483648.0 to 2147483648.0
void psy_dsp_quantize24in32bit(float *pin, int *piout, int c)
{
	// TODO Don't really know why, but the -100 is what made the clipping work correctly.
/*	int const max((1u << ((sizeof(int32_t) << 3) - 1)) - 100);
	int const min(-max - 1);
	for(int i = 0; i < c; ++i) {
		*piout++ = rint(psycle::helpers::math::clip(float(min), (*pin++) * 65536.0f, float(max)));
		*piout++ = rint(psycle::helpers::math::clip(float(min), (*pin++) * 65536.0f, float(max)));
	}*/
}

//In -> -8388608.0..8388608.0 in 4 bytes, out -8388608..8388608, aligned to 3 bytes, Big endian
void psy_dsp_quantize24be(float *pin, int *piout, int c)
{
    /*unsigned char *cptr = (unsigned char *) piout;
	for(int i = 0; i < c; ++i) {
		int outval = psycle::helpers::math::rint_clip<int32_t, 24>((*pin++) * 256.0f);
		*cptr++ = (unsigned char) ((outval >> 16) & 0xFF);
		*cptr++ = (unsigned char) ((outval >> 8) & 0xFF);
		*cptr++ = (unsigned char) (outval & 0xFF);

		outval = psycle::helpers::math::rint_clip<int32_t, 24>((*pin++) * 256.0f);
		*cptr++ = (unsigned char) ((outval >> 16) & 0xFF);
		*cptr++ = (unsigned char) ((outval >> 8) & 0xFF);
		*cptr++ = (unsigned char) (outval & 0xFF);
	}*/
}

//In -> -8388608.0..8388608.0 in 4 bytes, out -8388608..8388608, aligned to 3 bytes, little endian
void psy_dsp_quantize24le(float *pin, int *piout, int c)
{
/*    unsigned char *cptr = (unsigned char *) piout;
	for(int i = 0; i < c; ++i) {
		int outval = psycle::helpers::math::rint_clip<int32_t, 24>((*pin++) * 256.0f);
		*cptr++ = (unsigned char) (outval & 0xFF);
		*cptr++ = (unsigned char) ((outval >> 8) & 0xFF);
		*cptr++ = (unsigned char) ((outval >> 16) & 0xFF);

		outval = psycle::helpers::math::rint_clip<int32_t, 24>((*pin++) * 256.0f);
		*cptr++ = (unsigned char) (outval & 0xFF);
		*cptr++ = (unsigned char) ((outval >> 8) & 0xFF);
		*cptr++ = (unsigned char) ((outval >> 16) & 0xFF);
	}*/
}

//In -> -32768..32767 stereo interlaced, out -32768.0..32767.0 stereo deinterlaced
void psy_dsp_dequantize16anddeinterlace(short int *pin, float *poutleft,float *poutright,int c)
{
	do
	{
		*poutleft++ = *pin++;
		*poutright++ = *pin++;
	}
	while(--c);
}

//In -> -2147483648..2147483647 stereo interlaced, out -32768.0..32767.0 stereo deinterlaced
void psy_dsp_dequantize32anddeinterlace(int *pin, float *poutleft,float *poutright,int c)
{
	do
	{
		*poutleft++ = (float)((*pin++)*0.0000152587890625);
		*poutright++ = (float)((*pin++)*0.0000152587890625);
	}
	while(--c);
}

//In -> -1.0..1.0 stereo interlaced, out -32768.0..32767.0 stereo deinterlaced
void psy_dsp_deinterlacefloat(float *pin, float *poutleft,float *poutright,int c)
{
	do
	{
		*poutleft++ = (*pin++)*32768.f;
		*poutright++ = (*pin++)*32768.f;
	}
	while(--c);
}

