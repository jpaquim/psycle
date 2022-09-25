/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "gear.h"
#include "paramviews.h"
/* host */
#include "styles.h"
/* audio */
#include <songio.h>

/* GearButtons */

/* implementation */
void gearbuttons_init(GearButtons* self, psy_ui_Component* parent,
	ParamViews* paramviews)
{
	psy_ui_component_init(gearbuttons_base(self), parent, NULL);	
	psy_ui_component_set_default_align(gearbuttons_base(self),
		psy_ui_ALIGN_TOP, psy_ui_defaults_vmargin(psy_ui_defaults()));	
	psy_ui_button_init_text(&self->createreplace, gearbuttons_base(self),
		"gear.create-replace");
	psy_ui_button_init_text(&self->del, gearbuttons_base(self),
		"gear.delete");
	psy_ui_button_init_text(&self->parameters, gearbuttons_base(self),
		"gear.parameters");
	psy_ui_button_init_text(&self->properties, gearbuttons_base(self),
		"gear.properties");
	psy_ui_button_init_text(&self->exchange, gearbuttons_base(self),
		"gear.exchange");
	psy_ui_button_init_text(&self->clone, gearbuttons_base(self),
		"gear.clone");
	psy_ui_button_init_text(&self->showmaster, gearbuttons_base(self),
		"gear.show-master");	
}

/* Gear */

/* prototypes */
static void gear_init_title(Gear*);
static void gear_oncreate(Gear*, psy_ui_Component* sender);
static void gear_on_delete(Gear*, psy_ui_Component* sender);
static void gear_on_song_changed(Gear*, psy_audio_Player* sender);
static void gear_connect_song(Gear*);
static void gear_onclone(Gear*, psy_ui_Component* sender);
static void gear_on_exchange(Gear* self, psy_ui_Component* sender);
static void gear_on_parameters(Gear*, psy_ui_Component* sender);
static void gear_on_master(Gear*, psy_ui_Component* sender);
static void gear_connect_song(Gear*);
static void gear_on_close_button(Gear*, psy_ui_Button* sender);
static void gear_on_machine_selected(Gear*, psy_audio_Machines* sender,
	uintptr_t slot);
static void gear_show_generators(Gear*);
static void gear_show_effects(Gear*);
static void gear_on_tabbar_changed(Gear*, psy_ui_TabBar* sender,
	uintptr_t tabindex);

/* implementation */
void gear_init(Gear* self, psy_ui_Component* parent, ParamViews* param_views,
	Workspace* workspace)
{			
	psy_ui_component_init(gear_base(self), parent, NULL);	
	self->workspace = workspace;
	self->param_views = param_views;
	if (workspace_song(self->workspace)) {
		self->machines = &workspace_song(self->workspace)->machines;
	} else {
		self->machines = NULL;
	}
	psy_signal_connect(&workspace->player.signal_song_changed, self,
		gear_on_song_changed);
	/* client */
	psy_ui_component_init(&self->client, gear_base(self), NULL);
	psy_ui_component_set_align(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_style_type(&self->client, STYLE_SIDE_VIEW);	
	/* titlebar */
	gear_init_title(self);
	/* label */
	psy_ui_label_init_text(&self->label, &self->component, "Machines:Generator");
	psy_ui_component_set_align(psy_ui_label_base(&self->label), psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(psy_ui_label_base(&self->label),
		psy_ui_margin_make_em(0.0, 0.0, 0.25, 0.0));
	/* client */
	psy_ui_tabbar_init(&self->tabbar, &self->client);
	psy_ui_tabbar_append_tabs(&self->tabbar, "gear.generators", "gear.effects",
		"gear.instruments", "gear.waves", NULL);
	psy_ui_tabbar_select(&self->tabbar, 0);
	psy_ui_component_set_align(psy_ui_tabbar_base(&self->tabbar), psy_ui_ALIGN_BOTTOM);
	psy_ui_notebook_init(&self->notebook, &self->client);
	psy_ui_component_set_margin(psy_ui_notebook_base(&self->notebook),
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	machinesbox_init(&self->machinesboxgen, psy_ui_notebook_base(&self->notebook), 
		&workspace->song->machines, MACHINEBOX_GENERATOR, self->workspace);
	machinesbox_init(&self->machinesboxfx, psy_ui_notebook_base(&self->notebook), 
		&workspace->song->machines, MACHINEBOX_FX, self->workspace);
	instrumentsbox_init(&self->instrumentsbox,
		psy_ui_notebook_base(&self->notebook), 
		&workspace->song->instruments, workspace);
	psy_ui_component_set_margin(&self->instrumentsbox.groupheader, psy_ui_margin_zero());
	samplesbox_init(&self->samplesbox, psy_ui_notebook_base(&self->notebook),
		&workspace->song->samples, workspace);
	self->samplesbox.load_on_select = TRUE;
	psy_ui_notebook_connect_controller(&self->notebook,
		&self->tabbar.signal_change);
	psy_ui_tabbar_select(&self->tabbar, 0);
	gearbuttons_init(&self->buttons, &self->client, param_views);
	psy_ui_component_set_align(&self->buttons.component, psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->buttons.createreplace.signal_clicked, self,
		gear_oncreate);
	psy_signal_connect(&self->buttons.del.signal_clicked, self, gear_on_delete);
	psy_signal_connect(&self->buttons.clone.signal_clicked, self,
		gear_onclone);
	psy_signal_connect(&self->buttons.parameters.signal_clicked, self,
		gear_on_parameters);
	psy_signal_connect(&self->buttons.showmaster.signal_clicked, self,
		gear_on_master);	
	psy_signal_connect(&self->buttons.exchange.signal_clicked, self,
		gear_on_exchange);	
	psy_signal_connect(&self->tabbar.signal_change, self,
		gear_on_tabbar_changed);	
	gear_connect_song(self);
}

void gear_init_title(Gear* self)
{	
	titlebar_init(&self->titlebar, gear_base(self), "machinebar.gear");
	psy_signal_connect(&self->titlebar.hide.signal_clicked, self,
		gear_on_close_button);
}

void gear_oncreate(Gear* self, psy_ui_Component* sender)
{	
	if (workspace_song(self->workspace)) {
		psy_audio_Machines* machines;
		
		machines = &self->workspace->song->machines;
		if (psy_audio_machines_selected(machines) != psy_INDEX_INVALID) {
			self->workspace->insert.replace_mac =
				psy_audio_machines_selected(machines);
		} else {
			self->workspace->insert.replace_mac = psy_INDEX_INVALID;
		}
	}
	workspace_select_view(self->workspace, viewindex_make_all(VIEW_ID_MACHINEVIEW,
		SECTION_ID_MACHINEVIEW_NEWMACHINE, psy_INDEX_INVALID,
		psy_INDEX_INVALID));
}

void gear_on_delete(Gear* self, psy_ui_Component* sender)
{
	switch (psy_ui_tabbar_selected(&self->tabbar)) {
	case 0: machinesbox_remove(&self->machinesboxgen);
		break;
	case 1: machinesbox_remove(&self->machinesboxfx);
		break;
	default:
		break;
	}
}

void gear_on_song_changed(Gear* self, psy_audio_Player* sender)
{	
	psy_audio_Song* song;
	
	song = psy_audio_player_song(sender);
	if (song) {
		self->machines = &song->machines;
		machinesbox_setmachines(&self->machinesboxgen, &song->machines);
		machinesbox_setmachines(&self->machinesboxfx, &song->machines);
		instrumentsbox_setinstruments(&self->instrumentsbox,
			&song->instruments);
		samplesbox_setsamples(&self->samplesbox, &song->samples);
		gear_connect_song(self);
	} else {
		self->machines = NULL;
		machinesbox_setmachines(&self->machinesboxgen, NULL);
		machinesbox_setmachines(&self->machinesboxfx, NULL);
		instrumentsbox_setinstruments(&self->instrumentsbox, NULL);
		samplesbox_setsamples(&self->samplesbox, NULL);
	}
	psy_ui_component_invalidate(gear_base(self));
}

void gear_connect_song(Gear* self)
{
	psy_signal_connect(&self->machines->signal_slotchange, self,
		gear_on_machine_selected);
}

void gear_on_machine_selected(Gear* self, psy_audio_Machines* sender,
	uintptr_t macidx)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(sender, macidx);
	if (machine) {
		if (psy_audio_machine_mode(machine) == psy_audio_MACHMODE_GENERATOR) {
			gear_show_generators(self);
		} else if (psy_audio_machine_mode(machine) == psy_audio_MACHMODE_FX) {
			gear_show_effects(self);
		}
	} else if (macidx >= 0 && macidx < 0x40) {
		gear_show_generators(self);
	} else if (macidx >= 0x40 && macidx < 0x80) {
		gear_show_effects(self);
	}	
}

void gear_show_generators(Gear* self)
{
	if (psy_ui_tabbar_selected(&self->tabbar) != 0) {
		psy_ui_tabbar_select(&self->tabbar, 0);
	}
}

void gear_show_effects(Gear* self)
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

void gear_on_exchange(Gear* self, psy_ui_Component* sender)
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

void gear_on_parameters(Gear* self, psy_ui_Component* sender)
{
	switch (psy_ui_tabbar_selected(&self->tabbar)) {
	case 0: machinesbox_show_parameters(&self->machinesboxgen);
		break;
	case 1: machinesbox_show_parameters(&self->machinesboxfx);
		break;
	default:
		break;
	}
}

void gear_on_master(Gear* self, psy_ui_Component* sender)
{
	if (self->param_views) {
		paramviews_show(self->param_views, psy_audio_MASTER_INDEX);
	}
}

void gear_on_close_button(Gear* self, psy_ui_Button* sender)
{
	psy_property_set_item_bool(generalconfig_property(
		&self->workspace->config.general, "bench.showgear"), FALSE);
}

void gear_select(Gear* self, psy_List* machinelist)
{
	psy_List* p;

	machinesbox_deselect_all(&self->machinesboxfx);
	machinesbox_deselect_all(&self->machinesboxgen);
	for (p = machinelist; p != NULL; psy_list_next(&p)) {
		uintptr_t slot;

		slot = (uintptr_t)psy_list_entry(p);
		machinesbox_add_sel(&self->machinesboxfx, slot);
		machinesbox_add_sel(&self->machinesboxgen, slot);
	}
}

void gear_on_tabbar_changed(Gear* self, psy_ui_TabBar* sender,
	uintptr_t tabindex)
{
	switch (tabindex) {
	case 0:
	psy_ui_label_set_text(&self->label, "gear.labelgenerator");
		break;
	case 1:
	psy_ui_label_set_text(&self->label, "gear.labeleffect");
		break;
	case 2:
	psy_ui_label_set_text(&self->label, "gear.labelinstruments");
		break;
	case 3:
	psy_ui_label_set_text(&self->label, "gear.labelsamples");
		break;
	default:
		break;		
	}
}
