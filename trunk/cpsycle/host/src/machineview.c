// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineview.h"
// host
#include "resources/resource.h"
#include "skingraphics.h"
#include "styles.h"
#include "wireview.h"
// audio
#include <exclusivelock.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// MachineView
// prototypes
static void machineview_ondestroy(MachineView*);
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
// vtable
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
// implementation
void machineview_init(MachineView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	Tab* tab;

	psy_ui_component_init(machineview_base(self), parent, NULL);
	psy_ui_component_setvtable(machineview_base(self),
		machineview_vtable_init(self));
	psy_ui_component_setbackgroundmode(machineview_base(self),
		psy_ui_NOBACKGROUND);
	psy_ui_component_setstyletypes(&self->component,
		STYLE_MACHINEVIEW, STYLE_MACHINEVIEW, STYLE_MACHINEVIEW);
	self->workspace = workspace;	
	// skin init
	machineviewskin_init(&self->skin,	
		psycleconfig_macview(workspace_conf(self->workspace))->theme,
		dirconfig_skins(&self->workspace->config.directories));	
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		machineview_onsongchanged);
	// Machine Properties
	machineproperties_init(&self->properties, &self->component, NULL,
		&self->skin, workspace);
	psy_ui_component_setalign(&self->properties.component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->properties.component);
	psy_ui_notebook_init(&self->notebook, &self->component);	
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	// wireview
	machinewireview_init(&self->wireview,
		psy_ui_notebook_base(&self->notebook), tabbarparent, &self->skin,
		workspace);
	psy_ui_scroller_init(&self->scroller, &self->wireview.component,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	// stackview
	machinestackview_init(&self->stackview,
		psy_ui_notebook_base(&self->notebook), tabbarparent,
		&self->skin, NULL, workspace);
	// newmachine
	newmachine_init(&self->newmachine, psy_ui_notebook_base(&self->notebook),
		&self->skin, self->workspace);		
	psy_ui_component_setalign(&self->newmachine.component,
		psy_ui_ALIGN_CLIENT);
	tabbar_init(&self->tabbar, tabbarparent);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	tab = tabbar_append(&self->tabbar, "machineview.wires");
	psy_ui_bitmap_loadresource(&tab->icon, IDB_WIRES_DARK);
	psy_ui_bitmap_settransparency(&tab->icon, psy_ui_colour_make(0x00FFFFFF));	
	tab = tabbar_append(&self->tabbar, "machineview.stack");
	psy_ui_bitmap_loadresource(&tab->icon, IDB_MATRIX_DARK);
	psy_ui_bitmap_settransparency(&tab->icon, psy_ui_colour_make(0x00FFFFFF));
	tab = tabbar_append(&self->tabbar, "machineview.new-machine");		
	tabbar_tab(&self->tabbar, 0)->margin.left = psy_ui_value_makeew(1.0);
	psy_ui_bitmap_loadresource(&tab->icon, IDB_NEWMACHINE_DARK);
	psy_ui_bitmap_settransparency(&tab->icon, psy_ui_colour_make(0x00FFFFFF));	
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
		if (tabbar_selected(&self->tabbar) == SECTION_ID_MACHINEVIEW_WIRES) {
			self->newmachine.restoresection = SECTION_ID_MACHINEVIEW_WIRES;
		} else if (tabbar_selected(&self->tabbar) == SECTION_ID_MACHINEVIEW_STACK) {
			self->newmachine.restoresection = SECTION_ID_MACHINEVIEW_STACK;
		}
		psy_ui_component_selectsection(machineview_base(self),
			SECTION_ID_MACHINEVIEW_NEWMACHINE,
			NEWMACHINE_APPEND);
		psy_ui_mouseevent_stoppropagation(ev);
	}
}

void machineview_onmousedown(MachineView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_mouseevent_stoppropagation(ev);
}

void machineview_onmouseup(MachineView* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 2) {
		if (tabbar_selected(&self->tabbar) == SECTION_ID_MACHINEVIEW_WIRES ||
			tabbar_selected(&self->tabbar) == SECTION_ID_MACHINEVIEW_STACK) {
			if (!psy_ui_component_visible(&self->properties.component)) {
				psy_ui_component_show_align(&self->properties.component);
			}			
		} else if (tabbar_selected(&self->tabbar) ==
				SECTION_ID_MACHINEVIEW_NEWMACHINE) {			 
			tabbar_select(&self->tabbar, self->newmachine.restoresection);
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
			} else if (options & NEWMACHINE_APPENDSTACK) {
				self->newmachine.appendstack = TRUE;
				self->newmachine.restoresection = SECTION_ID_MACHINEVIEW_STACK;
				newmachine_appendmode(&self->newmachine);
			} else if (options & NEWMACHINE_ADDEFFECT) {
				self->wireview.addeffect = 1;
				self->wireview.randominsert = 0;
				newmachine_addeffectmode(&self->newmachine);
			} else if (options & NEWMACHINE_ADDEFFECTSTACK) {
				self->wireview.addeffect = 1;
				self->wireview.randominsert = 0;				
				newmachine_addeffectmode(&self->newmachine);
				self->newmachine.restoresection = NEWMACHINE_ADDEFFECTSTACK;
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
	machinestackpane_updateskin(&self->stackview.pane);
	newmachine_updateskin(&self->newmachine);
	psy_ui_component_invalidate(&self->component);
}

void machineview_onnewmachineselected(MachineView* self,
	psy_ui_Component* sender, psy_Property* plugininfo)
{
	psy_audio_Machine* machine;
	psy_audio_MachineInfo machineinfo;	
	
	machineinfo_init(&machineinfo);
	newmachine_selectedmachineinfo(&self->newmachine, &machineinfo);
	if (self->newmachine.restoresection == NEWMACHINE_ADDEFFECTSTACK) {
		self->newmachine.restoresection = psy_INDEX_INVALID;
		machinestackview_addeffect(&self->stackview, &machineinfo);
		machineinfo_dispose(&machineinfo);
		tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_STACK);
		return;
	}	
	machine = psy_audio_machinefactory_make_info(
		&self->workspace->machinefactory, &machineinfo);
	machineinfo_dispose(&machineinfo);
	if (machine) {
		if (self->newmachine.appendstack &&
			psy_audio_machine_mode(machine) == psy_audio_MACHMODE_FX) {
			psy_audio_machine_setbus(machine);
		}
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
		if (self->newmachine.restoresection == SECTION_ID_MACHINEVIEW_STACK) {
			self->newmachine.restoresection = psy_INDEX_INVALID;
			tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_STACK);			
		} else {
			tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);
		}
		
	} else {
		workspace_outputerror(self->workspace,
			self->workspace->machinefactory.errstr);
	}	
	self->newmachine.appendstack = FALSE;
}
