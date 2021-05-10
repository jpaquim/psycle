// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineview.h"
// host
#include "resources/resource.h"
#include "skingraphics.h"
#include "styles.h"
#include "wireview.h"
// ui
#include <uicolours.h>
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
static void machineview_initcomponent(MachineView*, psy_ui_Component* parent);
static void machineview_initpropertiesview(MachineView*);
static void machineview_initnotebook(MachineView*, psy_ui_Component* tabbarparent);
static void machineview_initwireview(MachineView*, psy_ui_Component* tabbarparent);
static void machineview_initstackview(MachineView*, psy_ui_Component* tabbarparent);
static void machineview_initnewmachine(MachineView*, psy_ui_Component* tabbarparent);
static void machineview_inittabbar(MachineView*, psy_ui_Component* tabbarparent);
static void machineview_connectsignals(MachineView*);
static uintptr_t machineview_section(const MachineView*);
static void machineview_onsongchanged(MachineView*, Workspace*, int flag,
	psy_audio_Song*);
static void machineview_onmousedown(MachineView*, psy_ui_MouseEvent*);
static void machineview_onmouseup(MachineView*, psy_ui_MouseEvent*);
static void machineview_onmousedoubleclick(MachineView*, psy_ui_MouseEvent*);
static void machineview_onkeydown(MachineView*, psy_ui_KeyboardEvent*);
static void machineview_onfocus(MachineView*);
static void machineview_selectsection(MachineView*, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options);
static void machineview_onconfigure(MachineView*, MachineViewConfig*,
	psy_Property*);
static void machineview_onthemechanged(MachineView*, MachineViewConfig*,
	psy_Property* theme);
static void machineview_onnewmachineselected(MachineView*,
	psy_ui_Component* sender, psy_Property*);
static void machineview_ontimer(MachineView*, uintptr_t timerid);
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
		machineview_vtable.section = (psy_ui_fp_component_section)
			machineview_section;
		machineview_vtable.ontimer = (psy_ui_fp_component_ontimer)
			machineview_ontimer;
		machineview_vtable.onfocus = (psy_ui_fp_component_onfocus)
			machineview_onfocus;
		machineview_vtable_initialized = TRUE;
	}
	return &machineview_vtable;
}
// implementation
void machineview_init(MachineView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	machineview_initcomponent(self, parent);
	self->shownewmachine = FALSE;
	self->workspace = workspace;
	paramviews_init(&self->paramviews, &self->component, workspace);	
	machineviewskin_init(&self->skin,	
		psycleconfig_macview(workspace_conf(self->workspace))->theme,
		dirconfig_skins(&self->workspace->config.directories));	
	machineview_initnotebook(self, tabbarparent);
	machineview_initpropertiesview(self);
	machineview_initwireview(self, tabbarparent);
	machineview_initstackview(self, tabbarparent);	
	machineview_initnewmachine(self, tabbarparent);	
	machineview_inittabbar(self, tabbarparent);	
	machineview_connectsignals(self);	
	psy_ui_tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void machineview_ondestroy(MachineView* self)
{	
	machineviewskin_dispose(&self->skin);
	paramviews_dispose(&self->paramviews);
}

void machineview_initcomponent(MachineView* self, psy_ui_Component* parent)
{
	psy_ui_component_init(machineview_base(self), parent, NULL);
	psy_ui_component_setvtable(machineview_base(self),
		machineview_vtable_init(self));
	psy_ui_component_setbackgroundmode(machineview_base(self),
		psy_ui_NOBACKGROUND);
	psy_ui_component_setstyletypes(&self->component,
		STYLE_MACHINEVIEW, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);	
}

void machineview_initpropertiesview(MachineView* self)
{
	machineproperties_init(&self->properties, &self->component, NULL,
		&self->skin, self->workspace);
	psy_ui_component_setalign(&self->properties.component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->properties.component);
}

void machineview_initnotebook(MachineView* self, psy_ui_Component* tabbarparent)
{
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
}

void machineview_initwireview(MachineView* self, psy_ui_Component* tabbarparent)
{
	machinewireview_init(&self->wireview,
		psy_ui_notebook_base(&self->notebook), tabbarparent, &self->skin,
		&self->paramviews, self->workspace);
	psy_ui_scroller_init(&self->scroller, &self->wireview.component,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_ui_component_setalign(&self->wireview.component, psy_ui_ALIGN_FIXED_RESIZE);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);

}

void machineview_initstackview(MachineView* self, psy_ui_Component* tabbarparent)
{
	machinestackview_init(&self->stackview,
		psy_ui_notebook_base(&self->notebook), tabbarparent,
		&self->skin, &self->paramviews, self->workspace);
}

void machineview_initnewmachine(MachineView* self, psy_ui_Component* tabbarparent)
{
	newmachine_init(&self->newmachine, psy_ui_notebook_base(&self->notebook),
		self->workspace);
	psy_ui_component_setalign(&self->newmachine.component,
		psy_ui_ALIGN_CLIENT);
}

void machineview_inittabbar(MachineView* self, psy_ui_Component* tabbarparent)
{
	psy_ui_Tab* tab;
	psy_ui_Colour transparencycolour;

	psy_ui_tabbar_init(&self->tabbar, tabbarparent);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);	
	transparencycolour = psy_ui_colour_make_argb(psy_ui_RGB_WHITE);
	tab = psy_ui_tabbar_append(&self->tabbar, "machineview.wires");
	psy_ui_bitmap_loadresource(&tab->icon, IDB_WIRES_DARK);
	psy_ui_bitmap_settransparency(&tab->icon, transparencycolour);
	tab = psy_ui_tabbar_append(&self->tabbar, "machineview.stack");
	psy_ui_bitmap_loadresource(&tab->icon, IDB_MATRIX_DARK);
	psy_ui_bitmap_settransparency(&tab->icon, transparencycolour);
	tab = psy_ui_tabbar_append(&self->tabbar, "machineview.new-machine");
	psy_ui_bitmap_loadresource(&tab->icon, IDB_NEWMACHINE_DARK);
	psy_ui_bitmap_settransparency(&tab->icon, transparencycolour);
}

void machineview_connectsignals(MachineView* self)
{
	psy_signal_connect(&self->component.signal_selectsection, self,
		machineview_selectsection);
	psy_signal_connect(&self->newmachine.signal_selected, self,
		machineview_onnewmachineselected);
	psy_signal_connect(
		&psycleconfig_macview(workspace_conf(self->workspace))->signal_themechanged,
		self, machineview_onthemechanged);
	psy_signal_connect(
		&psycleconfig_macview(workspace_conf(self->workspace))->signal_changed,
		self, machineview_onconfigure);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		machineview_onsongchanged);
}

void machineview_onmousedoubleclick(MachineView* self, psy_ui_MouseEvent* ev)
{			
	if (psy_ui_component_section(&self->component) != SECTION_ID_MACHINEVIEW_NEWMACHINE) {		
		self->newmachine.restoresection = psy_ui_component_section(&self->component);		
		self->shownewmachine = TRUE;		
	} else {
		self->shownewmachine = FALSE;
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void machineview_onmousedown(MachineView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_mouseevent_stop_propagation(ev);
}

void machineview_onmouseup(MachineView* self, psy_ui_MouseEvent* ev)
{
	if (self->shownewmachine) {
		if (psy_ui_component_section(&self->component) == SECTION_ID_MACHINEVIEW_STACK) {
			self->newmachine.restoresection = SECTION_ID_MACHINEVIEW_STACK;
			if (self->stackview.state.insertmachinemode == NEWMACHINE_ADDEFFECTSTACK) {
				workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
					SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_ADDEFFECTSTACK);
			} else {
				workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
					SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_APPENDSTACK);
			}
		} else {
			psy_ui_component_selectsection(machineview_base(self),
				SECTION_ID_MACHINEVIEW_NEWMACHINE,
				NEWMACHINE_APPEND);
		}
		self->shownewmachine = FALSE;
		psy_ui_mouseevent_stop_propagation(ev);
	}	else if (ev->button == 2) {
		if (psy_ui_tabbar_selected(&self->tabbar) ==
				SECTION_ID_MACHINEVIEW_NEWMACHINE) {
			psy_ui_tabbar_select(&self->tabbar,
				self->newmachine.restoresection);
		} else {		
			psy_ui_component_togglevisibility(
				machineproperties_base(&self->properties));
		}
	}
}

void machineview_onkeydown(MachineView* self, psy_ui_KeyboardEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_ESCAPE) {
		if (psy_ui_component_section(&self->component) ==
				SECTION_ID_MACHINEVIEW_NEWMACHINE) {
			psy_ui_tabbar_select(&self->tabbar,
				self->newmachine.restoresection);
			if (psy_ui_notebook_activepage(&self->notebook)) {
				psy_ui_component_setfocus(
					psy_ui_notebook_activepage(&self->notebook));
			}
		} else if (self->workspace->gearvisible) {
			workspace_togglegear(self->workspace);
		}
		psy_ui_keyboardevent_stop_propagation(ev);
	} 
}

void machineview_onfocus(MachineView* self)
{
	if (psy_ui_notebook_activepage(&self->notebook)) {
		psy_ui_component_setfocus(
			psy_ui_notebook_activepage(&self->notebook));
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

void machineview_onsongchanged(MachineView* self, Workspace* workspace,
	int flag, psy_audio_Song* song)
{	
	psy_ui_tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);	
}

void machineview_onconfigure(MachineView* self, MachineViewConfig* sender,
	psy_Property* property)
{
	self->skin.drawvumeters = machineviewconfig_vumeters(sender);
	self->skin.drawmachineindexes = machineviewconfig_machineindexes(sender);
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

void machineview_onthemechanged(MachineView* self, MachineViewConfig* sender,
	psy_Property* theme)
{
	machineviewskin_settheme(&self->skin, theme,
		dirconfig_skins(&self->workspace->config.directories));
	machinewireview_updateskin(&self->wireview);
	machinestackpane_updateskin(&self->stackview.pane);	
	psy_ui_component_invalidate(&self->component);
}

void machineview_onnewmachineselected(MachineView* self,
	psy_ui_Component* sender, psy_Property* plugininfo)
{
	psy_audio_Machine* machine;
	psy_audio_MachineInfo machineinfo;	
	
	machineinfo_init(&machineinfo);
	newmachine_selectedmachineinfo(&self->newmachine, &machineinfo);
	if (self->newmachine.restoresection == SECTION_ID_MACHINEVIEW_STACK &&
			self->stackview.state.insertmachinemode == NEWMACHINE_ADDEFFECTSTACK) {
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
				psy_audio_wire_make(psy_audio_machines_selectedwire(self->wireview.machines).src, slot));
			psy_audio_machines_connect(self->wireview.machines,
				psy_audio_wire_make(slot, psy_audio_machines_selectedwire(self->wireview.machines).dst));
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
		workspace_outputerror(self->workspace,
			self->workspace->machinefactory.errstr);
	}	
	self->newmachine.appendstack = FALSE;
}

uintptr_t machineview_section(const MachineView* self)
{
	switch (psy_ui_tabbar_selected(&self->tabbar)) {
	case 0:
		return SECTION_ID_MACHINEVIEW_WIRES;
		break;
	case 1:
		return SECTION_ID_MACHINEVIEW_STACK;
		break;
	case 2:
		return SECTION_ID_MACHINEVIEW_NEWMACHINE;
		break;
	default:
		break;
	}
	return SECTION_ID_MACHINEVIEW_WIRES;
}

void machineview_ontimer(MachineView* self, uintptr_t timerid)
{
	machinewireview_idle(&self->wireview);
	machinestackview_idle(&self->stackview);
	machineproperties_idle(&self->properties);
}
