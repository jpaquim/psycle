#include "machinebar.h"

static void OnSize(MachineBar*, ui_component* sender, int width, int height);
static void OnDestroy(MachineBar*, ui_component* component);
static void BuildMachineBox(MachineBar* self);
static int OnEnumMachines(MachineBar*, int slot, Machine* machine);
static void OnMachinesInsert(MachineBar*, Machines* machines, int slot);
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

void InitMachineBar(MachineBar* self, ui_component* parent, Workspace* workspace)
{			
	self->selchange = 0;
	self->player = &workspace->player;
	self->machines = &workspace->song->machines;	
	self->instruments = &workspace->song->instruments;
	InitIntHashTable(&self->comboboxslots, 256);
	InitIntHashTable(&self->slotscombobox, 256);
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);	
	ui_combobox_init(&self->machinebox, &self->component);	
	ui_component_move(&self->machinebox.component, 0, 0);
	ui_component_resize(&self->machinebox.component, 200, 20);	
	BuildMachineBox(self);
	signal_connect(&self->machinebox.signal_selchanged, self, OnMachineBoxSelChange);	
	self->prevent_selchange_notify = FALSE;	
	ui_combobox_init(&self->instparambox, &self->component);	
	ui_component_move(&self->instparambox.component, 203, 0);
	ui_component_resize(&self->instparambox.component, 200, 20);
	BuildInstrumentList(self);
	ui_combobox_setcursel(&self->instparambox, 0);
	signal_connect(&self->instparambox.signal_selchanged, self, OnInstParamBoxSelChange);
	ConnectSongSignals(self);
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
}

void OnDestroy(MachineBar* self, ui_component* component)
{
	DisposeIntHashTable(&self->comboboxslots);
}

void ClearMachineBox(MachineBar* self)
{
	ui_combobox_clear(&self->machinebox);
	DisposeIntHashTable(&self->comboboxslots);
	InitIntHashTable(&self->comboboxslots, 256);
	DisposeIntHashTable(&self->slotscombobox);
	InitIntHashTable(&self->slotscombobox, 256);
}

void SelectMachineBarSlot(MachineBar* self, int slot)
{
	ui_combobox_setcursel(&self->machinebox, slot);
}

void OnSize(MachineBar* self, ui_component* sender, int width, int height)
{	
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
		machines_enumerate(self->machines, self, OnEnumMachines);
	}
}

int OnEnumMachines(MachineBar* self, int slot, Machine* machine)
{			
	if (slot != MASTER_INDEX &&
			machine->info(machine) && machine->info(machine)->ShortName) {
		int comboboxindex;

		char buffer[128];
		_snprintf(buffer, 128, "%02X: %s", slot, machine->info(machine)->ShortName); 
		comboboxindex = ui_combobox_addstring(&self->machinebox, buffer);
		InsertIntHashTable(&self->comboboxslots, comboboxindex, (void*)slot);
		InsertIntHashTable(&self->slotscombobox, slot, (void*) comboboxindex);
	}
	return 1;
}

void OnMachineBoxSelChange(MachineBar* self, ui_component* sender, int sel)
{	
	int slot;
	
	List* slots = self->machinebox.signal_selchanged.slots;
	self->machinebox.signal_selchanged.slots = 0;
	slot = (int)SearchIntHashTable(&self->comboboxslots, sel);
	machines_changeslot(self->machines, slot);	
	self->machinebox.signal_selchanged.slots = slots;
}

void OnMachinesSlotChange(MachineBar* self, Machines* machines, int slot)
{	
	int comboboxindex;

	comboboxindex = (int) SearchIntHashTable(&self->slotscombobox, slot);
	ui_combobox_setcursel(&self->machinebox, comboboxindex);	
}

void BuildInstrumentList(MachineBar* self)
{
	Instrument* instrument;
	int slot = 0;
	char buffer[20];

	ui_combobox_clear(&self->instparambox);
	for ( ; slot < 256; ++slot) {		
		if (instrument = SearchIntHashTable(&self->player->song->instruments.container, slot)) {
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