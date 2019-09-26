// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "machinesbox.h"
#include <stdio.h>

static void OnDestroy(MachinesBox*, ui_component*);
static void ClearMachineBox(MachinesBox*);
static void BuildMachinesList(MachinesBox*);
static void InsertSlot(MachinesBox* self, int slot, Machine* machine);
static int OnEnumMachines(MachinesBox*, int slot, Machine*);
static int CheckMachineMode(MachinesBox*, Machine*);
static void AddString(MachinesBox*, const char* text);
static void OnMachineSlotChanged(MachinesBox*, Machines* sender, int slot);
static void OnMachinesInsert(MachinesBox*, Machines* sender, int slot);
static void OnMachinesRemoved(MachinesBox*, Machines* machines, int slot);
static void OnMachinesListChanged(MachinesBox*, ui_component* sender,
	int slot);

void InitMachinesBox(MachinesBox* self, ui_component* parent,
	Machines* machines, MachineBoxMode mode)
{	
	self->mode = mode;
	self->showslots = 1;
	InitIntHashTable(&self->listboxslots, 256);
	InitIntHashTable(&self->slotslistbox, 256);
	ui_listbox_init_multiselect(&self->machinelist, parent);	
	SetMachines(self, machines);	
	signal_connect(&self->machinelist.signal_selchanged, self,
		OnMachinesListChanged);
	signal_connect(&self->machinelist.component.signal_destroy, self, OnDestroy);
}

void OnDestroy(MachinesBox* self, ui_component* component)
{
	DisposeIntHashTable(&self->listboxslots);
	DisposeIntHashTable(&self->slotslistbox);
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
		machines_enumerate(self->machines, self, OnEnumMachines);
	}
}

void InsertSlot(MachinesBox* self, int slot, Machine* machine)
{
		int listboxindex;

		char buffer[128];
		_snprintf(buffer, 128, "%02X:", slot);
		if (machine) {
			strcat(buffer, machine->info(machine)->ShortName); 
		}
		listboxindex = ui_listbox_addstring(&self->machinelist, buffer);
		InsertIntHashTable(&self->listboxslots, listboxindex, (void*)slot);
		InsertIntHashTable(&self->slotslistbox, slot, (void*) listboxindex);
}

int OnEnumMachines(MachinesBox* self, int slot, Machine* machine)
{	
	if (CheckMachineMode(self, machine) && machine->info(machine) && 
			machine->info(machine)->ShortName) {
		InsertSlot(self, slot, machine);
	}
	return 1;
}

int CheckMachineMode(MachinesBox* self, Machine* machine)
{
	if (!machine) {
		return 0;
	}
	if (self->mode == MACHINEBOX_FX && machine->mode(machine) == MACHMODE_GENERATOR) {
		return 0;
	}
	if (self->mode == MACHINEBOX_GENERATOR && machine->mode(machine) == MACHMODE_FX) {
		return 0;
	}
	if (machine->mode(machine) == MACHMODE_MASTER) {
		return 0;
	}
	return 1;
}

void ClearMachineBox(MachinesBox* self)
{
	ui_listbox_clear(&self->machinelist);
	DisposeIntHashTable(&self->listboxslots);
	InitIntHashTable(&self->listboxslots, 256);
	DisposeIntHashTable(&self->slotslistbox);
	InitIntHashTable(&self->slotslistbox, 256);
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
	slot = (int)SearchIntHashTable(&self->listboxslots, sel);
	machines_changeslot(self->machines, slot);	
	self->machinelist.signal_selchanged.slots = slots;

}

void OnMachinesInsert(MachinesBox* self, Machines* machines, int slot)
{	
	if (CheckMachineMode(self, machines_at(self->machines, slot))) {
		int boxindex;

		BuildMachinesList(self);
		boxindex = (int)SearchIntHashTable(&self->slotslistbox, slot);
		ui_listbox_setcursel(&self->machinelist, boxindex);
	}
}

void OnMachineSlotChanged(MachinesBox* self, Machines* sender, int slot)
{
	if (ExistsIntHashTable(&self->slotslistbox, slot)) {
		ui_listbox_setcursel(&self->machinelist, slot);	
	}
}

void OnMachinesRemoved(MachinesBox* self, Machines* machines, int slot)
{	
	if (ExistsIntHashTable(&self->slotslistbox, slot)) {
		BuildMachinesList(self);
		ui_listbox_setcursel(&self->machinelist, machines->slot);
	}
}

void MachinesBoxClone(MachinesBox* self)
{
	int selection[256];
	int numsel;
	int i;
	Machine* srcmachine = 0;

	ui_listbox_selitems(&self->machinelist,
		selection, 256);
	numsel = ui_listbox_numselitems(&self->machinelist);
	for (i = 0; i < numsel; ++i) {				
		if (ExistsIntHashTable(&self->listboxslots, selection[i])) {
			int slot;
			Machine* machine;
			
			slot = (int) SearchIntHashTable(&self->listboxslots,
				selection[i]);
			machine = machines_at(self->machines, slot);
			if (machine && srcmachine == 0) {
				srcmachine = machine;
				break;
			}
		}
	}
	if (srcmachine) {
		for (i = 0; i < numsel; ++i) {				
			if (ExistsIntHashTable(&self->listboxslots,
					selection[i])) {
				int slot;
				Machine* machine;
				
				slot = (int) SearchIntHashTable(&self->listboxslots,
					selection[i]);
				machine = machines_at(self->machines, slot);
				if (machine != srcmachine) {
					Machine* clone = srcmachine->clone(srcmachine);
					if (clone) {
						machines_insert(self->machines, slot, clone);
					}
				}
			}
		}
	}
}

void MachinesBoxRemove(MachinesBox* self)
{
	int selection[256];
	int numsel;
	int i;

	ui_listbox_selitems(&self->machinelist,
	selection, 256);
	numsel = ui_listbox_numselitems(&self->machinelist);
	for (i = 0; i < numsel; ++i) {				
		if (ExistsIntHashTable(&self->listboxslots, selection[i])) {
			int slot;
			Machine* machine;
			
			slot = (int) SearchIntHashTable(&self->listboxslots, selection[i]);
			machine = machines_at(self->machines, slot);
			if (machine) {					
				machines_remove(self->machines, slot);
				machine->dispose(machine);
				free(machine);
			}
		}
	}
}

void MachinesBoxExchange(MachinesBox* self)
{
	int selection[256];
	int numsel;
	int i;
	int srcslot = -1;

	ui_listbox_selitems(&self->machinelist,
		selection, 256);
	numsel = ui_listbox_numselitems(&self->machinelist);
	for (i = 0; i < numsel; ++i) {				
		if (ExistsIntHashTable(&self->listboxslots, selection[i])) {			
			srcslot = (int) SearchIntHashTable(&self->listboxslots,
				selection[i]);			
			break;			
		}
	}
	if (srcslot != -1) {
		for (i = 0; i < numsel; ++i) {				
			if (ExistsIntHashTable(&self->listboxslots, selection[i])) {
				int slot;				
				
				slot = (int) SearchIntHashTable(&self->listboxslots,
					selection[i]);				
				if (slot != srcslot) {		
					machines_exchange(self->machines, srcslot, slot);
					break;
				}
			}
		}
	}
}

void MachinesBoxShowParameters(MachinesBox* self)
{
	int selection[256];
	int numsel;
	int i;

	ui_listbox_selitems(&self->machinelist,
	selection, 256);
	numsel = ui_listbox_numselitems(&self->machinelist);
	for (i = 0; i < numsel; ++i) {				
		if (ExistsIntHashTable(&self->listboxslots, selection[i])) {
			int slot;
			Machine* machine;
			
			slot = (int) SearchIntHashTable(&self->listboxslots, selection[i]);
			machine = machines_at(self->machines, slot);
			if (machine) {					
				machines_showparameters(self->machines, slot);
			}
		}
	}
}

void SetMachines(MachinesBox* self, Machines* machines)
{
	self->machines = machines;
	BuildMachinesList(self);
	signal_connect(&self->machines->signal_insert, self, OnMachinesInsert);
	signal_connect(&self->machines->signal_removed, self, OnMachinesRemoved);	
	signal_connect(&self->machines->signal_slotchange, self, OnMachineSlotChanged);
}
