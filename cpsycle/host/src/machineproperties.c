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
static void machineproperties_onapply(MachineProperties*,
	psy_ui_Component* sender);
static void machineproperties_onremove(MachineProperties*,
	psy_ui_Component* sender);
static void machineproperties_onhide(MachineProperties*,
	psy_ui_Component* sender);
static void machineproperties_onkeydown(MachineProperties*, psy_ui_KeyEvent*);
static void machineproperties_onkeyup(MachineProperties*, psy_ui_KeyEvent*);
static void machineproperties_onfocus(MachineProperties*);
static void machineproperties_updateskin(MachineProperties*);
static void machineproperties_onmachineselected(MachineProperties*,
	psy_audio_Machines*, uintptr_t slot);
static void machineproperties_onmachineinsert(MachineProperties*,
	psy_audio_Machines*, uintptr_t slot);
static void machineproperties_onmachineremoved(MachineProperties*,
	psy_audio_Machines*, uintptr_t slot);
static void machineproperties_onconnected(MachineProperties*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machineproperties_ondisconnected(MachineProperties*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);

static psy_ui_ComponentVtable machineproperties_vtable;
static bool machineproperties_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machineproperties_vtable_init(MachineProperties* self)
{
	if (!machineproperties_vtable_initialized) {
		machineproperties_vtable = *(self->component.vtable);
		machineproperties_vtable.onkeydown = (psy_ui_fp_component_onkeyevent)
			machineproperties_onkeydown;
		machineproperties_vtable.onkeyup = (psy_ui_fp_component_onkeyevent)
			machineproperties_onkeyup;
		machineproperties_vtable.onfocus = (psy_ui_fp_component_onfocus)
			machineproperties_onfocus;
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
	psy_ui_button_init(&self->issolobypass, &self->component, NULL);
	psy_ui_button_init_text(&self->ismute, &self->component, NULL,
		"Mute");
	psy_ui_button_init_text(&self->isbus, &self->component, NULL,
		"Bus");
	psy_signal_connect(&self->isbus.signal_clicked, self,
		machineproperties_ontogglebus);
	psy_ui_label_init(&self->namelabel, &self->component);
	psy_ui_label_settext(&self->namelabel, "Machine Name");
	psy_ui_label_settextalignment(&self->namelabel, psy_ui_ALIGNMENT_LEFT);
	psy_ui_edit_init(&self->nameedit, &self->component);
	psy_ui_edit_settext(&self->nameedit, "No Machine");
	psy_ui_edit_setcharnumber(&self->nameedit, 40);	
	psy_ui_button_init_connect(&self->applybutton, &self->component, NULL, self,
		machineproperties_onapply);
	psy_ui_button_settext(&self->applybutton, "Change");	
	psy_ui_button_init_text_connect(&self->remove, &self->component, NULL,
		"Delete", self, machineproperties_onremove);
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
		psy_ui_edit_settext(&self->nameedit, psy_audio_machine_editname(machine));
		if (psy_audio_machine_isbus(self->machine)) {
			psy_ui_button_highlight(&self->isbus);
		} else {
			psy_ui_button_disablehighlight(&self->isbus);
		}
		if (psy_audio_machine_muted(machine)) {
			psy_ui_button_highlight(&self->ismute);
		} else {
			psy_ui_button_disablehighlight(&self->ismute);
		}
		if (psy_audio_machine_mode(machine) == psy_audio_MACHMODE_GENERATOR) {
			psy_ui_button_settext(&self->issolobypass, "Solo");
			psy_ui_component_hide_align(psy_ui_button_base(&self->isbus));			
		} else {
			psy_ui_button_settext(&self->issolobypass, "Bypass");
			psy_ui_component_show_align(psy_ui_button_base(&self->isbus));
		}
	} else {
		self->macid = psy_INDEX_INVALID;
		psy_ui_edit_settext(&self->nameedit, "");
		psy_ui_component_hide_align(psy_ui_button_base(&self->isbus));
		psy_ui_button_disablehighlight(&self->isbus);
		psy_ui_button_disablehighlight(&self->ismute);
	}	
}

void machineproperties_ontogglebus(MachineProperties* self,
	psy_ui_Component* sender)
{
	if (self->machine) {
		if (psy_audio_machine_isbus(self->machine)) {
			psy_audio_machine_unsetbus(self->machine);
			psy_ui_button_disablehighlight(&self->isbus);
		} else {
			psy_audio_machine_setbus(self->machine);
			psy_ui_button_highlight(&self->isbus);
		}
	}
}

void machineproperties_onapply(MachineProperties* self,
	psy_ui_Component* sender)
{
	if (workspace_song(self->workspace) && self->machine) {
		psy_audio_machine_seteditname(self->machine, 
			psy_ui_edit_text(&self->nameedit));		
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

void machineproperties_onkeydown(MachineProperties* self, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_RETURN) {
		machineproperties_onapply(self, &self->component);
		psy_ui_keyevent_preventdefault(ev);
	} else if (ev->keycode == psy_ui_KEY_ESCAPE) {
		psy_ui_edit_settext(&self->nameedit,
			psy_audio_machine_editname(self->machine));
		psy_ui_keyevent_preventdefault(ev);
	}	
	psy_ui_keyevent_stoppropagation(ev);
}

void machineproperties_onkeyup(MachineProperties* self, psy_ui_KeyEvent* ev)
{
	psy_ui_keyevent_stoppropagation(ev);
}

void machineproperties_onfocus(MachineProperties* self)
{	
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

void machineproperties_onsongchanged(MachineProperties* self, Workspace* workspace, int flag,
	psy_audio_Song* song)
{
	self->machines = (workspace_song(workspace))
		? &workspace_song(workspace)->machines
		: NULL;
	machineproperties_connectsongsignals(self);
}

void machineproperties_connectsongsignals(MachineProperties* self)
{
	if (workspace_song(self->workspace)) {
		psy_signal_connect(&self->machines->signal_slotchange, self,
			machineproperties_onmachineselected);		
		psy_signal_connect(&self->machines->signal_insert, self,
			machineproperties_onmachineinsert);
		psy_signal_connect(&self->machines->signal_removed, self,
			machineproperties_onmachineremoved);
		psy_signal_connect(&self->machines->connections.signal_connected, self,
			machineproperties_onconnected);
		psy_signal_connect(&self->machines->connections.signal_disconnected, self,
			machineproperties_ondisconnected);
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

void machineproperties_onmachineinsert(MachineProperties* self,
	psy_audio_Machines* sender, uintptr_t slot)
{

}

void machineproperties_onmachineremoved(MachineProperties* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	if (self->macid == slot) {
		machineproperties_setmachine(self, NULL);
	}
}

void machineproperties_onconnected(MachineProperties* self,
	psy_audio_Connections* sender, uintptr_t outputslot, uintptr_t inputslot)
{

}

void machineproperties_ondisconnected(MachineProperties* self,
	psy_audio_Connections* sender, uintptr_t outputslot, uintptr_t inputslot)
{
	
}

