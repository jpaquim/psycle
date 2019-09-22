// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEBAR_H)
#define MACHINEBAR_H

#include "uibutton.h"
#include "uicombobox.h"
#include "workspace.h"

typedef struct {
	ui_component component;
	ui_combobox machinebox;
	ui_button prevmachinebutton;
	ui_button nextmachinebutton;
	ui_button gear;
	ui_combobox instparambox;	
	Player* player;	
	void (*selchange)(void*, int);
	void* eventcontext;
	Machines* machines;
	Instruments* instruments;
	int prevent_selchange_notify;
	IntHashTable comboboxslots;
	IntHashTable slotscombobox;
} MachineBar;

void InitMachineBar(MachineBar*, ui_component* parent, Workspace* );
void SelectMachineBarSlot(MachineBar*, int slot);

#endif
