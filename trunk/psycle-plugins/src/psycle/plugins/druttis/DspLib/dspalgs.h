/* -*- mode:c++, indent-tabs-mode:t -*- */
//////////////////////////////////////////////////////////////////////
//
//				dspalgs.h
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <math.h>

//////////////////////////////////////////////////////////////////////
//
//				Defines
//
//////////////////////////////////////////////////////////////////////

#define PI 3.14159265f
#define TWOPI 6.28318531f

#define     SPTS    (16)
#define     SPDS    (6)
#define     SBW     0.9

//////////////////////////////////////////////////////////////////////
//
//				generate_wave
//
//////////////////////////////////////////////////////////////////////

void generate_wave(float *psamples, int nsamples, float *pcoeffs, int ncoeffs);

//////////////////////////////////////////////////////////////////////
//
//				downsample
//
//////////////////////////////////////////////////////////////////////

inline void downsample(float *in, float *out, int nsamples, int levels, float *sinc)
{
	int count;
	float tmp;
	//float fac = 1.0f / (float) levels;
	float *snc;
	
	--in;
	--out;
	--sinc;

	do {

		tmp = 0.0f;
		count = levels;
		snc = sinc;

		do {
			tmp += *++in * *++snc;
		} while (--count);

		*++out = tmp;

	} while (--nsamples);
}

//////////////////////////////////////////////////////////////////////
//
//				v2m
//
//				Computes a (2^n)-1 value
//
//////////////////////////////////////////////////////////////////////

inline int v2m(int v)
{
	int m = 0;
	while (m < v)
		m = (m << 1) + 1;
	return m;
}

//////////////////////////////////////////////////////////////////////
//
//				get_freq
//
//////////////////////////////////////////////////////////////////////

inline float get_freq(float note, int size, int samplerate)
{
	return (float) size * 440.0f * (float) pow(2.0, (note - 69.0) / 12.0) / (float) samplerate;
}

//////////////////////////////////////////////////////////////////////
//
//				f2i
//
//				Another version of float to int conversion
//
//				Positive numbers are truncated while negative numbers are
//				truncated - 1, (f2i(-1.25) = -2 etc...)
//
//////////////////////////////////////////////////////////////////////

extern unsigned short cwTrunc;

inline int f__2i(float x)
{ 
	#if defined _MSC_VER && defined _M_IX86
	__asm
	{
		FLD								DWORD PTR [x]
		FLDCW				WORD PTR [cwTrunc]
		FISTP				DWORD PTR [x]
		NOP
		MOV								EAX, DWORD PTR [x]
	}
	#else
		return static_cast<int>(x - 0.5f);
	#endif
}

//////////////////////////////////////////////////////////////////////
//
//				floor2int : fast floor thing
//
//////////////////////////////////////////////////////////////////////

inline int f2i(float x)
{ 
	#if defined _MSC_VER && defined _M_IX86
	__asm
	{
		FLD								DWORD PTR [x]
		FIST				DWORD PTR [ESP-8]
		SUB								ESP, 8
		FISUB				DWORD PTR [ESP]
		NOP
		FSTP				DWORD PTR [ESP+4]
		POP								EAX
		POP								EDX
		ADD								EDX, 7FFFFFFFH
		SBB								EAX, 0
	}
	#else
		return std::floor(x);
	#endif
}

//////////////////////////////////////////////////////////////////////
//
//				fand
//
//				Floating point and operator emulator :)
//
//				This works only because of the call to f2i.
//
//////////////////////////////////////////////////////////////////////

inline float fand(float phase, int mask)
{
	int pos = f2i(phase);
	float frac = phase - (float) pos;
	return (float) (pos & mask) + frac;
}

//////////////////////////////////////////////////////////////////////
//
//				get_sample_n
//
//				Retrieve a sample with no particular algorithm
//
//////////////////////////////////////////////////////////////////////

inline float get_sample_n(float *samples, float phase, int mask)
{
	return samples[f2i(phase) & mask];
}

//////////////////////////////////////////////////////////////////////
//
//				get_sample_l
//
//				Retrieve a sample with linear interpollation algorithm
//
//////////////////////////////////////////////////////////////////////

inline float get_sample_l(float *samples, float phase, int mask)
{
	int pos = f2i(phase);
	float frac = phase - (float) pos;
	float out = samples[pos & mask];
	++pos;
	return out + frac * (samples[pos & mask] - out);
}
