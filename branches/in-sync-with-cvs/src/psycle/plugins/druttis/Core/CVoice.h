//============================================================================
//
//	CVoice.h
//	--------
//	druttis@darkface.pp.se
//
//============================================================================
#pragma once
#include <math.h>
#include "..\CDsp.h"
#include "..\wtfmlib.h"
//============================================================================
//	Voice globals
//============================================================================
typedef struct globals_t
{
	//
	//	This should always be here
	int			samplingrate;
	//
	//	Declare your global synth variables here

} GLOBALS;
//============================================================================
//	CVoice class
//============================================================================
class CVoice
{
	//------------------------------------------------------------------------
	//	Data
	//------------------------------------------------------------------------
public:
	//
	//	These two should always be here
	GLOBALS		*globals;
	int			ticks_remaining;
	//	
	//	Declare your runtime variables here
	float		phase;
	float		speed;
	bool		active;
	//------------------------------------------------------------------------
	//	Methods
	//------------------------------------------------------------------------
public:
	CVoice();
	~CVoice();
	void Stop();
	void NoteOff();
	void NoteOn(int note, int volume);
	//------------------------------------------------------------------------
	//	IsFinished
	//	returns true if voice is done playing
	//------------------------------------------------------------------------
	__forceinline bool IsActive()
	{
		return active;
	}
	//------------------------------------------------------------------------
	//	GlobalTick
	//	Method to handle parameter inertia and suchs things
	//------------------------------------------------------------------------
	__forceinline static void GlobalTick()
	{
	}
	//------------------------------------------------------------------------
	//	VoiceTick
	//	Method to handle voice specific things as LFO and envelopes
	//	* tips, dont handle amplitude envelopes or lfo's here
	//------------------------------------------------------------------------
	__forceinline void VoiceTick()
	{
	}
	//------------------------------------------------------------------------
	//	Work
	//	all sound generation is done here
	//------------------------------------------------------------------------
	__forceinline void Work(float *psamplesleft, float *psamplesright, int numsamples)
	{
		float out;
		do {
			out = (float) sin(phase * PI2) * 16384.0f;
			phase += speed;
			*++psamplesleft += out;
			*++psamplesright += out;
		} while (--numsamples);
		phase -= (float) f2i(phase);
	}
};
