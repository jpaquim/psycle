// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEBAR_H)
#define MACHINEBAR_H

#include "uibutton.h"
#include "uicombobox.h"
#include "workspace.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_ComboBox machinebox;
	psy_ui_Button prevmachinebutton;
	psy_ui_Button nextmachinebutton;
	psy_ui_Button gear;
	psy_ui_Button editor;
	psy_ui_ComboBox instparambox;	
	psy_audio_Player* player;	
	void (*selchange)(void*, int);
	void* eventcontext;
	psy_audio_Machines* machines;
	psy_audio_Instruments* instruments;
	int prevent_selchange_notify;
	psy_Table comboboxslots;
	psy_Table slotscombobox;
} MachineBar;

void machinebar_init(MachineBar*, psy_ui_Component* parent, Workspace* );

#endif
