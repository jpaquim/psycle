//////////////////////////////////////////////////////////////////////
//
//	DspMath.h
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include <cmath>
//////////////////////////////////////////////////////////////////////
//
//	PI constants
//
//////////////////////////////////////////////////////////////////////
#define PI		3.1415926536f
#define PI2		6.2831853072f
#define HALFPI	1.5707963268f
//////////////////////////////////////////////////////////////////////
//
//	pow2 constants
//
//////////////////////////////////////////////////////////////////////
#define	POW2TABLESIZE	16384
#define POW2TABLEMASK	16383
extern const float POW2TABLEFACT;
//////////////////////////////////////////////////////////////////////
//
//	pow2 table
//
//////////////////////////////////////////////////////////////////////
extern float pow2table[POW2TABLESIZE];
//////////////////////////////////////////////////////////////////////
//
//	float2int :
//
//////////////////////////////////////////////////////////////////////
extern unsigned short cwTrunc;
extern const double fimagic;
extern const double fihalf;
extern double fitmp;

inline int f2i(double x)
{
	fitmp = x - fihalf + fimagic;
	return *(int *) &fitmp;
}

//////////////////////////////////////////////////////////////////////
//
//	floor2int : fast floor thing
//
//////////////////////////////////////////////////////////////////////
inline int floor2i(float x)
{
	#if defined _MSC_VER && defined _M_IX86
	__asm
	{
		FLD		DWORD PTR [x]
		FIST	DWORD PTR [ESP-8]
		SUB		ESP, 8
		FISUB	DWORD PTR [ESP]
		NOP
		FSTP	DWORD PTR [ESP+4]
		POP		EAX
		POP		EDX
		ADD		EDX, 7FFFFFFFH
		SBB		EAX, 0 // [bohan] does the compiler understand that, since there no return statement? ... this code might be what's making druttis plugins behave weirdly when built with msvc 8
	}
	#else
		return std::floor(x);
	#endif
}
//////////////////////////////////////////////////////////////////////
//
//	fand : like fmod(x, m - 1) but faster and handles neg. x
//
//////////////////////////////////////////////////////////////////////
inline float fand(float val, int mask)
{
	const int index = floor2i(val);
	return (float) (index & mask) + (val - (float) index);
}
//////////////////////////////////////////////////////////////////////
//
//	fastexp
//
//////////////////////////////////////////////////////////////////////
#pragma warning( disable : 4035 )
inline float fastexp(double x)
{
	#if defined _MSC_VER && defined _M_IX86
	__asm
	{
        FLDL2E
        FLD     QWORD PTR [x]				  ; x
        FMUL                                  ; z = x*log2(e)
        FIST    DWORD PTR [x]                 ; round(z)
        SUB     ESP, 12

        MOV     DWORD PTR [ESP], 0
        MOV     DWORD PTR [ESP+4], 80000000H
        FISUB   DWORD PTR [x]                 ; z - round(z)
        MOV     EAX, DWORD PTR [x]
        ADD     EAX,3FFFH
        MOV     [ESP+8],EAX
        JLE     my_underflow
        CMP     EAX,8000H
        JGE     my_overflow
        F2XM1
        FLD1
        FADD                                  ; 2^(z-round(z))
        FLD     TBYTE PTR [ESP]               ; 2^(round(z))

        ADD     ESP,12
        FMUL                                  ; 2^z = e^x
        JMP		my_end
my_underflow:
        FSTP    ST
        FLDZ                                  ; return 0
        ADD     ESP,12
        JMP		my_end
my_overflow:
        PUSH    07F800000H                    ; +infinity
        FSTP    ST
        FLD     DWORD PTR [ESP]               ; return infinity
        ADD     ESP,16
my_end:
	}
	#else
		return std::exp(x);
	#endif
}
#pragma warning( default : 4035 )
//////////////////////////////////////////////////////////////////////
//
//	fastpow2 : fast, using lookup table, handles negative exponents
//
//////////////////////////////////////////////////////////////////////
inline float fastpow2(float x)
{
	long *px = (long*) &x;
	const long  lx = floor2i(x);
	const float dx = x - (float) lx;
	x = pow2table[int(dx * POW2TABLEFACT)];
	*px += (lx<<23);
	return x;
}
//////////////////////////////////////////////////////////////////////
//
//	Converts milliseconds to samples
//
//////////////////////////////////////////////////////////////////////
inline int millis2samples(int ms, int samplerate)
{
	return ms * samplerate / 1000;
}
//////////////////////////////////////////////////////////////////////
//
//	Converts a note to a phase increment
//
//////////////////////////////////////////////////////////////////////
inline float note2incr(int size, float note, int samplerate)
{
	return (float) size * 440.0f * (float) pow(2.0, (note - 69.0) / 12.0) / (float) samplerate;
}
//////////////////////////////////////////////////////////////////////
//
//	Get random number
//
//////////////////////////////////////////////////////////////////////
inline unsigned long rnd_number()
{ 
	static unsigned long randSeed = 22222; 
	randSeed = (randSeed * 196314165) + 907633515; 
	return randSeed; 
}
//////////////////////////////////////////////////////////////////////
//
//	Get random signal
//
//////////////////////////////////////////////////////////////////////
inline float rnd_signal()
{
	return (float) ((int) rnd_number()  & 0xffff) * 0.000030517578125f - 1.0f;
}
