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
#include "combfilter.hpp"

#define LDELAY    0
#define LDRY      1
#define LWET      2
#define RDELAY    3
#define RDRY      4
#define RWET      5
#define LOCKDELAY 6
#define LOCKDRY   7
#define LOCKWET   8
#define PARNUM    9

#define PARCOLS 3

#define VDPLUGINNAME "vd's echo filter"
#define VDSHORTNAME  "Echo"
#define VDAUTHOR     "V. Demasi (Built on "__DATE__")"
#define VDCOMMAND    "License"

#define MIN_DELAY  0
#define MAX_DELAY  COMB_FILTER_MAX_DELAY
#define MIN_GAIN   0
#define MAX_GAIN   100
#define GAIN_NORM  100.0f
#define SET_LDELAY (MAX_DELAY / 2)
#define SET_RDELAY (MAX_DELAY / 2)
#define SET_LDRY  (MAX_GAIN)
#define SET_LWET  (MAX_GAIN * 6 / 10)
#define SET_RDRY  (MAX_GAIN)
#define SET_RWET  (MAX_GAIN * 6 / 10)
#define MIN_LOCK     0
#define MAX_LOCK     1
#define SET_LOCK 1

CMachineParameter const parLeftDelay =
{
	"Left Delay", "Left Delay (samples)", MIN_DELAY, MAX_DELAY, MPF_STATE, SET_LDELAY
};

CMachineParameter const parLeftDry =
{
	"Left Dry", "Left Dry (%)", MIN_GAIN, MAX_GAIN, MPF_STATE, SET_LDRY
};

CMachineParameter const parLeftWet =
{
	"Left Wet", "Left Wet (%)", MIN_GAIN, MAX_GAIN, MPF_STATE, SET_LWET
};

CMachineParameter const parRightDelay =
{
	"Right Delay", "Right Delay (samples)", MIN_DELAY, MAX_DELAY, MPF_STATE, SET_RDELAY
};

CMachineParameter const parRightDry =
{
	"Right Dry", "Right Dry (%)", MIN_GAIN, MAX_GAIN, MPF_STATE, SET_RDRY
};

CMachineParameter const parRightWet =
{
	"Right Wet", "Right Wet (%)", MIN_GAIN, MAX_GAIN, MPF_STATE, SET_RWET
};

CMachineParameter const parLockDelay =
{
	"Lock Delay", "Lock Delay (On/Off)", MIN_LOCK, MAX_LOCK, MPF_STATE, SET_LOCK
};

CMachineParameter const parLockDry =
{
	"Lock Dry", "Lock Dry (On/Off)", MIN_LOCK, MAX_LOCK, MPF_STATE, SET_LOCK
};

CMachineParameter const parLockWet =
{
	"Lock Wet", "Lock Wet (On/Off)", MIN_LOCK, MAX_LOCK, MPF_STATE, SET_LOCK
};

CMachineParameter const *pParameters[] =
{
	&parLeftDelay,
	&parLeftDry,
	&parLeftWet,
	&parRightDelay,
	&parRightDry,
	&parRightWet,
	&parLockDelay,
	&parLockDry,
	&parLockWet
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
	CombFilter leftFilter;
	CombFilter rightFilter;
	float lDryGain, lWetGain, rDryGain, rWetGain;
	int lastDelayModified, lastDryModified, lastWetModified;
};
