// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineview.h"
// host
#include "skingraphics.h"
#include "wireview.h"
// audio
#include <exclusivelock.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// MachineViewBar
static void machineviewbar_onsongchanged(MachineViewBar*, Workspace*,
	int flag, psy_audio_Song*);
void machineviewbar_setmachines(MachineViewBar*, psy_audio_Machines*);
static void machineviewbar_onmixerconnectmodeclick(MachineViewBar*,
	psy_ui_Component* sender);
static void machineviewbar_updateconnectasmixersend(MachineViewBar*);
static void machineviewbar_onmachineinsert(MachineViewBar*,
	psy_audio_Machines*, uintptr_t slot);
static void machineviewbar_onmachineremoved(MachineViewBar*,
	psy_audio_Machines*, uintptr_t slot);

void machineviewbar_init(MachineViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(machineviewbar_base(self), parent);	
	psy_ui_component_setdefaultalign(machineviewbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_margin_make(
			psy_ui_value_makepx(0.0), psy_ui_value_makeew(4.0),
			psy_ui_value_makepx(0.0), psy_ui_value_makepx(0.0)));	
	psy_ui_checkbox_init_text(&self->mixersend, machineviewbar_base(self),
		"machineview.connect-to-mixer-send-return-input");		
	psy_ui_component_hide(psy_ui_checkbox_base(&self->mixersend));
	psy_signal_connect(&self->mixersend.signal_clicked, self,
		machineviewbar_onmixerconnectmodeclick);		
	psy_ui_label_init(&self->status, machineviewbar_base(self));
	psy_ui_label_preventtranslation(&self->status);
	psy_ui_label_setcharnumber(&self->status, 44.0);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		machineviewbar_onsongchanged);
	if (workspace_song(workspace)) {
		machineviewbar_setmachines(self,
			psy_audio_song_machines(workspace_song(workspace)));
	} else {
		machineviewbar_setmachines(self, NULL);
	}
}

void machineviewbar_settext(MachineViewBar* self, const char* text)
{	
	psy_ui_label_settext(&self->status, text);
	psy_ui_label_fadeout(&self->status);
}

void machineviewbar_onmixerconnectmodeclick(MachineViewBar* self,
	psy_ui_Component* sender)
{
	if (psy_ui_checkbox_checked(&self->mixersend)) {
		psy_audio_machines_connectasmixersend(self->machines);		
	} else {
		psy_audio_machines_connectasmixerinput(self->machines);		
	}    
}

void machineviewbar_updateconnectasmixersend(MachineViewBar* self)
{
	if (self->machines &&
		psy_audio_machines_isconnectasmixersend(self->machines)) {
		psy_ui_checkbox_check(&self->mixersend);
	} else {
		psy_ui_checkbox_disablecheck(&self->mixersend);
	}
}

void machineviewbar_onsongchanged(MachineViewBar* self, Workspace* sender,
	int flag, psy_audio_Song* song)
{		
	if (song) {
		machineviewbar_setmachines(self, psy_audio_song_machines(song));
	} else {
		machineviewbar_setmachines(self, NULL);
	}		
}

void machineviewbar_setmachines(MachineViewBar* self,
	psy_audio_Machines* machines)
{
	self->machines = machines;
	if (machines) {		
		psy_signal_connect(&self->machines->signal_insert, self,
			machineviewbar_onmachineinsert);
		psy_signal_connect(&self->machines->signal_removed, self,
			machineviewbar_onmachineremoved);
	}
	machineviewbar_updateconnectasmixersend(self);
}

void machineviewbar_onmachineinsert(MachineViewBar* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	if (psy_audio_machines_hasmixer(sender)) {		
		psy_ui_component_show_align(psy_ui_checkbox_base(&self->mixersend));
	}
	if (psy_audio_machines_at(sender, slot)) {
		char text[128];

		psy_snprintf(text, 128, "%s inserted at slot %u",
			psy_audio_machine_editname(psy_audio_machines_at(sender, slot)),
			(unsigned int)slot);
		machineviewbar_settext(self, text);
	}	
}

void machineviewbar_onmachineremoved(MachineViewBar* self,
	psy_audio_Machines* sender, uintptr_t slot)
{	
	char text[128];

	if (!psy_audio_machines_hasmixer(sender)) {		
		psy_ui_component_hide_align(psy_ui_checkbox_base(&self->mixersend));
	}		
	psy_snprintf(text, 128, "Machine removed from slot %u",
		(unsigned int)slot);
	machineviewbar_settext(self, text);	
}

// MachineView
static void machineview_ondestroy(MachineView*);
static void machineview_updatetext(MachineView*, psy_Translator*);
static void machineview_onsongchanged(MachineView*, Workspace*, int flag,
	psy_audio_Song*);
static void machineview_onmousedown(MachineView*, psy_ui_MouseEvent*);
static void machineview_onmouseup(MachineView*, psy_ui_MouseEvent*);
static void machineview_onmousedoubleclick(MachineView*, psy_ui_MouseEvent*);
static void machineview_onkeydown(MachineView*, psy_ui_KeyEvent*);
static void machineview_onfocus(MachineView*, psy_ui_Component* sender);
static void machineview_selectsection(MachineView*, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options);
static void machineview_onconfigure(MachineView*, MachineViewConfig*,
	psy_Property*);
static void machineview_onthemechanged(MachineView*, MachineViewConfig*,
	psy_Property* theme);
static void machineview_onnewmachineselected(MachineView*,
	psy_ui_Component* sender, psy_Property*);

static psy_ui_ComponentVtable machineview_vtable;
static bool machineview_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machineview_vtable_init(MachineView* self)
{
	if (!machineview_vtable_initialized) {
		machineview_vtable = *(self->component.vtable);
		machineview_vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			machineview_ondestroy;
		machineview_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			machineview_onmousedown;
		machineview_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)
			machineview_onmouseup;
		machineview_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			machineview_onmousedoubleclick;
		machineview_vtable.onkeydown = (psy_ui_fp_component_onkeyevent)
			machineview_onkeydown;
		machineview_vtable_initialized = TRUE;
	}
	return &machineview_vtable;
}

void machineview_init(MachineView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	psy_ui_component_init(machineview_base(self), parent);	
	psy_ui_component_setvtable(machineview_base(self),
		machineview_vtable_init(self));
	psy_ui_component_setbackgroundmode(machineview_base(self),
		psy_ui_BACKGROUND_NONE);
	self->workspace = workspace;	
	// skin init
	machineviewskin_init(&self->skin,	
		psycleconfig_macview(workspace_conf(self->workspace))->theme,
		dirconfig_skins(&self->workspace->config.directories));	
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		machineview_onsongchanged);
	psy_ui_notebook_init(&self->notebook, &self->component);	
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	// wireview
	machinewireview_init(&self->wireview,
		psy_ui_notebook_base(&self->notebook), tabbarparent, &self->skin,
		workspace);
	psy_ui_scroller_init(&self->scroller, &self->wireview.component,
		psy_ui_notebook_base(&self->notebook));	
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	// stackview
	machinestackview_init(&self->stackview,
		psy_ui_notebook_base(&self->notebook), tabbarparent, NULL,
		&self->skin, workspace);
	psy_ui_scroller_init(&self->stackscroller, &self->stackview.component,
		psy_ui_notebook_base(&self->notebook));
	// newmachine
	newmachine_init(&self->newmachine, psy_ui_notebook_base(&self->notebook),
		&self->skin, self->workspace);		
	psy_ui_component_setalign(&self->newmachine.component,
		psy_ui_ALIGN_CLIENT);
	tabbar_init(&self->tabbar, tabbarparent);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	tabbar_append_tabs(&self->tabbar, "machineview.wires",
		"machineview.stack", "machineview.new-machine", NULL);
	tabbar_tab(&self->tabbar, 0)->margin.left = psy_ui_value_makeew(1.0);
	psy_signal_connect(&self->component.signal_selectsection, self,
		machineview_selectsection);
	psy_ui_notebook_select(&self->notebook, SECTION_ID_MACHINEVIEW_WIRES);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	psy_signal_connect(&self->newmachine.signal_selected, self,
		machineview_onnewmachineselected);			
	psy_signal_connect(&self->component.signal_focus, self,
		machineview_onfocus);
	psy_signal_connect(
		&psycleconfig_macview(workspace_conf(workspace))->signal_themechanged,
		self, machineview_onthemechanged);
	psy_signal_connect(
		&psycleconfig_macview(workspace_conf(workspace))->signal_changed,
		self, machineview_onconfigure);		
}

void machineview_ondestroy(MachineView* self)
{
	machineviewskin_dispose(&self->skin);
}

void machineview_onmousedoubleclick(MachineView* self, psy_ui_MouseEvent* ev)
{		
	if (ev->button == 1 && tabbar_selected(&self->tabbar) ==
			SECTION_ID_MACHINEVIEW_WIRES) {
		psy_ui_component_selectsection(machineview_base(self),
			SECTION_ID_MACHINEVIEW_NEWMACHINE,
			NEWMACHINE_APPEND);
	}
}

void machineview_onmousedown(MachineView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_mouseevent_stoppropagation(ev);
}

void machineview_onmouseup(MachineView* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 2) {
		if (tabbar_selected(&self->tabbar) == SECTION_ID_MACHINEVIEW_WIRES) {
			workspace_togglegear(self->workspace);
		} else if (tabbar_selected(&self->tabbar) ==
				SECTION_ID_MACHINEVIEW_NEWMACHINE) {
			tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);			
		}
	}
}

void machineview_onkeydown(MachineView* self, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_ESCAPE) {
		if (tabbar_selected(&self->tabbar) ==
				SECTION_ID_MACHINEVIEW_NEWMACHINE) {
			tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);
			psy_ui_component_setfocus(machinewireview_base(&self->wireview));
		} else if (self->workspace->gearvisible) {
			workspace_togglegear(self->workspace);
		}
		psy_ui_keyevent_stoppropagation(ev);
	} 
}

void machineview_onfocus(MachineView* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(machinewireview_base(&self->wireview));
}


void machineview_selectsection(MachineView* self, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options)
{
	switch (section) {
		case SECTION_ID_MACHINEVIEW_NEWMACHINE:
			if (options & NEWMACHINE_INSERT) {
				newmachine_insertmode(&self->newmachine);
			} else if (options & NEWMACHINE_APPEND) {
				newmachine_appendmode(&self->newmachine);
			} else if (options & NEWMACHINE_ADDEFFECT) {
				self->wireview.addeffect = 1;
				self->wireview.randominsert = 0;
				newmachine_addeffectmode(&self->newmachine);
			}
			break;
		default:
			break;
	}
	tabbar_select(&self->tabbar, section);
}

void machineview_onsongchanged(MachineView* self, Workspace* workspace,
	int flag, psy_audio_Song* song)
{	
	tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);	
}

void machineview_onconfigure(MachineView* self, MachineViewConfig* sender,
	psy_Property* property)
{
	machinewireview_configure(&self->wireview, sender);		
}

void machineview_onthemechanged(MachineView* self, MachineViewConfig* sender,
	psy_Property* theme)
{
	machineviewskin_settheme(&self->skin, theme,
		dirconfig_skins(&self->workspace->config.directories));
	machinewireview_updateskin(&self->wireview);
	machinestackview_updateskin(&self->stackview);
	newmachine_updateskin(&self->newmachine);
	psy_ui_component_invalidate(&self->component);
}

void machineview_onnewmachineselected(MachineView* self,
	psy_ui_Component* sender, psy_Property* plugininfo)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machinefactory_makemachinefrompath(
		&self->workspace->machinefactory,
		(MachineType)psy_property_at_int(plugininfo, "type", MACH_UNDEFINED),
		psy_property_at_str(plugininfo, "path", ""),
		psy_property_at_int(plugininfo, "shellidx", 0),
		psy_INDEX_INVALID);
	if (machine) {
		intptr_t favorite;

		favorite = psy_property_at_int(plugininfo, "favorite", 0);
		psy_property_set_int(plugininfo, "favorite", ++favorite);
		if (self->wireview.addeffect) {
			uintptr_t slot;

			slot = psy_audio_machines_append(self->wireview.machines, machine);
			psy_audio_machines_disconnect(self->wireview.machines,
				self->wireview.selectedwire);
			psy_audio_machines_connect(self->wireview.machines,
				psy_audio_wire_make(self->wireview.selectedwire.src, slot));
			psy_audio_machines_connect(self->wireview.machines,
				psy_audio_wire_make(slot, self->wireview.selectedwire.dst));
			psy_audio_machines_select(self->wireview.machines, slot);
			self->wireview.addeffect = 0;
		} else if (self->newmachine.mode == NEWMACHINE_INSERT) {
			psy_audio_machines_insert(self->wireview.machines,
				psy_audio_machines_selected(self->wireview.machines), machine);
		} else {
			psy_audio_machines_select(self->wireview.machines,
				psy_audio_machines_append(self->wireview.machines, machine));
		}
		tabbar_select(&self->tabbar, 0);
	} else {
		workspace_outputerror(self->workspace,
			self->workspace->machinefactory.errstr);
	}
}
