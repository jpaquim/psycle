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
	self->machine.numInputs = 2; //self->machine.mode(self) == MACHMODE_FX ? 2 : 0;;
	self->machine.numOutputs = 2;
	self->machine.inputs = (float**)malloc(sizeof(float*)*self->machine.numInputs);
	self->machine.outputs = (float**)malloc(sizeof(float*)*self->machine.numOutputs);
	self->machine.outputs[0] = 0;
	self->machine.outputs[1] = 0;	
}

void master_dispose(Master* self)
{
	free(self->machine.inputs);
	free(self->machine.outputs);	
	machine_dispose(&self->machine);
}



