// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinesbox.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../detail/portable.h"

static void machinesbox_ondestroy(MachinesBox*, psy_ui_Component*);
static void machinesbox_clearmachinebox(MachinesBox*);
static void machinesbox_buildmachineslist(MachinesBox*);
static void machinesbox_insertslot(MachinesBox*, uintptr_t slot, psy_audio_Machine*);
static void machinesbox_insertmachine(MachinesBox*, uintptr_t slot, psy_audio_Machine*);
static int machinesbox_checkmachinemode(MachinesBox*, psy_audio_Machine*);
static void machinesbox_onmachineslotchanged(MachinesBox*, psy_audio_Machines* sender, uintptr_t slot);
static void machinesbox_onmachinesinsert(MachinesBox*, psy_audio_Machines* sender, uintptr_t slot);
static void machinesbox_onmachinesremoved(MachinesBox*, psy_audio_Machines* machines, uintptr_t slot);
static void machinesbox_onmachineslistchanged(MachinesBox*, psy_ui_Component* sender,
	int slot);

void machinesbox_init(MachinesBox* self, psy_ui_Component* parent,
	psy_audio_Machines* machines, MachineBoxMode mode, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);	
	self->workspace = workspace;
	self->mode = mode;
	self->showslots = 1;
	psy_table_init(&self->listboxslots);
	psy_table_init(&self->slotslistbox);
	psy_ui_listbox_init_multiselect(&self->listbox, &self->component);
	psy_ui_component_setalign(&self->listbox.component, psy_ui_ALIGN_CLIENT);
	machinesbox_setmachines(self, machines);
	//psy_signal_connect(&self->listbox.signal_selchanged, self,
		//machinesbox_onmachineslistchanged);
	psy_signal_connect(&self->listbox.component.signal_destroy, self,
		machinesbox_ondestroy);
}

void machinesbox_ondestroy(MachinesBox* self, psy_ui_Component* component)
{
	psy_table_dispose(&self->listboxslots);
	psy_table_dispose(&self->slotslistbox);
}

void machinesbox_buildmachineslist(MachinesBox* self)
{
	machinesbox_clearmachinebox(self);
	if (self->showslots) {
		int slot;
		int start;
		int end;

		start = self->mode == MACHINEBOX_FX ? 0x40 : 0;
		end = self->mode == MACHINEBOX_ALL ? 0xFF : start + 0x3F;

		for (slot = start; slot <= end; ++slot) {
			machinesbox_insertslot(self, slot, psy_audio_machines_at(self->machines, slot));
		}
	} else {
		psy_TableIterator it;
	
		for (it = psy_audio_machines_begin(self->machines); 
				!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {			
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
			machinesbox_insertmachine(self, psy_tableiterator_key(&it), machine);
		}
	}
}

void machinesbox_insertslot(MachinesBox* self, uintptr_t slot, psy_audio_Machine* machine)
{
		uintptr_t listboxindex;

		char buffer[128];
		psy_snprintf(buffer, 128, "%02X:", slot);
		if (machine && machine->vtable->info(machine)) {
			strcat(buffer, machine->vtable->info(machine)->ShortName); 
		} else {
			strcat(buffer, ""); 
		}
		listboxindex = psy_ui_listbox_addtext(&self->listbox, buffer);
		psy_table_insert(&self->listboxslots, listboxindex, (void*)slot);
		psy_table_insert(&self->slotslistbox, slot, (void*) listboxindex);
}

void machinesbox_insertmachine(MachinesBox* self, uintptr_t slot, psy_audio_Machine* machine)
{	
	if (machinesbox_checkmachinemode(self, machine) && psy_audio_machine_info(machine) &&
			psy_audio_machine_info(machine)->ShortName) {
		machinesbox_insertslot(self, slot, machine);
	}	
}

int machinesbox_checkmachinemode(MachinesBox* self, psy_audio_Machine* machine)
{
	if (!machine) {
		return 0;
	}
	if (self->mode == MACHINEBOX_FX && 
		psy_audio_machine_mode(machine) == MACHMODE_GENERATOR) {
		return 0;
	}
	if (self->mode == MACHINEBOX_GENERATOR &&
		psy_audio_machine_mode(machine) == MACHMODE_FX) {
		return 0;
	}
	if (psy_audio_machine_mode(machine) == MACHMODE_MASTER) {
		return 0;
	}
	return 1;
}

void machinesbox_clearmachinebox(MachinesBox* self)
{
	psy_ui_listbox_clear(&self->listbox);
	psy_table_dispose(&self->listboxslots);
	psy_table_init(&self->listboxslots);
	psy_table_dispose(&self->slotslistbox);
	psy_table_init(&self->slotslistbox);
}

void machinesbox_onmachineslistchanged(MachinesBox* self, psy_ui_Component* sender, int sel)
{
	uintptr_t slot;

	psy_List* slots = self->listbox.signal_selchanged.slots;
	self->listbox.signal_selchanged.slots = 0;
	slot = (uintptr_t)psy_table_at(&self->listboxslots, sel);
	psy_audio_machines_changeslot(self->machines, slot);
	self->listbox.signal_selchanged.slots = slots;
}

void machinesbox_onmachinesinsert(MachinesBox* self, psy_audio_Machines* machines, uintptr_t slot)
{	
	if (machinesbox_checkmachinemode(self, psy_audio_machines_at(self->machines, slot))) {
		uintptr_t boxindex;

		machinesbox_buildmachineslist(self);
		boxindex = (uintptr_t)psy_table_at(&self->slotslistbox, slot);
		psy_ui_listbox_setcursel(&self->listbox, boxindex);
	}
}

void machinesbox_onmachineslotchanged(MachinesBox* self, psy_audio_Machines* sender, uintptr_t slot)
{
	if (psy_table_exists(&self->slotslistbox, slot)) {
		psy_ui_listbox_setcursel(&self->listbox, slot);
		psy_ui_component_invalidate(&self->listbox.component);
	}
}

void machinesbox_onmachinesremoved(MachinesBox* self, psy_audio_Machines* machines, uintptr_t slot)
{	
	if (psy_table_exists(&self->slotslistbox, slot)) {
		machinesbox_buildmachineslist(self);
		psy_ui_listbox_setcursel(&self->listbox, machines->slot);
	}
}

void machinesbox_clone(MachinesBox* self)
{
	intptr_t selcount;
	
	selcount = psy_ui_listbox_selcount(&self->listbox);
	if (selcount) {
		int selection[256];	
		int i;
		psy_audio_Machine* srcmachine = 0;

		psy_ui_listbox_selitems(&self->listbox, selection, (int)selcount);
		for (i = 0; i < selcount; ++i) {				
			if (psy_table_exists(&self->listboxslots, selection[i])) {
				uintptr_t slot;
				psy_audio_Machine* machine;
				
				slot = (uintptr_t)psy_table_at(&self->listboxslots,
					selection[i]);
				machine = psy_audio_machines_at(self->machines, slot);
				if (machine && srcmachine == 0) {
					srcmachine = machine;
					break;
				}
			}
		}
		if (srcmachine) {
			for (i = 0; i < selcount; ++i) {				
				if (psy_table_exists(&self->listboxslots,
						selection[i])) {
					uintptr_t slot;
					psy_audio_Machine* machine;
					
					slot = (uintptr_t)psy_table_at(&self->listboxslots,
						selection[i]);
					machine = psy_audio_machines_at(self->machines, slot);
					if (machine != srcmachine) {
						psy_audio_Machine* clone;

						clone = psy_audio_machine_clone(srcmachine);
						if (clone) {
							psy_audio_machines_insert(self->machines, slot, clone);
						}
					}
				}
			}
		}
	}
}

void machinesbox_remove(MachinesBox* self)
{	
	int selcount;	
	
	selcount = (int)psy_ui_listbox_selcount(&self->listbox);
	if (selcount > 0) {
		int* selection;
		int i;

		selection = (int*)malloc(selcount * sizeof(int));
		psy_ui_listbox_selitems(&self->listbox, selection, selcount);
		for (i = 0; i < selcount; ++i) {				
			if (psy_table_exists(&self->listboxslots, selection[i])) {
				uintptr_t slot;
				
				slot = (uintptr_t)psy_table_at(&self->listboxslots,
					selection[i]);
				psy_audio_machines_remove(self->machines, slot);			
			}
		}
		free(selection);
	}
}

void machinesbox_exchange(MachinesBox* self)
{
	int selcount;	

	selcount = (int)psy_ui_listbox_selcount(&self->listbox);
	if (selcount > 0) {
		int selection[256];	
		int i;
		uintptr_t srcslot = UINTPTR_MAX;

		psy_ui_listbox_selitems(&self->listbox, selection, selcount);
		for (i = 0; i < selcount; ++i) {				
			if (psy_table_exists(&self->listboxslots, selection[i])) {			
				srcslot = (uintptr_t)psy_table_at(&self->listboxslots,
					selection[i]);			
				break;			
			}
		}
		if (srcslot != UINTPTR_MAX) {
			for (i = 0; i < selcount; ++i) {				
				if (psy_table_exists(&self->listboxslots, selection[i])) {
					uintptr_t slot;
					
					slot = (uintptr_t)psy_table_at(&self->listboxslots,
						selection[i]);				
					if (slot != srcslot) {		
						psy_audio_machines_exchange(self->machines, srcslot,
							slot);
						break;
					}
				}
			}
		}
	}
}

void machinesbox_showparameters(MachinesBox* self)
{	
	int selcount;	
	
	selcount = (int)psy_ui_listbox_selcount(&self->listbox);
	if (selcount > 0) {
		int selection[256];
		int i;

		psy_ui_listbox_selitems(&self->listbox, selection, selcount);
		for (i = 0; i < selcount; ++i) {				
			if (psy_table_exists(&self->listboxslots, selection[i])) {
				uintptr_t slot;
				psy_audio_Machine* machine;
				
				slot = (uintptr_t)psy_table_at(&self->listboxslots, selection[i]);
				machine = psy_audio_machines_at(self->machines, slot);
				if (machine) {					
					workspace_showparameters(self->workspace, slot);
				}
			}
		}
	}
}

void machinesbox_setmachines(MachinesBox* self, psy_audio_Machines* machines)
{
	self->machines = machines;
	machinesbox_buildmachineslist(self);
	psy_signal_connect(&self->machines->signal_insert, self,
		machinesbox_onmachinesinsert);
	psy_signal_connect(&self->machines->signal_removed, self,
		machinesbox_onmachinesremoved);
	psy_signal_connect(&self->machines->signal_slotchange, self,
		machinesbox_onmachineslotchanged);
}
