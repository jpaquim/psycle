//============================================================================
//	CDsp.h
//	------
//	Digital Signal Processing Library using Floating Point
//	by Joakim Dahlström a.k.a. Druttis
//============================================================================
#pragma once
#include <math.h>
//=============================================================================
//	Get rid of (Warning, method does not return a god damn value)
//=============================================================================
#pragma warning ( disable : 4035 )
//=============================================================================
//	Constants
//=============================================================================
const double PI2 = 6.28318530717958647693;
//=============================================================================
//	Defines
//=============================================================================
#define ALIGNIT __asm nop; __asm align 16
//=============================================================================
//	Filter things
//=============================================================================
#pragma pack (1)
typedef struct filter_t
{
	float	fa;
	float	fb;
	float	buf0;
	float	buf1;
} FILTER;
#pragma pack ( )
//=============================================================================
//	The class!
//=============================================================================
class CDsp
{
public:
	//------------------------------------------------------------------------
	//	CreateWavetable
	//------------------------------------------------------------------------
	static void CreateWavetable(float *coeffs, int ncoeffs, float *psamples, int nsamples);
	//------------------------------------------------------------------------
	//	GetFreq
	//------------------------------------------------------------------------
	static __forceinline float GetFreq(float note, int factor, int samplerate)
	{
		return (float) factor * 440.0f * (float) pow(2.0, (note - 69.0) / 12.0) / (float) samplerate;
	}
	//------------------------------------------------------------------------
	//	Fill : a[0-n] = v
	//------------------------------------------------------------------------
	static __forceinline void Fill(float *a, float v, int n)
	{
		--a;
		do {
			*++a = v;
		} while (--n);
	}
	//------------------------------------------------------------------------
	//	Add : a[0-n] = a[0-n] + b[0-n]
	//------------------------------------------------------------------------
	static __forceinline void Add(float *a, float *b, int n)
	{
		--a;
		--b;
		do {
			*++a += *++b;
		} while (--n);
	}
	//------------------------------------------------------------------------
	//	Sub : a[0-n] = a[0-n] - b[0-n]
	//------------------------------------------------------------------------
	static __forceinline void Sub(float *a, float *b, int n)
	{
		--a;
		--b;
		do {
			*++a -= *++b;
		} while (--n);
	}
	//------------------------------------------------------------------------
	//	Mul : a[0-n] = a[0-n] * b[0-n]
	//------------------------------------------------------------------------
	static __forceinline void Mul(float *a, float *b, int n)
	{
		--a;
		--b;
		do {
			*++a *= *++b;
		} while (--n);
	}
	//------------------------------------------------------------------------
	//	Add : a[0-n] = a[0-n] + v
	//------------------------------------------------------------------------
	static __forceinline void Add(float *a, float v, int n)
	{
		--a;
		do {
			*++a += v;
		} while (--n);
	}
	//------------------------------------------------------------------------
	//	Sub : a[0-n] = a[0-n] - v
	//------------------------------------------------------------------------
	static __forceinline void Sub(float *a, float v, int n)
	{
		--a;
		do {
			*++a -= v;
		} while (--n);
	}
	//------------------------------------------------------------------------
	//	Mul : a[0-n] = a[0-n] * v
	//------------------------------------------------------------------------
	static __forceinline void Mul(float *a, float v, int n)
	{
		--a;
		do {
			*++a *= v;
		} while (--n);
	}
	//------------------------------------------------------------------------
	//	LPFilter12 (Lowpass 12 db)
	//------------------------------------------------------------------------
	static __forceinline float LPFilter12(FILTER *f, float in)
	{
		f->buf0 = f->buf0 + f->fa * (in - f->buf0 + f->fb * (f->buf0 - f->buf1));
		f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
		return f->buf1;
	}

	static __forceinline void LPFilter12(FILTER *f, float *in, int nsamples)
	{
		--in;
		do {
			f->buf0 = f->buf0 + f->fa * (*++in - f->buf0 + f->fb * (f->buf0 - f->buf1));
			f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
			*in = f->buf1;
		} while (--nsamples);
	}
	//------------------------------------------------------------------------
	//	LPFilter24 (Lowpass 24 db)
	//------------------------------------------------------------------------
	static __forceinline float LPFilter24(FILTER *f, float in)
	{
		f->buf0 = f->buf0 + f->fa * (in - f->buf0 + f->fb * (f->buf0 - f->buf1));
		f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
		f->buf0 = f->buf0 + f->fa * (f->buf1 - f->buf0 + f->fb * (f->buf0 - f->buf1));
		f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
		return f->buf1;
	}

	static __forceinline void LPFilter24(FILTER *f, float *in, int nsamples)
	{
		--in;
		do {
			f->buf0 = f->buf0 + f->fa * (*++in - f->buf0 + f->fb * (f->buf0 - f->buf1));
			f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
			*in = f->buf1;
			f->buf0 = f->buf0 + f->fa * (*in - f->buf0 + f->fb * (f->buf0 - f->buf1));
			f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
			*in = f->buf1;
		} while (--nsamples);
	}
	//------------------------------------------------------------------------
	//	LPFilter36 (Lowpass 36 db)
	//------------------------------------------------------------------------
	static __forceinline float LPFilter36(FILTER *f, float in)
	{
		f->buf0 = f->buf0 + f->fa * (in - f->buf0 + f->fb * (f->buf0 - f->buf1));
		f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
		f->buf0 = f->buf0 + f->fa * (f->buf1 - f->buf0 + f->fb * (f->buf0 - f->buf1));
		f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
		f->buf0 = f->buf0 + f->fa * (f->buf1 - f->buf0 + f->fb * (f->buf0 - f->buf1));
		f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
		return f->buf1;
	}

	static __forceinline void LPFilter36(FILTER *f, float *in, int nsamples)
	{
		--in;
		do {
			f->buf0 = f->buf0 + f->fa * (*++in - f->buf0 + f->fb * (f->buf0 - f->buf1));
			f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
			*in = f->buf1;
			f->buf0 = f->buf0 + f->fa * (*in - f->buf0 + f->fb * (f->buf0 - f->buf1));
			f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
			*in = f->buf1;
			f->buf0 = f->buf0 + f->fa * (*in - f->buf0 + f->fb * (f->buf0 - f->buf1));
			f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
			*in = f->buf1;
		} while (--nsamples);
	}
	//------------------------------------------------------------------------
	//	HPFilter (Highpass 12 db)
	//------------------------------------------------------------------------
	static __forceinline float HPFilter12(FILTER *f, float in)
	{
		f->buf0 = f->buf0 + f->fa * (in - f->buf0 + f->fb * (f->buf0 - f->buf1));
		f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
		return in - f->buf1;
	}

	static __forceinline void HPFilter12(FILTER *f, float *in, int nsamples)
	{
		--in;
		do {
			f->buf0 = f->buf0 + f->fa * (*++in - f->buf0 + f->fb * (f->buf0 - f->buf1));
			f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
			*in -= f->buf1;
		} while (--nsamples);
	}
	//------------------------------------------------------------------------
	//	HPFilter (Highpass 24 db)
	//------------------------------------------------------------------------
	static __forceinline float HPFilter24(FILTER *f, float in)
	{
		f->buf0 = f->buf0 + f->fa * (in - f->buf0 + f->fb * (f->buf0 - f->buf1));
		f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
		in -= f->buf1;
		f->buf0 = f->buf0 + f->fa * (in - f->buf0 + f->fb * (f->buf0 - f->buf1));
		f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
		return in - f->buf1;
	}

	static __forceinline void HPFilter24(FILTER *f, float *in, int nsamples)
	{
		--in;
		do {
			f->buf0 = f->buf0 + f->fa * (*++in - f->buf0 + f->fb * (f->buf0 - f->buf1));
			f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
			*in -= f->buf1;
			f->buf0 = f->buf0 + f->fa * (*in - f->buf0 + f->fb * (f->buf0 - f->buf1));
			f->buf1 = f->buf1 + f->fa * (f->buf0 - f->buf1);
			*in -= f->buf1;
		} while (--nsamples);
	}

	static __forceinline void InitFilter(FILTER *f, float freq, float res)
	{
		f->fa = freq;
		if (f->fa < 0.01f)
			f->fa = 0.01f;
		if (f->fa > 0.99f)
			f->fa = 0.99f;
		f->fb = res + res / (1.0f - f->fa);
	}

	static __forceinline unsigned long GetRandomNumber()
	{ 
		static unsigned long randSeed = 22222; 
		randSeed = (randSeed * 196314165) + 907633515; 
		return randSeed; 
	}

	static __forceinline float GetRandomSignal()
	{
		return (float) ((int) GetRandomNumber()  & 0xffff) * 0.000030517578125f - 1.0f;
	}
};
