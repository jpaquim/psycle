// -*- mode:c++; indent-tabs-mode:t -*-
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

#include "vdEcho.hpp"
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
	leftFilter.setDelay(SET_LDELAY);
	rightFilter.setDelay(SET_RDELAY);
	lDryGain = SET_LDRY / GAIN_NORM;
	lWetGain = SET_LWET / GAIN_NORM;
	rDryGain = SET_RDRY / GAIN_NORM;
	rWetGain = SET_RWET / GAIN_NORM;
	lastDelayModified = LDELAY;
	lastDryModified = LDRY;
	lastWetModified = LWET;
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
	switch (par)
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
	case LDRY:
		lDryGain = val / GAIN_NORM;
		if(Vals[LOCKDRY])
		{
			Vals[RDRY] = val;
			rDryGain = lDryGain;
		}
		lastDryModified = LDRY;
		break;
	case LWET:
		lWetGain = Vals[LWET] / GAIN_NORM;
		if(Vals[LOCKWET])
		{
			Vals[RWET] = val;
			rWetGain = lWetGain;
		}
		lastWetModified = LWET;
		break;
	case RDRY:
		rDryGain = Vals[RDRY] / GAIN_NORM;
		if(Vals[LOCKDRY])
		{
			Vals[LDRY] = val;
			lDryGain = rDryGain;
		}
		lastDryModified = RDRY;
		break;
	case RWET:
		rWetGain = Vals[RWET] / GAIN_NORM;
		if(Vals[LOCKWET])
		{
			Vals[LWET] = val;
			lWetGain = rWetGain;
		}
		lastWetModified = RWET;
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
	case LOCKDRY:
		if(val)
			if(lastDryModified == LDRY)
			{
				Vals[RDRY] = Vals[LDRY];
				rDryGain = lDryGain;
			}
			else
			{
				Vals[LDRY] = Vals[RDRY];
				lDryGain = rDryGain;
			}
		break;
	case LOCKWET:
		if(val)
			if(lastWetModified == LWET)
			{
				Vals[RWET] = Vals[LWET];
				rWetGain = lWetGain;
			}
			else
			{
				Vals[LWET] = Vals[RWET];
				lWetGain = rWetGain;
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
	case RDELAY: sprintf(txt, "%d (%.1f ms)", Vals[param], (float)Vals[param] / 44.100f); return true;
	case LWET:
	case LDRY:
	case RWET:
	case RDRY: sprintf(txt, "%d%%", Vals[param]); return true;
	case LOCKDELAY:
	case LOCKDRY:
	case LOCKWET: sprintf(txt, "%s", value ? "on" : "off"); return true;
	}
	return false;
}

void mi::Work(float *pleftsamples, float *prightsamples , int samplesnum, int tracks)
{
	do
	{
		*pleftsamples = *pleftsamples * lDryGain + leftFilter.process(*pleftsamples) * lWetGain;
		*prightsamples = *prightsamples * rDryGain + rightFilter.process(*prightsamples) * rWetGain;
		++pleftsamples;
		++prightsamples;
	}
	while(--samplesnum);
}
