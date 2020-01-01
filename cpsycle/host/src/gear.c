// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "gear.h"

static void gear_connectsongsignals(Gear*);
static void gear_ondelete(Gear*, psy_ui_Component* sender);
static void gear_onsongchanged(Gear*, Workspace*);
static void gear_onclone(Gear*, psy_ui_Component* sender);
static void gear_onexchange(Gear* self, psy_ui_Component* sender);
static void gear_onparameters(Gear*, psy_ui_Component* sender);
static void gear_onmaster(Gear*, psy_ui_Component* sender);

void gearbuttons_init(GearButtons* self, psy_ui_Component* parent)
{
	ui_margin margin;	

	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);	
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
	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(0.5),
		ui_value_makeeh(0.5), ui_value_makepx(0));				
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_TOP,
		&margin));
}

void gear_init(Gear* self, psy_ui_Component* parent, Workspace* workspace)
{		
	self->workspace = workspace;
	self->machines = &workspace->song->machines;
	psy_signal_connect(&workspace->signal_songchanged, self,
		gear_onsongchanged);
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	tabbar_init(&self->tabbar, &self->component);	
	tabbar_append(&self->tabbar, "Generators");
	tabbar_append(&self->tabbar, "Effects");	
	tabbar_append(&self->tabbar, "Instruments");
	tabbar_append(&self->tabbar, "Waves");	
	tabbar_select(&self->tabbar, 0);
	ui_component_setalign(&self->tabbar.component, UI_ALIGN_BOTTOM);
	ui_notebook_init(&self->notebook, &self->component);
	ui_component_setalign(&self->notebook.component, UI_ALIGN_CLIENT);	
	machinesbox_init(&self->machinesboxgen, &self->notebook.component, 
		&workspace->song->machines, MACHINEBOX_GENERATOR, self->workspace);
	machinesbox_init(&self->machinesboxfx, &self->notebook.component, 
		&workspace->song->machines, MACHINEBOX_FX, self->workspace);
	instrumentsbox_init(&self->instrumentsbox, &self->notebook.component, 
		&workspace->song->instruments);
	samplesbox_init(&self->samplesbox, &self->notebook.component, 
		&workspace->song->samples, &workspace->song->instruments);
	gear_connectsongsignals(self);
	ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	tabbar_select(&self->tabbar, 0);
	gearbuttons_init(&self->buttons, &self->component);
	ui_component_setalign(&self->buttons.component, UI_ALIGN_RIGHT);
	psy_signal_connect(&self->buttons.del.signal_clicked, self, gear_ondelete);
	psy_signal_connect(&self->buttons.clone.signal_clicked, self,
		gear_onclone);
	psy_signal_connect(&self->buttons.parameters.signal_clicked, self,
		gear_onparameters);
	psy_signal_connect(&self->buttons.showmaster.signal_clicked, self,
		gear_onmaster);
	psy_signal_connect(&self->buttons.exchange.signal_clicked, self,
		gear_onexchange);	
}

void gear_connectsongsignals(Gear* self)
{
	// psy_signal_connect(&self->instruments->signal_insert, self, OnInstrumentInsert);
	// psy_signal_connect(&self->instruments->signal_slotchange, self, OnInstrumentSlotChanged);	
}

void gear_ondelete(Gear* self, psy_ui_Component* sender)
{
	switch (self->tabbar.selected) {
		case 0:
			MachinesBoxRemove(&self->machinesboxgen);
		break;
		case 1:
			MachinesBoxRemove(&self->machinesboxfx);
		break;
		default:
		break;
	}
}

void gear_onsongchanged(Gear* self, Workspace* workspace)
{	
	self->machines = &workspace->song->machines;		
	SetMachines(&self->machinesboxgen, &workspace->song->machines);
	SetMachines(&self->machinesboxfx, &workspace->song->machines);
	instrumentsbox_setinstruments(&self->instrumentsbox,
		&workspace->song->instruments);
	samplesbox_setsamples(&self->samplesbox, &workspace->song->samples,
		&workspace->song->instruments);
	gear_connectsongsignals(self);
	ui_component_invalidate(&self->component);
}

void gear_onclone(Gear* self, psy_ui_Component* sender)
{
	switch (self->tabbar.selected) {
		case 0:
			MachinesBoxClone(&self->machinesboxgen);
		break;
		case 1:
			MachinesBoxClone(&self->machinesboxfx);
		break;
		default:
		break;
	}
}

void gear_onexchange(Gear* self, psy_ui_Component* sender)
{
	switch (self->tabbar.selected) {
		case 0:
			MachinesBoxExchange(&self->machinesboxgen);
		break;
		case 1:
			MachinesBoxExchange(&self->machinesboxfx);
		break;
		default:
		break;
	}
}

void gear_onparameters(Gear* self, psy_ui_Component* sender)
{
	switch (self->tabbar.selected) {
		case 0:
			MachinesBoxShowParameters(&self->machinesboxgen);
		break;
		case 1:
			MachinesBoxShowParameters(&self->machinesboxfx);
		break;
		default:
		break;
	}
}

void gear_onmaster(Gear* self, psy_ui_Component* sender)
{
	workspace_showparameters(self->workspace, MASTER_INDEX);
}
