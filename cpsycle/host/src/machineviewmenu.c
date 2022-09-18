/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

/* host */
#include "styles.h"
#include "machineviewmenu.h"
#include "paramviews.h"
#include "wireframes.h"

/* platforn */
#include "../../detail/portable.h"


/* MachineMenuState */

/* implementation */
void machinemenustate_init(MachineMenuState* self, WireFrames* wireframes,
	psy_ui_Component* menu)
{
	assert(self);
	
	self->machines = NULL;
	self->wireframes = wireframes;
	self->mac_id = psy_INDEX_INVALID;
	self->menu = menu;
}

void machinemenustate_hide_menu(MachineMenuState* self)
{
	assert(self);
	
	if (self->menu) {
		psy_ui_component_hide_align(self->menu);
	}
}

/* MachineConnectionsMenu */

/* prototypes */
static void machineconnectionsmenu_on_destroyed(MachineConnectionsMenu*);
static void machineconnectionsmenu_append(MachineConnectionsMenu*, char* str,
	uintptr_t idx, psy_audio_Wire);
static void machineconnectionsmenu_fill(MachineConnectionsMenu*);
static void machineconnectionsmenu_on_connection(MachineConnectionsMenu*,
	psy_ui_Button* sender);
	
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MachineConnectionsMenu* self)
{
	assert(self);
	
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			machineconnectionsmenu_on_destroyed;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);	
}	

/* implementation */
void machineconnectionsmenu_init(MachineConnectionsMenu* self,
	psy_ui_Component* parent, MachineMenuState* state)
{
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->state = state;
	psy_table_init(&self->wires);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,		
		psy_ui_margin_zero());
	machineconnectionsmenu_fill(self);
}

void machineconnectionsmenu_on_destroyed(MachineConnectionsMenu* self)
{
	assert(self);
	
	psy_table_dispose_all(&self->wires, NULL);
}

void machineconnectionsmenu_fill(MachineConnectionsMenu* self)
{
	psy_audio_MachineSockets* sockets;	
	uintptr_t idx;
	
	assert(self);
	
	psy_ui_component_clear(&self->component);
	psy_table_dispose_all(&self->wires, NULL);
	psy_table_init(&self->wires);		
	if (!self->state->machines) {
		return;
	}
	if (self->state->mac_id == psy_INDEX_INVALID) {
		return;
	}
	sockets = psy_audio_connections_at(&self->state->machines->connections,
		self->state->mac_id);
	idx = 0;
	if (sockets) {
		psy_ui_Component* separator;
		psy_ui_Label* label;		
		psy_TableIterator it;
		/* inputs */
		label = psy_ui_label_allocinit(&self->component);
		psy_ui_label_set_text(label, "Inputs");
		for (it = psy_audio_wiresockets_begin(&sockets->inputs);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (socket->slot != psy_INDEX_INVALID) {				
				psy_audio_Machine* machine;
					
				machine = psy_audio_machines_at(self->state->machines,
					socket->slot);
				if (machine) {
					char str[64];
						
					psy_snprintf(str, 64, "%02X: %s", socket->slot, 
						psy_audio_machine_editname(machine));
					machineconnectionsmenu_append(self, str, idx,
						psy_audio_wire_make(socket->slot, self->state->mac_id));
					++idx;
				}
			}
		}		
		/* separator */
		separator = psy_ui_component_allocinit(&self->component, NULL);
		psy_ui_component_set_style_type(separator, STYLE_SEPARATOR);
			psy_ui_component_set_margin(separator, psy_ui_margin_make_em(
			0.4, 0.0, 0.4, 0.0));
		/* outputs */	
		label = psy_ui_label_allocinit(&self->component);
		psy_ui_label_set_text(label, "Outputs");
		for (it = psy_audio_wiresockets_begin(&sockets->outputs);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (socket->slot != psy_INDEX_INVALID) {				
				psy_audio_Machine* machine;
					
				machine = psy_audio_machines_at(self->state->machines,
					socket->slot);
				if (machine) {
					char str[64];
						
					psy_snprintf(str, 64, "%02X: %s", socket->slot, 
						psy_audio_machine_editname(machine));
					machineconnectionsmenu_append(self, str, idx,
						psy_audio_wire_make(self->state->mac_id, socket->slot));
					++idx;				
				}
			}
		}
	} else {		
		psy_ui_Label* label;		
		
		label = psy_ui_label_allocinit(&self->component);
		psy_ui_label_set_text(label, "No connections");
	}
}

void machineconnectionsmenu_append(MachineConnectionsMenu* self, char* str,
	uintptr_t idx, psy_audio_Wire wire)
{
	psy_ui_Button* button;
	psy_audio_Wire* newwire;
	
	assert(self);
	
	button = psy_ui_button_allocinit(&self->component);
	newwire = (psy_audio_Wire*)malloc(sizeof(psy_audio_Wire));	
	*newwire = wire;
	psy_table_insert(&self->wires, idx, newwire);
	psy_ui_component_set_id(psy_ui_button_base(button), idx);
	psy_ui_button_prevent_translation(button);
	psy_ui_button_set_text(button, str);
	psy_signal_connect(&button->signal_clicked, self,
		machineconnectionsmenu_on_connection);
}

void machineconnectionsmenu_on_connection(MachineConnectionsMenu* self,
	psy_ui_Button* sender)
{
	psy_audio_Wire* wire;
	
	assert(self);
	
	if (!self->state->machines) {
		return;
	}	
	wire = (psy_audio_Wire*)psy_table_at(&self->wires, psy_ui_component_id(
		psy_ui_button_base(sender)));
	if (wire) {
		wireframes_show(self->state->wireframes, *wire);
	}
	psy_ui_component_hide(&self->component);
	machinemenustate_hide_menu(self->state);		
}

/* MachineConnectToMenu */

/* prototypes */
static void machineconnecttomenu_fill(MachineConnectToMenu*);
static void machineconnecttomenu_append(MachineConnectToMenu*, char* str,
	uintptr_t dst_mac_id);
static void machineconnecttomenu_on_connection(MachineConnectToMenu*,
	psy_ui_Button* sender);

/* implementation */
void machineconnecttomenu_init(MachineConnectToMenu* self,
	psy_ui_Component* parent, MachineMenuState* state)
{
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	self->state = state;
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,		
		psy_ui_margin_zero());
	machineconnecttomenu_fill(self);
}

void machineconnecttomenu_fill(MachineConnectToMenu* self)
{	
	psy_TableIterator it;
	bool connections;	
	
	assert(self);
	
	psy_ui_component_clear(&self->component);
	if (!self->state->machines) {
		return;
	}
	if (self->state->mac_id == psy_INDEX_INVALID ||
			self->state->mac_id == psy_audio_MASTER_INDEX) {
		return;
	}
	connections = FALSE;
	for (it = psy_audio_machines_begin(self->state->machines);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_Wire wire;
		
		wire = psy_audio_wire_make(self->state->mac_id,
			psy_tableiterator_key(&it));
		if (psy_audio_machines_valid_connection(self->state->machines, wire) &&
				(!psy_audio_machines_connected(self->state->machines, wire))) {
			psy_audio_Machine* machine;
			char str[64];

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
			assert(machine);		
			psy_snprintf(str, 64, "%02X: %s", psy_tableiterator_key(&it), 
				psy_audio_machine_editname(machine));
			machineconnecttomenu_append(self, str, wire.dst);
			connections = TRUE;
		}
	}
	if (!connections) {
		psy_ui_Label* label;		
		
		label = psy_ui_label_allocinit(&self->component);
		psy_ui_label_set_text(label, "No destinations");
	}
}

void machineconnecttomenu_append(MachineConnectToMenu* self, char* str,
	uintptr_t dst)
{
	psy_ui_Button* button;
	
	assert(self);
	
	button = psy_ui_button_allocinit(&self->component);
	psy_ui_button_prevent_translation(button);
	psy_ui_button_set_text(button, str);
	psy_ui_component_set_id(psy_ui_button_base(button), dst);
	psy_signal_connect(&button->signal_clicked, self,
		machineconnecttomenu_on_connection);
}

void machineconnecttomenu_on_connection(MachineConnectToMenu* self,
	psy_ui_Button* sender)
{
	psy_audio_Wire wire;
	
	assert(self);
	
	if (!self->state->machines) {
		return;
	}
	if (self->state->mac_id == psy_INDEX_INVALID ||
			self->state->mac_id == psy_audio_MASTER_INDEX) {
		return;
	}
	wire = psy_audio_wire_make(self->state->mac_id, 
		psy_ui_component_id(psy_ui_button_base(sender)));	
	if (psy_audio_machines_valid_connection(self->state->machines,
			wire)) {
		psy_audio_machines_connect(self->state->machines, wire);		
	}
	psy_ui_component_hide(&self->component);
	machinemenustate_hide_menu(self->state);
}

/* MachineMenu */

/* prototypes */
static void machinemenu_on_connect_to(MachineMenu*, psy_ui_Component* sender);
static void machinemenu_on_connections(MachineMenu*, psy_ui_Component* sender);
static void machinemenu_on_open_parameters(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_clone(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_delete(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_mute(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_bypass(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_solo(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_update_machine_label(MachineMenu*);
static void machinemenu_editname(MachineMenu*, char* rv, uintptr_t strlen);
static void machinemenu_update_machine(MachineMenu*);

/* implementation */
void machinemenu_init(MachineMenu* self, psy_ui_Component* parent,
	WireFrames* wireframes)
{
	assert(self);
		
	psy_ui_component_init(&self->component, parent, NULL);
	machinemenustate_init(&self->state, wireframes, &self->component);	
	psy_ui_component_init(&self->pane, &self->component, NULL);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_default_align(&self->pane, psy_ui_ALIGN_TOP,		
		psy_ui_margin_zero());
	psy_ui_label_init(&self->machine, &self->pane);
	psy_ui_label_prevent_translation(&self->machine);
	psy_ui_label_set_text_alignment(&self->machine,
		psy_ui_ALIGNMENT_CENTER);
	psy_ui_component_set_margin(psy_ui_label_base(&self->machine),
		psy_ui_margin_make_em(0.0, 0.0, 1.0, 0.0));
	psy_ui_button_init_text_connect(&self->parameters, &self->pane,
		"Open parameters", self, machinemenu_on_open_parameters);
	psy_ui_button_init_text(&self->properties, &self->pane, "Open properties");	
	psy_ui_button_init_text(&self->bank, &self->pane, "Open bank manager");
	psy_ui_component_init(&self->separator1, &self->pane, NULL);
	psy_ui_component_set_style_type(&self->separator1, STYLE_SEPARATOR);
	psy_ui_component_set_margin(&self->separator1, psy_ui_margin_make_em(
		0.4, 0.0, 0.4, 0.0));
	psy_ui_button_init_text_connect(&self->connect, &self->pane, "Connect to",
		self, machinemenu_on_connect_to);
	psy_ui_button_init_text_connect(&self->connections, &self->pane,
		"Connections",
		self, machinemenu_on_connections);
	psy_ui_component_init(&self->separator2, &self->pane, NULL);
	psy_ui_component_set_style_type(&self->separator2, STYLE_SEPARATOR);
	psy_ui_component_set_margin(&self->separator2, psy_ui_margin_make_em(
		0.4, 0.0, 0.4, 0.0));
	psy_ui_button_init_text(&self->replace, &self->pane, "Replace machine");
	psy_ui_button_init_text_connect(&self->clone, &self->pane, "Clone machine",
		self, machinemenu_on_machine_clone);
	psy_ui_button_init_text(&self->insertbefore, &self->pane,
		"Insert effect before");
	psy_ui_button_init_text(&self->insertafter, &self->pane,
		"Insert effect after");
	psy_ui_button_init_text_connect(&self->del, &self->pane, "Delete machine",
		self, machinemenu_on_machine_delete);
	psy_ui_component_init(&self->separator3, &self->pane, NULL);
	psy_ui_component_set_style_type(&self->separator3, STYLE_SEPARATOR);
	psy_ui_component_set_margin(&self->separator3, psy_ui_margin_make_em(
		0.4, 0.0, 0.4, 0.0));
	psy_ui_button_init_text_connect(&self->mute, &self->pane, "Mute",
		self, machinemenu_on_machine_mute);
	psy_ui_button_init_text_connect(&self->solo, &self->pane, "Solo",
		self, machinemenu_on_machine_solo);
	psy_ui_button_init_text_connect(&self->bypass, &self->pane, "Bypass",
		self, machinemenu_on_machine_bypass);
	machineconnecttomenu_init(&self->connect_to_menu, &self->component,
		&self->state);
	psy_ui_component_set_align(&self->connect_to_menu.component,
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->connect_to_menu.component);
	machineconnectionsmenu_init(&self->connections_menu, &self->component,
		&self->state);
	psy_ui_component_set_align(&self->connections_menu.component,
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->connections_menu.component);
	psy_ui_component_hide(machinemenu_base(self));
}

void machinemenu_on_connect_to(MachineMenu* self, psy_ui_Component* sender)
{	
	assert(self);
	
	psy_ui_component_hide(&self->connections_menu.component);
	psy_ui_component_toggle_visibility(&self->connect_to_menu.component);	
	psy_ui_component_align(psy_ui_component_parent(&self->component));
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
}

void machinemenu_on_connections(MachineMenu* self, psy_ui_Component* sender)
{
	assert(self);
	
	psy_ui_component_hide(&self->connect_to_menu.component);	
	psy_ui_component_toggle_visibility(&self->connections_menu.component);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
}

void machinemenu_set_machines(MachineMenu* self, psy_audio_Machines* machines)
{	
	assert(self);
	
	self->state.machines = machines;	
	machineconnecttomenu_fill(&self->connect_to_menu);
	machineconnectionsmenu_fill(&self->connections_menu);
}

void machinemenu_select(MachineMenu* self, uintptr_t mac_id)
{
	assert(self);
	
	self->state.mac_id = mac_id;
	machinemenu_update_machine(self);	
}

void machinemenu_update_machine(MachineMenu* self)
{
	assert(self);
	
	machineconnectionsmenu_fill(&self->connections_menu);
	machineconnecttomenu_fill(&self->connect_to_menu);
	machinemenu_update_machine_label(self);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
}

void machinemenu_update_machine_label(MachineMenu* self)
{
	assert(self);
	
	if (self->state.machines && self->state.mac_id != psy_INDEX_INVALID) {
		psy_audio_Machine* machine;
		
		machine = psy_audio_machines_at(self->state.machines,
			self->state.mac_id);
		if (machine) {
			char editname[130];
	
			machinemenu_editname(self, editname, 130);			
			psy_ui_label_set_text(&self->machine, editname);
		}
	} else {
		psy_ui_label_set_text(&self->machine, "No machine selected");
	}
}

void machinemenu_editname(MachineMenu* self, char* rv, uintptr_t strlen)
{
	assert(self);

	rv[0] = '\0';
	if (self->state.machines && self->state.mac_id != psy_INDEX_INVALID) {
		psy_audio_Machine* machine;
		
		machine = psy_audio_machines_at(self->state.machines,
			self->state.mac_id);
		if (machine) {			
			psy_snprintf(rv, strlen, "%.2X:%s", (int)
				psy_audio_machine_slot(machine),
				psy_audio_machine_editname(machine));
		}
	}
}

void machinemenu_on_open_parameters(MachineMenu* self,
	psy_ui_Component* sender)
{
	ParamViews* paramviews;
	
	assert(self);
	
	paramviews = self->state.wireframes->workspace->paramviews;
	if (paramviews) {
		paramviews_show(paramviews, self->state.mac_id);
	}
}

void machinemenu_on_machine_mute(MachineMenu* self, psy_ui_Component* sender)
{
	assert(self);
	
	if (self->state.machines && self->state.mac_id != psy_INDEX_INVALID) {
		psy_audio_Machine* machine;
		
		machine = psy_audio_machines_at(self->state.machines,
			self->state.mac_id);
		if (machine) {			
			if (psy_audio_machine_muted(machine)) {
				psy_audio_machine_unmute(machine);
			} else {
				psy_audio_machine_mute(machine);
			}
		}		
	}
	machinemenu_hide(self);
}

void machinemenu_on_machine_clone(MachineMenu* self, psy_ui_Component* sender)
{
	assert(self);
	
	if (self->state.machines && self->state.mac_id != psy_INDEX_INVALID) {
			/* todo */
	}	
}

void machinemenu_on_machine_delete(MachineMenu* self, psy_ui_Component* sender)
{
	assert(self);
	
	if (self->state.machines && self->state.mac_id != psy_INDEX_INVALID &&
			self->state.mac_id != psy_audio_MASTER_INDEX) {
		psy_audio_machines_remove(self->state.machines, self->state.mac_id,
			TRUE);		
	}
	machinemenu_hide(self);
}

void machinemenu_on_machine_bypass(MachineMenu* self, psy_ui_Component* sender)
{
	assert(self);
	
	if (self->state.machines && self->state.mac_id != psy_INDEX_INVALID) {
		psy_audio_Machine* machine;
		
		machine = psy_audio_machines_at(self->state.machines,
			self->state.mac_id);
		if (machine) {			
			if (psy_audio_machine_bypassed(machine)) {
				psy_audio_machine_bypass(machine);
			} else {
				psy_audio_machine_bypass(machine);
			}
		}
	}
	machinemenu_hide(self);
}

void machinemenu_on_machine_solo(MachineMenu* self, psy_ui_Component* sender)
{
	assert(self);
	
	if (self->state.machines && self->state.mac_id != psy_INDEX_INVALID) {
			psy_audio_machines_solo(self->state.machines, self->state.mac_id);		
	}
	machinemenu_hide(self);
}

void machinemenu_hide(MachineMenu* self)
{
	assert(self);
	
	psy_ui_component_hide(&self->connect_to_menu.component);
	psy_ui_component_hide(&self->connections_menu.component);
	psy_ui_component_hide_align(&self->component);	
}
