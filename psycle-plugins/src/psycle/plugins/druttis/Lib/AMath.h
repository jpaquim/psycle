/* -*- mode:c++, indent-tabs-mode:t -*- */
//============================================================================
//
//				AMath.h
//
//				druttis@darkface.pp.se
//
//============================================================================
#pragma once
#include <cmath>
//=============================================================================
//				Constants
//=============================================================================
const double PI2 = 6.28318530717958647693;
//============================================================================
//				Function to replace ordinary float to int operation
//============================================================================
inline int f2i(double d)
{
#ifdef __BIG_ENDIAN__
  return lrintf(d - 0.5);
#else
	const double magic = 6755399441055744.0;
	union tmp_union
	{
		double d;
		int i;
	} tmp;
	tmp.d = (d - 0.5) + magic;
	return tmp.i;
#endif
}
//=============================================================================
//				Returns frequency of a note (midi note?)
//=============================================================================
static inline float GetFreq(float note)
{
	return (float) 440.0f * (float) pow(2.0, (note - 69.0) / 12.0);
}
//=============================================================================
//				GetRandomLong
//=============================================================================
static inline unsigned long GetRandomLong()
{ 
	static unsigned long randSeed = 22222; 
	randSeed = (randSeed * 196314165) + 907633515; 
	return randSeed; 
}
//=============================================================================
//				GetRandomSignal
//=============================================================================
static inline float GetRandomSignal()
{
	return (float) ((int) GetRandomLong()  & 0xffff) * 0.000030517578125f - 1.0f;
}
