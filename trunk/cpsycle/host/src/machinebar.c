// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinebar.h"

static void OnDestroy(MachineBar*, ui_component* component);
static void BuildMachineBox(MachineBar* self);
static void OnMachinesInsert(MachineBar*, Machines* machines, int slot);
static int insertmachine(MachineBar*, size_t slot, Machine*);
static void OnMachinesRemoved(MachineBar*, Machines* machines, int slot);
static void OnMachinesSlotChange(MachineBar*, Machines* machines, int slot);
static void OnMachineBoxSelChange(MachineBar*, ui_component* sender, int sel);
static void OnInstParamBoxSelChange(MachineBar*, ui_component* sender, int sel);
static void BuildInstrumentList(MachineBar* self);
static void AddString(MachineBar* self, const char* text);
static void OnInstrumentInsert(MachineBar* self, ui_component* sender, int slot);
static void OnInstrumentSlotChanged(MachineBar* self, Instrument* sender, int slot);
static void OnInstrumentListChanged(MachineBar* self, ui_component* sender, int slot);
static void OnSongChanged(MachineBar*, Workspace*);
static void ConnectSongSignals(MachineBar* self);
static void ClearMachineBox(MachineBar* self);
static void OnPrevMachine(MachineBar*, ui_component* sender);
static void OnNextMachine(MachineBar*, ui_component* sender);

void InitMachineBar(MachineBar* self, ui_component* parent, Workspace* workspace)
{				
	self->selchange = 0;	
	self->player = &workspace->player;
	self->machines = &workspace->song->machines;	
	self->instruments = &workspace->song->instruments;
	table_init(&self->comboboxslots);
	table_init(&self->slotscombobox);
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);	
	ui_combobox_init(&self->machinebox, &self->component);
	ui_combobox_setcharnumber(&self->machinebox, 25);
//	self->machinebox.component.justify = UI_JUSTIFY_NONE;
	ui_component_resize(&self->machinebox.component, 200, 20);
	//ui_button_init(&self->prevmachinebutton, &self->component);
	//ui_button_settext(&self->prevmachinebutton, "<");	
	//signal_connect(&self->prevmachinebutton.signal_clicked, self, OnPrevMachine);
	//ui_button_init(&self->nextmachinebutton, &self->component);
	//ui_button_settext(&self->nextmachinebutton, ">");	
	//signal_connect(&self->nextmachinebutton.signal_clicked, self, OnNextMachine);
	//ui_component_resize(&self->prevmachinebutton.component, 20, 0);	
	//ui_component_resize(&self->nextmachinebutton.component, 20, 0);
	ui_button_init(&self->gear, &self->component);
	ui_button_settext(&self->gear, "Gear Rack");	
	BuildMachineBox(self);
	signal_connect(&self->machinebox.signal_selchanged, self, OnMachineBoxSelChange);	
	self->prevent_selchange_notify = FALSE;	
	ui_combobox_init(&self->instparambox, &self->component);
	ui_combobox_setcharnumber(&self->instparambox, 20);
	//self->instparambox.component.justify = UI_JUSTIFY_NONE;
	ui_component_resize(&self->instparambox.component, 200, 20);
	BuildInstrumentList(self);
	ui_combobox_setcursel(&self->instparambox, 0);
	signal_connect(&self->instparambox.signal_selchanged, self, OnInstParamBoxSelChange);
	ConnectSongSignals(self);
	{		
		ui_margin margin = { 0, 3, 3, 0 };
				
		list_free(ui_components_setalign(
			ui_component_children(&self->component, 0),
			UI_ALIGN_LEFT,
			&margin));		
	}
}

void OnDestroy(MachineBar* self, ui_component* component)
{
	table_dispose(&self->comboboxslots);
	table_dispose(&self->slotscombobox);
}

void ClearMachineBox(MachineBar* self)
{
	ui_combobox_clear(&self->machinebox);
	table_dispose(&self->comboboxslots);
	table_init(&self->comboboxslots);
	table_dispose(&self->slotscombobox);
	table_init(&self->slotscombobox);
}

void SelectMachineBarSlot(MachineBar* self, int slot)
{
	ui_combobox_setcursel(&self->machinebox, slot);
}

void OnInstrumentInsert(MachineBar* self, ui_component* sender, int slot)
{
	BuildInstrumentList(self);
	ui_combobox_setcursel(&self->instparambox, slot);	
}

void OnInstrumentSlotChanged(MachineBar* self, Instrument* sender, int slot)
{
	ui_combobox_setcursel(&self->instparambox, slot);	
}

void OnMachinesInsert(MachineBar* self, Machines* machines, int slot)
{	
	BuildMachineBox(self);
	ui_combobox_setcursel(&self->machinebox, machines->slot);
}

void OnMachinesRemoved(MachineBar* self, Machines* machines, int slot)
{
	BuildMachineBox(self);
	ui_combobox_setcursel(&self->machinebox, machines->slot);	
}

void BuildMachineBox(MachineBar* self)
{
	ClearMachineBox(self);	
	if (machines_size(self->machines) == 1) {
		ui_combobox_addstring(&self->machinebox, "No Machines Loaded");
		ui_combobox_setcursel(&self->machinebox, 0);
	} else {
		TableIterator it;
	
		for (it = machines_begin(self->machines); !tableiterator_equal(&it, table_end());
			tableiterator_inc(&it)) {			
			Machine* machine;

			machine = (Machine*)tableiterator_value(&it);
			insertmachine(self, tableiterator_key(&it),  machine);
		}
	}
}

int insertmachine(MachineBar* self, size_t slot, Machine* machine)
{			
	if (slot != MASTER_INDEX &&
			machine->info(machine) && machine->info(machine)->ShortName) {
		intptr_t comboboxindex;

		char buffer[128];
		_snprintf(buffer, 128, "%02X: %s", slot, machine->info(machine)->ShortName); 
		comboboxindex = ui_combobox_addstring(&self->machinebox, buffer);
		table_insert(&self->comboboxslots, comboboxindex, (void*)slot);
		table_insert(&self->slotscombobox, slot, (void*) comboboxindex);
	}
	return 1;
}

void OnMachineBoxSelChange(MachineBar* self, ui_component* sender, int sel)
{	
	size_t slot;
	
	List* slots = self->machinebox.signal_selchanged.slots;
	self->machinebox.signal_selchanged.slots = 0;
	slot = (size_t)table_at(&self->comboboxslots, sel);
	machines_changeslot(self->machines, slot);	
	self->machinebox.signal_selchanged.slots = slots;
}

void OnMachinesSlotChange(MachineBar* self, Machines* machines, int slot)
{	
	intptr_t comboboxindex;

	comboboxindex = (intptr_t) table_at(&self->slotscombobox, slot);
	ui_combobox_setcursel(&self->machinebox, comboboxindex);	
}

void BuildInstrumentList(MachineBar* self)
{
	Instrument* instrument;
	int slot = 0;
	char buffer[20];

	ui_combobox_clear(&self->instparambox);
	for ( ; slot < 256; ++slot) {		
		if (instrument = table_at(&self->player->song->instruments.container, slot)) {
			_snprintf(buffer, 20, "%02X:%s", slot, instrument_name(instrument));
		} else {
			_snprintf(buffer, 20, "%02X:%s", slot, "");
		}
		AddString(self, buffer);
	}
}

void AddString(MachineBar* self, const char* text)
{
	ui_combobox_addstring(&self->instparambox, text);
}

void OnInstParamBoxSelChange(MachineBar* self, ui_component* sender, int sel)
{
	signal_prevent(&self->instruments->signal_slotchange, self, OnInstrumentSlotChanged);
	instruments_changeslot(self->instruments, sel);
	signal_enable(&self->instruments->signal_slotchange, self, OnInstrumentSlotChanged);
}

void OnSongChanged(MachineBar* self, Workspace* workspace)
{	
	self->machines = &workspace->song->machines;
	self->instruments = &workspace->song->instruments;	
	ConnectSongSignals(self);
	BuildMachineBox(self);	
	BuildInstrumentList(self);
	ui_combobox_setcursel(&self->instparambox,
		workspace->song->instruments.slot);
}

void ConnectSongSignals(MachineBar* self)
{
	signal_connect(&self->machines->signal_insert, self, OnMachinesInsert);
	signal_connect(&self->machines->signal_removed, self, OnMachinesRemoved);	
	signal_connect(&self->machines->signal_slotchange, self, OnMachinesSlotChange);
	signal_connect(&self->instruments->signal_insert, self, OnInstrumentInsert);
	signal_connect(&self->instruments->signal_slotchange, self, OnInstrumentSlotChanged);	
}

void OnNextMachine(MachineBar* self, ui_component* sender)
{
	if (self->machines && machines_slot(self->machines) > 0) {
		machines_changeslot(self->machines, machines_slot(self->machines) - 1);
	}
}

void OnPrevMachine(MachineBar* self, ui_component* sender)
{
	if (self->machines) {
		machines_changeslot(self->machines, machines_slot(self->machines) + 1);
	}
}
