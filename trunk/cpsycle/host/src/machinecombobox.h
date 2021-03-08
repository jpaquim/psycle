// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINECOMBOBOX_H)
#define MACHINECOMBOBOX_H

#include "uibutton.h"
#include "uicombobox.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MachineComboBox {
	// inherits
	psy_ui_ComboBox machinebox;
	// signals
	psy_Signal signal_selected;
	// internal	
	// references
	psy_audio_Machines* machines;
	psy_audio_Instruments* instruments;
	int prevent_selchange_notify;
	psy_Table comboboxslots;
	psy_Table slotscombobox;
	Workspace* workspace;
	bool sync_machines_select;
	bool showmaster;
	uintptr_t column;
} MachineComboBox;

void machinecombobox_init(MachineComboBox*, psy_ui_Component* parent,
	bool showmaster, Workspace*);

void machinecombobox_select(MachineComboBox*, uintptr_t slot);

INLINE psy_ui_Component* machinecombobox_base(MachineComboBox* self)
{
	return &self->machinebox.component;
}

#ifdef __cplusplus
}
#endif

#endif /* MACHINECOMBOBOX_H */
