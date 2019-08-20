#include "machinebar.h"

static void OnSize(MachineBar* self, ui_component* sender, int width, int height);
static void OnDestroy(MachineBar* self, ui_component* component);
static int OnEnumMachines(MachineBar* self, int slot, Machine* machine);
static void OnMachinesInsert(MachineBar* self, Machines* machines, int slot);
static void OnMachinesSlotChange(MachineBar* self, Machines* machines, int slot);
static void OnMachineBoxSelChange(MachineBar* self, ui_component* sender, int);

void InitMachineBar(MachineBar* self, ui_component* parent, Player* player)
{			
	self->selchange = 0;
	self->machines = &player->song->machines;
	self->player = player;		
	ui_component_init(self, &self->component, parent);	
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);	
	ui_combobox_init(&self->machinebox.component, &self->machinebox, parent);	
	ui_component_move(&self->machinebox.component, 100, 0);
	ui_component_resize(&self->machinebox.component, 200, 20);
	ui_combobox_addstring(&self->machinebox, "No Machines loaded");
	ui_combobox_setcursel(&self->machinebox, 0);
	signal_connect(&self->machinebox.signal_selchanged, self, OnMachineBoxSelChange);
	self->machinebox.component.events.target = self;
	self->prevent_selchange_notify = FALSE;
	signal_connect(&self->machines->signal_insert, self, OnMachinesInsert);
	signal_connect(&self->machines->signal_slotchange, self, OnMachinesSlotChange);
}

void SelectMachineBarSlot(MachineBar* self, int slot)
{
	ui_combobox_setcursel(&self->machinebox, slot - 1);
}

void OnSize(MachineBar* self, ui_component* sender, int width, int height)
{	
}

void OnDestroy(MachineBar* self, ui_component* component)
{
}

void OnMachinesInsert(MachineBar* self, Machines* machines, int slot)
{
	ui_combobox_clear(&self->machinebox);
	machines_enumerate(self->machines, self, OnEnumMachines);
	ui_combobox_setcursel(&self->machinebox, slot - 1);
}

int OnEnumMachines(MachineBar* self, int slot, Machine* machine)
{			
	if (machine->info && machine->info(machine)->ShortName) {
		char buffer[128];
		_snprintf(buffer, 128, "%02X: %s", slot, machine->info(machine)->ShortName); 
		ui_combobox_addstring(&self->machinebox, buffer);
	}
	return 1;
}

void OnMachineBoxSelChange(MachineBar* self, ui_component* sender, int sel)
{	
	List* slots = self->machinebox.signal_selchanged.slots;
	self->machinebox.signal_selchanged.slots = 0;
	machines_changeslot(self->machines, sel + 1);	
	self->machinebox.signal_selchanged.slots = slots;
}

void OnMachinesSlotChange(MachineBar* self, Machines* machines, int slot)
{
	ui_combobox_setcursel(&self->machinebox, slot - 1);	
}