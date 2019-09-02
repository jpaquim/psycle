// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "machine.h"

static void OnMachineDispose(Machine* self) { }
static int OnMachineMode(Machine* self) { return MACHMODE_FX; }
static int OnMasterMode(Machine* self) { return MACHMODE_MASTER; }

void machine_init(Machine* self)
{	
	memset(self, 0, sizeof(Machine));
	self->dispose = OnMachineDispose;	
	self->mode = OnMachineMode;
	signal_init(&self->signal_worked);
}

void machine_dispose(Machine* self)
{
	signal_dispose(&self->signal_worked);
}

void master_init(Master* self)
{
	memset(self, 0, sizeof(Master));
	machine_init(&self->machine);	
	self->machine.mode = OnMasterMode;
	buffer_init(&self->machine.inputs, 2);
	buffer_init(&self->machine.outputs, 2);	
}

void master_dispose(Master* self)
{	
	buffer_dispose(&self->machine.inputs);
	buffer_dispose(&self->machine.outputs);	
	machine_dispose(&self->machine);
}



