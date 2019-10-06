// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(MACHINESBOX_H)
#define MACHINESBOX_H

#include <uilistbox.h>
#include "machines.h"

typedef enum {
	MACHINEBOX_ALL,
	MACHINEBOX_FX,
	MACHINEBOX_GENERATOR
} MachineBoxMode;

typedef struct {
	ui_listbox machinelist;	
	Machines* machines;
	Table listboxslots;
	Table slotslistbox;
	MachineBoxMode mode;
	int showslots;
} MachinesBox;

void InitMachinesBox(MachinesBox*, ui_component* parent, Machines*, MachineBoxMode);
void MachinesBoxClone(MachinesBox*);
void MachinesBoxRemove(MachinesBox*);
void MachinesBoxExchange(MachinesBox*);
void MachinesBoxShowParameters(MachinesBox*);
void SetMachines(MachinesBox*, Machines*);

#endif