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


/* prototypes */
static void machineview_init_component(MachineView*, psy_ui_Component* parent);
static void machineview_init_notebook(MachineView*,
	psy_ui_Component* tabbarparent);
static void machineview_init_wire_view(MachineView*,
	psy_ui_Component* tabbarparent);
static void machineview_init_stack_view(MachineView*,
	psy_ui_Component* tabbarparent);
static void machineview_init_new_machine(MachineView*,
	psy_ui_Component* tabbarparent);
static void machineview_init_tabbar(MachineView*,
	psy_ui_Component* tabbarparent);
static void machineview_init_menu(MachineView*);
static void machineview_on_tabbar_changed(MachineView*, psy_ui_TabBar* sender,
	uintptr_t index);
static void machineview_connect_signals(MachineView*);
static uintptr_t machineview_section(const MachineView*);
static void machineview_on_song_changed(MachineView*,
	psy_audio_Player* sender);
static void machineview_set_song(MachineView*, psy_audio_Song*);	
static void machineview_on_mouse_down(MachineView*, psy_ui_MouseEvent*);
static void machineview_on_mouse_up(MachineView*, psy_ui_MouseEvent*);
static void machineview_on_mouse_double_click(MachineView*, psy_ui_MouseEvent*);
static void machineview_on_key_down(MachineView*, psy_ui_KeyboardEvent*);
static void machineview_on_focus(MachineView*);
static void machineview_select_section(MachineView*, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options);
static void machineview_on_vu_meters(MachineView*, psy_Property* sender);
static void machineview_on_machine_index(MachineView*, psy_Property* sender);
static void machineview_on_select_section(MachineView*, psy_ui_Component* sender,
	uintptr_t param1, uintptr_t param2);


/* vtable */
static psy_ui_ComponentVtable machineview_vtable;
static bool machineview_vtable_initialized = FALSE;

static void machineview_vtable_init(MachineView* self)
{
	assert(self);
	
	if (!machineview_vtable_initialized) {
		machineview_vtable = *(self->component.vtable);		
		machineview_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			machineview_on_mouse_down;
		machineview_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			machineview_on_mouse_up;
		machineview_vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			machineview_on_mouse_double_click;
		machineview_vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			machineview_on_key_down;
		machineview_vtable.section =
			(psy_ui_fp_component_section)
			machineview_section;
		machineview_vtable.on_focus =
			(psy_ui_fp_component)
			machineview_on_focus;		
		machineview_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(machineview_base(self),
		&machineview_vtable);	
}

/* implementation */
void machineview_init(MachineView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	assert(self);
	
	machineview_init_component(self, parent);	
	self->shownewmachine = FALSE;
	self->workspace = workspace;
	machineviewbar_init(&self->machineviewbar, &self->component,
		&self->workspace->player);
	machineview_init_notebook(self, tabbarparent);
	machineview_init_wire_view(self, tabbarparent);
	machineview_init_stack_view(self, tabbarparent);	
	machineview_init_new_machine(self, tabbarparent);	
	machineview_init_tabbar(self, tabbarparent);
	machineview_init_menu(self);
	machineview_set_song(self, workspace_song(workspace));
	machineview_connect_signals(self);		
	psy_ui_tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);
	psy_signal_connect(&self->component.signal_selectsection, self,
		machineview_on_select_section);
}

void machineview_init_component(MachineView* self, psy_ui_Component* parent)
{
	assert(self);
	
	psy_ui_component_init(machineview_base(self), parent, NULL);
	machineview_vtable_init(self);	
	psy_ui_component_set_title(machineview_base(self), "main.machines");
	psy_ui_component_set_id(machineview_base(self), VIEW_ID_MACHINEVIEW);
}

void machineview_init_notebook(MachineView* self,
	psy_ui_Component* tabbarparent)
{
	assert(self);
	
	psy_ui_notebook_init(&self->notebook, machineview_base(self));
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
}

void machineview_init_wire_view(MachineView* self,
	psy_ui_Component* tabbarparent)
{
	assert(self);
	
	machinewireview_init(&self->wireview,
		psy_ui_notebook_base(&self->notebook), tabbarparent,
		self->workspace->paramviews, &self->machine_menu,
		self->workspace);	
	psy_ui_component_set_align(machinewireview_base(&self->wireview),
		psy_ui_ALIGN_CLIENT);	
}

void machineview_init_stack_view(MachineView* self,
	psy_ui_Component* tabbarparent)
{
	assert(self);
	
	machinestackview_init(&self->stackview,
		psy_ui_notebook_base(&self->notebook), tabbarparent,
		self->workspace->paramviews, &self->machine_menu, self->workspace);
}

void machineview_init_new_machine(MachineView* self,
	psy_ui_Component* tabbarparent)
{
	assert(self);
	
	newmachine_init(&self->newmachine, psy_ui_notebook_base(&self->notebook),
		self->workspace);
	psy_ui_component_set_align(&self->newmachine.component,
		psy_ui_ALIGN_CLIENT);
}

void machineview_init_tabbar(MachineView* self, psy_ui_Component* tabbarparent)
{
	assert(self);
	
	psy_ui_tabbar_init(&self->tabbar, tabbarparent);
	psy_ui_component_set_id(psy_ui_tabbar_base(&self->tabbar),
		VIEW_ID_MACHINEVIEW);
	psy_ui_component_set_align(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_LEFT);	
	psy_ui_tabbar_append(&self->tabbar, "machineview.wires",
		SECTION_ID_MACHINEVIEW_WIRES, IDB_WIRES_LIGHT, IDB_WIRES_DARK,
		psy_ui_colour_white());
	psy_ui_tabbar_append(&self->tabbar, "machineview.stack",
		SECTION_ID_MACHINEVIEW_STACK, IDB_MATRIX_LIGHT, IDB_MATRIX_DARK,
		psy_ui_colour_white());
	psy_ui_tabbar_append(&self->tabbar, "machineview.new-machine",
		SECTION_ID_MACHINEVIEW_NEWMACHINE, IDB_NEWMACHINE_LIGHT,
		IDB_NEWMACHINE_DARK, psy_ui_colour_white());
}

void machineview_init_menu(MachineView* self)
{
	assert(self);
	
	machinemenu_init(&self->machine_menu, &self->component,
		&self->wireview.pane.wireframes);
	psy_ui_component_set_align(&self->machine_menu.component,
		psy_ui_ALIGN_RIGHT);	
	psy_ui_component_hide(&self->machine_menu.component);
}

void machineview_connect_signals(MachineView* self)
{
	assert(self);
	
	psy_signal_connect(&self->component.signal_selectsection, self,
		machineview_select_section);	
	psy_ui_notebook_connect_controller(&self->notebook,
		&self->tabbar.signal_change);
	psy_signal_connect(&self->tabbar.signal_change, self,
		machineview_on_tabbar_changed);
	psy_signal_connect(&self->workspace->player.signal_song_changed, self,
		machineview_on_song_changed);
	machineviewconfig_connect(&self->workspace->config.visual.macview,
		"drawvumeters", self, machineview_on_vu_meters);
	machineviewconfig_connect(&self->workspace->config.visual.macview,
		"drawmachineindexes", self, machineview_on_machine_index);
}

void machineview_on_mouse_double_click(MachineView* self, psy_ui_MouseEvent* ev)
{		
	assert(self);
		
	if (psy_ui_mouseevent_button(ev) == 1) {
		if (psy_ui_component_section(&self->component) !=
				SECTION_ID_MACHINEVIEW_NEWMACHINE) {		
			self->newmachine.restoresection = psy_ui_component_section(
				&self->component);		
			self->shownewmachine = TRUE;
			return;
		} else {
			self->shownewmachine = FALSE;
		}
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void machineview_on_mouse_down(MachineView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_mouseevent_stop_propagation(ev);
}

void machineview_on_mouse_up(MachineView* self, psy_ui_MouseEvent* ev)
{
	assert(self);
	
	if (self->shownewmachine) {		
		psy_ui_component_select_section(machineview_base(self),
			SECTION_ID_MACHINEVIEW_NEWMACHINE,
			psy_INDEX_INVALID);		
		self->shownewmachine = FALSE;
		psy_ui_mouseevent_stop_propagation(ev);
	} else if (psy_ui_mouseevent_button(ev) == 2) {
		if (psy_ui_tabbar_selected(&self->tabbar) ==
				SECTION_ID_MACHINEVIEW_NEWMACHINE) {
			psy_ui_tabbar_select(&self->tabbar,
				self->newmachine.restoresection);
		}
	}
}

void machineview_on_key_down(MachineView* self, psy_ui_KeyboardEvent* ev)
{
	assert(self);
	
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_ESCAPE) {
		if (psy_ui_component_section(&self->component) ==
				SECTION_ID_MACHINEVIEW_NEWMACHINE) {
			psy_ui_tabbar_select(&self->tabbar,
				self->newmachine.restoresection);
			if (psy_ui_notebook_active_page(&self->notebook)) {
				psy_ui_component_set_focus(
					psy_ui_notebook_active_page(&self->notebook));
			}
		}
		psy_ui_keyboardevent_stop_propagation(ev);
	} 
}

void machineview_on_focus(MachineView* self)
{
	assert(self);
	
	if (psy_ui_notebook_active_page(&self->notebook)) {
		psy_ui_component_set_focus(
			psy_ui_notebook_active_page(&self->notebook));
	}
}

void machineview_on_tabbar_changed(MachineView* self, psy_ui_TabBar* sender,
	uintptr_t index)
{
	ViewIndex curr;

	assert(self);
	
	curr = workspace_current_view(self->workspace);
	if (curr.id != VIEW_ID_MACHINEVIEW || curr.section != index) {
		workspace_add_view(self->workspace, viewindex_make_all(
			VIEW_ID_MACHINEVIEW, index, psy_INDEX_INVALID, psy_INDEX_INVALID));
	}
}

void machineview_select_section(MachineView* self, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options)
{	
	assert(self);	
		
	psy_ui_tabbar_select_id(&self->tabbar, section);
}

uintptr_t machineview_section(const MachineView* self)
{
	const psy_ui_Component* curr;
	
	assert(self);
	
	curr = psy_ui_notebook_active_page_const(&self->notebook);
	if (curr) {		
		return psy_ui_component_id(curr);
	}	
	return SECTION_ID_MACHINEVIEW_WIRES; 
}

void machineview_on_song_changed(MachineView* self, psy_audio_Player* sender)
{	
	assert(self);
			
	machineview_set_song(self, psy_audio_player_song(sender));
	psy_ui_tabbar_select_id(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);	
}

void machineview_set_song(MachineView* self, psy_audio_Song* song)
{
	assert(self);
	
	if (song) {		
		machinemenu_set_machines(&self->machine_menu,
			psy_audio_song_machines(song));
	} else {
		machinemenu_set_machines(&self->machine_menu, NULL);
	}
}

void machineview_idle(MachineView* self)
{		
	machinewireview_idle(&self->wireview);	
	machinestackview_idle(&self->stackview);	
}

void machineview_on_vu_meters(MachineView* self, psy_Property* sender)
{
	assert(self);
	
	if (psy_property_item_bool(sender)) {
		machineui_enable_vumeter();
	} else {
		machineui_prevent_vumeter();
	}
}

void machineview_on_machine_index(MachineView* self, psy_Property* sender)
{
	assert(self);
	
	if (psy_property_item_bool(sender)) {
		machineui_enable_macindex();
	} else {
		machineui_prevent_macindex();
	}
}

void machineview_on_select_section(MachineView* self, psy_ui_Component* sender,
	uintptr_t param1, uintptr_t param2)
{
	assert(self);	
	
	if (param1 == SECTION_ID_MACHINEVIEW_BANK_MANGER) {
		machinemenu_select(&self->machine_menu, param2);
		machinemenu_show_bank_manager(&self->machine_menu);
		if (!psy_ui_component_visible(&self->machine_menu.component)) {			
			psy_ui_component_show_align(&self->machine_menu.component);
			psy_ui_component_invalidate(&self->machine_menu.component);
		}
	}
}
