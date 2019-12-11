// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinesbox.h"
#include <stdio.h>
#include <portable.h>

static void OnDestroy(MachinesBox*, ui_component*);
static void ClearMachineBox(MachinesBox*);
static void BuildMachinesList(MachinesBox*);
static void InsertSlot(MachinesBox* self, int slot, Machine* machine);
static void insertmachine(MachinesBox* self, int slot, Machine* machine);
static int CheckMachineMode(MachinesBox*, Machine*);
static void AddString(MachinesBox*, const char* text);
static void OnMachineSlotChanged(MachinesBox*, Machines* sender, int slot);
static void OnMachinesInsert(MachinesBox*, Machines* sender, int slot);
static void OnMachinesRemoved(MachinesBox*, Machines* machines, int slot);
static void OnMachinesListChanged(MachinesBox*, ui_component* sender,
	int slot);

void InitMachinesBox(MachinesBox* self, ui_component* parent,
	Machines* machines, MachineBoxMode mode, Workspace* workspace)
{	
	self->workspace = workspace;
	self->mode = mode;
	self->showslots = 1;
	table_init(&self->listboxslots);
	table_init(&self->slotslistbox);
	ui_listbox_init_multiselect(&self->machinelist, parent);	
	SetMachines(self, machines);	
	psy_signal_connect(&self->machinelist.signal_selchanged, self,
		OnMachinesListChanged);
	psy_signal_connect(&self->machinelist.component.signal_destroy, self,
		OnDestroy);
}

void OnDestroy(MachinesBox* self, ui_component* component)
{
	table_dispose(&self->listboxslots);
	table_dispose(&self->slotslistbox);
}

void BuildMachinesList(MachinesBox* self)
{
	ClearMachineBox(self);	
	if (self->showslots) {
		int slot;
		int start;
		int end;

		start = self->mode == MACHINEBOX_FX ? 0x40 : 0;
		end = self->mode == MACHINEBOX_ALL ? 0xFF : start + 0x3F;

		for (slot = start; slot <= end; ++slot) {
			InsertSlot(self, slot, machines_at(self->machines, slot));
		}
	} else {
		TableIterator it;
	
		for (it = machines_begin(self->machines); 
				!tableiterator_equal(&it, table_end());
			tableiterator_inc(&it)) {			
			Machine* machine;

			machine = (Machine*)tableiterator_value(&it);
			insertmachine(self, tableiterator_key(&it), machine);
		}
	}
}

void InsertSlot(MachinesBox* self, int slot, Machine* machine)
{
		int listboxindex;

		char buffer[128];
		psy_snprintf(buffer, 128, "%02X:", slot);
		if (machine && machine->vtable->info(machine)) {
			strcat(buffer, machine->vtable->info(machine)->ShortName); 
		} else {
			strcat(buffer, ""); 
		}
		listboxindex = ui_listbox_addstring(&self->machinelist, buffer);
		table_insert(&self->listboxslots, listboxindex, (void*)slot);
		table_insert(&self->slotslistbox, slot, (void*) listboxindex);
}

void insertmachine(MachinesBox* self, int slot, Machine* machine)
{	
	if (CheckMachineMode(self, machine) && machine->vtable->info(machine) && 
			machine->vtable->info(machine)->ShortName) {
		InsertSlot(self, slot, machine);
	}	
}

int CheckMachineMode(MachinesBox* self, Machine* machine)
{
	if (!machine) {
		return 0;
	}
	if (self->mode == MACHINEBOX_FX && 
			machine->vtable->mode(machine) == MACHMODE_GENERATOR) {
		return 0;
	}
	if (self->mode == MACHINEBOX_GENERATOR &&
			machine->vtable->mode(machine) == MACHMODE_FX) {
		return 0;
	}
	if (machine->vtable->mode(machine) == MACHMODE_MASTER) {
		return 0;
	}
	return 1;
}

void ClearMachineBox(MachinesBox* self)
{
	ui_listbox_clear(&self->machinelist);
	table_dispose(&self->listboxslots);
	table_init(&self->listboxslots);
	table_dispose(&self->slotslistbox);
	table_init(&self->slotslistbox);
}

void AddString(MachinesBox* self, const char* text)
{
	ui_listbox_addstring(&self->machinelist, text);
}

void OnMachinesListChanged(MachinesBox* self, ui_component* sender, int sel)
{
	int slot;

	List* slots = self->machinelist.signal_selchanged.slots;
	self->machinelist.signal_selchanged.slots = 0;
	slot = (int)table_at(&self->listboxslots, sel);
	machines_changeslot(self->machines, slot);	
	self->machinelist.signal_selchanged.slots = slots;

}

void OnMachinesInsert(MachinesBox* self, Machines* machines, int slot)
{	
	if (CheckMachineMode(self, machines_at(self->machines, slot))) {
		int boxindex;

		BuildMachinesList(self);
		boxindex = (int)table_at(&self->slotslistbox, slot);
		ui_listbox_setcursel(&self->machinelist, boxindex);
	}
}

void OnMachineSlotChanged(MachinesBox* self, Machines* sender, int slot)
{
	if (table_exists(&self->slotslistbox, slot)) {
		ui_listbox_setcursel(&self->machinelist, slot);	
	}
}

void OnMachinesRemoved(MachinesBox* self, Machines* machines, int slot)
{	
	if (table_exists(&self->slotslistbox, slot)) {
		BuildMachinesList(self);
		ui_listbox_setcursel(&self->machinelist, machines->slot);
	}
}

void MachinesBoxClone(MachinesBox* self)
{
	int selcount;
	
	selcount = ui_listbox_selcount(&self->machinelist);
	if (selcount) {
		int selection[256];	
		int i;
		Machine* srcmachine = 0;

		ui_listbox_selitems(&self->machinelist, selection, selcount);		
		for (i = 0; i < selcount; ++i) {				
			if (table_exists(&self->listboxslots, selection[i])) {
				int slot;
				Machine* machine;
				
				slot = (int) table_at(&self->listboxslots,
					selection[i]);
				machine = machines_at(self->machines, slot);
				if (machine && srcmachine == 0) {
					srcmachine = machine;
					break;
				}
			}
		}
		if (srcmachine) {
			for (i = 0; i < selcount; ++i) {				
				if (table_exists(&self->listboxslots,
						selection[i])) {
					int slot;
					Machine* machine;
					
					slot = (int) table_at(&self->listboxslots,
						selection[i]);
					machine = machines_at(self->machines, slot);
					if (machine != srcmachine) {
						Machine* clone = 
							srcmachine->vtable->clone(srcmachine);
						if (clone) {
							machines_insert(self->machines, slot, clone);
						}
					}
				}
			}
		}
	}
}

void MachinesBoxRemove(MachinesBox* self)
{	
	int selcount;	
	
	selcount = ui_listbox_selcount(&self->machinelist);
	if (selcount > 0) {
		int* selection;
		int i;

		selection = (int*)malloc(selcount * sizeof(int));
		ui_listbox_selitems(&self->machinelist, selection, selcount);
		for (i = 0; i < selcount; ++i) {				
			if (table_exists(&self->listboxslots, selection[i])) {
				int slot;			
				
				slot = (int) table_at(&self->listboxslots,
					selection[i]);
				machines_remove(self->machines, slot);			
			}
		}
		free(selection);
	}
}

void MachinesBoxExchange(MachinesBox* self)
{
	int selcount;	

	selcount = ui_listbox_selcount(&self->machinelist);
	if (selcount > 0) {
		int selection[256];	
		int i;
		int srcslot = -1;

		ui_listbox_selitems(&self->machinelist, selection, selcount);		
		for (i = 0; i < selcount; ++i) {				
			if (table_exists(&self->listboxslots, selection[i])) {			
				srcslot = (int) table_at(&self->listboxslots,
					selection[i]);			
				break;			
			}
		}
		if (srcslot != -1) {
			for (i = 0; i < selcount; ++i) {				
				if (table_exists(&self->listboxslots, selection[i])) {
					int slot;				
					
					slot = (int) table_at(&self->listboxslots,
						selection[i]);				
					if (slot != srcslot) {		
						machines_exchange(self->machines, srcslot, slot);
						break;
					}
				}
			}
		}
	}
}

void MachinesBoxShowParameters(MachinesBox* self)
{	
	int selcount;	
	
	selcount = ui_listbox_selcount(&self->machinelist);
	if (selcount > 0) {
		int selection[256];
		int i;

		ui_listbox_selitems(&self->machinelist, selection, selcount);
		for (i = 0; i < selcount; ++i) {				
			if (table_exists(&self->listboxslots, selection[i])) {
				int slot;
				Machine* machine;
				
				slot = (int) table_at(&self->listboxslots, selection[i]);
				machine = machines_at(self->machines, slot);
				if (machine) {					
					workspace_showparameters(self->workspace, slot);
				}
			}
		}
	}
}

void SetMachines(MachinesBox* self, Machines* machines)
{
	self->machines = machines;
	BuildMachinesList(self);
	psy_signal_connect(&self->machines->signal_insert, self,
		OnMachinesInsert);
	psy_signal_connect(&self->machines->signal_removed, self,
		OnMachinesRemoved);	
	psy_signal_connect(&self->machines->signal_slotchange, self,
		OnMachineSlotChanged);
}
