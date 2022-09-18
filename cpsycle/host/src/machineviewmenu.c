/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

/* host */
#include "styles.h"
#include "machineviewmenu.h"
#include "wireframes.h"

/* platforn */
#include "../../detail/portable.h"


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
	uintptr_t mac_id;
	uintptr_t idx;
	
	assert(self);
	
	psy_ui_component_clear(&self->component);
	psy_table_dispose_all(&self->wires, NULL);
	psy_table_init(&self->wires);		
	if (!self->state->machines) {
		return;
	}
	mac_id = psy_audio_machines_selected(self->state->machines);
	sockets = psy_audio_connections_at(&self->state->machines->connections,
		mac_id);
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
						psy_audio_wire_make(socket->slot, mac_id));
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
						psy_audio_wire_make(mac_id, socket->slot));
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
}

/* MachineConnectToMenu */

/* prototypes */
static void machineconnecttomenu_append(MachineConnectToMenu* self, char* str);
static void machineconnecttomenu_fill(MachineConnectToMenu* self);

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
	uintptr_t mac_id;
	char str[64];
	
	assert(self);
	
	if (!self->state->machines) {
		return;
	}
	mac_id = psy_audio_machines_selected(self->state->machines);	
	psy_ui_component_clear(&self->component);	
	psy_snprintf(str, 64, "%02X: %s", psy_audio_MASTER_INDEX, "Master");
	machineconnecttomenu_append(self, str);
}

void machineconnecttomenu_append(MachineConnectToMenu* self, char* str)
{
	psy_ui_Button* button;
	
	assert(self);
	
	button = psy_ui_button_allocinit(&self->component);
	psy_ui_button_prevent_translation(button);
	psy_ui_button_set_text(button, str);	
}


/* MachineMenu */

/* prototypes */
static void machinemenu_on_connect_to(MachineMenu*, psy_ui_Component* sender);
static void machinemenu_on_connections(MachineMenu*, psy_ui_Component* sender);

/* implementation */
void machinemenu_init(MachineMenu* self, psy_ui_Component* parent,
	WireFrames* wireframes)
{
	assert(self);
		
	psy_ui_component_init(&self->component, parent, NULL);
	self->state.machines = NULL;
	self->state.wireframes = wireframes;
	psy_ui_component_init(&self->pane, &self->component, NULL);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_default_align(&self->pane, psy_ui_ALIGN_TOP,		
		psy_ui_margin_zero());
	psy_ui_button_init_text(&self->parameters, &self->pane, "Open parameters");
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
	psy_ui_button_init_text(&self->clone, &self->pane, "Clone machine");
	psy_ui_button_init_text(&self->insertbefore, &self->pane,
		"Insert effect before");
	psy_ui_button_init_text(&self->insertafter, &self->pane,
		"Insert effect after");
	psy_ui_button_init_text(&self->del, &self->pane, "Delete machine");
	psy_ui_component_init(&self->separator3, &self->pane, NULL);
	psy_ui_component_set_style_type(&self->separator3, STYLE_SEPARATOR);
	psy_ui_component_set_margin(&self->separator3, psy_ui_margin_make_em(
		0.4, 0.0, 0.4, 0.0));
	psy_ui_button_init_text(&self->mute, &self->pane, "Mute");
	psy_ui_button_init_text(&self->solo, &self->pane, "Solo");
	psy_ui_button_init_text(&self->bypass, &self->pane, "Bypass");
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
	
	psy_ui_component_toggle_visibility(&self->connect_to_menu.component);	
	psy_ui_component_align(psy_ui_component_parent(&self->component));
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
}

void machinemenu_on_connections(MachineMenu* self, psy_ui_Component* sender)
{
	assert(self);
	
	if (!psy_ui_component_visible(&self->connections_menu.component)) {
		machineconnectionsmenu_fill(&self->connections_menu);
	}
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
