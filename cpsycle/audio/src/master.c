// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "master.h"
#include <string.h>

static int master_mode(Master* self) { return MACHMODE_MASTER; }
static void master_dispose(Master* self);
static void parametertweak(Master* self, int par, int val);
static int describevalue(Master*, char* txt, int const param, int const value);
static int value(Master*, int const param);
static const CMachineInfo* info(Master* self);
static unsigned int numinputs(Master* self);
static unsigned int numoutputs(Master* self);

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

void master_init(Master* self, MachineCallback callback)
{
	memset(self, 0, sizeof(Master));
	machine_init(&self->machine, callback);	
	self->machine.mode = master_mode;
	self->machine.dispose = master_dispose;
	self->machine.info = info;
	self->machine.parametertweak = parametertweak;
	self->machine.describevalue = describevalue;
	self->machine.numinputs = numinputs;
	self->machine.numoutputs = numoutputs;
}

void master_dispose(Master* self)
{		
	machine_dispose(&self->machine);
}

void parametertweak(Master* self, int param, int value)
{
}

int describevalue(Master* self, char* txt, int const param, int const value)
{ 	
	return 0;
}

int value(Master* self, int const param)
{		
	return 0;
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