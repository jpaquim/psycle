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

#pragma once

#include <psycle/plugin_interface.hpp>
#include "../vdabout.hpp"
#include "fastverbfilter.hpp"

#define LDELAY       0
#define LFEEDBACK    1
#define RDELAY       2
#define RFEEDBACK    3
#define LOCKDELAY    4
#define LOCKFEEDBACK 5
#define PARNUM       6

#define PARCOLS   3

#define VDPLUGINNAME "vd's fastverb filter"
#define VDSHORTNAME  "FastVerb"
#define VDAUTHOR     "V. Demasi (Built on "__DATE__")"
#define VDCOMMAND    "License"

#define MIN_DELAY  1
#define MAX_DELAY  FASTVERB_FILTER_MAX_DELAY
#define MIN_FEEDBACK   0
#define MAX_FEEDBACK   100
#define SET_LDELAY (MAX_DELAY / 2)
#define SET_RDELAY (MAX_DELAY / 2)
#define SET_LFEEDBACK  (MAX_FEEDBACK / 2)
#define SET_RFEEDBACK  (MAX_FEEDBACK / 2)
#define GAIN_NORM 100.0f
#define MIN_LOCK 0
#define MAX_LOCK 1
#define SET_LOCK 1

CMachineParameter const parLeftDelay =
{
	"Left Delay", "Left Delay (samples)", MIN_DELAY, MAX_DELAY, MPF_STATE, SET_LDELAY
};

CMachineParameter const parLeftFeedback =
{
	"Left Feedback", "Left Feedback (%)", MIN_FEEDBACK, MAX_FEEDBACK, MPF_STATE, SET_LFEEDBACK
};

CMachineParameter const parRightDelay =
{
	"Right Delay", "Right Delay (samples)", MIN_DELAY, MAX_DELAY, MPF_STATE, SET_RDELAY
};

CMachineParameter const parRightFeedback =
{
	"Right Feedback", "Right Feedback (%)", MIN_FEEDBACK, MAX_FEEDBACK, MPF_STATE, SET_RFEEDBACK
};

CMachineParameter const parLockDelay =
{
	"Lock Delay", "Lock Delay (on/off)", MIN_LOCK, MAX_LOCK, MPF_STATE, SET_LOCK
};

CMachineParameter const parLockFeedback =
{
	"Lock Feedback", "Lock Feedback (on/off)", MIN_LOCK, MAX_LOCK, MPF_STATE, SET_LOCK
};

CMachineParameter const *pParameters[] =
{
	&parLeftDelay,
	&parLeftFeedback,
	&parRightDelay,
	&parRightFeedback,
	&parLockDelay,
	&parLockFeedback
};

CMachineInfo const MacInfo =
{
	MI_VERSION,
	EFFECT,																																				// flags
	PARNUM,																																				// numParameters
	pParameters,																												// Pointer to parameters
#ifdef _DEBUG
	VDPLUGINNAME " (Debug Build)",												// name
#else
	VDPLUGINNAME,																												// name
#endif
	VDSHORTNAME,																												// short name
	VDAUTHOR,																																// author
	VDCOMMAND,																																// A command, that could be use for open an editor, etc...
	PARCOLS
};

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init();
	virtual void Command();
	virtual void ParameterTweak(int par, int val);
	virtual void SequencerTick();
	virtual bool DescribeValue(char *txt,int const param, int const value);
	virtual void Work(float *pleftsamples, float *prightsamples, int samplesnum, int tracks);
private:
	FastverbFilter leftFilter;
	FastverbFilter rightFilter;
	int lastDelayModified, lastFeedbackModified;
	float lFeedback, rFeedback;
};
