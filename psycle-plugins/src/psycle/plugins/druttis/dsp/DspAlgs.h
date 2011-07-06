//////////////////////////////////////////////////////////////////////
//
//				dspalgs.h
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include <cmath>

//////////////////////////////////////////////////////////////////////
//
//				Defines
//
//////////////////////////////////////////////////////////////////////

#define PI 3.14159265f

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
//				floor2int : fast floor thing
//
//////////////////////////////////////////////////////////////////////

#include <diversalis/compiler.hpp>
inline int f2i(float x) { 
	#ifdef DIVERSALIS__COMPILER__FEATURE__WARNING
		#warning "portability: someone come back and centralise this"
	#else
		#pragma message(__FILE__ "(" DIVERSALIS__STRINGIZE(__LINE__) ") : warning: portability: someone come back and centralise this")
	#endif
	#if defined _MSC_VER && defined _M_IX86
		__asm {
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
