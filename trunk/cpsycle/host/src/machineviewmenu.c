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


#define PREVENT_MASTER 0
#define ALLOW_MASTER 1


void machinemenuconfirm_init(MachineMenuConfirm* self,
	psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_label_init_text(&self->msg, &self->component, "mvmenu.sure");
	psy_ui_component_set_style_type(&self->msg.component,
		STYLE_TERM_BUTTON_WARNING);
	psy_ui_label_prevent_wrap(&self->msg);
	psy_ui_label_set_text_alignment(&self->msg, psy_ui_ALIGNMENT_CENTER);
	psy_ui_component_set_align(&self->msg.component, psy_ui_ALIGN_TOP);
	psy_ui_component_init_align(&self->client, &self->component, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_init_align(&self->buttons, &self->client, NULL,
		psy_ui_ALIGN_CENTER);
	psy_ui_button_init_text(&self->ok, &self->buttons, "mvmenu.yes");
	
	psy_ui_component_set_align(&self->ok.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_init_text(&self->cancel, &self->buttons, "mvmenu.no");
	psy_ui_component_set_align(&self->cancel.component, psy_ui_ALIGN_LEFT);
}

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

bool machinemenustate_invalid(const MachineMenuState* self)
{
	assert(self);
	
	return ((!self->machines) || (self->mac_id == psy_INDEX_INVALID));
}

psy_audio_Machine* machinemenustate_machine(MachineMenuState* self,
	bool allow_master)
{
	assert(self);
	
	if (!allow_master && (self->mac_id == psy_audio_MASTER_INDEX)) {		
		return NULL;
	}
	if (machinemenustate_invalid(self)) {		
		return NULL;
	}
	return psy_audio_machines_at(self->machines, self->mac_id);
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
	self->state = state;
	psy_table_init(&self->wires);
	psy_ui_component_set_padding(&self->component, psy_ui_margin_make_em(
		0.0, 1.0, 0.0, 1.0));	
	psy_ui_component_init_align(&self->pane, &self->component, NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_scroll_step_height(&self->pane,
		psy_ui_value_make_eh(1.0));
	psy_ui_component_set_wheel_scroll(&self->pane, 4);
	psy_ui_component_set_overflow(&self->pane, psy_ui_OVERFLOW_VSCROLL);
	psy_ui_component_set_default_align(&self->pane, psy_ui_ALIGN_TOP,		
		psy_ui_margin_zero());
	psy_ui_scroller_init(&self->scroller, &self->component, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->pane);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
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
	
	psy_ui_component_clear(&self->pane);
	psy_table_dispose_all(&self->wires, NULL);
	psy_table_init(&self->wires);		
	if (machinemenustate_invalid(self->state)) {
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
		label = psy_ui_label_allocinit(&self->pane);
		psy_ui_label_set_text(label, "mvmenu.inputs");
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
		separator = psy_ui_component_allocinit(&self->pane, NULL);
		psy_ui_component_set_style_type(separator, STYLE_SEPARATOR);
			psy_ui_component_set_margin(separator, psy_ui_margin_make_em(
			0.4, 0.0, 0.4, 0.0));
		/* outputs */	
		label = psy_ui_label_allocinit(&self->pane);
		psy_ui_label_set_text(label, "mvmenu.outputs");
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
		
		label = psy_ui_label_allocinit(&self->pane);
		psy_ui_label_set_text(label, "mvmenu.noconnections");
	}
}

void machineconnectionsmenu_append(MachineConnectionsMenu* self, char* str,
	uintptr_t idx, psy_audio_Wire wire)
{
	psy_ui_Button* button;
	psy_audio_Wire* newwire;
	
	assert(self);
	
	button = psy_ui_button_allocinit(&self->pane);
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
	psy_ui_component_set_padding(&self->component, psy_ui_margin_make_em(
		0.0, 1.0, 0.0, 1.0));
	psy_ui_component_init_align(&self->pane, &self->component, NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_scroll_step_height(&self->pane,
		psy_ui_value_make_eh(1.0));
	psy_ui_component_set_wheel_scroll(&self->pane, 4);
	psy_ui_component_set_overflow(&self->pane, psy_ui_OVERFLOW_VSCROLL);
	psy_ui_component_set_default_align(&self->pane, psy_ui_ALIGN_TOP,		
		psy_ui_margin_zero());
	psy_ui_scroller_init(&self->scroller, &self->component, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->pane);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
	machineconnecttomenu_fill(self);
}

void machineconnecttomenu_fill(MachineConnectToMenu* self)
{	
	psy_TableIterator it;
	bool connections;	
	
	assert(self);
	
	psy_ui_component_clear(&self->pane);
	connections = FALSE;
	if (self->state->mac_id != psy_audio_MASTER_INDEX &&
			!machinemenustate_invalid(self->state)) {				
		for (it = psy_audio_machines_begin(self->state->machines);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_Wire wire;
			
			wire = psy_audio_wire_make(self->state->mac_id,
				psy_tableiterator_key(&it));
			if (psy_audio_machines_valid_connection(self->state->machines,
						wire) &&
					(!psy_audio_machines_connected(self->state->machines,
						wire))) {
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
	}
	if (!connections) {
		psy_ui_Label* label;		
		
		label = psy_ui_label_allocinit(&self->pane);
		psy_ui_label_set_text(label, "mvmenu.nodestinations");
	}
}

void machineconnecttomenu_append(MachineConnectToMenu* self, char* str,
	uintptr_t dst)
{
	psy_ui_Button* button;
	
	assert(self);
	
	button = psy_ui_button_allocinit(&self->pane);
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

/* MachineMenuTitle */

static void machinemenutitle_on_accept_edit(MachineMenuTitle*,
	psy_ui_TextArea* sender);

/* implementation */
void machinemenutitle_init(MachineMenuTitle* self, psy_ui_Component* parent,
	MachineMenuState* state)
{
	assert(self);

	psy_ui_component_init_align(&self->component, parent, NULL,
		psy_ui_ALIGN_TOP);
	self->state = state;	
	psy_ui_component_set_style_type(&self->component, STYLE_HEADER);
	psy_ui_component_init_align(&self->client, &self->component, NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_label_init(&self->mac_id, &self->component);
	psy_ui_label_set_char_number(&self->mac_id, 4);
	psy_ui_component_set_align(&self->mac_id.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_prevent_translation(&self->mac_id);
	psy_ui_textarea_init_single_line(&self->title, &self->client);
	psy_ui_textarea_set_char_number(&self->title, 32);
	psy_ui_component_set_align(&self->title.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->title.signal_accept, self,
		machinemenutitle_on_accept_edit);
	psy_ui_textarea_enable_input_field(&self->title);
	psy_ui_button_init(&self->hide, &self->component);
	psy_ui_button_prevent_translation(&self->hide);
	psy_ui_button_set_text(&self->hide, "X");	
	psy_ui_component_set_align(&self->hide.component, psy_ui_ALIGN_RIGHT);
}

void machinemenutitle_update(MachineMenuTitle* self)
{	
	psy_audio_Machine* machine;	
	char str[64];
	
	assert(self);
	
	machine = machinemenustate_machine(self->state, ALLOW_MASTER);
	if (machine) {
		psy_ui_textarea_set_text(&self->title,
			psy_audio_machine_editname(machine));
		psy_snprintf(str, 64, "%.2X", (int)psy_audio_machine_slot(machine));
		psy_ui_label_set_text(&self->mac_id, str);
	} else {
		psy_ui_textarea_set_text(&self->title,
			"No machine selected");
		psy_ui_label_set_text(&self->mac_id, "--");
	}
}

void machinemenutitle_on_accept_edit(MachineMenuTitle* self,
	psy_ui_TextArea* sender)
{
	psy_audio_Machine* machine;	
	
	assert(self);
	
	if (machinemenustate_invalid(self->state) || (self->state->mac_id ==
			psy_audio_MASTER_INDEX)) {
		return;
	}	
	psy_audio_machines_rename(self->state->machines, self->state->mac_id,
		psy_ui_textarea_text(&self->title));
}


/* MachineMenu */

/* prototypes */
static void machinemenu_on_connect_to(MachineMenu*, psy_ui_Component* sender);
static void machinemenu_on_connections(MachineMenu*, psy_ui_Component* sender);
static void machinemenu_on_open_parameters(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_clone(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_insert_before(MachineMenu*,
	psy_ui_Component* sender);	
static void machinemenu_on_machine_insert_after(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_replace(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_delete_confirm(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_confirm_reject(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_delete(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_mute(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_bypass(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_on_machine_solo(MachineMenu*,
	psy_ui_Component* sender);
static void machinemenu_update_machine(MachineMenu*);
static void machinemenu_on_timer(MachineMenu*, uintptr_t timerid);
static void machinemenu_on_close_button(MachineMenu*, psy_ui_Button* sender);

/* vtable */
static psy_ui_ComponentVtable machinemenu_vtable;
static bool machinemenu_vtable_initialized = FALSE;

static void machinemenu_vtable_init(MachineMenu* self)
{
	assert(self);
	
	if (!machinemenu_vtable_initialized) {
		machinemenu_vtable = *(self->component.vtable);
		machinemenu_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			machinemenu_on_timer;
		machinemenu_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &machinemenu_vtable);	
}	

/* implementation */
void machinemenu_init(MachineMenu* self, psy_ui_Component* parent,
	WireFrames* wireframes)
{
	assert(self);
		
	psy_ui_component_init(&self->component, parent, NULL);
	machinemenu_vtable_init(self);
	machinemenustate_init(&self->state, wireframes, &self->component);	
	psy_ui_component_init(&self->pane, &self->component, NULL);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_default_align(&self->pane, psy_ui_ALIGN_TOP,		
		psy_ui_margin_zero());
	machinemenutitle_init(&self->title_bar, &self->pane, &self->state);
	psy_ui_component_set_margin(&self->title_bar.component,
		psy_ui_margin_make_em(0.0, 0.0, 1.0, 0.0));
	psy_signal_connect(&self->title_bar.hide.signal_clicked, self,
		machinemenu_on_close_button);	
	psy_ui_button_init_text_connect(&self->parameters, &self->pane,
		"mvmenu.parameters", self, machinemenu_on_open_parameters);	
	psy_ui_button_init_text(&self->bank, &self->pane, "mvmenu.bank");
	psy_ui_component_init(&self->separator1, &self->pane, NULL);
	psy_ui_component_set_style_type(&self->separator1, STYLE_SEPARATOR);
	psy_ui_component_set_margin(&self->separator1, psy_ui_margin_make_em(
		0.4, 0.0, 0.4, 0.0));
	psy_ui_button_init_text_connect(&self->connect, &self->pane, "mvmenu.connect",
		self, machinemenu_on_connect_to);
	psy_ui_button_init_text_connect(&self->connections, &self->pane,
		"mvmenu.connections",
		self, machinemenu_on_connections);
	psy_ui_component_init(&self->separator2, &self->pane, NULL);
	psy_ui_component_set_style_type(&self->separator2, STYLE_SEPARATOR);
	psy_ui_component_set_margin(&self->separator2, psy_ui_margin_make_em(
		0.4, 0.0, 0.4, 0.0));
	psy_ui_button_init_text_connect(&self->replace, &self->pane, "mvmenu.replace",
		self, machinemenu_on_machine_replace);
	psy_ui_button_init_text_connect(&self->clone, &self->pane, "mvmenu.clone",
		self, machinemenu_on_machine_clone);
	psy_ui_button_init_text_connect(&self->insertbefore, &self->pane,
		"mvmenu.insertbefore", self, machinemenu_on_machine_insert_before);
	psy_ui_button_init_text_connect(&self->insertafter, &self->pane,
		"mvmenu.insertafter", self, machinemenu_on_machine_insert_after);
	psy_ui_button_init_text_connect(&self->del, &self->pane, "mvmenu.delete",
		self, machinemenu_on_machine_delete_confirm);
	machinemenuconfirm_init(&self->confirm, &self->pane);
	psy_signal_connect(&self->confirm.ok.signal_clicked, self,
		machinemenu_on_machine_delete);
	psy_signal_connect(&self->confirm.cancel.signal_clicked, self,
		machinemenu_on_machine_confirm_reject);		
	psy_ui_component_hide(&self->confirm.component);
	psy_ui_component_init(&self->separator3, &self->pane, NULL);
	psy_ui_component_set_style_type(&self->separator3, STYLE_SEPARATOR);
	psy_ui_component_set_margin(&self->separator3, psy_ui_margin_make_em(
		0.4, 0.0, 0.4, 0.0));
	psy_ui_button_init_text_connect(&self->mute, &self->pane, "mvmenu.mute",
		self, machinemenu_on_machine_mute);
	psy_ui_button_init_text_connect(&self->solo, &self->pane, "mvmenu.solo",
		self, machinemenu_on_machine_solo);
	psy_ui_button_init_text_connect(&self->bypass, &self->pane, "mvmenu.bypass",
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
	psy_ui_component_start_timer(&self->component, 0, 100);
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
	machinemenutitle_update(&self->title_bar);	
	psy_ui_component_align(psy_ui_component_parent(&self->component));
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
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
	psy_audio_Machine* machine;
	
	assert(self);
		
	machinemenu_hide(self);
	machine = machinemenustate_machine(&self->state, PREVENT_MASTER);
	if (machine) {
		if (psy_audio_machine_muted(machine)) {
			psy_audio_machine_unmute(machine);
		} else {
			psy_audio_machine_mute(machine);
		}
	}	
}

void machinemenu_on_machine_clone(MachineMenu* self, psy_ui_Component* sender)
{
	psy_audio_Machine* machine;
	assert(self);
	
	machinemenu_hide(self);
	machine = machinemenustate_machine(&self->state, PREVENT_MASTER);
	if (machine) {
		psy_audio_Machine* clone;		
		
		clone = psy_audio_machine_clone(machine);
		if (clone) {
			Workspace* workspace;
			double x, y;
			psy_ui_Style* style;
			psy_ui_Size size;
			const psy_ui_TextMetric* tm;
			
			if (psy_audio_machine_mode(machine) == psy_audio_MACHMODE_FX) {
				style = psy_ui_style(STYLE_MV_EFFECT);
			} else {
				style = psy_ui_style(STYLE_MV_GENERATOR);
			}
			size = psy_ui_style_size(style);
			psy_audio_machine_position(machine, &x, &y);
			tm = psy_ui_component_textmetric(&self->component);
			x += 32;
			y += psy_ui_value_px(&size.height, tm, NULL) + 8;
			psy_audio_machine_setposition(clone, x, y);
			workspace = self->state.wireframes->workspace;
			workspace->insert.random_position = TRUE;			
			psy_audio_machines_append(self->state.machines, clone);
		}
	}	
}

void machinemenu_on_machine_insert_before(MachineMenu* self,
	psy_ui_Component* sender)
{
	Workspace* workspace;
	psy_audio_WireSocket* socket;
	psy_audio_Wire wire;
	
	assert(self);
	
	machinemenu_hide(self);
	if (machinemenustate_invalid(&self->state)) {
		return;
	}	
	workspace = self->state.wireframes->workspace;	
	socket = psy_audio_machines_input_socket(self->state.machines,
		self->state.mac_id, 0);
	wire.dst = self->state.mac_id;
	if (socket) {
		wire.src = socket->slot;
	} else {
		wire.src = psy_INDEX_INVALID;
	}
	machineinsert_append(&workspace->insert, wire);
	workspace_select_view(workspace, viewindex_make_section(
		VIEW_ID_MACHINEVIEW, SECTION_ID_MACHINEVIEW_NEWMACHINE));
}
	
void machinemenu_on_machine_insert_after(MachineMenu* self,
	psy_ui_Component* sender)
{
	Workspace* workspace;
	psy_audio_WireSocket* socket;
	psy_audio_Wire wire;
	
	assert(self);
	
	machinemenu_hide(self);
	if (machinemenustate_invalid(&self->state) ||
			self->state.mac_id == psy_audio_MASTER_INDEX) {
		return;
	}	
	workspace = self->state.wireframes->workspace;
	socket = psy_audio_machines_output_socket(self->state.machines,
		self->state.mac_id, 0);
	wire.src = self->state.mac_id;
	if (socket) {
		wire.dst = socket->slot;
	} else {
		wire.dst = psy_INDEX_INVALID;
	}	
	machineinsert_append(&workspace->insert, wire);
	workspace_select_view(workspace, viewindex_make_section(
		VIEW_ID_MACHINEVIEW, SECTION_ID_MACHINEVIEW_NEWMACHINE));
}

void machinemenu_on_machine_replace(MachineMenu* self,
	psy_ui_Component* sender)
{
	Workspace* workspace;
	
	machinemenu_hide(self);
	if (machinemenustate_invalid(&self->state) ||
			self->state.mac_id == psy_audio_MASTER_INDEX) {
		return;
	}
	workspace = self->state.wireframes->workspace;
	machineinsert_replace(&workspace->insert, self->state.mac_id);
	workspace_select_view(workspace, viewindex_make_section(
		VIEW_ID_MACHINEVIEW, SECTION_ID_MACHINEVIEW_NEWMACHINE));
}

void machinemenu_on_machine_delete_confirm(MachineMenu* self,
	psy_ui_Component* sender)
{
	psy_ui_component_toggle_visibility(&self->confirm.component);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
}

void machinemenu_on_machine_delete(MachineMenu* self, psy_ui_Component* sender)
{
	assert(self);
		
	machinemenu_hide(self);	
	if (machinemenustate_invalid(&self->state) ||
			self->state.mac_id == psy_audio_MASTER_INDEX) {
		return;
	}	
	psy_audio_machines_remove(self->state.machines, self->state.mac_id, TRUE);		
}

void machinemenu_on_machine_confirm_reject(MachineMenu* self,
	psy_ui_Component* sender)
{
	machinemenu_hide(self);
}

void machinemenu_on_machine_bypass(MachineMenu* self, psy_ui_Component* sender)
{
	psy_audio_Machine* machine;
	
	assert(self);
		
	machinemenu_hide(self);	
	machine = machinemenustate_machine(&self->state, PREVENT_MASTER);
	if (machine) {			
		if (psy_audio_machine_bypassed(machine)) {
			psy_audio_machine_bypass(machine);
		} else {
			psy_audio_machine_bypass(machine);	
		}
	}	
}

void machinemenu_on_machine_solo(MachineMenu* self, psy_ui_Component* sender)
{
	assert(self);
	
	machinemenu_hide(self);
	if (machinemenustate_invalid(&self->state) ||
			(self->state.mac_id == psy_audio_MASTER_INDEX)) {		
		return;
	}
	psy_audio_machines_solo(self->state.machines, self->state.mac_id);	
}

void machinemenu_hide(MachineMenu* self)
{
	assert(self);
	
	psy_ui_component_hide(&self->connect_to_menu.component);
	psy_ui_component_set_scroll_top_px(&self->connect_to_menu.pane, 0.0);
	psy_ui_component_hide(&self->connections_menu.component);
	psy_ui_component_set_scroll_top_px(&self->connections_menu.pane, 0.0);
	psy_ui_component_hide(&self->confirm.component);
	psy_ui_component_hide_align(&self->component);	
}

void machinemenu_on_timer(MachineMenu* self, uintptr_t timerid)
{
	psy_audio_Machine* machine;
	
	assert(self);
			
	machine = machinemenustate_machine(&self->state, PREVENT_MASTER);
	if (machine) {	
		if (self->state.mac_id == psy_audio_machines_soloed(
				self->state.machines)) {
			psy_ui_button_highlight(&self->solo);
		} else {
			psy_ui_button_disable_highlight(&self->solo);
		}
		if (psy_audio_machine_muted(machine)) {
			psy_ui_button_highlight(&self->mute);
		} else {
			psy_ui_button_disable_highlight(&self->mute);
		}
		if (psy_audio_machine_bypassed(machine)) {
			psy_ui_button_highlight(&self->bypass);
		} else {
			psy_ui_button_disable_highlight(&self->bypass);
		}
	}
}

void machinemenu_on_close_button(MachineMenu* self, psy_ui_Button* sender)
{
	assert(self);
	
	machinemenu_hide(self);
}
