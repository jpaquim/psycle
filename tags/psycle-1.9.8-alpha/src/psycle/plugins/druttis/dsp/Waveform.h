//////////////////////////////////////////////////////////////////////
//
//	Waveform.h
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include "DspMath.h"
//////////////////////////////////////////////////////////////////////
//
//	Constants
//
//////////////////////////////////////////////////////////////////////
//
//
#define WAVESIBI 8
#define WAVESIZE 256
#define WAVEMASK 255
#define WAVEFSIZE 256.0f
//
//	Waveform constants
#define WF_BLANK 0
#define WF_SINE 1
#define WF_BLTRIANGLE 2
#define WF_BLSQUARE 3
#define WF_BLSAWTOOTH 4
#define WF_BLPARABOLA 5
#define WF_TRIANGLE 6
#define WF_SQUARE 7
#define WF_SAWTOOTH 8
#define WF_REVSAWTOOTH 9
//////////////////////////////////////////////////////////////////////
//
//	WAVEFORM type
//
//////////////////////////////////////////////////////////////////////
typedef struct waveform_t
{
	int			index;		// Index of waveform (wavenumber)
	int			count;		// How many shares this wave now.
	char		*pname;		// Name :)
	float		*pdata;		// Data, partial or non partial
	int			*preverse;	// Lookup table to find pdata offset
}
WAVEFORM;
//////////////////////////////////////////////////////////////////////
//
//	WAVEFORM type
//
//////////////////////////////////////////////////////////////////////
extern float incr2freq;
//////////////////////////////////////////////////////////////////////
//
//	Waveform class
//
//////////////////////////////////////////////////////////////////////
class Waveform
{
	//////////////////////////////////////////////////////////////////
	//
	//	Internal variables
	//
	//////////////////////////////////////////////////////////////////
private:
	WAVEFORM	m_wave;
	//////////////////////////////////////////////////////////////////
	//
	//	Constructor / Destructor
	//
	//////////////////////////////////////////////////////////////////
public:
	Waveform();
	~Waveform();
	//////////////////////////////////////////////////////////////////
	//
	//	Methods
	//
	//////////////////////////////////////////////////////////////////
	__forceinline WAVEFORM *Get()
	{
		return &m_wave;
	}
	bool Get(int index);
	//////////////////////////////////////////////////////////////////
	//
	//	Returns a linear interpolated sample by phase (NO BANDLIMIT)
	//
	//////////////////////////////////////////////////////////////////
	__forceinline float GetSample(float phase)
	{
		register int offset = f2i(phase);
		const float frac = phase - (float) offset;
		const float out = m_wave.pdata[offset & WAVEMASK];
		return out + (m_wave.pdata[++offset & WAVEMASK] - out) * frac;
	}
	//////////////////////////////////////////////////////////////////
	//
	//	Returns a linear interpolated sample by phase and index
	//
	//////////////////////////////////////////////////////////////////
	#pragma warning( disable : 4035 )
	__forceinline float GetSample(float phase, int index)
	{
/*		__asm
		{
			MOV		EAX, DWORD PTR [index]
			
			SHL		EAX, WAVESIBI
			LEA		EDX, 
		}*/
		const float *pdata = &m_wave.pdata[m_wave.preverse[index] << WAVESIBI];
		register int offset = f2i(phase);
		const float frac = phase - (float) offset;
		const float out = pdata[offset & WAVEMASK];
		return out + (pdata[++offset & WAVEMASK] - out) * frac;
	}
	#pragma warning( default : 4035 )
	//////////////////////////////////////////////////////////////////
	//
	//	Returns a linear interpolated sample by phase and incr
	//
	//////////////////////////////////////////////////////////////////
	__forceinline float GetSample(float phase, float incr)
	{
		return GetSample(phase, f2i(incr * incr2freq) & 0xffff);
	}
};
