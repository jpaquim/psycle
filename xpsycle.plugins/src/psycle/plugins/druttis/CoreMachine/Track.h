//////////////////////////////////////////////////////////////////////
//
//	Track.h
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Globals.h"
#include "../DspLib/Dsp.h"
#include "../DspLib/DspAlgs.h"

//////////////////////////////////////////////////////////////////////
//
//	Track class
//
//////////////////////////////////////////////////////////////////////

class Track
{

	//////////////////////////////////////////////////////////////////
	//
	//	Variables
	//
	//////////////////////////////////////////////////////////////////

public:

	GLOBALS		*globals;
	int			ticks_remaining;

	/* Custom fields here */

	float		phase;
	float		incr;
	bool		active;


	//////////////////////////////////////////////////////////////////
	//
	//	Methods
	//
	//////////////////////////////////////////////////////////////////

public:

	Track();
	virtual ~Track();
	void NoteStop();
	void NoteOff();
	void NoteOn(int note, float velocity);
	void NoteCommand(int cmd, int val);

	//////////////////////////////////////////////////////////////////
	//
	//	IsActive
	//
	//////////////////////////////////////////////////////////////////
	
	inline bool IsActive()
	{
		return active;
	}

	//////////////////////////////////////////////////////////////////
	//
	//	Update
	//
	//////////////////////////////////////////////////////////////////

	inline void Update()
	{
	}

	//////////////////////////////////////////////////////////////////
	//
	//	Work
	//
	//////////////////////////////////////////////////////////////////

	inline void Work(float *psamplesleft, float *psamplesright, int numsamples)
	{
		float out;
		do {
			out = get_sample_l(globals->wtbl, phase, 4095);
			phase = fand(phase + incr, 4095);
			out *= 16384.0f;
			*++psamplesleft += out;
			*++psamplesright += out;
		} while (--numsamples);
	}

};
