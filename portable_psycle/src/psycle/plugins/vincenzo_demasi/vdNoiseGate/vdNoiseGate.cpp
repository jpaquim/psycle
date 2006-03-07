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
#include <stdio.h>
#include <math.h>
#include "vdNoiseGate.hpp"

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
	case LTHRESHOLD:
		lThreshold = (float)val;
		if(Vals[LOCKTHRESHOLD])
		{
			Vals[RTHRESHOLD] = val;
			rThreshold = (float)val;
		}
		lastThresholdModified = LTHRESHOLD;
		break;
	case RTHRESHOLD:
		rThreshold = (float)val;
		if(Vals[LOCKTHRESHOLD])
		{
			Vals[LTHRESHOLD] = val;
			lThreshold = (float)val;
		}
		lastThresholdModified = RTHRESHOLD;
		break;
	case LOCKTHRESHOLD:
		if(val)
			if(lastThresholdModified == LTHRESHOLD)
			{
				Vals[RTHRESHOLD] = Vals[LTHRESHOLD];
				rThreshold = lThreshold;
			}
			else
			{
				Vals[LTHRESHOLD] = Vals[RTHRESHOLD];
				lThreshold = rThreshold;
			}
		break;
	case CUTOFF:
		leftFilter.setCutOff(val / GAIN_NORM);
		rightFilter.setCutOff(val / GAIN_NORM);
		break;
	case RESONANCE:
		leftFilter.setResonance(val / GAIN_NORM);
		rightFilter.setResonance(val / GAIN_NORM);
		break;
	case LOWPASS:
		lowPassOn = val;
		break;
	case PREGAIN:
		pregain = Vals[PREGAIN] / GAIN_NORM; break;
	case GAIN:
		gain = Vals[GAIN] / GAIN_NORM; break;
	}
}

void mi::SequencerTick()
{
	// Called on each tick while sequencer is playing
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char *txt,int const param, int const value)
{
	switch(param)
	{
	case LTHRESHOLD:
	case RTHRESHOLD:
		sprintf(txt, "%d", value); return true;
	case LOCKTHRESHOLD:
	case LOWPASS:
		sprintf(txt, "%s", value ? "on" : "off"); return true;
	case CUTOFF:
	case RESONANCE:
		sprintf(txt, "0.%d", value); return true;
	case PREGAIN:
	case GAIN:
		sprintf(txt, "%d%%", value); return true;
	}

	return false;
}

void mi::Work(float *pleftsamples, float *prightsamples , int samplesnum, int tracks)
{
	if(lowPassOn)
		do
		{
			*pleftsamples *= pregain;
			if(*pleftsamples >= -lThreshold && *pleftsamples <= lThreshold)
				*pleftsamples = 0.001f; // if set to 0.0f lowpass filter is cpu hungry
			else
				*pleftsamples *= gain;
			*prightsamples *= pregain;
			if(*prightsamples >= -rThreshold && *prightsamples <= rThreshold)
				*prightsamples = 0.001f;
			else
				*prightsamples *= gain;
			*pleftsamples = leftFilter.process(*pleftsamples);
			*prightsamples = rightFilter.process(*prightsamples);
			++pleftsamples;
			++prightsamples;
		}
		while(--samplesnum);
	else
		do
		{
			*pleftsamples *= pregain;
			if(*pleftsamples >= -lThreshold && *pleftsamples <= lThreshold)
				*pleftsamples = 0.0f;
			else
				*pleftsamples *= gain;
			*prightsamples *= pregain;
			if(*prightsamples >= -rThreshold && *prightsamples <= rThreshold)
				*prightsamples = 0.0f;
			else
				*prightsamples *= gain;
			++pleftsamples;
			++prightsamples;
		}
	while(--samplesnum);
}
