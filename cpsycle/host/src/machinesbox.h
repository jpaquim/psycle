/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/


#if !defined(MACHINESBOX_H)
#define MACHINESBOX_H

/* host */
#include "machines.h"
#include "workspace.h"
/* ui */
#include <uilistbox.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** Displays the machines of the current songand machine slot used
** by the pattern and machine view inside a listbox. The view is
** synchronized with 'psy_audio_Machines', which stores the machines
** and the selected machine slot of the song.
*/

typedef enum {
	MACHINEBOX_ALL,
	MACHINEBOX_FX,
	MACHINEBOX_GENERATOR
} MachineBoxMode;

typedef struct {
	psy_ui_Component component;
	psy_ui_ListBox listbox;	
	psy_audio_Machines* machines;
	psy_Table listboxslots;
	psy_Table slotslistbox;
	MachineBoxMode mode;
	int showslots;
	Workspace* workspace;
} MachinesBox;

void machinesbox_init(MachinesBox*, psy_ui_Component* parent,
	psy_audio_Machines*, MachineBoxMode, Workspace*);
void machinesbox_clone(MachinesBox*);
void machinesbox_remove(MachinesBox*);
void machinesbox_exchange(MachinesBox*);
void machinesbox_show_parameters(MachinesBox*);
void machinesbox_muteunmute(MachinesBox*);
void machinesbox_connecttomaster(MachinesBox*);
void machinesbox_setmachines(MachinesBox*, psy_audio_Machines*);
void machinesbox_add_sel(MachinesBox*, uintptr_t slot);
void machinesbox_deselect_all(MachinesBox*);

#ifdef __cplusplus
}
#endif

#endif
