// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "master.h"
#include "machines.h"
#include <string.h>
#include <math.h>

static int master_mode(Master* self) { return MACHMODE_MASTER; }
static void master_dispose(Master* self);
static void parametertweak(Master* self, int par, int val);
static int describevalue(Master*, char* txt, int const param, int const value);
static int value(Master*, int const param);
static const CMachineInfo* info(Master* self);
static unsigned int numinputs(Master* self);
static unsigned int numoutputs(Master* self);
static int intparamvalue(float value);
static float floatparamvalue(int value);

static CMachineParameter const paraMaster = { 
	"Master", "Master Vol",	0, 65535, MPF_STATE, 0
};

static CMachineParameter const paraM1 = { 
	"m1 Vol", "m1 Vol",	0, 65535, MPF_STATE, 0
};

static CMachineParameter const paraM2 = { 
	"m2 Vol", "m2 Vol",	0, 65535, MPF_STATE, 0
};

static CMachineParameter const paraM3 = { 
	"m3 Vol", "m3 Vol",	0, 65535, MPF_STATE, 0
};

static CMachineParameter const paraM4 = { 
	"m4 Vol", "m4 Vol",	0, 65535, MPF_STATE, 0
};

static CMachineParameter const paraM5 = { 
	"m5 Vol", "m5 Vol",	0, 65535, MPF_STATE, 0
};

static CMachineParameter const paraM6 = { 
	"m6 Vol", "m6 Vol",	0, 65535, MPF_STATE, 0
};

static CMachineParameter const paraM7 = { 
	"m7 Vol", "m7 Vol",	0, 65535, MPF_STATE, 0
};

static CMachineParameter const paraM8 = { 
	"m8 Vol", "m8 Vol",	0, 65535, MPF_STATE, 0
};

static CMachineParameter const paraM9 = { 
	"m9 Vol", "m9 Vol",	0, 65535, MPF_STATE, 0
};

static CMachineParameter const paraM10 = { 
	"m10 Vol", "m10 Vol",	0, 65535, MPF_STATE, 0
};

static CMachineParameter const paraM11 = { 
	"m11 Vol", "m11 Vol", 0, 65535, MPF_STATE, 0
};

static CMachineParameter const paraM12 = { 
	"m12 Vol", "m12 Vol", 0, 65535, MPF_STATE, 0
};

static CMachineParameter const *pParameters[] = {
	&paraMaster,
	&paraM1,
	&paraM2,
	&paraM3,
	&paraM4,
	&paraM5,
	&paraM6,
	&paraM7,
	&paraM8,
	&paraM9,
	&paraM10,
	&paraM11,
	&paraM12
};

static CMachineInfo const MacInfo = {
	MI_VERSION,
	0x0250,
	EFFECT | 32 | 64,
	sizeof pParameters / sizeof *pParameters,
	pParameters,
	"Master"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Master",
	"Psycledelics",
	"help",
	3
};

const CMachineInfo* master_info(void) { return &MacInfo; }

void master_init(Master* self, MachineCallback callback)
{
	memset(self, 0, sizeof(Master));
	machine_init(&self->machine, callback);	
	self->machine.mode = master_mode;
	self->machine.info = info;
	self->machine.dispose = master_dispose;
	self->machine.info = info;
	self->machine.parametertweak = parametertweak;
	self->machine.describevalue = describevalue;
	self->machine.value = value;
	self->machine.numinputs = numinputs;
	self->machine.numoutputs = numoutputs;	
}

void master_dispose(Master* self)
{		
	machine_dispose(&self->machine);
}

void parametertweak(Master* self, int param, int value)
{
	if (param == 0) {
		Machines* machines = self->machine.callback.machines(
			self->machine.callback.context);
		if (machines) {			
			machines_setvolume(machines,
				floatparamvalue(value) * floatparamvalue(value) * 4.f);
		}		
	}
}

int describevalue(Master* self, char* txt, int const param, int const value)
{ 	
	if (param == 0) {
		Machines* machines = self->machine.callback.machines(
			self->machine.callback.context);

		float db = (float)(20 * log10(machines_volume(machines)));
		_snprintf(txt, 10, "%.2f dB", db);
		return 1;
	}
	return 0;
}

int value(Master* self, int const param)
{	
	if (param == 0) {
		Machines* machines = self->machine.callback.machines(
			self->machine.callback.context);

		if (machines) {
			return intparamvalue(
				(float)sqrt(machines_volume(machines)) * 0.5f);
		}
	}
	return 0;
}

int intparamvalue(float value)
{	
	return (int)((value * 65535.f));	
}

float floatparamvalue(int value)
{
	return value / 65535.f;	
}

const CMachineInfo* info(Master* self)
{	
	return &MacInfo;
}

unsigned int numinputs(Master* self)
{
	return 2;
}

unsigned int numoutputs(Master* self)
{
	return 2;
}

