/* -*- mode:c++, indent-tabs-mode:t -*- */
/*      Copyright (C) 2002 Vincenzo Demasi.

		This plugin is free software; you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation; either version 2 of the License, or
		(at your option) any later version.\n"\

		This plugin is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU General Public License for more details.

		You should have received a copy of the GNU General Public License
		along with this program; if not, write to the Free Software
		Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

		Vincenzo Demasi. E-Mail: <v.demasi@tiscali.it>
*/

#include <packageneric/pre-compiled.private.hpp>
#include "vdAllPass.hpp"
#include <stdio.h>

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	// The constructor zone
	Vals = new int[PARNUM];
}

mi::~mi()
{
	// Destroy dinamically allocated objects/memory here
	delete[] Vals;
}

void mi::Init()
{
	// Initialize your stuff here
	lastDelayModified = LDELAY;
	lastGainModified = LGAIN;
	lGain = rGain = 0.0f;
}

void mi::Command()
{
	// Called when user presses editor button
	// Probably you want to show your custom window here
	// or an about button
	pCB->MessBox(VDABOUTMESSAGE, VDABOUTCAPTION(VDPLUGINNAME), VDABOUTYPE);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par] = val;
	switch(par)
	{
	case LDELAY:
		leftFilter.setDelay(val);
		if(Vals[LOCKDELAY])
		{
			Vals[RDELAY] = val;
			rightFilter.setDelay(val);
		}
		lastDelayModified = LDELAY;
		break;
	case RDELAY:
		rightFilter.setDelay(val);
		if(Vals[LOCKDELAY])
		{
			Vals[LDELAY] = val;
			leftFilter.setDelay(val);
		}
		lastDelayModified = RDELAY;
		break;
	case LGAIN:
		leftFilter.setGain(lGain = val / GAIN_NORM);
		if(Vals[LOCKGAIN])
		{
			Vals[RGAIN] = val;
			rightFilter.setGain(rGain = lGain);
		}
		lastGainModified = LGAIN;
		break;
	case RGAIN:
		rightFilter.setGain(rGain = val / GAIN_NORM);
		if(Vals[LOCKGAIN])
		{
			Vals[LGAIN] = val;
			leftFilter.setGain(lGain = rGain);
		}
		lastGainModified = RGAIN;
		break;
	case LOCKDELAY:
		if(val)
			if(lastDelayModified == LDELAY)
			{
				Vals[RDELAY] = Vals[LDELAY];
				rightFilter.setDelay(Vals[RDELAY]);
			}
			else
			{
				Vals[LDELAY] = Vals[RDELAY];
				leftFilter.setDelay(Vals[LDELAY]);
			}
		break;
	case LOCKGAIN:
		if(val)
			if(lastGainModified == LGAIN)
			{
				Vals[RGAIN] = Vals[LGAIN];
				rightFilter.setGain(rGain = lGain);
			}
			else
			{
				Vals[LGAIN] = Vals[RGAIN];
				leftFilter.setGain(lGain = rGain);
			}
		break;
	}
}

void mi::SequencerTick()
{
	// Called on each tick while sequencer is playing
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char *txt,int const param, int const value)
{
	switch (param)
	{
	case LDELAY:
	case RDELAY: sprintf(txt, "%d (%.1f ms)", value, value / 44.100f); return true;
	case LGAIN: sprintf(txt, "%.2f", lGain); return true;
	case RGAIN: sprintf(txt, "%.2f", rGain); return true;
	case LOCKDELAY:
	case LOCKGAIN: sprintf(txt, "%s", value ? "on" : "off"); return true;
	}
	return false;
}

void mi::Work(float *pleftsamples, float *prightsamples , int samplesnum, int tracks)
{
	do
	{
		*pleftsamples = leftFilter.process(*pleftsamples);
		*prightsamples = rightFilter.process(*prightsamples);
		++pleftsamples;
		++prightsamples;
	}
	while(--samplesnum);
}
