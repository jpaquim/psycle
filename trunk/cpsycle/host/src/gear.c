// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "gear.h"

static void OnSize(Gear*, ui_component* sender, int width, int height);
static void OnDestroy(Gear*, ui_component* component);
static void ConnectSongSignals(Gear*);
static void BuildListBox(Gear*);
static void ClearListBox(Gear*);
static int OnEnumMachines(Gear*, int slot, Machine*);
static void OnMachinesSlotChange(Gear*, Machines* machines, int slot);
static void OnMachineBoxSelChange(Gear*, ui_component* sender, int sel);
static void OnDelete(Gear*, ui_component* sender);
static void OnSongChanged(Gear*, Workspace*);


void InitGearButtons(GearButtons* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);	
	ui_button_init(&self->createreplace, &self->component);
	ui_button_settext(&self->createreplace, "Create/Replace");
	ui_button_init(&self-> del, &self->component);
	ui_button_settext(&self->del, "Delete");	
	ui_button_init(&self->parameters, &self->component);
	ui_button_settext(&self->parameters, "Parameters");
	ui_button_init(&self->properties, &self->component);
	ui_button_settext(&self->properties, "Properties");
	ui_button_init(&self->exchange, &self->component);
	ui_button_settext(&self->exchange, "Exchange");
	ui_button_init(&self->clone, &self->component);
	ui_button_settext(&self->clone, "Clone");
	ui_button_init(&self->showmaster, &self->component);
	ui_button_settext(&self->showmaster, "Show master");
	{
		ui_margin margin = { 3, 3, 3, 3 };
		List* p;
		for (p = ui_component_children(&self->component, 0); p != 0; p = p->next) {
			ui_component* component;
			
			component = (ui_component*)p->entry;
			ui_component_setalign(component, UI_ALIGN_TOP);
			ui_component_setmargin(component, &margin);
			ui_component_resize(component, 0, 20);
		}
	}
}

void InitGear(Gear* self, ui_component* parent, Workspace* workspace)
{		
	self->machines = &workspace->song->machines;
	InitIntHashTable(&self->listboxslots, 256);
	InitIntHashTable(&self->slotslistbox, 256);
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);	
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	InitTabBar(&self->tabbar, &self->component);	
	ui_component_move(&self->tabbar.component, 0, 0);
	ui_component_resize(&self->tabbar.component, 0, 20);
	tabbar_append(&self->tabbar, "Generators");
	tabbar_append(&self->tabbar, "Effects");	
	tabbar_append(&self->tabbar, "Instruments");
	tabbar_append(&self->tabbar, "Waves");	
	tabbar_select(&self->tabbar, 0);		
	signal_connect(&self->component.signal_size, self, OnSize);	
	ui_listbox_init(&self->listbox, &self->component);
	ui_component_setposition(&self->listbox.component, 0, 0, 200, 20);
	BuildListBox(self);
	signal_connect(&self->listbox.signal_selchanged, self, OnMachineBoxSelChange);
	ConnectSongSignals(self);
	InitGearButtons(&self->buttons, &self->component);
	signal_connect(&self->buttons.del.signal_clicked, self, OnDelete);
	ui_component_resize(&self->buttons.component, 100, 0);
}

void OnDestroy(Gear* self, ui_component* component)
{
	DisposeIntHashTable(&self->listboxslots);
	DisposeIntHashTable(&self->slotslistbox);
}

void OnSize(Gear* self, ui_component* sender, int width, int height)
{
	ui_size buttonssize;

	buttonssize = ui_component_size(&self->buttons.component);
	ui_component_move(&self->tabbar.component, 0, height - 20);
	ui_component_resize(&self->tabbar.component, width, 20);
	ui_component_resize(&self->listbox.component, width - buttonssize.width, height - 20);
	ui_component_move(&self->buttons.component, width - buttonssize.width , 0);
	ui_component_resize(&self->buttons.component, buttonssize.width, height - 20);
}

void BuildListBox(Gear* self)
{
	ClearListBox(self);	
	if (machines_size(self->machines) == 1) {
		ui_listbox_addstring(&self->listbox, "No Machines Loaded");
		ui_listbox_setcursel(&self->listbox, 0);
	} else {
		machines_enumerate(self->machines, self, OnEnumMachines);
	}
}

void ClearListBox(Gear* self)
{
	ui_listbox_clear(&self->listbox);
	DisposeIntHashTable(&self->listboxslots);
	InitIntHashTable(&self->listboxslots, 256);
	DisposeIntHashTable(&self->slotslistbox);
	InitIntHashTable(&self->slotslistbox, 256);
}

int OnEnumMachines(Gear* self, int slot, Machine* machine)
{			
	if (slot != MASTER_INDEX &&
			machine->info(machine) && machine->info(machine)->ShortName) {
		int listboxindex;

		char buffer[128];
		_snprintf(buffer, 128, "%02X: %s", slot, machine->info(machine)->ShortName); 
		listboxindex = ui_listbox_addstring(&self->listbox, buffer);
		InsertIntHashTable(&self->listboxslots, listboxindex, (void*)slot);
		InsertIntHashTable(&self->slotslistbox, slot, (void*) listboxindex);
	}
	return 1;
}

void OnMachinesInsert(Gear* self, Machines* machines, int slot)
{	
	BuildListBox(self);
	ui_listbox_setcursel(&self->listbox, machines->slot);
}

void OnMachinesRemoved(Gear* self, Machines* machines, int slot)
{
	BuildListBox(self);
	ui_listbox_setcursel(&self->listbox, machines->slot);	
}

void ConnectSongSignals(Gear* self)
{
	signal_connect(&self->machines->signal_insert, self, OnMachinesInsert);
	signal_connect(&self->machines->signal_removed, self, OnMachinesRemoved);	
	signal_connect(&self->machines->signal_slotchange, self, OnMachinesSlotChange);
	// signal_connect(&self->instruments->signal_insert, self, OnInstrumentInsert);
	// signal_connect(&self->instruments->signal_slotchange, self, OnInstrumentSlotChanged);	
}

void OnMachineBoxSelChange(Gear* self, ui_component* sender, int sel)
{	
	int slot;
	
	List* slots = self->listbox.signal_selchanged.slots;
	self->listbox.signal_selchanged.slots = 0;
	slot = (int)SearchIntHashTable(&self->listboxslots, sel);
	machines_changeslot(self->machines, slot);	
	self->listbox.signal_selchanged.slots = slots;
}

void OnMachinesSlotChange(Gear* self, Machines* machines, int slot)
{	
	int listboxindex;

	listboxindex = (int) SearchIntHashTable(&self->slotslistbox, slot);
	ui_listbox_setcursel(&self->listbox, listboxindex);	
}

void OnDelete(Gear* self, ui_component* sender)
{
	if (self->machines && machines_slot(self->machines) != MASTER_INDEX) {
		machines_remove(self->machines, machines_slot(self->machines));
	}
}

void OnSongChanged(Gear* self, Workspace* workspace)
{	
	self->machines = &workspace->song->machines;		
	ConnectSongSignals(self);
	ui_invalidate(&self->component);
}
