// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "gear.h"
#include "songio.h"

static void gear_connectsongsignals(Gear*);
static void gear_ondelete(Gear*, psy_ui_Component* sender);
static void gear_onsongchanged(Gear*, Workspace*, int flag, psy_audio_SongFile*);
static void gear_onclone(Gear*, psy_ui_Component* sender);
static void gear_onexchange(Gear* self, psy_ui_Component* sender);
static void gear_onparameters(Gear*, psy_ui_Component* sender);
static void gear_onmaster(Gear*, psy_ui_Component* sender);

void gearbuttons_init(GearButtons* self, psy_ui_Component* parent)
{
	psy_ui_Margin margin;	

	psy_ui_component_init(gearbuttons_base(self), parent);	
	psy_ui_component_enablealign(gearbuttons_base(self));	
	psy_ui_button_init(&self->createreplace, gearbuttons_base(self));
	psy_ui_button_settext(&self->createreplace, "Create/Replace");
	psy_ui_button_init(&self-> del, gearbuttons_base(self));
	psy_ui_button_settext(&self->del, "Delete");	
	psy_ui_button_init(&self->parameters, gearbuttons_base(self));
	psy_ui_button_settext(&self->parameters, "Parameters");
	psy_ui_button_init(&self->properties, gearbuttons_base(self));
	psy_ui_button_settext(&self->properties, "Properties");
	psy_ui_button_init(&self->exchange, gearbuttons_base(self));
	psy_ui_button_settext(&self->exchange, "Exchange");
	psy_ui_button_init(&self->clone, gearbuttons_base(self));
	psy_ui_button_settext(&self->clone, "Clone");
	psy_ui_button_init(&self->showmaster, gearbuttons_base(self));
	psy_ui_button_settext(&self->showmaster, "Show master");			
	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));				
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(gearbuttons_base(self), 0),
		psy_ui_ALIGN_TOP,
		&margin));
}

psy_ui_Component* gearbuttons_base(GearButtons* self)
{
	return &self->component;
}

void gear_init(Gear* self, psy_ui_Component* parent, Workspace* workspace)
{		
	self->workspace = workspace;
	self->machines = &workspace->song->machines;
	psy_signal_connect(&workspace->signal_songchanged, self,
		gear_onsongchanged);
	psy_ui_component_init(gear_base(self), parent);
	psy_ui_component_enablealign(gear_base(self));
	tabbar_init(&self->tabbar, gear_base(self));	
	tabbar_append_tabs(&self->tabbar, "Generators", "Effects", "Instruments",
		"Waves", NULL);
	tabbar_select(&self->tabbar, 0);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_BOTTOM);
	psy_ui_notebook_init(&self->notebook, gear_base(self));
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	machinesbox_init(&self->machinesboxgen, psy_ui_notebook_base(&self->notebook), 
		&workspace->song->machines, MACHINEBOX_GENERATOR, self->workspace);
	machinesbox_init(&self->machinesboxfx, psy_ui_notebook_base(&self->notebook), 
		&workspace->song->machines, MACHINEBOX_FX, self->workspace);
	instrumentsbox_init(&self->instrumentsbox,
		psy_ui_notebook_base(&self->notebook), 
		&workspace->song->instruments);
	samplesbox_init(&self->samplesbox, psy_ui_notebook_base(&self->notebook),
		&workspace->song->samples, &workspace->song->instruments);
	gear_connectsongsignals(self);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	tabbar_select(&self->tabbar, 0);
	gearbuttons_init(&self->buttons, gear_base(self));
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_RIGHT);
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

void gear_onsongchanged(Gear* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
{	
	self->machines = &workspace->song->machines;		
	SetMachines(&self->machinesboxgen, &workspace->song->machines);
	SetMachines(&self->machinesboxfx, &workspace->song->machines);
	instrumentsbox_setinstruments(&self->instrumentsbox,
		&workspace->song->instruments);
	samplesbox_setsamples(&self->samplesbox, &workspace->song->samples,
		&workspace->song->instruments);
	gear_connectsongsignals(self);
	psy_ui_component_invalidate(gear_base(self));
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

psy_ui_Component* gear_base(Gear* self)
{
	return &self->component;
}
