/* -*- mode:c++, indent-tabs-mode:t -*- */
// ThunderPalace SoftSat - saturator/waveshaper plugin for Psycle

/*
	* Copyright (c) 2005 Thunder Palace Entertainment
	*
	* Permission to use, copy, modify, and distribute this software for any
	* purpose with or without fee is hereby granted, provided that the above
	* copyright notice and this permission notice appear in all copies.
	*
	* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
	* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
	* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
	* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
	* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
	* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
	* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
	*
	* Alternatively, this software may be used under the terms of the GNU
	* General Public License, version 2 or later, as published by the Free
	* Software Foundation.
	*/

// This was made by Catatonic Porpoise <graue@oceanbase.org>

// It is based on the "Arguru's Distortion" machine which was released
// to the public domain.

// This effect uses a simple algorithm that was not invented by me,
// but rather, was posted on www.musicdsp.org by a Bram de Jong, to
// whom I am very grateful. If the link is still valid by the time
// you read this, you can see this original posting here:
//
//   http://www.musicdsp.org/showone.php?id=42

// version 0.2 - threshold now defaults to 32768 rather than to 512

#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

CMachineParameter const paraThreshold =
{
	"Threshold",
	"Threshold level", // description
	16,                // MinValue				
	32768,             // MaxValue
	MPF_STATE,         // Flags
	32768,             // default value
};

CMachineParameter const paraHardness =
{
	"Hardness",
	"Hardness",        // description
	1,                 // MinValue				
	2048,              // MaxValue
	MPF_STATE,         // Flags
	1024,              // default value
};

CMachineParameter const *pParameters[] = 
{ 
	&paraThreshold,
	&paraHardness
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,
	0,                                     // flags
	2,                                     // numParameters
	pParameters,                           // Pointer to parameters
#ifndef NDEBUG
	"ThunderPalace SoftSat (Debug build)", // name
#else
	"ThunderPalace SoftSat",               // name
#endif
	"SoftSat",                             // short name
	"Catatonic Porpoise",                  // author
	"About",                 // A command that is used to display an about box
	3
};


class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init();
	virtual void SequencerTick();
	virtual void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);

private:
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	// The constructor zone
	Vals = new int[2];
}

mi::~mi()
{
	delete Vals;
// Destroy dynamically allocated objects/memory here
}

void mi::Init()
{
// Initialize your stuff here
}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
}

void mi::Command()
{
// Called when user presses editor button
// Show the about box
	pCB->MessBox("Made September 9, 2005 by Catatonic Porpoise","About SoftSat",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par] = val;
}

// Work... where all is cooked
void mi::Work(float *psamplesleft, float *psamplesright, int numsamples, int tracks)
{
	const float gradation = ((float) Vals[1]) / 2049.0f;       // paraHardness
	const float range = ((float) Vals[0]) / ((gradation+1)/2); // paraThreshold

	do
	{
		float sl = *psamplesleft / range;
		float sr = *psamplesright / range;

		if (sl > 0)
		{
			if (sl > gradation)
			{
				sl = gradation + (sl-gradation)
					/ (1+((sl-gradation)/(1-gradation))
						* ((sl-gradation)/(1-gradation)));
			}
			if (sl > 1.0f)
				sl = (gradation + 1.0f) / 2.0f;
		}
		else
		{
			sl = -sl;
			if (sl > gradation)
			{
				sl = gradation + (sl-gradation)
					/ (1+((sl-gradation)/(1-gradation))
						* ((sl-gradation)/(1-gradation)));
			}
			if (sl > 1.0f)
				sl = (gradation + 1.0f) / 2.0f;
			sl = -sl;
		}

		if (sr > 0)
		{
			if (sr > gradation)
			{
				sr = gradation + (sr-gradation)
					/ (1+((sr-gradation)/(1-gradation))
						* ((sr-gradation)/(1-gradation)));
			}
			if (sr > 1.0f)
				sr = (gradation + 1.0f) / 2.0f;
		}
		else
		{
			sr = -sr;
			if (sr > gradation)
			{
				sr = gradation + (sr-gradation)
					/ (1+((sr-gradation)/(1-gradation))
						* ((sr-gradation)/(1-gradation)));
			}
			if (sr > 1.0f)
				sr = (gradation + 1.0f) / 2.0f;
			sr = -sr;
		}

		*psamplesleft = sl * range;
		*psamplesright = sr * range;

		++psamplesleft;
		++psamplesright;

	} while(--numsamples);
}

// Function that describes value on client's display
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	if (param == 1) // hardness
	{
		sprintf(txt, "%.3f", ((float) value) / 2048.0f);
		return true;
	}

	return false; // use default (integer) display mechanism?
}