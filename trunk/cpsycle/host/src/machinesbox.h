// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net


#if !defined(MACHINESBOX_H)
#define MACHINESBOX_H

#include <uilistbox.h>
#include "machines.h"
#include "workspace.h"

// aim: displays the machines of the current song and  machine slot used
//      by the pattern and machine view inside a listbox. The view is
//      synchronized with 'psy_audio_Machines', which stores the machines
//      and the selected machine slot of the song.

typedef enum {
	MACHINEBOX_ALL,
	MACHINEBOX_FX,
	MACHINEBOX_GENERATOR
} MachineBoxMode;

typedef struct {
	psy_ui_ListBox machinelist;	
	psy_audio_Machines* machines;
	psy_Table listboxslots;
	psy_Table slotslistbox;
	MachineBoxMode mode;
	int showslots;
	Workspace* workspace;
} MachinesBox;

void machinesbox_init(MachinesBox*, psy_ui_Component* parent,
	psy_audio_Machines*, MachineBoxMode, Workspace*);
void MachinesBoxClone(MachinesBox*);
void MachinesBoxRemove(MachinesBox*);
void MachinesBoxExchange(MachinesBox*);
void MachinesBoxShowParameters(MachinesBox*);
void SetMachines(MachinesBox*, psy_audio_Machines*);

#endif
