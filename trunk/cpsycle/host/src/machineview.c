/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "machineview.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
#include "wireview.h"
/* audio */
#include <exclusivelock.h>

/* MachineView */

/* prototypes */
static void machineview_on_destroyed(MachineView*);
static void machineview_initcomponent(MachineView*, psy_ui_Component* parent);
static void machineview_initpropertiesview(MachineView*);
static void machineview_initnotebook(MachineView*,
	psy_ui_Component* tabbarparent);
static void machineview_initwireview(MachineView*,
	psy_ui_Component* tabbarparent);
static void machineview_initstackview(MachineView*,
	psy_ui_Component* tabbarparent);
static void machineview_initnewmachine(MachineView*,
	psy_ui_Component* tabbarparent);
static void machineview_init_tabbar(MachineView*,
	psy_ui_Component* tabbarparent);
static void machineview_on_tabbar_changed(MachineView*, psy_ui_TabBar* sender,
	uintptr_t index);
static void machineview_connectsignals(MachineView*);
static uintptr_t machineview_section(const MachineView*);
static void machineview_onsongchanged(MachineView*, Workspace* sender);
static void machineview_on_mouse_down(MachineView*, psy_ui_MouseEvent*);
static void machineview_on_mouse_up(MachineView*, psy_ui_MouseEvent*);
static void machineview_onmousedoubleclick(MachineView*, psy_ui_MouseEvent*);
static void machineview_on_key_down(MachineView*, psy_ui_KeyboardEvent*);
static void machineview_on_focus(MachineView*);
static void machineview_selectsection(MachineView*, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options);
static void machineview_onconfigure(MachineView*, MachineViewConfig*,
	psy_Property*);
static void machineview_onnewmachineselected(MachineView*,
	psy_ui_Component* sender, psy_Property*);
static void machineview_onshow(MachineView*);

/* vtable */
static psy_ui_ComponentVtable machineview_vtable;
static bool machineview_vtable_initialized = FALSE;

static void machineview_vtable_init(MachineView* self)
{
	if (!machineview_vtable_initialized) {
		machineview_vtable = *(self->component.vtable);
		machineview_vtable.on_destroyed =
			(psy_ui_fp_component)
			machineview_on_destroyed;
		machineview_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			machineview_on_mouse_down;
		machineview_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			machineview_on_mouse_up;
		machineview_vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			machineview_onmousedoubleclick;
		machineview_vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			machineview_on_key_down;
		machineview_vtable.section =
			(psy_ui_fp_component_section)
			machineview_section;
		machineview_vtable.on_focus =
			(psy_ui_fp_component)
			machineview_on_focus;
		machineview_vtable.show =
			(psy_ui_fp_component)
			machineview_onshow;
		machineview_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(machineview_base(self),
		&machineview_vtable);	
}

/* implementation */
void machineview_init(MachineView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	machineview_initcomponent(self, parent);
	self->shownewmachine = FALSE;
	self->workspace = workspace;	
	machineview_initnotebook(self, tabbarparent);
	machineview_initpropertiesview(self);
	machineview_initwireview(self, tabbarparent);
	machineview_initstackview(self, tabbarparent);	
	machineview_initnewmachine(self, tabbarparent);	
	machineview_init_tabbar(self, tabbarparent);
	machineview_connectsignals(self);	
	psy_ui_tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);	
}

void machineview_on_destroyed(MachineView* self)
{		
	
}

void machineview_initcomponent(MachineView* self, psy_ui_Component* parent)
{
	psy_ui_component_init(machineview_base(self), parent, NULL);
	machineview_vtable_init(self);	
	psy_ui_component_set_style_type(&self->component, STYLE_MV);
	psy_ui_component_set_title(machineview_base(self), "main.machines");
	psy_ui_component_set_id(machineview_base(self), VIEW_ID_MACHINEVIEW);
}

void machineview_initpropertiesview(MachineView* self)
{
	machineproperties_init(&self->properties, machineview_base(self),
		self->workspace);
	psy_ui_component_set_align(machineproperties_base(&self->properties),
		psy_ui_ALIGN_TOP);
	psy_ui_component_hide(machineproperties_base(&self->properties));
}

void machineview_initnotebook(MachineView* self, psy_ui_Component* tabbarparent)
{
	psy_ui_notebook_init(&self->notebook, machineview_base(self));
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
}

void machineview_initwireview(MachineView* self, psy_ui_Component* tabbarparent)
{
	machinewireview_init(&self->wireview,
		psy_ui_notebook_base(&self->notebook), tabbarparent,
		self->workspace->paramviews, self->workspace);
	psy_ui_scroller_init(&self->scroller, psy_ui_notebook_base(
		&self->notebook), NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->wireview.component);
	psy_ui_component_set_align(&self->wireview.component,
		psy_ui_ALIGN_FIXED);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
}

void machineview_initstackview(MachineView* self,
	psy_ui_Component* tabbarparent)
{
	machinestackview_init(&self->stackview,
		psy_ui_notebook_base(&self->notebook), tabbarparent,
		self->workspace->paramviews, self->workspace);
}

void machineview_initnewmachine(MachineView* self,
	psy_ui_Component* tabbarparent)
{
	newmachine_init(&self->newmachine, psy_ui_notebook_base(&self->notebook),
		self->workspace);
	psy_ui_component_set_align(&self->newmachine.component,
		psy_ui_ALIGN_CLIENT);
}

void machineview_init_tabbar(MachineView* self, psy_ui_Component* tabbarparent)
{
	psy_ui_tabbar_init(&self->tabbar, tabbarparent);
	psy_ui_component_set_id(psy_ui_tabbar_base(&self->tabbar),
		VIEW_ID_MACHINEVIEW);
	psy_ui_component_set_align(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_LEFT);	
	psy_ui_tabbar_append(&self->tabbar, "machineview.wires",
		psy_INDEX_INVALID, IDB_WIRES_LIGHT, IDB_WIRES_DARK,
		psy_ui_colour_white());
	psy_ui_tabbar_append(&self->tabbar, "machineview.stack",
		psy_INDEX_INVALID, IDB_MATRIX_LIGHT, IDB_MATRIX_DARK,
		psy_ui_colour_white());
	psy_ui_tabbar_append(&self->tabbar, "machineview.new-machine",
		psy_INDEX_INVALID, IDB_NEWMACHINE_LIGHT, IDB_NEWMACHINE_DARK,
		psy_ui_colour_white());
}

void machineview_connectsignals(MachineView* self)
{
	psy_signal_connect(&self->component.signal_selectsection, self,
		machineview_selectsection);
	psy_signal_connect(&self->newmachine.signal_selected, self,
		machineview_onnewmachineselected);	
	psy_signal_connect(
		&psycleconfig_macview(workspace_conf(self->workspace))->signal_changed,
		self, machineview_onconfigure);
	psy_ui_notebook_connect_controller(&self->notebook,
		&self->tabbar.signal_change);
	psy_signal_connect(&self->tabbar.signal_change, self,
		machineview_on_tabbar_changed);
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		machineview_onsongchanged);
}

void machineview_onmousedoubleclick(MachineView* self, psy_ui_MouseEvent* ev)
{			
	if (psy_ui_component_section(&self->component) !=
			SECTION_ID_MACHINEVIEW_NEWMACHINE) {		
		self->newmachine.restoresection = psy_ui_component_section(
			&self->component);		
		self->shownewmachine = TRUE;
		return;
	} else {
		self->shownewmachine = FALSE;
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void machineview_on_mouse_down(MachineView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_mouseevent_stop_propagation(ev);
}

void machineview_on_mouse_up(MachineView* self, psy_ui_MouseEvent* ev)
{
	if (self->shownewmachine) {
		if (psy_ui_component_section(&self->component) ==
				SECTION_ID_MACHINEVIEW_STACK) {
			self->newmachine.restoresection = SECTION_ID_MACHINEVIEW_STACK;
			if (self->stackview.state.insertmachinemode ==
					NEWMACHINE_ADDEFFECTSTACK) {
				workspace_select_view(self->workspace,
					viewindex_make(
					VIEW_ID_MACHINEVIEW,
					SECTION_ID_MACHINEVIEW_NEWMACHINE,
					NEWMACHINE_ADDEFFECTSTACK,
					psy_INDEX_INVALID));
			} else {
				workspace_select_view(self->workspace,
					viewindex_make(VIEW_ID_MACHINEVIEW,
					SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_APPENDSTACK,
					psy_INDEX_INVALID));
			}
		} else {
			psy_ui_component_select_section(machineview_base(self),
				SECTION_ID_MACHINEVIEW_NEWMACHINE,
				NEWMACHINE_APPEND);
		}
		self->shownewmachine = FALSE;
		psy_ui_mouseevent_stop_propagation(ev);
	}	else if (psy_ui_mouseevent_button(ev) == 2) {
		if (psy_ui_tabbar_selected(&self->tabbar) ==
				SECTION_ID_MACHINEVIEW_NEWMACHINE) {
			psy_ui_tabbar_select(&self->tabbar,
				self->newmachine.restoresection);
		} else {		
			psy_ui_component_toggle_visibility(
				machineproperties_base(&self->properties));
		}
	}
}

void machineview_on_key_down(MachineView* self, psy_ui_KeyboardEvent* ev)
{
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_ESCAPE) {
		if (psy_ui_component_section(&self->component) ==
				SECTION_ID_MACHINEVIEW_NEWMACHINE) {
			psy_ui_tabbar_select(&self->tabbar,
				self->newmachine.restoresection);
			if (psy_ui_notebook_active_page(&self->notebook)) {
				psy_ui_component_set_focus(
					psy_ui_notebook_active_page(&self->notebook));
			}
		} else if (self->workspace->gearvisible) {
			workspace_toggle_gear(self->workspace);
		}
		psy_ui_keyboardevent_stop_propagation(ev);
	} 
}

void machineview_on_focus(MachineView* self)
{
	if (psy_ui_notebook_active_page(&self->notebook)) {
		psy_ui_component_set_focus(
			psy_ui_notebook_active_page(&self->notebook));
	}
}

void machineview_on_tabbar_changed(MachineView* self, psy_ui_TabBar* sender, uintptr_t index)
{
	ViewIndex entry;

	entry = workspace_current_view(self->workspace);
	if (entry.id != VIEW_ID_MACHINEVIEW || entry.section != index) {
		workspace_on_view_changed(self->workspace, viewindex_make(
			VIEW_ID_MACHINEVIEW, index, psy_INDEX_INVALID,
			psy_INDEX_INVALID));
	}
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
			newmachine_appendmode(&self->newmachine);
		} else if (options & NEWMACHINE_ADDEFFECT) {
			self->wireview.addeffect = 1;
			self->wireview.randominsert = 0;
			newmachine_addeffectmode(&self->newmachine);
		} else if (options & NEWMACHINE_ADDEFFECTSTACK) {
			self->wireview.addeffect = 1;
			self->wireview.randominsert = 0;				
			newmachine_addeffectmode(&self->newmachine);				
		}
		break;
	default:
		break;
	}
	psy_ui_tabbar_select(&self->tabbar, section);
}

void machineview_onsongchanged(MachineView* self, Workspace* sender)
{	
	psy_ui_tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);	
}

void machineview_onconfigure(MachineView* self, MachineViewConfig* sender,
	psy_Property* property)
{
	if (machineviewconfig_vumeters(sender)) {
		machineui_enable_vumeter();
	} else {
		machineui_prevent_vumeter();
	}
	if (machineviewconfig_machineindexes(sender)) {
		machineui_enable_macindex();
	} else {
		machineui_prevent_macindex();
	}
	self->wireview.showwirehover = machineviewconfig_wirehover(sender);	
	if (machineviewconfig_virtualgenerators(sender)) {
		machinewireview_showvirtualgenerators(&self->wireview);
		machinestackview_showvirtualgenerators(&self->stackview);
	} else {
		machinewireview_hidevirtualgenerators(&self->wireview);
		machinestackview_hidevirtualgenerators(&self->stackview);
	}
	if (machineviewconfig_stackview_drawsmalleffects(sender)) {
		machinestackview_drawsmalleffects(&self->stackview);
	} else {
		machinestackview_drawfulleffects(&self->stackview);
	}
}

void machineview_onnewmachineselected(MachineView* self,
	psy_ui_Component* sender, psy_Property* plugininfo)
{
	psy_audio_Machine* machine;
	psy_audio_MachineInfo machineinfo;	
	
	machineinfo_init(&machineinfo);
	newmachine_selectedmachineinfo(&self->newmachine, &machineinfo);
	if (self->newmachine.restoresection == SECTION_ID_MACHINEVIEW_STACK &&
			self->stackview.state.insertmachinemode ==
			NEWMACHINE_ADDEFFECTSTACK) {
		self->newmachine.restoresection = psy_INDEX_INVALID;
		machinestackview_addeffect(&self->stackview, &machineinfo);
		machineinfo_dispose(&machineinfo);
		psy_ui_tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_STACK);
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
				psy_audio_machines_selectedwire(self->wireview.machines));				
			psy_audio_machines_connect(self->wireview.machines,
				psy_audio_wire_make(psy_audio_machines_selectedwire(
					self->wireview.machines).src, slot));
			psy_audio_machines_connect(self->wireview.machines,
				psy_audio_wire_make(slot, psy_audio_machines_selectedwire(
					self->wireview.machines).dst));
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
			psy_ui_tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_STACK);			
		} else {
			psy_ui_tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);
		}
		
	} else {
		workspace_output_error(self->workspace,
			self->workspace->machinefactory.errstr);
	}	
	self->newmachine.appendstack = FALSE;
}

uintptr_t machineview_section(const MachineView* self)
{
	switch (psy_ui_tabbar_selected(&self->tabbar)) {
	case 0:
		return SECTION_ID_MACHINEVIEW_WIRES;		
	case 1:
		return SECTION_ID_MACHINEVIEW_STACK;
	case 2:
		return SECTION_ID_MACHINEVIEW_NEWMACHINE;		
	default:
		return SECTION_ID_MACHINEVIEW_WIRES;
	}	
}

void machineview_idle(MachineView* self)
{	
	machinewireview_idle(&self->wireview);	
	machinestackview_idle(&self->stackview);
	machineproperties_idle(&self->properties);
}

void machineview_onshow(MachineView* self)
{
	if (self->wireview.centermaster) {
		machinewireview_centermaster(&self->wireview);
		self->wireview.centermaster = FALSE;
	}
}
