// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineproperties.h"
// host
#include "machineviewskin.h"
// platform
#include "../../detail/portable.h"

static void machineproperties_onsongchanged(MachineProperties*, Workspace*,
	int flag, psy_audio_Song*);
static void machineproperties_connectsongsignals(MachineProperties*);
static void machineproperties_ontogglebus(MachineProperties*,
	psy_ui_Component* sender);
static void machineproperties_onremove(MachineProperties*,
	psy_ui_Component* sender);
static void machineproperties_onhide(MachineProperties*,
	psy_ui_Component* sender);
static void machineproperties_updateskin(MachineProperties*);
static void machineproperties_onmachineselected(MachineProperties*,
	psy_audio_Machines*, uintptr_t slot);
static void machineproperties_onmachineremoved(MachineProperties*,
	psy_audio_Machines*, uintptr_t slot);
static void machineproperties_oneditaccept(MachineProperties*,
	psy_ui_Edit* sender);
static void machineproperties_oneditreject(MachineProperties*,
	psy_ui_Edit* sender);
static void machineproperties_ontogglemute(MachineProperties*,
	psy_ui_Button* sender);
static void machineproperties_onsolobypass(MachineProperties*,
	psy_ui_Button* sender);

static psy_ui_ComponentVtable machineproperties_vtable;
static bool machineproperties_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machineproperties_vtable_init(MachineProperties* self)
{
	if (!machineproperties_vtable_initialized) {
		machineproperties_vtable = *(self->component.vtable);
		machineproperties_vtable_initialized = TRUE;
	}
	return &machineproperties_vtable;
}

void machineproperties_init(MachineProperties* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, MachineViewSkin* skin,
	Workspace* workspace)
{
	self->workspace = workspace;
	self->machine = machine;
	self->machines = (workspace_song(workspace)) ? &workspace_song(workspace)->machines : NULL;
	self->skin = skin;
	self->macid = psy_INDEX_INVALID;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setvtable(&self->component,
		machineproperties_vtable_init(self));
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make(psy_ui_value_makepx(0), psy_ui_value_makeew(2.0),
			psy_ui_value_makeeh(1.0), psy_ui_value_makepx(0)));
	machineproperties_updateskin(self);
	psy_ui_button_init_text_connect(&self->issolobypass, &self->component,
		NULL, "machineview.pwr", self, machineproperties_onsolobypass);
	psy_ui_button_init_text_connect(&self->ismute, &self->component,
		NULL, "machineview.mute", self, machineproperties_ontogglemute);
	psy_ui_button_init_text_connect(&self->isbus, &self->component, NULL,
		"Bus", self, machineproperties_ontogglebus);
	psy_ui_label_init_text(&self->namelabel, &self->component, NULL,
		"machineview.editname");	
	psy_ui_edit_init(&self->nameedit, &self->component);
	psy_ui_edit_settext(&self->nameedit, "No Machine");
	psy_ui_edit_setcharnumber(&self->nameedit, 40);	
	psy_ui_edit_enableinputfield(&self->nameedit);
	psy_signal_connect(&self->nameedit.signal_accept, self,
		machineproperties_oneditaccept);
	psy_signal_connect(&self->nameedit.signal_reject, self,
		machineproperties_oneditreject);
	psy_ui_button_init_text_connect(&self->remove, &self->component, NULL,
		"machineview.delete", self, machineproperties_onremove);
	psy_ui_button_init_connect(&self->cancel, &self->component, NULL, self,
		machineproperties_onhide);
	psy_ui_button_preventtranslation(&self->cancel);
	psy_ui_button_settext(&self->cancel, "X");
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		machineproperties_onsongchanged);
	machineproperties_connectsongsignals(self);
}

void machineproperties_setmachine(MachineProperties* self,
	psy_audio_Machine* machine)
{
	self->machine = machine;
	if (self->machine) {
		psy_ui_edit_settext(&self->nameedit,
			psy_audio_machine_editname(machine));
		if (psy_audio_machine_mode(machine) == psy_audio_MACHMODE_GENERATOR) {
			psy_ui_component_hide_align(psy_ui_button_base(&self->isbus));
		} else {
			psy_ui_component_show_align(psy_ui_button_base(&self->isbus));
		}
		psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
	} else {
		self->macid = psy_INDEX_INVALID;
		psy_ui_edit_settext(&self->nameedit, "");
		psy_ui_component_hide_align(psy_ui_button_base(&self->isbus));
		psy_ui_button_disablehighlight(&self->issolobypass);
		psy_ui_button_disablehighlight(&self->isbus);
		psy_ui_button_disablehighlight(&self->ismute);		
		psy_ui_component_align(&self->component);
		psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
	}	
}

void machineproperties_ontogglebus(MachineProperties* self,
	psy_ui_Component* sender)
{
	if (self->machine) {
		if (psy_audio_machine_isbus(self->machine)) {
			psy_audio_machine_unsetbus(self->machine);			
		} else {
			psy_audio_machine_setbus(self->machine);			
		}
	}
}

void machineproperties_onhide(MachineProperties* self,
	psy_ui_Component* sender)
{
	psy_ui_component_hide_align(&self->component);
}

void machineproperties_onremove(MachineProperties* self,
	psy_ui_Component* sender)
{
	if (self->macid != psy_INDEX_INVALID) {
		uintptr_t macid;

		macid = self->macid;				
		machineproperties_setmachine(self, NULL);
		psy_audio_machines_remove(self->machines, macid, TRUE);
	}
}

void machineproperties_onmachinenamechanged(MachineProperties* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(machines, slot);
	if (machine && machine == self->machine) {
		psy_ui_edit_settext(&self->nameedit,
			psy_audio_machine_editname(machine));
	}
}

void machineproperties_oneditaccept(MachineProperties* self,
	psy_ui_Edit* sender)
{
	if (workspace_song(self->workspace) && self->machine) {
		psy_audio_machine_seteditname(self->machine,
			psy_ui_edit_text(&self->nameedit));
	}
	psy_ui_component_setfocus(&self->component);	
}

void machineproperties_oneditreject(MachineProperties* self,
	psy_ui_Edit* sender)
{
	if (self->machine) {
		psy_ui_edit_settext(&self->nameedit,
			psy_audio_machine_editname(self->machine));
	}	
	psy_ui_component_setfocus(&self->component);
}

void machineproperties_onsongchanged(MachineProperties* self, Workspace* workspace, int flag,
	psy_audio_Song* song)
{
	self->machines = (workspace_song(workspace))
		? &workspace_song(workspace)->machines
		: NULL;
	machineproperties_setmachine(self, NULL);
	machineproperties_connectsongsignals(self);
}

void machineproperties_connectsongsignals(MachineProperties* self)
{
	if (workspace_song(self->workspace)) {
		psy_signal_connect(&self->machines->signal_slotchange, self,
			machineproperties_onmachineselected);		
		psy_signal_connect(&self->machines->signal_removed, self,
			machineproperties_onmachineremoved);	
	}
}

void machineproperties_updateskin(MachineProperties* self)
{
	psy_ui_component_setbackgroundcolour(&self->component,
		self->skin->colour);
	psy_ui_component_setcolour(&self->component,
		self->skin->wirecolour);
}

void machineproperties_onmachineselected(MachineProperties* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	machineproperties_setmachine(self, psy_audio_machines_at(sender, slot));
	self->macid = slot;
}

void machineproperties_onmachineremoved(MachineProperties* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	if (self->macid == slot) {
		machineproperties_setmachine(self, NULL);
	}
}

void machineproperties_idle(MachineProperties* self)
{
	if (self->machine) {
		if (psy_audio_machine_muted(self->machine)) {
			psy_ui_button_highlight(&self->ismute);
		} else {
			psy_ui_button_disablehighlight(&self->ismute);
		}
		if (psy_audio_machine_hasstandby(self->machine)) {
			psy_ui_button_highlight(&self->issolobypass);
		} else {
			psy_ui_button_disablehighlight(&self->issolobypass);
		}
		if (psy_audio_machine_isbus(self->machine)) {
			psy_ui_button_highlight(&self->isbus);
		} else {
			psy_ui_button_disablehighlight(&self->isbus);
		}
	}
}

void machineproperties_ontogglemute(MachineProperties* self,
	psy_ui_Button* sender)
{
	if (self->machine) {
		if (psy_audio_machine_muted(self->machine)) {
			psy_audio_machine_unmute(self->machine);
		} else {
			psy_audio_machine_mute(self->machine);
		}
	}
}

void machineproperties_onsolobypass(MachineProperties* self,
	psy_ui_Button* sender)
{
	if (self->machine) {
		if (psy_audio_machine_hasstandby(self->machine)) {
			psy_audio_machine_deactivatestandby(self->machine);
		} else {
			psy_audio_machine_standby(self->machine);
		}
	}
}
