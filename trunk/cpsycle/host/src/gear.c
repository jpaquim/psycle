// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "gear.h"
// host
#include "styles.h"
// audio
#include <songio.h>

// GearButtons
// implementation
void gearbuttons_init(GearButtons* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(gearbuttons_base(self), parent, NULL);
	psy_ui_component_setstyletype(&self->component,
		STYLE_RECENTVIEW_MAINSECTION);
	psy_ui_component_setdefaultalign(gearbuttons_base(self), psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));	
	psy_ui_button_init_text(&self->createreplace, gearbuttons_base(self), NULL,
		"gear.create-replace");
	psy_ui_button_init_text(&self->del, gearbuttons_base(self), NULL,
		"gear.delete");
	psy_ui_button_init_text(&self->parameters, gearbuttons_base(self), NULL,
		"gear.parameters");
	psy_ui_button_init_text(&self->properties, gearbuttons_base(self), NULL,
		"gear.properties");
	psy_ui_button_init_text(&self->exchange, gearbuttons_base(self), NULL,
		"gear.exchange");
	psy_ui_button_init_text(&self->clone, gearbuttons_base(self), NULL,
		"gear.clone");
	psy_ui_button_init_text(&self->showmaster, gearbuttons_base(self), NULL,
		"gear.show-master");
	psy_ui_button_init_text(&self->connecttomaster, gearbuttons_base(self), NULL,
		"gear.connecttomaster");
	psy_ui_button_init_text(&self->muteunmute, gearbuttons_base(self), NULL,
		"gear.mute-unmute");
}

// Gear
// prototypes
static void gear_oncreate(Gear*, psy_ui_Component* sender);
static void gear_ondelete(Gear*, psy_ui_Component* sender);
static void gear_onsongchanged(Gear*, Workspace*, int flag, psy_audio_Song*);
static void gear_connectsongsignals(Gear*);
static void gear_onclone(Gear*, psy_ui_Component* sender);
static void gear_onexchange(Gear* self, psy_ui_Component* sender);
static void gear_onparameters(Gear*, psy_ui_Component* sender);
static void gear_onmaster(Gear*, psy_ui_Component* sender);
static void gear_onconnecttomaster(Gear*, psy_ui_Component* sender);
static void gear_onmuteunmute(Gear*, psy_ui_Component* sender);
static void gear_connectsongsignals(Gear*);
static void gear_onhide(Gear*);
static void gear_onmachineselected(Gear*, psy_audio_Machines* sender,
	uintptr_t slot);
static void gear_showgenerators(Gear*);
static void gear_showeffects(Gear*);

// implementation
void gear_init(Gear* self, psy_ui_Component* parent, Workspace* workspace)
{		
	psy_ui_Margin margin;
		
	psy_signal_connect(&workspace->signal_songchanged, self,
		gear_onsongchanged);
	psy_ui_component_init(gear_base(self), parent, NULL);
	//psy_ui_component_setstyletypes(&self->component,
		//psy_ui_STYLE_SIDEMENU, psy_INDEX_INVALID, psy_INDEX_INVALID);
	self->workspace = workspace;
	self->machines = &workspace->song->machines;
	// client
	psy_ui_component_init(&self->client, gear_base(self), NULL);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setmargin(&self->client,
		psy_ui_defaults_cmargin(psy_ui_defaults()));
	// titlebar
	psy_ui_component_init_align(&self->titlebar, &self->client,
		psy_ui_ALIGN_TOP);
	psy_ui_component_setstyletype(&self->titlebar, STYLE_HEADER);	
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 0.5, 0.0);		
	psy_ui_component_setmargin(&self->titlebar, margin);
	psy_ui_label_init_text(&self->title, &self->titlebar, NULL, "machinebar.gear");	
	psy_ui_component_setalign(&self->title.component, psy_ui_ALIGN_CLIENT);
	psy_ui_button_init_connect(&self->hide, &self->titlebar, NULL,
		self, gear_onhide);
	psy_ui_button_settext(&self->hide, "X");
	psy_ui_component_setalign(&self->hide.component, psy_ui_ALIGN_RIGHT);
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);		
	psy_ui_component_setmargin(&self->hide.component, margin);
	// client
	psy_ui_tabbar_init(&self->tabbar, &self->client);
	psy_ui_tabbar_append_tabs(&self->tabbar, "gear.generators", "gear.effects",
		"gear.instruments", "gear.waves", NULL);
	psy_ui_tabbar_select(&self->tabbar, 0);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar), psy_ui_ALIGN_BOTTOM);
	psy_ui_notebook_init(&self->notebook, &self->client);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	machinesbox_init(&self->machinesboxgen, psy_ui_notebook_base(&self->notebook), 
		&workspace->song->machines, MACHINEBOX_GENERATOR, self->workspace);
	machinesbox_init(&self->machinesboxfx, psy_ui_notebook_base(&self->notebook), 
		&workspace->song->machines, MACHINEBOX_FX, self->workspace);
	instrumentsbox_init(&self->instrumentsbox,
		psy_ui_notebook_base(&self->notebook), 
		&workspace->song->instruments, workspace);
	psy_ui_component_setmargin(&self->instrumentsbox.groupheader, psy_ui_margin_zero());
	samplesbox_init(&self->samplesbox, psy_ui_notebook_base(&self->notebook),
		&workspace->song->samples, workspace);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	psy_ui_tabbar_select(&self->tabbar, 0);
	gearbuttons_init(&self->buttons, &self->client, workspace);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->buttons.createreplace.signal_clicked, self,
		gear_oncreate);
	psy_signal_connect(&self->buttons.del.signal_clicked, self, gear_ondelete);
	psy_signal_connect(&self->buttons.clone.signal_clicked, self,
		gear_onclone);
	psy_signal_connect(&self->buttons.parameters.signal_clicked, self,
		gear_onparameters);
	psy_signal_connect(&self->buttons.showmaster.signal_clicked, self,
		gear_onmaster);
	psy_signal_connect(&self->buttons.connecttomaster.signal_clicked, self,
		gear_onconnecttomaster);
	psy_signal_connect(&self->buttons.exchange.signal_clicked, self,
		gear_onexchange);
	psy_signal_connect(&self->buttons.muteunmute.signal_clicked, self,
		gear_onmuteunmute);
	gear_connectsongsignals(self);
}

void gear_oncreate(Gear* self, psy_ui_Component* sender)
{	
	workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
		SECTION_ID_MACHINEVIEW_NEWMACHINE,
		(psy_audio_machines_selected(&self->workspace->song->machines)
			!= psy_INDEX_INVALID)
		? NEWMACHINE_INSERT
		: NEWMACHINE_APPEND);
}

void gear_ondelete(Gear* self, psy_ui_Component* sender)
{
	switch (self->tabbar.selected) {
	case 0: machinesbox_remove(&self->machinesboxgen);
		break;
	case 1: machinesbox_remove(&self->machinesboxfx);
		break;
	default:
		break;
	}
}

void gear_onsongchanged(Gear* self, Workspace* workspace, int flag, psy_audio_Song* song)
{	
	self->machines = &workspace->song->machines;		
	machinesbox_setmachines(&self->machinesboxgen, &workspace->song->machines);
	machinesbox_setmachines(&self->machinesboxfx, &workspace->song->machines);
	instrumentsbox_setinstruments(&self->instrumentsbox,
		&workspace->song->instruments);
	samplesbox_setsamples(&self->samplesbox, &workspace->song->samples);
	gear_connectsongsignals(self);
	psy_ui_component_invalidate(gear_base(self));
}

void gear_connectsongsignals(Gear* self)
{
	psy_signal_connect(&self->machines->signal_slotchange, self,
		gear_onmachineselected);
}

void gear_onmachineselected(Gear* self, psy_audio_Machines* sender,
	uintptr_t macidx)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(sender, macidx);
	if (machine) {
		if (psy_audio_machine_mode(machine) == psy_audio_MACHMODE_GENERATOR) {
			gear_showgenerators(self);
		} else if (psy_audio_machine_mode(machine) == psy_audio_MACHMODE_FX) {
			gear_showeffects(self);
		}
	} else if (macidx >= 0 && macidx < 0x40) {
		gear_showgenerators(self);
	} else if (macidx >= 0x40 && macidx < 0x80) {
		gear_showeffects(self);
	}	
}

void gear_showgenerators(Gear* self)
{
	if (psy_ui_tabbar_selected(&self->tabbar) != 0) {
		psy_ui_tabbar_select(&self->tabbar, 0);
	}
}

void gear_showeffects(Gear* self)
{
	if (psy_ui_tabbar_selected(&self->tabbar) != 1) {
		psy_ui_tabbar_select(&self->tabbar, 1);
	}
}

void gear_onclone(Gear* self, psy_ui_Component* sender)
{
	switch (psy_ui_tabbar_selected(&self->tabbar)) {
	case 0: machinesbox_clone(&self->machinesboxgen);
		break;
	case 1: machinesbox_clone(&self->machinesboxfx);
		break;
	default:
		break;
	}
}

void gear_onexchange(Gear* self, psy_ui_Component* sender)
{
	switch (psy_ui_tabbar_selected(&self->tabbar)) {
	case 0: machinesbox_exchange(&self->machinesboxgen);
		break;
	case 1: machinesbox_exchange(&self->machinesboxfx);
		break;
	default:
		break;
	}
}

void gear_onmuteunmute(Gear* self, psy_ui_Component* sender)
{
	machinesbox_muteunmute(&self->machinesboxgen);
	machinesbox_muteunmute(&self->machinesboxfx);	
}

void gear_onparameters(Gear* self, psy_ui_Component* sender)
{
	switch (psy_ui_tabbar_selected(&self->tabbar)) {
	case 0: machinesbox_showparameters(&self->machinesboxgen);
		break;
	case 1: machinesbox_showparameters(&self->machinesboxfx);
		break;
	default:
		break;
	}
}

void gear_onmaster(Gear* self, psy_ui_Component* sender)
{
	workspace_showparameters(self->workspace, psy_audio_MASTER_INDEX);
}

void gear_onhide(Gear* self)
{	
	psy_ui_component_hide(&self->component);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}

void gear_onconnecttomaster(Gear* self, psy_ui_Component* sender)
{
	switch (psy_ui_tabbar_selected(&self->tabbar)) {
	case 0: machinesbox_connecttomaster(&self->machinesboxgen);
		break;
	case 1: machinesbox_connecttomaster(&self->machinesboxfx);
		break;
	default:
		break;
	}	
}

void gear_select(Gear* self, psy_List* machinelist)
{
	psy_List* p;

	machinesbox_deselectall(&self->machinesboxfx);
	machinesbox_deselectall(&self->machinesboxgen);
	for (p = machinelist; p != NULL; psy_list_next(&p)) {
		uintptr_t slot;

		slot = (uintptr_t)psy_list_entry(p);
		machinesbox_addsel(&self->machinesboxfx, slot);
		machinesbox_addsel(&self->machinesboxgen, slot);
	}
}
