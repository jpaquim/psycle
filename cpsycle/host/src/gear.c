// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "gear.h"
#include "songio.h"


static void gearbuttons_updatetext(GearButtons*, Translator*);
static void gearbuttons_onlanguagechanged(GearButtons*, Translator* sender);

void gearbuttons_init(GearButtons* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin margin;	

	psy_ui_component_init(gearbuttons_base(self), parent);	
	psy_ui_component_enablealign(gearbuttons_base(self));	
	psy_ui_button_init(&self->createreplace, gearbuttons_base(self));
	psy_ui_button_init(&self->del, gearbuttons_base(self));
	psy_ui_button_init(&self->parameters, gearbuttons_base(self));
	psy_ui_button_init(&self->properties, gearbuttons_base(self));
	psy_ui_button_init(&self->exchange, gearbuttons_base(self));
	psy_ui_button_init(&self->clone, gearbuttons_base(self));
	psy_ui_button_init(&self->showmaster, gearbuttons_base(self));
	gearbuttons_updatetext(self, &workspace->translator);
	psy_signal_connect(&workspace->signal_languagechanged, self,
		gearbuttons_onlanguagechanged);
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));				
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(gearbuttons_base(self), psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		&margin));
}

void gearbuttons_updatetext(GearButtons* self, Translator* translator)
{
	psy_ui_button_settext(&self->createreplace,
		translator_translate(translator, "gear.create-replace"));
	psy_ui_button_settext(&self->del,
		translator_translate(translator, "gear.delete"));
	psy_ui_button_settext(&self->parameters,
		translator_translate(translator, "gear.parameters"));
	psy_ui_button_settext(&self->properties,
		translator_translate(translator, "gear.properties"));
	psy_ui_button_settext(&self->exchange,
		translator_translate(translator, "gear.exchange"));
	psy_ui_button_settext(&self->clone,
		translator_translate(translator, "gear.clone"));
	psy_ui_button_settext(&self->showmaster,
		translator_translate(translator, "gear.show-master"));
}

void gearbuttons_onlanguagechanged(GearButtons* self, Translator* sender)
{
	gearbuttons_updatetext(self, sender);
	psy_ui_component_align(gearbuttons_base(self));
}

static void gear_updatetext(Gear*, Translator*);
static void gear_onlanguagechanged(Gear*, Translator* sender);
static void gear_ondelete(Gear*, psy_ui_Component* sender);
static void gear_onsongchanged(Gear*, Workspace*, int flag, psy_audio_SongFile*);
static void gear_onclone(Gear*, psy_ui_Component* sender);
static void gear_onexchange(Gear* self, psy_ui_Component* sender);
static void gear_onparameters(Gear*, psy_ui_Component* sender);
static void gear_onmaster(Gear*, psy_ui_Component* sender);
static void gear_onhide(Gear*);

void gear_init(Gear* self, psy_ui_Component* parent, Workspace* workspace)
{		
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	self->workspace = workspace;
	self->machines = &workspace->song->machines;
	psy_signal_connect(&workspace->signal_songchanged, self,
		gear_onsongchanged);
	psy_ui_component_init(gear_base(self), parent);
	psy_ui_component_enablealign(gear_base(self));
	// titlebar
	psy_ui_component_init(&self->titlebar, gear_base(self));
	psy_ui_component_setalign(&self->titlebar, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->titlebar, &margin);
	psy_ui_label_init(&self->title, &self->titlebar);
	psy_ui_label_settext(&self->title, "Gear Rack");
	psy_ui_component_setalign(&self->title.component, psy_ui_ALIGN_CLIENT);
	psy_ui_button_init(&self->hide, &self->titlebar);
	psy_ui_button_settext(&self->hide, "X");
	psy_signal_connect(&self->hide.signal_clicked, self, gear_onhide);
	psy_ui_component_setalign(&self->hide.component, psy_ui_ALIGN_RIGHT);
	// client
	tabbar_init(&self->tabbar, gear_base(self));	
	tabbar_append_tabs(&self->tabbar, "", "", "", "", NULL);
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
		&workspace->song->instruments, workspace);
	samplesbox_init(&self->samplesbox, psy_ui_notebook_base(&self->notebook),
		&workspace->song->samples, workspace);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	tabbar_select(&self->tabbar, 0);
	gearbuttons_init(&self->buttons, gear_base(self), workspace);
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
	gear_updatetext(self, &workspace->translator);
	psy_signal_connect(&self->workspace->signal_languagechanged, self,
		gear_onlanguagechanged);
}

void gear_updatetext(Gear* self, Translator* translator)
{
	tabbar_rename_tabs(&self->tabbar,
		translator_translate(translator, "gear.generators"),
		translator_translate(translator, "gear.effects"),
		translator_translate(translator, "gear.instruments"),
		translator_translate(translator, "gear.waves"),
		NULL);
}

void gear_onlanguagechanged(Gear* self, Translator* sender)
{
	gear_updatetext(self, sender);
}

void gear_ondelete(Gear* self, psy_ui_Component* sender)
{
	switch (self->tabbar.selected) {
		case 0:
			machinesbox_remove(&self->machinesboxgen);
		break;
		case 1:
			machinesbox_remove(&self->machinesboxfx);
		break;
		default:
		break;
	}
}

void gear_onsongchanged(Gear* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
{	
	self->machines = &workspace->song->machines;		
	machinesbox_setmachines(&self->machinesboxgen, &workspace->song->machines);
	machinesbox_setmachines(&self->machinesboxfx, &workspace->song->machines);
	instrumentsbox_setinstruments(&self->instrumentsbox,
		&workspace->song->instruments);
	samplesbox_setsamples(&self->samplesbox, &workspace->song->samples);
	psy_ui_component_invalidate(gear_base(self));
}

void gear_onclone(Gear* self, psy_ui_Component* sender)
{
	switch (tabbar_selected(&self->tabbar)) {
		case 0:
			machinesbox_clone(&self->machinesboxgen);
			break;
		case 1:
			machinesbox_clone(&self->machinesboxfx);
			break;
		default:
			break;
	}
}

void gear_onexchange(Gear* self, psy_ui_Component* sender)
{
	switch (tabbar_selected(&self->tabbar)) {
		case 0:
			machinesbox_exchange(&self->machinesboxgen);
			break;
		case 1:
			machinesbox_exchange(&self->machinesboxfx);
		break;
		default:
			break;
	}
}

void gear_onparameters(Gear* self, psy_ui_Component* sender)
{
	switch (tabbar_selected(&self->tabbar)) {
		case 0:
			machinesbox_showparameters(&self->machinesboxgen);
			break;
		case 1:
			machinesbox_showparameters(&self->machinesboxfx);
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
