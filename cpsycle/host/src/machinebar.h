// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEBAR_H)
#define MACHINEBAR_H

#include "uicomponent.h"
#include "uicombobox.h"
#include "player.h"

typedef struct {
	ui_component component;
	ui_combobox machinebox;
	Player* player;	
	void (*selchange)(void*, int);
	void* eventcontext;
	Machines* machines;
	int prevent_selchange_notify;
} MachineBar;

void InitMachineBar(MachineBar*, ui_component* parent, Player* player);
void SelectMachineBarSlot(MachineBar*, int slot);

#endif
