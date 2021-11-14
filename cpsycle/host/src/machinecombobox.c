// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinecombobox.h"
#include <songio.h>
#include <containerconvert.h>
#include <plugin_interface.h>
// 
#include "../../detail/portable.h"

static void machinecombobox_ondestroy(MachineComboBox*, psy_ui_Component* component);
static void machinecombobox_onmachineboxselchange(MachineComboBox*,
	psy_ui_Component* sender, int sel);
static void machinecombobox_buildmachinebox(MachineComboBox*);
static void machinecombobox_onmachinesinsert(MachineComboBox*, psy_audio_Machines* sender, uintptr_t slot);
static int machinecombobox_insertmachine(MachineComboBox*, size_t slot, psy_audio_Machine*);
static void machinecombobox_onmachinesremoved(MachineComboBox*, psy_audio_Machines* sender, uintptr_t slot);
static void machinecombobox_onmachineselect(MachineComboBox*, psy_audio_Machines* sender, uintptr_t slot);
static void machinecombobox_onsongchanged(MachineComboBox*, Workspace*, int flag);
static void machinecombobox_connectsongsignals(MachineComboBox*);
static void machinecombobox_connectinstrumentsignals(MachineComboBox*);
static void machinecombobox_clearmachinebox(MachineComboBox* self);

void machinecombobox_init(MachineComboBox* self, psy_ui_Component* parent,
	bool showmaster, Workspace* workspace)
{		
	psy_ui_Margin margin;

	self->workspace = workspace;
	self->sync_machines_select = TRUE;
	self->showmaster = showmaster;
	self->column = psy_INDEX_INVALID;
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);	
	self->machines = &workspace->song->machines;	
	self->instruments = &workspace->song->instruments;	
	psy_table_init(&self->comboboxslots);
	psy_table_init(&self->slotscombobox);	
	// Machine ComboBox
	psy_ui_combobox_init(&self->machinebox, parent, NULL);
	psy_ui_combobox_setcharnumber(&self->machinebox, 10);
	psy_signal_connect(&self->machinebox.component.signal_destroy,
		self, machinecombobox_ondestroy);
	psy_signal_connect(&self->machinebox.signal_selchanged, self,
		machinecombobox_onmachineboxselchange);
	machinecombobox_buildmachinebox(self);	
	//psy_signal_connect(&workspace->signal_songchanged, self,
		//machinecombobox_onsongchanged);
	//machinecombobox_connectsongsignals(self);		
	psy_signal_init(&self->signal_selected);
}

void machinecombobox_ondestroy(MachineComboBox* self, psy_ui_Component* component)
{
	psy_table_dispose(&self->comboboxslots);
	psy_table_dispose(&self->slotscombobox);
	if (self->machines) {
		psy_signal_disconnect(&self->workspace->signal_songchanged, self,
			machinecombobox_onsongchanged);
		psy_signal_disconnect(&self->machines->signal_insert, self,
			machinecombobox_onmachinesinsert);
		psy_signal_disconnect(&self->machines->signal_removed, self,
			machinecombobox_onmachinesremoved);
		psy_signal_disconnect(&self->machines->signal_slotchange, self,
			machinecombobox_onmachineselect);
	}
	psy_signal_dispose(&self->signal_selected);
}

void machinecombobox_clearmachinebox(MachineComboBox* self)
{
	psy_ui_combobox_clear(&self->machinebox);
	psy_table_dispose(&self->comboboxslots);
	psy_table_init(&self->comboboxslots);
	psy_table_dispose(&self->slotscombobox);
	psy_table_init(&self->slotscombobox);
}

void machinecombobox_onmachinesinsert(MachineComboBox* self, psy_audio_Machines* sender,
	uintptr_t slot)
{	
	//machinecombobox_buildmachinebox(self);
	//psy_ui_combobox_setcursel(&self->machinebox,
		//psy_audio_machines_selected(sender));
}

void machinecombobox_onmachinesremoved(MachineComboBox* self, psy_audio_Machines* sender,
	uintptr_t slot)
{
	//machinecombobox_buildmachinebox(self);
	//psy_ui_combobox_setcursel(&self->machinebox,
		//psy_audio_machines_selected(sender));
}

void machinecombobox_buildmachinebox(MachineComboBox* self)
{
	machinecombobox_clearmachinebox(self);
	if (psy_audio_machines_size(self->machines) == 1) {
		psy_ui_combobox_addtext(&self->machinebox, psy_ui_translate(
			"machineview.no-machines-loaded"));
		psy_ui_combobox_setcursel(&self->machinebox, 0);
	} else if (self->machines &&
			(psy_audio_machines_size(self->machines) > 0)) {
		uintptr_t i;

		for (i = 0; i <= psy_audio_machines_maxindex(self->machines); ++i) {
			psy_audio_Machine* machine;

			machine = psy_audio_machines_at(self->machines, i);
			if (machine) {
				machinecombobox_insertmachine(self, i, machine);
			}
		}
		i = psy_ui_combobox_addtext(&self->machinebox,
			"------------------------------");
		psy_table_insert(&self->comboboxslots, i, (void*)psy_INDEX_INVALID);
		psy_table_insert(&self->slotscombobox, psy_INDEX_INVALID, (void*)i);
	}
}

int machinecombobox_insertmachine(MachineComboBox* self, size_t slot, psy_audio_Machine* machine)
{				
	if ((slot != psy_audio_MASTER_INDEX || self->showmaster) && psy_audio_machine_info(machine) &&
			psy_audio_machine_info(machine)->shortname) {
		intptr_t comboboxindex;
		char buffer[128];

		psy_snprintf(buffer, 128, "%02X: %s", slot, 
			machine->vtable->info(machine)->shortname); 
		comboboxindex = psy_ui_combobox_addtext(&self->machinebox, buffer);
		psy_table_insert(&self->comboboxslots, comboboxindex, (void*)slot);
		psy_table_insert(&self->slotscombobox, slot, (void*) comboboxindex);
	}
	return 1;
}

void machinecombobox_onmachineboxselchange(MachineComboBox* self,
	psy_ui_Component* sender, int sel)
{	
	if (self->sync_machines_select) {
		size_t slot;

		slot = (size_t)psy_table_at(&self->comboboxslots, sel);
		psy_audio_machines_select(self->machines, slot);
	}
	psy_signal_emit(&self->signal_selected, self, 0);
}

void machinecombobox_onmachineselect(MachineComboBox* self, psy_audio_Machines* machines,
	uintptr_t slot)
{	
	if (self->sync_machines_select) {
		machinecombobox_select(self, slot);		
	}
}


void machinecombobox_onsongchanged(MachineComboBox* self, Workspace* workspace, int flag)
{	
	self->machines = &workspace->song->machines;
	self->instruments = &workspace->song->instruments;	
	machinecombobox_connectsongsignals(self);
	machinecombobox_buildmachinebox(self);
}

void machinecombobox_connectsongsignals(MachineComboBox* self)
{
	psy_signal_connect(&self->machines->signal_insert, self,
		machinecombobox_onmachinesinsert);
	psy_signal_connect(&self->machines->signal_removed, self,
		machinecombobox_onmachinesremoved);
	psy_signal_connect(&self->machines->signal_slotchange, self,
		machinecombobox_onmachineselect);
}

void machinecombobox_select(MachineComboBox* self, uintptr_t slot)
{
	intptr_t comboboxindex;

	if (psy_table_exists(&self->slotscombobox, slot)) {
		comboboxindex = (intptr_t)psy_table_at(&self->slotscombobox, slot);
	} else {
		comboboxindex = psy_ui_combobox_count(&self->machinebox) - 1;
	}	
	psy_ui_combobox_setcursel(&self->machinebox, comboboxindex);	
}
