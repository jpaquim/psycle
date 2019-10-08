// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "dummy.h"

const CMachineInfo* dummymachine_info(void)
{
	static CMachineInfo const macinfo = {
		MI_VERSION,
		0x0250,
		EFFECT | 32 | 64,
		0,
		0,
		"Dummy plug"
			#ifndef NDEBUG
			" (debug build)"
			#endif
			,
		"Dummy Plug",
		"Psycledelics",
		"help",
		3
	};
	return &macinfo;
}

static const CMachineInfo* info(DummyMachine* self) {
	return dummymachine_info();
}
static int mode(DummyMachine* self) { return self->mode; }
static unsigned int numinputs(DummyMachine* self) { return 2; }
static unsigned int numoutputs(DummyMachine* self) { return 2; }

void dummymachine_init(DummyMachine* self, MachineCallback callback)
{	
	machine_init(&self->machine, callback);	
	self->machine.mode = mode;
	self->machine.info = info;
	self->machine.numinputs = numinputs;
	self->machine.numoutputs = numoutputs;
	self->mode = MACHMODE_FX;
}
