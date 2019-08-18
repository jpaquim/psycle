// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "operations.h"
#include <string.h>

void dsp_add(float *src, float *dst, int num, float vol)
{
	--src;
	--dst;
	do
	{
		*++dst += *++src*vol;
	}
	while (--num);
}
	
void dsp_mul(float *dst, int num, float mul)
{
	--dst;
	do 
	{
		*++dst *= mul;
	}
	while (--num);
}
	
void dsp_movmul(float *src, float *dst, int num, float mul)
{
	--src;
	--dst;
	do
	{
		*++dst = *++src*mul;
	}
	while (--num);
}
	
void dsp_clear(float *dst, int num)
{
	memset(dst,0,num*sizeof(float));
}
