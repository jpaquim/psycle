/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "paramgear.h"

/* host */
#include "styles.h"
/* platform */
#include "../../detail/portable.h"


/* ParamRackBox */
/* prototypes */
static void paramrackbox_onmousedoubleclick(ParamRackBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void paramrackbox_onaddeffect(ParamRackBox*, psy_ui_Button* sender);

/* implementation */
void paramrackbox_init(ParamRackBox* self, psy_ui_Component* parent,
	uintptr_t slot, Workspace* workspace)
{	
	psy_audio_Machine* machine;
	psy_ui_Margin margin;

	machine = psy_audio_machines_at(&workspace->song->machines, slot);
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_set_style_type(&self->component, STYLE_MACPARAM_TITLE);	
	self->workspace = workspace;
	self->slot = slot;	
	psy_ui_component_init(&self->header, &self->component, NULL);	
	psy_ui_component_set_style_type(&self->header, STYLE_MACPARAM_TITLE);
	psy_ui_component_set_align(&self->header, psy_ui_ALIGN_TOP);	
	/* title label */
	psy_ui_label_init(&self->title, &self->header);	
	psy_ui_component_set_style_type(&self->title.component, STYLE_MACPARAM_TITLE);
	psy_ui_label_prevent_translation(&self->title);	
	if (machine) {
		psy_ui_label_set_text(&self->title, psy_audio_machine_editname(machine));
	}
	psy_ui_component_set_align(&self->title.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->title.component.signal_mousedoubleclick, self,
		paramrackbox_onmousedoubleclick);	
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 0.5, 0.5);		
	//psy_ui_component_set_padding(&self->title.component, &margin);
	/* Insert Effect */
	if (machine && slot != psy_audio_MASTER_INDEX) {
		psy_ui_button_init_text(&self->inserteffect, &self->header, "+");
		psy_ui_component_set_align(&self->inserteffect.component, psy_ui_ALIGN_RIGHT);
		psy_signal_connect(&self->inserteffect.signal_clicked, self,
			paramrackbox_onaddeffect);
	}
	/* Parameter List */
	parameterlistbox_init(&self->parameters, &self->component,
		psy_audio_machines_at(&workspace->song->machines, slot),
		psycleconfig_macparam(workspace_conf(workspace)));
	psy_ui_component_set_align(&self->parameters.component, psy_ui_ALIGN_CLIENT);
	self->nextbox = NULL;	
}

ParamRackBox* paramrackbox_alloc(void)
{
	return(ParamRackBox*)malloc(sizeof(ParamRackBox));
}

ParamRackBox* paramrackbox_allocinit(psy_ui_Component* parent, uintptr_t slot,
	Workspace* workspace)
{
	ParamRackBox* rv;
	
	rv = paramrackbox_alloc();
	if (rv) {
		paramrackbox_init(rv, parent, slot, workspace);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void paramrackbox_select(ParamRackBox* self)
{
	psy_ui_component_invalidate(&self->header);
}

void paramrackbox_deselect(ParamRackBox* self)
{
	psy_ui_component_invalidate(&self->header);
}

void paramrackbox_onmousedoubleclick(ParamRackBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	workspace_show_parameters(self->workspace, self->slot);	
}

void paramrackbox_onaddeffect(ParamRackBox* self, psy_ui_Button* sender)
{
	if (self->workspace && workspace_song(self->workspace)) {
		if (self->nextbox) {
			psy_audio_machines_selectwire(&self->workspace->song->machines,
				psy_audio_wire_make(self->slot, self->nextbox->slot));
			workspace_select_view(self->workspace,
				viewindex_make(VIEW_ID_MACHINEVIEW,
				SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_ADDEFFECT,
					psy_INDEX_INVALID));
		} else {
			workspace_select_view(self->workspace,
				viewindex_make(VIEW_ID_MACHINEVIEW,
					SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_APPEND,
					psy_INDEX_INVALID));
		}
	}
}

/* ParamRackPane */

/* implementation */
static void paramrackpane_on_destroy(ParamRackPane*);
static void paramrackpane_build(ParamRackPane*);
static void paramrackpane_buildall(ParamRackPane*);
static void paramrackpane_buildinputs(ParamRackPane*);
static void paramrackpane_buildoutputs(ParamRackPane*);
static void paramrackpane_buildinchain(ParamRackPane*);
static void paramrackpane_buildoutchain(ParamRackPane*, uintptr_t slot);
static void paramrackpane_buildlevel(ParamRackPane*, uintptr_t level);
static void paramrackpane_insertbox(ParamRackPane*, uintptr_t slot);
static void paramrackpane_removebox(ParamRackPane*, uintptr_t slot);
static void paramrackpane_clear(ParamRackPane*);
static void paramrackpane_connectsong(ParamRackPane*);
static void paramrackpane_onmachinesinsert(ParamRackPane*,
	psy_audio_Machines* sender, uintptr_t slot);
static void paramrackpane_onmachinesremoved(ParamRackPane*,
	psy_audio_Machines* sender, uintptr_t slot);
static void paramrackpane_onconnected(ParamRackPane*,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot);
static void paramrackpane_ondisconnected(ParamRackPane*,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot);
static void paramrackpane_onsongchanged(ParamRackPane*, Workspace* sender);
static void paramrackpane_onmachineselected(ParamRackPane*,
	psy_audio_Machines* sender, uintptr_t slot);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ParamRackPane* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroy =
			(psy_ui_fp_component_event)
			paramrackpane_on_destroy;		
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}

/* implementation */
void paramrackpane_init(ParamRackPane* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);	
	self->lastselected = psy_INDEX_INVALID;
	self->level = 2;	
	psy_ui_component_set_defaultalign(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_margin_make(
		psy_ui_value_make_eh(0.0), psy_ui_value_make_ew(0.1),
		psy_ui_value_make_eh(0.0), psy_ui_value_make_ew(0.0)));	
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_set_scroll_step_width(&self->component, psy_ui_value_make_px(100));	
	self->mode = PARAMRACK_OUTCHAIN;
	self->workspace = workspace;
	self->lastinserted = NULL;
	psy_table_init(&self->boxes);	
	paramrackpane_connectsong(self);
	paramrackpane_build(self);
	psy_signal_connect(&workspace->signal_songchanged, self,
		paramrackpane_onsongchanged);
}

void paramrackpane_on_destroy(ParamRackPane* self)
{
	psy_table_dispose(&self->boxes);
}

void paramrackpane_setmode(ParamRackPane* self, ParamRackMode mode)
{
	if (self->mode != mode) {
		self->mode = mode;
		paramrackpane_build(self);
	}
}

void paramrackpane_build(ParamRackPane* self)
{		
	psy_ui_component_hide(&self->component);
	paramrackpane_clear(self);
	if (!self->machines) {	
		psy_ui_component_show(&self->component);
		psy_ui_component_align_full(psy_ui_component_parent(&self->component));		
		return;
	}	
	self->lastselected = psy_audio_machines_selected(self->machines);
	self->lastinserted = NULL;
	switch (self->mode) {
	case PARAMRACK_INPUTS:		
		paramrackpane_buildinputs(self);			
	break;
	case PARAMRACK_OUTPUTS:					
		paramrackpane_buildoutputs(self);			
		break;
	case PARAMRACK_INCHAIN:		
		paramrackpane_buildinchain(self);			
		break;
	case PARAMRACK_OUTCHAIN: {		
		paramrackpane_buildoutchain(self, self->lastselected);			
		break; }
	case PARAMRACK_LEVEL: {		
		paramrackpane_buildlevel(self, self->level);			
		break; }		
	case PARAMRACK_ALL:
	default:			
		paramrackpane_buildall(self);
		break;
	}	
	psy_ui_component_show(&self->component);
	psy_ui_component_align_full(psy_ui_component_parent(&self->component));	
}

void paramrackpane_onsongchanged(ParamRackPane* self, Workspace* sender)
{	
	self->lastselected = psy_INDEX_INVALID;
	paramrackpane_connectsong(self);
	paramrackpane_build(self);
}

void paramrackpane_clear(ParamRackPane* self)
{
	psy_ui_component_clear(&self->component);
	psy_table_clear(&self->boxes);
}

void paramrackpane_buildall(ParamRackPane* self)
{
	psy_TableIterator it;	
	
	for (it = psy_audio_machines_begin(self->machines);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		if (!psy_audio_machines_isvirtualgenerator(self->machines,
				psy_tableiterator_key(&it))) {
			paramrackpane_insertbox(self, psy_tableiterator_key(&it));
		}
	}
}

void paramrackpane_buildinputs(ParamRackPane* self)
{
	psy_audio_MachineSockets* sockets;

	paramrackpane_insertbox(self, self->lastselected);
	sockets = psy_audio_connections_at(&self->machines->connections,
		self->lastselected);
	if (sockets) {
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->inputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (socket->slot != psy_INDEX_INVALID) {
				if (!psy_table_exists(&self->boxes, socket->slot)) {
					paramrackpane_insertbox(self, socket->slot);
				}
			}
		}
	}
}

void paramrackpane_buildoutputs(ParamRackPane* self)
{	
	psy_audio_MachineSockets* sockets;

	paramrackpane_insertbox(self, self->lastselected);
	sockets = psy_audio_connections_at(&self->machines->connections,
		self->lastselected);
	if (sockets) {
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->outputs);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (socket->slot != psy_INDEX_INVALID) {
				if (!psy_table_exists(&self->boxes, socket->slot)) {
					paramrackpane_insertbox(self, socket->slot);
				}
			}
		}
	}
}

void paramrackpane_buildinchain(ParamRackPane* self)
{
	if (self->machines) {
		MachineList* path;
			
		path = psy_audio_compute_path(self->machines, self->lastselected,
			FALSE);
		for (; path != 0; path = path->next) {
			uintptr_t slot;

			slot = (size_t)path->entry;
			if (slot == psy_INDEX_INVALID) {
				// delimits the machines that could be processed parallel
				// todo: add thread functions
				continue;
			}
			paramrackpane_insertbox(self, slot);
		}
		psy_list_free(path);
	}
}

void paramrackpane_buildoutchain(ParamRackPane* self, uintptr_t slot)
{
	psy_audio_MachineSockets* sockets;
	
	paramrackpane_insertbox(self, slot);
	sockets = psy_audio_connections_at(&self->machines->connections, slot);
	if (sockets) {
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->outputs);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (socket->slot != psy_INDEX_INVALID) {
				if (!psy_table_exists(&self->boxes, socket->slot)) {
					paramrackpane_buildoutchain(self, socket->slot);
				}
			}
		}
	}
}

void paramrackpane_buildlevel(ParamRackPane* self, uintptr_t level)
{
	if (self->machines) {
		MachineList* path;

		path = psy_audio_machines_level(self->machines, psy_audio_MASTER_INDEX, level);
		for (; path != 0; path = path->next) {
			uintptr_t slot;

			slot = (size_t)path->entry;
			if (slot == psy_INDEX_INVALID) {
				continue;
			}
			paramrackpane_insertbox(self, slot);
		}
		psy_list_free(path);
	}
}

 void paramrackpane_insertbox(ParamRackPane* self, uintptr_t slot)
{
	if (psy_audio_machines_at(self->machines, slot)) {
		ParamRackBox* box;

		if (psy_table_exists(&self->boxes, slot)) {
			paramrackpane_removebox(self, slot);
		}
		box = paramrackbox_allocinit(&self->component, slot, self->workspace);
		if (box) {												
			if (self->lastinserted) {
				self->lastinserted->nextbox = box;
			}
			self->lastinserted = box;
			psy_ui_component_set_align(&box->component, psy_ui_ALIGN_LEFT);
			psy_table_insert(&self->boxes, slot, box);			
		}		
	}
}

void paramrackpane_removebox(ParamRackPane* self, uintptr_t slot)
{
	if (psy_table_exists(&self->boxes, slot)) {
		ParamRackBox* box;

		box = (ParamRackBox*)psy_table_at(&self->boxes, slot);
		psy_ui_component_deallocate(&box->component);		
		psy_table_remove(&self->boxes, slot);		
	}
}

void paramrackpane_connectsong(ParamRackPane* self)
{
	if (workspace_song(self->workspace)) {
		self->machines = &self->workspace->song->machines;
		psy_signal_connect(&self->machines->signal_slotchange, self,
			paramrackpane_onmachineselected);
		psy_signal_connect(&self->machines->signal_insert, self,
			paramrackpane_onmachinesinsert);
		psy_signal_connect(&self->machines->signal_removed, self,
			paramrackpane_onmachinesremoved);
		psy_signal_connect(&self->machines->connections.signal_connected, self,
			paramrackpane_onconnected);
		psy_signal_connect(&self->machines->connections.signal_disconnected, self,
			paramrackpane_ondisconnected);
	} else {
		self->machines = NULL;
	}
}

void paramrackpane_onmachinesinsert(ParamRackPane* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	// handled in select
}

void paramrackpane_onmachinesremoved(ParamRackPane* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	paramrackpane_build(self);
}

void paramrackpane_onconnected(ParamRackPane* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{
	if (self->mode != PARAMRACK_ALL) {
		paramrackpane_build(self);
	}
}

void paramrackpane_ondisconnected(ParamRackPane* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{
	if (self->mode != PARAMRACK_ALL) {
		paramrackpane_build(self);
	}
}

void paramrackpane_onmachineselected(ParamRackPane* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	if (self->lastselected == slot) {
		return;
	}
	if (self->mode == PARAMRACK_LEVEL) {
		uintptr_t level;

		level = psy_audio_machines_levelofmachine(self->machines, slot);
		if (level != psy_INDEX_INVALID) {
			paramrackpane_build(self);			
		}
	} else if (self->mode == PARAMRACK_ALL) {
		if (psy_table_exists(&self->boxes, self->lastselected)) {
			paramrackbox_deselect((ParamRackBox*)
				psy_table_at(&self->boxes, self->lastselected));
		}
		if (psy_table_exists(&self->boxes, slot)) {
			paramrackbox_select((ParamRackBox*)
				psy_table_at(&self->boxes, slot));
			self->lastselected = slot;
		} else {
			self->lastselected = psy_INDEX_INVALID;
		}
	} else {
		self->lastselected = slot;
		paramrackpane_build(self);
	}
}

/* ParamRackBatchBar */
void paramrackbatchbar_init(ParamRackBatchBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_defaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_button_init_text(&self->select, &self->component, "Select in Gear");
}

/* ParamRackModeBar */
static void paramrackmodebar_on_destroy(ParamRackModeBar*);
static void paramrackmodebar_onmodeselect(ParamRackModeBar*, psy_ui_Button* sender);

/* vtable */
static psy_ui_ComponentVtable paramrackmodebar_vtable;
static bool paramrackmodebar_vtable_initialized = FALSE;

static void paramrackmodebar_vtable_init(ParamRackModeBar* self)
{
	if (!paramrackmodebar_vtable_initialized) {
		paramrackmodebar_vtable = *(self->component.vtable);
		paramrackmodebar_vtable.on_destroy =
			(psy_ui_fp_component_event)
			paramrackmodebar_on_destroy;
		paramrackmodebar_vtable_initialized = TRUE;
	}
	self->component.vtable = &paramrackmodebar_vtable;
}
// implementation
void paramrackmodebar_init(ParamRackModeBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	paramrackmodebar_vtable_init(self);
	psy_ui_component_set_defaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_signal_init(&self->signal_select);
	psy_ui_button_init_text_connect(&self->all, &self->component,
		"All", self, paramrackmodebar_onmodeselect);
	psy_ui_button_init_text_connect(&self->inputs, &self->component,
		"Inputs", self, paramrackmodebar_onmodeselect);
	psy_ui_button_init_text_connect(&self->outputs, &self->component,
		"Outputs", self, paramrackmodebar_onmodeselect);
	psy_ui_button_init_text_connect(&self->inchain, &self->component,
		"Inchain", self, paramrackmodebar_onmodeselect);
	psy_ui_button_init_text_connect(&self->outchain, &self->component,
		"Outchain", self, paramrackmodebar_onmodeselect);
	psy_ui_button_init_text_connect(&self->level, &self->component,
		"Level", self, paramrackmodebar_onmodeselect);
}

void paramrackmodebar_on_destroy(ParamRackModeBar* self)
{
	psy_signal_dispose(&self->signal_select);
}

void paramrackmodebar_onmodeselect(ParamRackModeBar* self, psy_ui_Button* sender)
{
	ParamRackMode mode;
		
	mode = PARAMRACK_NONE;
	if (sender == &self->all) {		
		mode = PARAMRACK_ALL;
	} else if (sender == &self->inputs) {
		mode = PARAMRACK_INPUTS;
	} else if (sender == &self->outputs) {
		mode = PARAMRACK_OUTPUTS;
	} else if (sender == &self->inchain) {
		mode = PARAMRACK_INCHAIN;
	} else if (sender == &self->outchain) {
		mode = PARAMRACK_OUTCHAIN;
	} else if (sender == &self->level) {
		mode = PARAMRACK_LEVEL;
	}
	paramrackmodebar_setmode(self, mode);
}

void paramrackmodebar_setmode(ParamRackModeBar* self, ParamRackMode mode)
{
	psy_ui_button_disable_highlight(&self->all);
	psy_ui_button_disable_highlight(&self->inputs);
	psy_ui_button_disable_highlight(&self->outputs);
	psy_ui_button_disable_highlight(&self->inchain);
	psy_ui_button_disable_highlight(&self->outchain);
	psy_ui_button_disable_highlight(&self->level);
	switch (mode) {
	case PARAMRACK_ALL:
		psy_ui_button_highlight(&self->all);
		break;
	case PARAMRACK_INPUTS:
		psy_ui_button_highlight(&self->inputs);
		break;
	case PARAMRACK_OUTPUTS:
		psy_ui_button_highlight(&self->outputs);
		break;
	case PARAMRACK_INCHAIN:
		psy_ui_button_highlight(&self->inchain);
		break;
	case PARAMRACK_OUTCHAIN:
		psy_ui_button_highlight(&self->outchain);
		break;
	case PARAMRACK_LEVEL:
		psy_ui_button_highlight(&self->level);
		break;
	default:
		break;
	}
	if (mode != PARAMRACK_NONE) {
		psy_signal_emit(&self->signal_select, self, 1, mode);
	}
}

/* ParamRack */
/* prototypes */
static void paramrack_onmodeselected(ParamRack*, ParamRackModeBar* sender,
	intptr_t index);
static void paramrack_onalign(ParamRack*, psy_ui_Component* sender);
static void paramrack_onlevelchanged(ParamRack*, IntEdit* sender);
static void paramrack_onselect(ParamRack*, psy_ui_Button* sender);
static void paramrack_onmachineselected(ParamRack*,
	psy_audio_Machines*, uintptr_t slot);
static void paramrack_onsongchanged(ParamRack*, Workspace* sender);
static void paramrack_connectsong(ParamRack*);

/* implementation */
void paramrack_init(ParamRack* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(0.0, 10.0));		
	self->workspace = workspace;	
	// Bottom
	psy_ui_component_init_align(&self->bottom, &self->component, NULL,
		psy_ui_ALIGN_BOTTOM);	
	// IntEdit
	intedit_init(&self->leveledit, &self->bottom, "", 2, 0, INT32_MAX);
	psy_ui_component_set_align(&self->leveledit.component, psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->leveledit.signal_changed, self, paramrack_onlevelchanged);
	// ChainMode
	paramrackmodebar_init(&self->modebar, &self->bottom);		
	psy_ui_component_set_align(&self->modebar.component,
		psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->modebar.signal_select, self,
		paramrack_onmodeselected);
	// BatchBar
	paramrackbatchbar_init(&self->batchbar, &self->bottom);
	psy_ui_component_set_align(&self->batchbar.component, psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->batchbar.select.signal_clicked, self,
		paramrack_onselect);
	/* Pane */
	paramrackpane_init(&self->pane, &self->component, workspace);	
	psy_ui_component_set_overflow(&self->pane.component,
		psy_ui_OVERFLOW_HSCROLL);	
	/* connect scrollbar */
	psy_ui_scroller_init(&self->scroller, &self->component, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->pane.component);
	psy_ui_component_set_align(&self->pane.component,
		psy_ui_ALIGN_VCLIENT);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setbackgroundmode(&self->scroller.pane, psy_ui_SETBACKGROUND);
	psy_signal_connect(&self->component.signal_align, self,
		paramrack_onalign);		
	psy_signal_connect(&workspace->signal_songchanged, self,
		paramrack_onsongchanged);
	paramrack_connectsong(self);
	paramrackmodebar_setmode(&self->modebar, self->pane.mode);	
}

void paramrack_onmodeselected(ParamRack* self, ParamRackModeBar* sender,
	intptr_t index)
{			
	paramrackpane_setmode(&self->pane, (ParamRackMode)index);
	psy_ui_component_show_align(&self->pane.component);	
}

void paramrack_onalign(ParamRack* self, psy_ui_Component* sender)
{
	if (psy_ui_component_at(&self->pane.component, 0)) {		
		psy_ui_Size limit;
		psy_ui_Size preferredboxsize;

		// update scroll step
		limit = psy_ui_component_scrollsize(&self->component);
		preferredboxsize = psy_ui_component_preferredsize(
			psy_ui_component_at(&self->pane.component, 0), &limit);
		psy_ui_component_set_scroll_step_width(&self->pane.component,
			preferredboxsize.width);
	}	
}

void paramrack_onlevelchanged(ParamRack* self, IntEdit* sender)
{
	if (self->pane.mode == PARAMRACK_LEVEL) {
		self->pane.level = intedit_value(&self->leveledit);
		paramrackpane_build(&self->pane);
	}
}

void paramrack_onselect(ParamRack* self, psy_ui_Button* sender)
{
	psy_TableIterator it;
	psy_List* slotlist;	

	if (!workspace_gear_visible(self->workspace)) {
		workspace_toggle_gear(self->workspace);
	}
	slotlist = NULL;		
	for (it = psy_table_begin(&self->pane.boxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		ParamRackBox* box;

		box = (ParamRackBox*)psy_tableiterator_value(&it);
		psy_list_append(&slotlist, (void*)box->slot);
	}
	workspace_multi_select_gear(self->workspace, slotlist);
	psy_list_free(slotlist);	
}

void paramrack_onsongchanged(ParamRack* self, Workspace* sender)
{		
	paramrack_connectsong(self);		
}

void paramrack_connectsong(ParamRack* self)
{
	if (workspace_song(self->workspace)) {
		psy_audio_Machines* machines;

		machines = &self->workspace->song->machines;
		psy_signal_connect(&machines->signal_slotchange, self,
			paramrack_onmachineselected);
	}
}

void paramrack_onmachineselected(ParamRack* self,
	psy_audio_Machines* sender, uintptr_t slot)
{	
	if (self->pane.mode == PARAMRACK_LEVEL && self->pane.lastselected != slot) {
		uintptr_t level;

		level = psy_audio_machines_levelofmachine(sender, slot);
		if (level != psy_INDEX_INVALID) {			
			intedit_setvalue(&self->leveledit, (int32_t)level);
		}
	}
}
