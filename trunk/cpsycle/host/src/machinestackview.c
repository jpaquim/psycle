// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinestackview.h"
// host
#include "skingraphics.h"
#include "paramview.h"
#include "sliderui.h"
#include "wireview.h"
#include "machinecombobox.h"
// audio
#include <exclusivelock.h>
// ui
#include <uicombobox.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// MachineStackColumn
void machinestackcolumn_init(MachineStackColumn* self, uintptr_t column)
{
	self->column = column;
	self->chain = NULL;
	self->wirevolume = NULL;
}

void machinestackcolumn_dispose(MachineStackColumn* self)
{
	if (self->wirevolume) {
	 	psy_audio_wiremachineparam_dispose(self->wirevolume);
		free(self->wirevolume);
		self->wirevolume = NULL;
	}
}

void machinestackcolumn_setwire(MachineStackColumn* self, psy_audio_Wire wire,
	psy_audio_Machines* machines)
{
	if (self->wirevolume) {
		psy_audio_wiremachineparam_dispose(self->wirevolume);
		free(self->wirevolume);
		self->wirevolume = NULL;
	}		
	self->wirevolume = (psy_audio_WireMachineParam*)
		malloc(sizeof(psy_audio_WireMachineParam));
	psy_audio_wiremachineparam_init(self->wirevolume, wire, machines);
}

psy_audio_WireMachineParam* machinestackcolumn_wire(MachineStackColumn* self)
{
	return self->wirevolume;
}

void machinestackcolumn_append(MachineStackColumn* self, uintptr_t macid)
{
	psy_list_append(&self->chain, (void*)macid);
}

// MachineStackState
void machinestackstate_init(MachineStackState* self)
{
	psy_table_init(&self->columns);
	self->machines = NULL;	
	self->selected = psy_INDEX_INVALID;
	self->size = psy_ui_size_makepx(160.0, 70.0);
}

void machinestackstate_dispose(MachineStackState* self)
{
	psy_table_disposeall(&self->columns, (psy_fp_disposefunc)
		machinestackcolumn_dispose);
}

void machinestackstate_clear(MachineStackState* self)
{
	psy_table_disposeall(&self->columns, (psy_fp_disposefunc)
		machinestackcolumn_dispose);
	psy_table_init(&self->columns);
}

MachineStackColumn* machinestackstate_insertcolumn(MachineStackState* self,
	uintptr_t columnindex)
{
	MachineStackColumn* column;
	
	column = (MachineStackColumn*)psy_table_at(&self->columns, columnindex);
	if (column) {
		machinestackcolumn_dispose(column);
		free(column);
	}
	column = (MachineStackColumn*)malloc(sizeof(MachineStackColumn));
	machinestackcolumn_init(column, columnindex);
	psy_table_insert(&self->columns, columnindex, column);
	return column;
}

MachineStackColumn* machinestackstate_column(MachineStackState* self,
	uintptr_t column)
{
	return (MachineStackColumn*)psy_table_at(&self->columns, column);
}

psy_List* machinestackstate_inputs(MachineStackState* self)
{
	if (self->machines) {
		psy_TableIterator it;
		psy_List* rv;

		assert(self);

		rv = NULL;
		for (it = psy_audio_machines_begin(self->machines);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
			if (psy_audio_machine_numinputs(machine) == 0) {
				psy_list_append(&rv, (void*)psy_tableiterator_key(&it));
			}
		}
		return rv;
	}
	return NULL;
}

// MachineStackLabels
// prototypes
static void machinestackdesc_onalign(MachineStackDesc* self);
static void machinestackdesc_onpreferredsize(MachineStackDesc*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable machinestackdesc_vtable;
static bool machinestackdesc_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackdesc_vtable_init(MachineStackDesc* self)
{
	if (!machinestackdesc_vtable_initialized) {
		machinestackdesc_vtable = *(self->component.vtable);
		machinestackdesc_vtable.onalign = (psy_ui_fp_component_onalign)
			machinestackdesc_onalign;
		machinestackdesc_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			machinestackdesc_onpreferredsize;
		machinestackdesc_vtable_initialized = TRUE;
	}
	return &machinestackdesc_vtable;
}

void machinestackdesc_init(MachineStackDesc* self, psy_ui_Component* parent,
	MachineStackView* view)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setvtable(&self->component,
		machinestackdesc_vtable_init(self));
	self->view = view;
	psy_ui_label_init_text(&self->inputs, &self->component, "Inputs");	
	psy_ui_label_init_text(&self->effects, &self->component, "Effects");	
	psy_ui_label_init_text(&self->outputs, &self->component, "Outputs");
	psy_ui_label_init_text(&self->volumes, &self->component, "Volumes");
	psy_ui_component_setcolour(&self->component,
		psy_ui_colour_make(0x00AABBBB));
}

void machinestackdesc_onalign(MachineStackDesc* self)
{	
	psy_ui_Size size;
	psy_ui_RealSize sizepx;
	psy_ui_Size insize;
	psy_ui_RealSize insizepx;
	psy_ui_Size effectsize;
	psy_ui_RealSize effectsizepx;
	psy_ui_Size outsize;
	psy_ui_RealSize outsizepx;
	psy_ui_Size volumesize;
	psy_ui_RealSize volumesizepx;
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size(&self->component);
	sizepx = psy_ui_component_sizepx(&self->component);
	insize = psy_ui_component_preferredsize(&self->view->inputs.component,
		NULL);
	insizepx = psy_ui_size_px(&insize, tm);
	effectsize = psy_ui_component_preferredsize(&self->view->pane.component,
		NULL);
	effectsizepx = psy_ui_size_px(&effectsize, tm);
	outsize = psy_ui_component_preferredsize(&self->view->outputs.component,
		NULL);
	outsizepx = psy_ui_size_px(&outsize, tm);
	volumesize = psy_ui_component_preferredsize(&self->view->volumes.component,
		NULL);
	volumesizepx = psy_ui_size_px(&volumesize, tm);
	volumesizepx.height = psy_max(200.0, volumesizepx.height);
	psy_ui_component_setposition(&self->inputs.component,
		psy_ui_rectangle_make(
			psy_ui_point_makepx(0.0, 0.0),
			psy_ui_size_make(size.width, psy_ui_value_makeeh(1.0))));
	psy_ui_component_setposition(&self->effects.component,
		psy_ui_rectangle_make(
			psy_ui_point_makepx(0.0, insizepx.height),
			psy_ui_size_make(size.width, psy_ui_value_makeeh(1.0))));
	psy_ui_component_setposition(&self->outputs.component,
		psy_ui_rectangle_make(
			psy_ui_point_makepx(0.0, sizepx.height - volumesizepx.height -
				outsizepx.height),
			psy_ui_size_make(size.width, psy_ui_value_makeeh(1.0))));
	psy_ui_component_setposition(&self->volumes.component,
		psy_ui_rectangle_make(
			psy_ui_point_makepx(0.0, sizepx.height - volumesizepx.height),
			psy_ui_size_make(size.width, psy_ui_value_makeeh(1.0))));
}

void machinestackdesc_onpreferredsize(MachineStackDesc* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_size_setem(rv, 10.0, 10.0);
}

// MachineStackInputs
// prototypes
static void machinestackinputs_onpreferredsize(MachineStackInputs*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void machinestackinputs_buildmachineuis(MachineStackInputs*);
static void machinestackinputs_ondraw(MachineStackInputs*, psy_ui_Graphics*);
static void machinestackinputs_onmousedoubleclick(MachineStackInputs*,
	psy_ui_MouseEvent * ev);
// vtable
static psy_ui_ComponentVtable machinestackinputs_vtable;
static bool machinestackinputs_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackinputs_vtable_init(MachineStackInputs* self)
{
	if (!machinestackinputs_vtable_initialized) {
		machinestackinputs_vtable = *(self->component.vtable);		
		machinestackinputs_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			machinestackinputs_onpreferredsize;
		machinestackinputs_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			machinestackinputs_onmousedoubleclick;
		machinestackinputs_vtable_initialized = TRUE;
	}
	return &machinestackinputs_vtable;
}
// implementation
void machinestackinputs_init(MachineStackInputs* self,
	psy_ui_Component* parent, psy_audio_Machines* machines,
	MachineStackState* state, MachineViewSkin* skin, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_setvtable(&self->component,
		machinestackinputs_vtable_init(self));
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	self->machines = machines;
	self->skin = skin;
	self->workspace = workspace;
	self->state = state;
}

void machinestackinputs_onpreferredsize(MachineStackInputs* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	*rv = self->state->size;
	psy_ui_value_mul_real(&rv->width, 10.0);
}

void machinestackinputs_setmachines(MachineStackInputs* self,
	psy_audio_Machines* machines)	
{
	self->machines = machines;
	machinestackinputs_buildmachineuis(self);
}

void machinestackinputs_buildmachineuis(MachineStackInputs* self)
{
	psy_ui_component_clear(&self->component);	
	if (self->machines) {		
		uintptr_t i;

		for (i = 0; psy_table_size(&self->state->columns) > 0 &&
			i <= psy_table_maxkey(&self->state->columns); ++i) {
			MachineStackColumn* column;
			uintptr_t inputmacid;

			column = psy_table_at(&self->state->columns, i);
			inputmacid = psy_INDEX_INVALID;
			if (column && column->chain) {
				inputmacid = (uintptr_t)psy_list_entry(column->chain);
				if (inputmacid >= 0x40) {
					inputmacid = psy_INDEX_INVALID;
				}
			}
			if (inputmacid != psy_INDEX_INVALID) {
				psy_ui_Component* rv;

				rv = machineui_create(
					psy_audio_machines_at(self->machines, inputmacid),
					inputmacid, self->skin, &self->component, &self->component, NULL,
					FALSE, self->workspace);
				psy_ui_component_setminimumsize(rv, self->state->size);
				psy_ui_component_setalign(rv, psy_ui_ALIGN_LEFT);
			} else {
				psy_ui_Component* spacer;

				spacer = (psy_ui_Component*)malloc(sizeof(psy_ui_Component));
				psy_ui_component_init(spacer, &self->component, &self->component);				
				spacer->deallocate = TRUE;				
				psy_ui_component_setminimumsize(spacer, self->state->size);
				psy_ui_component_setalign(spacer, psy_ui_ALIGN_LEFT);
			}
		}		
	}
	psy_ui_component_align(&self->component);
}

void machinestackinputs_onmousedoubleclick(MachineStackInputs* self,
	psy_ui_MouseEvent* ev)
{
	workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
		SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_APPENDSTACK);
	psy_ui_mouseevent_stoppropagation(ev);
}

void machinestackinputs_updatevus(MachineStackInputs* self)
{	
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	machineui_beginvuupdate();
	psy_ui_component_invalidate(&self->component);
	machineui_endvuupdate();
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_SET);
	psy_ui_component_update(&self->component);
}

// MachineStackOutputs
// prototypes
static void machinestackoutputs_onpreferredsize(MachineStackOutputs*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void machinestackoutputs_buildmachineuis(MachineStackOutputs*);
static void machinestackoutputs_ondraw(MachineStackOutputs*, psy_ui_Graphics*);
// vtable
static psy_ui_ComponentVtable machinestackoutputs_vtable;
static bool machinestackoutputs_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackoutputs_vtable_init(MachineStackOutputs* self)
{
	if (!machinestackoutputs_vtable_initialized) {
		machinestackoutputs_vtable = *(self->component.vtable);
		machinestackoutputs_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			machinestackoutputs_onpreferredsize;		
		machinestackoutputs_vtable_initialized = TRUE;
	}
	return &machinestackoutputs_vtable;
}
// implementation
void machinestackoutputs_init(MachineStackOutputs* self,
	psy_ui_Component* parent, psy_audio_Machines* machines,
	MachineStackState* state, MachineViewSkin* skin, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setvtable(&self->component,
		machinestackoutputs_vtable_init(self));
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	self->machines = machines;
	self->skin = skin;
	self->workspace = workspace;
	self->state = state;
}

void machinestackoutputs_onpreferredsize(MachineStackOutputs* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	*rv = self->state->size;
	psy_ui_value_mul_real(&rv->width, 10.0);	
}

void machinestackoutputs_setmachines(MachineStackOutputs* self,
	psy_audio_Machines* machines)
{
	self->machines = machines;
	machinestackoutputs_buildmachineuis(self);
}

void machinestackoutputs_buildmachineuis(MachineStackOutputs* self)
{		
	psy_ui_component_clear(&self->component);
	if (self->machines) {							
		uintptr_t i;		

		for (i = 0; psy_table_size(&self->state->columns) > 0 &&
				i <= psy_table_maxkey(&self->state->columns); ++i) {
			MachineStackColumn* column;
			MachineComboBox* rv;

			column = psy_table_at(&self->state->columns, i);
			rv = (MachineComboBox*)malloc(sizeof(MachineComboBox));
			if (rv) {
				machinecombobox_init(rv, &self->component, TRUE, self->workspace);
				rv->sync_machines_select = FALSE;
				machinecombobox_base(rv)->deallocate = TRUE;
				psy_ui_component_setminimumsize(machinecombobox_base(rv),
					psy_ui_size_make(
						self->state->size.width,
						psy_ui_value_makeeh(2.0)));
				psy_ui_component_setalign(machinecombobox_base(rv), psy_ui_ALIGN_LEFT);
				if (column && column->chain) {
					psy_List* p;
					uintptr_t slot;					

					p = psy_list_last(column->chain);
					slot = (uintptr_t)psy_list_entry(p);
					machinecombobox_select(rv, slot);
				}
			}
			/*if (column && column->chain) {
				psy_List* p;
				uintptr_t slot;
				psy_ui_Component* rv;

				p = psy_list_last(column->chain);
				slot = (uintptr_t)psy_list_entry(p);
				rv = machineui_create(
					psy_audio_machines_at(self->machines, slot),
					slot, self->skin, &self->component, &self->component, NULL,
					FALSE, self->workspace);
				psy_ui_component_setalign(rv, psy_ui_ALIGN_LEFT);
				psy_ui_component_setminimumsize(rv, self->state->size);
			} else {
				psy_ui_Component* spacer;

				spacer = (psy_ui_Component*)malloc(sizeof(psy_ui_Component));
				psy_ui_component_init(spacer, &self->component, &self->component);				
				spacer->deallocate = TRUE;
				psy_ui_component_preventalign(spacer);				
				psy_ui_component_setminimumsize(spacer, self->state->size);
				psy_ui_component_setalign(spacer, psy_ui_ALIGN_LEFT);
			}*/
		}		
	}
	psy_ui_component_align(&self->component);
}

// MachineStackPane
// prototypes
static void machinestackpane_ondestroy(MachineStackPane*);
static void machinestackpane_buildcolumns(MachineStackPane*);
static void machinestackpane_buildmachineuis(MachineStackPane*);
uintptr_t machinestackpane_buildcolumnoutchain(MachineStackPane*, uintptr_t slot,
	uintptr_t currcolumn);
static void machinestackpane_removeall(MachineStackPane*);
static void machinestackpane_onsongchanged(MachineStackPane*, Workspace*,
	int flag, psy_audio_Song*);
static void machinestackpane_setmachines(MachineStackPane*,
	psy_audio_Machines*);
static psy_ui_Component* machinestackpane_insert(MachineStackPane*, uintptr_t slot,
	psy_ui_Component* trackpane);
static psy_ui_Component* machinestackpane_at(MachineStackPane*, uintptr_t slot);
static void machinestackpane_remove(MachineStackPane*, uintptr_t slot);
static void machinestackpane_onmachineinsert(MachineStackPane*,
	psy_audio_Machines*, uintptr_t slot);
static void machinestackpane_onmachineremoved(MachineStackPane*,
	psy_audio_Machines*, uintptr_t slot);
static void machinestackpane_onconnected(MachineStackPane*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machinestackpane_ondisconnected(MachineStackPane*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
// vtable
static psy_ui_ComponentVtable machinestackpane_vtable;
static bool machinestackpane_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackpane_vtable_init(MachineStackPane* self)
{
	if (!machinestackpane_vtable_initialized) {
		machinestackpane_vtable = *(self->component.vtable);
		machinestackpane_vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			machinestackpane_ondestroy;		
		machinestackpane_vtable_initialized = TRUE;
	}
	return &machinestackpane_vtable;
}

void machinestackpane_init(MachineStackPane* self, psy_ui_Component* parent,
	MachineStackState* state, MachineViewSkin* skin,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setvtable(&self->component,
		machinestackpane_vtable_init(self));
	self->component.vtable = &machinestackpane_vtable;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);	
	self->workspace = workspace;
	self->skin = skin;
	self->state = state;	
	self->vudrawupdate = FALSE;
	self->opcount = 0;
	psy_signal_init(&self->signal_changed);
	machinestackpane_updateskin(self);	
	if (workspace_song(workspace)) {
		machinestackpane_setmachines(self,
			psy_audio_song_machines(workspace_song(workspace)));
	} else {
		machinestackpane_setmachines(self, NULL);
	}	
	machinestackpane_buildmachineuis(self);
	psy_signal_connect(&workspace->signal_songchanged, self,
		machinestackpane_onsongchanged);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void machinestackpane_ondestroy(MachineStackPane* self)
{
	psy_signal_dispose(&self->signal_changed);
}

void machinestackpane_buildcolumns(MachineStackPane* self)
{
	if (self->machines) {
		MachineList* inputs;
		MachineList* p;
		uintptr_t columnindex;		
		uintptr_t i;

		self->state->machines = self->machines;		
		machinestackstate_clear(self->state);
		inputs = machinestackstate_inputs(self->state);
		if (inputs == 0) {
			machinestackpane_buildcolumnoutchain(self, psy_INDEX_INVALID, 0);
		} else {
			columnindex = 0;
			for (p = inputs; p != NULL; psy_list_next(&p)) {
				columnindex = machinestackpane_buildcolumnoutchain(self, (uintptr_t)(p->entry),
					columnindex) + 1;
			}
			psy_list_free(inputs);
			psy_ui_component_clear(&self->component);
			for (i = 0; psy_table_size(&self->state->columns) > 0 &&
				i <= psy_table_maxkey(&self->state->columns); ++i) {
				MachineStackColumn* column;
				psy_audio_Wire wire;

				column = psy_table_at(&self->state->columns, i);
				if (column) {
					psy_List* lastnode;

					lastnode = psy_list_last(column->chain);
					if (lastnode && lastnode->prev) {
						uintptr_t src;
						uintptr_t dst;

						src = (uintptr_t)psy_list_entry(lastnode->prev);
						dst = (uintptr_t)psy_list_entry(lastnode);
						wire = psy_audio_wire_make(src, dst);
						machinestackcolumn_setwire(column, wire, self->machines);
					}
				}
			}
		}
	}
}

uintptr_t machinestackpane_buildcolumnoutchain(MachineStackPane* self, uintptr_t slot,
	uintptr_t columnindex)
{
	uintptr_t rv;
	MachineStackColumn* column;
	psy_audio_MachineSockets* sockets;
	
	rv = columnindex;		
	column = machinestackstate_column(self->state, rv);
	if (!machinestackstate_column(self->state, rv)) {
		column = machinestackstate_insertcolumn(self->state, rv);
	}	
	if (slot == psy_INDEX_INVALID) {
		return columnindex;
	}
	machinestackcolumn_append(column, slot);
	sockets = psy_audio_connections_at(&self->machines->connections, slot);
	if (sockets) {		
		uintptr_t i;
		uintptr_t c;

		for (c = 0, i = 0; psy_table_size(&sockets->outputs.sockets) > 0 &&
				i <= psy_table_maxkey(&sockets->outputs.sockets); ++i) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_audio_wiresockets_at(&sockets->outputs, i);
			if (socket && socket->slot != psy_INDEX_INVALID) {				
				rv = machinestackpane_buildcolumnoutchain(self, socket->slot, rv + c);
				++c;
			}
		}
	}
	return rv;
}

void machinestackpane_buildmachineuis(MachineStackPane* self)
{
	if (self->machines) {
		uintptr_t i;
		
		machinestackpane_buildcolumns(self);		
		psy_ui_component_clear(&self->component);		
		for (i = 0; psy_table_size(&self->state->columns) > 0 &&
				i <= psy_table_maxkey(&self->state->columns); ++i) {
			MachineStackPaneTrack* trackpane;

			trackpane = (MachineStackPaneTrack*)malloc(sizeof(MachineStackPaneTrack));
			if (trackpane) {
				MachineStackColumn* column;

				machinestackpanetrack_init(trackpane, &self->component, i, &self->component,
					self->state, self->workspace);
				trackpane->component.deallocate = TRUE;				
				psy_ui_component_setalign(&trackpane->component, psy_ui_ALIGN_LEFT);
				column = psy_table_at(&self->state->columns, i);
				if (column) {
					psy_List* p;
					bool insert;

					insert = FALSE;
					for (p = column->chain; p != NULL; psy_list_next(&p)) {
						uintptr_t slot;

						slot = (uintptr_t)psy_list_entry(p);
						if (slot >= 0x40 && slot < 0x80) {
							machinestackpane_insert(self, slot, &trackpane->component);
							insert = TRUE;
						}
					}
					if (!insert) {
						//psy_ui_component_preventalign(&trackpane->component);
						psy_ui_component_setminimumsize(&trackpane->component,
							self->state->size);
						psy_ui_component_setalign(&trackpane->component,
							psy_ui_ALIGN_LEFT);
					}
				} else {
					psy_ui_component_preventalign(&trackpane->component);
					psy_ui_component_setminimumsize(&trackpane->component,
						self->state->size);
					psy_ui_component_setalign(&trackpane->component,
						psy_ui_ALIGN_LEFT);
				}
				psy_ui_component_align(&trackpane->component);
			}
		}		
		psy_ui_component_align(&self->component);
		psy_signal_emit(&self->signal_changed, self, 0);
	} 
}

psy_ui_Component* machinestackpane_insert(MachineStackPane* self, uintptr_t slot,
	psy_ui_Component* trackpane)
{	
	psy_ui_Component* rv;

	rv =  machineui_create(
		psy_audio_machines_at(self->machines, slot),
		slot, self->skin, trackpane,
		&self->component, NULL,
		FALSE, self->workspace);	
	if (rv) {
		psy_ui_component_setminimumsize(rv, self->state->size);
		rv->deallocate = TRUE;
		psy_ui_component_setalign(rv, psy_ui_ALIGN_TOP);
		return rv;
	}	
	return NULL;
}

psy_ui_Component* machinestackpane_at(MachineStackPane* self, uintptr_t slot)
{
	return 0; // psy_list_at(&self->machineuis, slot);
}

void machinestackpane_remove(MachineStackPane* self, uintptr_t slot)
{
	//MachineUi* machineui;

	//machineui = (MachineUi*)psy_table_at(&self->machineuis, slot);
	//if (machineui) {
	//	machineui_dispose(machineui);
	//	free(machineui);
	//	psy_table_remove(&self->machineuis, slot);
	//}
}

void machinestackpane_removeall(MachineStackPane* self)
{
	
}

void machinestackpane_onsongchanged(MachineStackPane* self, Workspace* workspace,
	int flag, psy_audio_Song* song)
{
	if (song) {
		machinestackpane_setmachines(self, psy_audio_song_machines(song));
	} else {
		machinestackpane_setmachines(self, NULL);
	}
	machinestackpane_buildmachineuis(self);
}

void machinestackpane_setmachines(MachineStackPane* self,
	psy_audio_Machines* machines)
{	
	self->machines = machines;	
	if (self->machines) {
		// psy_signal_connect(&self->machines->signal_slotchange, self,
		//	machinewireview_onmachineselected);		
		psy_signal_connect(&self->machines->signal_insert, self,
			machinestackpane_onmachineinsert);
		psy_signal_connect(&self->machines->signal_removed, self,
			machinestackpane_onmachineremoved);
		psy_signal_connect(&self->machines->connections.signal_connected, self,
			machinestackpane_onconnected);
		psy_signal_connect(&self->machines->connections.signal_disconnected, self,
			machinestackpane_ondisconnected);
		//psy_signal_connect(
			//&psy_audio_machines_master(self->machines)->signal_worked, self,
			//machinewireview_onmasterworked);
	}
}

void machinestackpane_onmachineinsert(MachineStackPane* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	machinestackpane_buildmachineuis(self);
}

void machinestackpane_onmachineremoved(MachineStackPane* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	machinestackpane_buildmachineuis(self);
}

void machinestackpane_onconnected(MachineStackPane* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{
	machinestackpane_buildmachineuis(self);
}

void machinestackpane_ondisconnected(MachineStackPane* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{
	machinestackpane_buildmachineuis(self);
}

void machinestackpane_updateskin(MachineStackPane* self)
{
	psy_ui_component_setbackgroundcolour(&self->component,
		self->skin->colour);
}

void machinestackpane_updatevus(MachineStackPane* self)
{						
	psy_List* p;
	psy_List* q;
	
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	q = psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE);
	for (p = q; p != NULL; psy_list_next(&p)) {
		psy_ui_Component* trackpane;
		
		trackpane = (psy_ui_Component*)psy_list_entry(p);
		psy_ui_component_setbackgroundmode(trackpane,
			psy_ui_BACKGROUND_NONE);
		machineui_beginvuupdate();
		psy_ui_component_invalidate(trackpane);
		machineui_endvuupdate();
		psy_ui_component_setbackgroundmode(trackpane,
			psy_ui_BACKGROUND_SET);
	}
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_SET);
	psy_ui_component_update(&self->component);	
}

// MachineStackVolumes
// prototypes
void machinestackvolumes_init(MachineStackVolumes* self, psy_ui_Component* parent,
	MachineStackState* state, MachineViewSkin* skin, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_doublebuffer(&self->component);
	self->state = state;
	self->workspace = workspace;
	self->skin = skin;
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);	
}

void machinestackvolumes_build(MachineStackVolumes* self)
{
	uintptr_t i;

	assert(self);

	self->state->machines = (self->workspace->song)
		? &self->workspace->song->machines
		: NULL;
	psy_ui_component_clear(&self->component);
	for (i = 0; i < psy_table_size(&self->state->columns); ++i) {
		MachineStackColumn* column;		
		SliderUi* slider;

		column = (MachineStackColumn*)psy_table_at(&self->state->columns, i);
		if (column && machinestackcolumn_wire(column)) {
			slider = (SliderUi*)malloc(sizeof(SliderUi));
			if (slider) {
				sliderui_init(slider, &self->component, &self->component,
					&column->wirevolume->machineparam, self->workspace);
				slider->component.deallocate = TRUE;
				psy_ui_component_setalign(&slider->component, psy_ui_ALIGN_LEFT);
				psy_ui_component_setminimumsize(&slider->component,
					self->state->size);
			}
		} else {
			psy_ui_Component* spacer;

			spacer = (psy_ui_Component*)malloc(sizeof(psy_ui_Component));
			psy_ui_component_init(spacer, &self->component, &self->component);			
			spacer->deallocate = TRUE;
			psy_ui_component_preventalign(spacer);
			psy_ui_component_setminimumsize(spacer, self->state->size);			
			psy_ui_component_setalign(spacer, psy_ui_ALIGN_LEFT);			
		}
	}
	psy_ui_component_align(&self->component);
}

// machinestackpanetrack
static void machinestackpanetrack_onmousedoubleclick(MachineStackPaneTrack*,
	psy_ui_MouseEvent* ev);
// vtable
static psy_ui_ComponentVtable machinestackpanetrack_vtable;
static bool machinestackpanetrack_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackpanetrack_vtable_init(MachineStackPaneTrack* self)
{
	if (!machinestackpanetrack_vtable_initialized) {
		machinestackpanetrack_vtable = *(self->component.vtable);		
		machinestackpanetrack_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			machinestackpanetrack_onmousedoubleclick;
		machinestackpanetrack_vtable_initialized = TRUE;
	}
	return &machinestackpanetrack_vtable;
}

void machinestackpanetrack_init(MachineStackPaneTrack* self,
	psy_ui_Component* parent, uintptr_t column, psy_ui_Component* view,
	MachineStackState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, view);
	machinestackpanetrack_vtable_init(self);
	self->component.vtable = &machinestackpanetrack_vtable;
	self->component.debugflag = 8000;
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	//psy_ui_scroller_init(&self->scroller, NULL, &self->component, view);
	//psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	//psy_ui_component_init(&self->client, &self->component, view);
	//psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	///psy_ui_component_setalignexpand(&self->client, psy_ui_HORIZONTALEXPAND);
	
	//psy_ui_component_setoverflow(&self->client, psy_ui_OVERFLOW_VSCROLL);	
	//self->scroller.client = &self->client;
	//psy_ui_scroller_connectclient(&self->scroller);
	
	
	
	//psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	self->column = column;
	self->state = state;
	self->workspace = workspace;
}

void machinestackpanetrack_onmousedoubleclick(MachineStackPaneTrack* self,
	psy_ui_MouseEvent* ev)
{
	self->state->selected = self->column;
	workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
		SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_ADDEFFECTSTACK);
	psy_ui_mouseevent_stoppropagation(ev);
}

// MachineStackView
// prototypes
static void machinestackview_destroy(MachineStackView*);
static void machinestackview_onsongchanged(MachineStackView*, Workspace*,
	int flag, psy_audio_Song*);
static void machinestackview_onpanechanged(MachineStackView*, MachineStackPane*);
static void machinestackview_onmachineinsert(MachineStackView*,
	psy_audio_Machines*, uintptr_t slot);
static void machinestackview_onmachineremoved(MachineStackView*,
	psy_audio_Machines*, uintptr_t slot);
static void machinestackview_onconnected(MachineStackView*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machinestackview_ondisconnected(MachineStackView*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machinestackview_setmachines(MachineStackView*,
	psy_audio_Machines*);
static void machinestackview_ontimer(MachineStackView*, uintptr_t timerid);

// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(MachineStackView* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);		
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			machinestackview_destroy;
		vtable.ontimer = (psy_ui_fp_component_ontimer)
			machinestackview_ontimer;
		vtable_initialized = TRUE;
	}
	return &vtable;
}
// implementation
void machinestackview_init(MachineStackView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent,
	MachineViewSkin* skin, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->workspace = workspace;
	machinestackstate_init(&self->state);
	machinestackdesc_init(&self->desc, &self->component, self);
	psy_ui_component_setalign(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_init(&self->columns, &self->component, NULL);
	psy_ui_component_setalign(&self->columns, psy_ui_ALIGN_VCLIENT);
	psy_ui_component_setoverflow(&self->columns, psy_ui_OVERFLOW_HSCROLL);
	psy_ui_component_setmode(&self->columns, psy_ui_SCROLL_COMPONENTS);
	psy_ui_scroller_init(&self->scroller_columns, &self->columns,
		&self->component, NULL);
	psy_ui_component_setalign(&self->columns,
		psy_ui_ALIGN_VCLIENT);
	psy_ui_component_setalign(&self->scroller_columns.component,
		psy_ui_ALIGN_CLIENT);
	machinestackinputs_init(&self->inputs, &self->columns,
		(workspace->song)
		? &workspace->song->machines
		: NULL, &self->state,
		skin, workspace);
	psy_ui_component_setalign(&self->inputs.component, psy_ui_ALIGN_TOP);
	machinestackpane_init(&self->pane, &self->columns, &self->state, skin,
		workspace);	
	psy_ui_component_setalign(&self->pane.component, psy_ui_ALIGN_CLIENT);	
	machinestackvolumes_init(&self->volumes, &self->columns, &self->state, skin,
		workspace);
	psy_ui_component_setalign(&self->volumes.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setminimumsize(&self->volumes.component,
		psy_ui_size_makepx(150.0, 200.0));
	machinestackoutputs_init(&self->outputs, &self->columns,
		(workspace->song)
		? &workspace->song->machines
		: NULL, &self->state,
		skin, workspace);
	psy_ui_component_setalign(&self->outputs.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&workspace->signal_songchanged, self,
		machinestackview_onsongchanged);
	psy_signal_connect(&self->pane.signal_changed, self,
		machinestackview_onpanechanged);
	machinestackview_setmachines(self, (workspace->song) 
		? &workspace->song->machines : NULL);	
	psy_ui_component_setfont(&self->desc.component, &skin->font);
	psy_ui_component_setfont(&self->inputs.component, &skin->font);
	psy_ui_component_setfont(&self->outputs.component, &skin->font);
	psy_ui_component_setfont(&self->pane.component, &skin->font);
	psy_ui_component_starttimer(&self->component, 0, 60);
}

void machinestackview_destroy(MachineStackView* self)
{
	machinestackstate_dispose(&self->state);
}

void machinestackview_onsongchanged(MachineStackView* self,
	Workspace* workspace, int flag, psy_audio_Song* song)
{
	machinestackview_setmachines(self, (song) ? &song->machines : NULL);
}

void machinestackview_setmachines(MachineStackView* self,
	psy_audio_Machines* machines)
{
	if (machines) {
		psy_signal_connect(&machines->signal_insert, self,
			machinestackview_onmachineinsert);
		psy_signal_connect(&machines->signal_removed, self,
			machinestackview_onmachineremoved);
		psy_signal_connect(&machines->connections.signal_connected, self,
			machinestackview_onconnected);
		psy_signal_connect(&machines->connections.signal_disconnected, self,
			machinestackview_ondisconnected);
		machinestackinputs_setmachines(&self->inputs,
			machines);
		machinestackoutputs_setmachines(&self->outputs,
			machines);
		psy_ui_component_align(&self->columns);
	} else {
		machinestackinputs_setmachines(&self->inputs, NULL);
		machinestackoutputs_setmachines(&self->outputs, NULL);
		psy_ui_component_align(&self->desc.component);
	}
}

void machinestackview_onpanechanged(MachineStackView* self,
	MachineStackPane* sender)
{	
	machinestackinputs_buildmachineuis(&self->inputs);
	machinestackoutputs_buildmachineuis(&self->outputs);
	machinestackvolumes_build(&self->volumes);
	psy_ui_component_align(&self->desc.component);
	psy_ui_component_align(&self->columns);
	psy_ui_component_align(&self->component);
}

void machinestackview_onmachineinsert(MachineStackView* self,
	psy_audio_Machines* sender, uintptr_t slot)
{

}

void machinestackview_onmachineremoved(MachineStackView* self,
	psy_audio_Machines* sender, uintptr_t slot)
{

}

void machinestackview_onconnected(MachineStackView* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{

}

void machinestackview_ondisconnected(MachineStackView* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{
	
}

void machinestackview_ontimer(MachineStackView* self, uintptr_t timerid)
{
	if (psy_ui_component_drawvisible(&self->volumes.component)) {
		psy_ui_component_invalidate(&self->volumes.component);
		machinestackinputs_updatevus(&self->inputs);
		machinestackpane_updatevus(&self->pane);
	}
}

void machinestackview_addeffect(MachineStackView* self, psy_Property* plugininfo)
{
	if (self->state.selected != psy_INDEX_INVALID && self->workspace->song) {
		psy_audio_Machine* machine;
		MachineStackColumn* column;

		column = machinestackstate_column(&self->state, self->state.selected);
		if (column) {
			psy_audio_Machines* machines;

			machines = &self->workspace->song->machines;
			machine = psy_audio_machinefactory_makemachinefrompath(
				&self->workspace->machinefactory,
				(psy_audio_MachineType)psy_property_at_int(plugininfo, "type", psy_audio_UNDEFINED),
				psy_property_at_str(plugininfo, "path", ""),
				psy_property_at_int(plugininfo, "shellidx", 0),
				psy_INDEX_INVALID);
			if (machine) {
				intptr_t favorite;
				psy_List* last;

				favorite = psy_property_at_int(plugininfo, "favorite", 0);
				psy_property_set_int(plugininfo, "favorite", ++favorite);
				last = psy_list_last(column->chain);
				if (last) {					
					psy_audio_Machine* lastmachine;
					uintptr_t lastmacid;

					psy_audio_exclusivelock_enter();
					lastmacid = (uintptr_t)psy_list_entry(last);					
					lastmachine = psy_audio_machines_at(machines, lastmacid);
					if (lastmachine && lastmacid != psy_audio_MASTER_INDEX) {
						uintptr_t slot;
						
						slot = psy_audio_machines_append(machines, machine);
						psy_audio_machines_connect(machines,
							psy_audio_wire_make(lastmacid, slot));
						psy_audio_machines_select(machines, slot);						
					} else {
						uintptr_t slot;
						psy_audio_Machine* prevmachine;
						uintptr_t prevmacid;

						if (last->prev) {
							prevmacid = (uintptr_t)psy_list_entry(last->prev);
							prevmachine = psy_audio_machines_at(machines, prevmacid);							
							psy_audio_machines_disconnect(machines,
								psy_audio_wire_make(prevmacid, psy_audio_MASTER_INDEX));
						}						
						slot = psy_audio_machines_append(machines, machine);
						psy_audio_machines_connect(machines,
							psy_audio_wire_make(prevmacid, slot));
						psy_audio_machines_connect(machines,
							psy_audio_wire_make(slot, psy_audio_MASTER_INDEX));						
					}
					psy_audio_exclusivelock_leave();
				}
			}
		}
	}
}
