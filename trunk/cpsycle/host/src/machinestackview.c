// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinestackview.h"
// host
#include "labelui.h"
#include "levelui.h"
#include "sliderui.h"
#include "switchui.h"
// audio
#include <exclusivelock.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// RouteMachineParam
static int masterrouteparam_type(MasterRouteParam* self)
{
	return MPF_SWITCH | MPF_SMALL;
}

static void masterrouteparam_tweak(MasterRouteParam*, float val);
static float masterrouteparam_normvalue(MasterRouteParam*);
static int masterrouteparam_name(MasterRouteParam*, char* text);
// vtable
static MachineParamVtable masterrouteparam_vtable;
static bool masterrouteparam_vtable_initialized = FALSE;

static void masterrouteparam_vtable_init(MasterRouteParam* self)
{
	if (!masterrouteparam_vtable_initialized) {
		masterrouteparam_vtable = *(self->machineparam.vtable);
		masterrouteparam_vtable.normvalue = (fp_machineparam_normvalue)
			masterrouteparam_normvalue;
		masterrouteparam_vtable.tweak = (fp_machineparam_tweak)
			masterrouteparam_tweak;
		masterrouteparam_vtable.type = (fp_machineparam_type)
			masterrouteparam_type;
		masterrouteparam_vtable.name = (fp_machineparam_name)
			masterrouteparam_name;
		masterrouteparam_vtable_initialized = TRUE;
	}
}
// implementation
void masterrouteparam_init(MasterRouteParam* self, psy_audio_Machines* machines)
{
	psy_audio_machineparam_init(&self->machineparam);
	masterrouteparam_vtable_init(self);
	self->machineparam.vtable = &masterrouteparam_vtable;	
	self->macid = psy_INDEX_INVALID;
	self->machines = machines;
}

void masterrouteparam_dispose(MasterRouteParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
}

void masterrouteparam_tweak(MasterRouteParam* self, float val)
{
	if (self->macid != psy_INDEX_INVALID) {
		psy_audio_Wire wire;
		psy_audio_Machines* machines;

		wire = psy_audio_wire_make(self->macid, psy_audio_MASTER_INDEX);
		machines = self->machines;
		psy_audio_exclusivelock_enter();
		if (val == 0.f) {
			psy_audio_machines_disconnect(machines, wire);
		} else {
			psy_audio_machines_connect(machines, wire);
		}
		self->machineparam.crashed = TRUE;
		psy_audio_exclusivelock_leave();
	}
}

float masterrouteparam_normvalue(MasterRouteParam* self)
{		
	if (psy_audio_machines_connected(self->machines,
			psy_audio_wire_make(self->macid, psy_audio_MASTER_INDEX))) {
		return 1.f;
	}
	return 0.f;
}

int masterrouteparam_name(MasterRouteParam* self, char* text)
{
	psy_snprintf(text, 128, "%s", "Master");
	return 1;
}

// MachineStackColumn
// prototypes
static void  machinestackcolumn_level_normvalue(MachineStackColumn*,
	psy_audio_IntMachineParam* sender, float* rv);
// implementation
void machinestackcolumn_init(MachineStackColumn* self, uintptr_t column,
	psy_audio_Machines* machines)
{
	self->column = column;
	self->chain = NULL;
	self->wirevolume = NULL;
	self->machines = machines;
	masterrouteparam_init(&self->masterroute, self->machines);
	psy_audio_intmachineparam_init(&self->level_param,
		"Level", "Level", MPF_SLIDERLEVEL | MPF_SMALL, NULL, 0, 100);
	psy_signal_connect(&self->level_param.machineparam.signal_normvalue, self,
		machinestackcolumn_level_normvalue);
}

void machinestackcolumn_dispose(MachineStackColumn* self)
{
	if (self->wirevolume) {
		psy_audio_wiremachineparam_deallocate(self->wirevolume);
		self->wirevolume = NULL;
	}
	masterrouteparam_dispose(&self->masterroute);
}

void machinestackcolumn_setwire(MachineStackColumn* self, psy_audio_Wire wire)
{
	psy_audio_Machine* machine;
	const char* editname;

	if (self->wirevolume) {
		psy_audio_wiremachineparam_deallocate(self->wirevolume);		
	}	
	machine = psy_audio_machines_at(self->machines, wire.src);
	if (machine) {
		editname = psy_audio_machine_editname(machine);
	} else {
		editname = NULL;
	}
	self->wirevolume = psy_audio_wiremachineparam_allocinit(
		editname, wire, self->machines);
}

void machinestackcolumn_append(MachineStackColumn* self, uintptr_t macid)
{
	psy_list_append(&self->chain, (void*)macid);
	if (macid != psy_audio_MASTER_INDEX) {		
		self->masterroute.macid = macid;		
	}
}

uintptr_t machinestackcolumn_append_effect(MachineStackColumn* self,
	psy_audio_Machine* machine)
{
	uintptr_t mac;
	uintptr_t lastmac;

	mac = psy_audio_machines_append(self->machines, machine);
	lastmac = machinestackcolumn_lastbeforemaster(self);
	if (lastmac != psy_INDEX_INVALID) {
		bool masterconnected;

		masterconnected = machinestackcolumn_connectedtomaster(self);
		psy_audio_exclusivelock_enter();
		if (masterconnected) {
			psy_audio_machines_disconnect(self->machines,
				psy_audio_wire_make(lastmac, psy_audio_MASTER_INDEX));
		}
		psy_audio_machines_connect(self->machines,
			psy_audio_wire_make(lastmac, mac));
		if (masterconnected) {
			psy_audio_machines_connect(self->machines,
				psy_audio_wire_make(mac, psy_audio_MASTER_INDEX));
		}
		psy_audio_exclusivelock_leave();
	}
	return mac;
}

uintptr_t machinestackcolumn_lastbeforemaster(const MachineStackColumn* self)
{	
	if (self->chain) {
		const psy_List* last;

		last = psy_list_last_const(self->chain);
		if (last) {
			uintptr_t macid;

			macid = (uintptr_t)psy_list_entry_const(last);
			if (macid != psy_audio_MASTER_INDEX) {
				return macid;
			}
			if (last->prev) {
				return (uintptr_t)psy_list_entry_const(last->prev);
			}
		}
	}
	return psy_INDEX_INVALID;
}

bool machinestackcolumn_connectedtomaster(const MachineStackColumn* self)
{
	return (psy_audio_machineparam_normvalue(&self->masterroute.machineparam) >
		0.f);
}

void  machinestackcolumn_level_normvalue(MachineStackColumn* self,
	psy_audio_IntMachineParam* sender, float* rv)
{	
	psy_audio_Machine* machine;
	psy_audio_Buffer* memory;

	*rv = 0.f;	
	machine = psy_audio_machines_at(self->machines,
		machinestackcolumn_lastbeforemaster(self));
	if (machine) {
		memory = psy_audio_machine_buffermemory(machine);
		if (memory) {
			psy_dsp_amp_t temp;
			psy_dsp_amp_t volume;

			volume = 1.f;
			temp = ((int)(50.0f * log10((double)volume))) / 97.f;
			*rv = psy_audio_buffer_rmsdisplay(memory) + temp;
		}
	}
}

// MachineStackState
// prototypes
static uintptr_t machinestackstate_buildcolumnoutchain(MachineStackState*,
	uintptr_t slot, uintptr_t columnindex);
// implementation
void machinestackstate_init(MachineStackState* self)
{
	psy_table_init(&self->columns);
	self->machines = NULL;	
	self->selected = psy_INDEX_INVALID;
	self->size = psy_ui_size_makepx(160.0, 70.0);
	self->update = FALSE;
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

void machinestackstate_setmachines(MachineStackState* self,
	psy_audio_Machines* machines)
{
	self->machines = machines;
}

MachineStackColumn* machinestackstate_insertcolumn(MachineStackState* self,
	uintptr_t columnindex)
{
	MachineStackColumn* column;
	
	column = machinestackstate_column(self, columnindex);
	if (column) {
		machinestackcolumn_dispose(column);
		free(column);
	}
	column = (MachineStackColumn*)malloc(sizeof(MachineStackColumn));
	machinestackcolumn_init(column, columnindex, self->machines);
	psy_table_insert(&self->columns, columnindex, column);
	return column;
}

MachineStackColumn* machinestackstate_column(MachineStackState* self,
	uintptr_t column)
{
	return (MachineStackColumn*)psy_table_at(&self->columns, column);
}

MachineStackColumn* machinestackstate_selectedcolumn(MachineStackState* self)
{
	return machinestackstate_column(self, self->selected);
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

void machinestackstate_buildcolumns(MachineStackState* self)
{
	machinestackstate_clear(self);
	if (self->machines) {
		MachineList* inputs;
		MachineList* p;
		uintptr_t columnindex;
		uintptr_t i;
		
		inputs = machinestackstate_inputs(self);
		if (inputs == 0) {
			machinestackstate_buildcolumnoutchain(self, psy_INDEX_INVALID, 0);
		} else {
			columnindex = 0;
			for (p = inputs; p != NULL; psy_list_next(&p)) {
				columnindex = machinestackstate_buildcolumnoutchain(self, (uintptr_t)(p->entry),
					columnindex) + 1;
			}
			psy_list_free(inputs);
			for (i = 0; i < psy_table_maxsize(&self->columns); ++i) {
				MachineStackColumn* column;				

				column = psy_table_at(&self->columns, i);
				if (column) {					
					psy_List* lastnode;

					lastnode = psy_list_last(column->chain);
					if (lastnode && lastnode->prev) {
						uintptr_t src;
						uintptr_t dst;

						src = (uintptr_t)psy_list_entry(lastnode->prev);
						dst = (uintptr_t)psy_list_entry(lastnode);						
						machinestackcolumn_setwire(column,
							psy_audio_wire_make(src, dst));
					}
				}
			}
		}
	}
}

uintptr_t machinestackstate_buildcolumnoutchain(MachineStackState* self, uintptr_t slot,
	uintptr_t columnindex)
{
	uintptr_t rv;
	MachineStackColumn* column;
	psy_audio_MachineSockets* sockets;

	if (slot == psy_INDEX_INVALID) {
		return columnindex;
	}
	rv = columnindex;
	column = machinestackstate_column(self, columnindex);
	if (!machinestackstate_column(self, columnindex)) {
		column = machinestackstate_insertcolumn(self, columnindex);
	}
	machinestackcolumn_append(column, slot);
	sockets = psy_audio_connections_at(&self->machines->connections, slot);
	if (sockets) {
		uintptr_t i;
		uintptr_t c;
		uintptr_t maxnumoutputs;
				
		maxnumoutputs = psy_table_maxsize(&sockets->outputs.sockets);
		for (c = 0, i = 0; i < maxnumoutputs; ++i) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_audio_wiresockets_at(
				&sockets->outputs, i);
			if (socket && socket->slot != psy_INDEX_INVALID) {
				rv = machinestackstate_buildcolumnoutchain(self, socket->slot, rv + c);
				++c;
			}
		}
	}
	return rv;
}

uintptr_t machinestackstate_maxnumcolumns(const MachineStackState* self)
{
	return psy_table_maxsize(&self->columns);
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
	if (psy_ui_component_visible(&self->view->scroller_columns.bottom)) {
		sizepx.height -= tm->tmHeight * 1.5;
	}
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
static void machinestackinputs_ondraw(MachineStackInputs*, psy_ui_Graphics*);
static void machinestackinputs_onmousedoubleclick(MachineStackInputs*,
	psy_ui_MouseEvent* ev);
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
	psy_ui_Component* parent, MachineStackState* state, MachineViewSkin* skin,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_setvtable(&self->component,
		machinestackinputs_vtable_init(self));
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);	
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

void machinestackinputs_build(MachineStackInputs* self)
{
	psy_ui_component_clear(&self->component);	
	if (self->state->machines) {		
		uintptr_t i;
		uintptr_t maxnumcolumns;

		maxnumcolumns = machinestackstate_maxnumcolumns(self->state);
		for (i = 0; i < maxnumcolumns; ++i) {
			MachineStackColumn* column;
			uintptr_t inputmacid;
			psy_ui_Component* component;

			column = machinestackstate_column(self->state, i);
			inputmacid = psy_INDEX_INVALID;
			if (column && column->chain) {
				inputmacid = (uintptr_t)psy_list_entry(column->chain);
				if (inputmacid >= 0x40) {
					inputmacid = psy_INDEX_INVALID;
				}
			}
			if (inputmacid != psy_INDEX_INVALID) {
				component = machineui_create(
					psy_audio_machines_at(self->state->machines, inputmacid),
					inputmacid, self->skin, &self->component, &self->component, NULL,
					FALSE, self->workspace);
			} else {
				component = (psy_ui_Component*)malloc(sizeof(psy_ui_Component));
				psy_ui_component_init(component, &self->component, &self->component);
			}
			if (component) {				
				component->deallocate = TRUE;				
				psy_ui_component_setminimumsize(component, self->state->size);
				psy_ui_component_setalign(component, psy_ui_ALIGN_LEFT);
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
	psy_ui_Component* parent, MachineStackState* state,
	ParamSkin* skin)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setvtable(&self->component,
		machinestackoutputs_vtable_init(self));
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);	
	self->skin = skin;	
	self->state = state;
}

void machinestackoutputs_onpreferredsize(MachineStackOutputs* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = self->state->size.width;	
	rv->height = psy_ui_value_makeeh(2.0);	
	psy_ui_value_mul_real(&rv->width, 10.0);	
}

void machinestackoutputs_build(MachineStackOutputs* self)
{		
	psy_ui_component_clear(&self->component);
	if (self->state->machines) {							
		uintptr_t i;
		uintptr_t maxnumcolumns;

		maxnumcolumns = machinestackstate_maxnumcolumns(self->state);
		for (i = 0; i < maxnumcolumns; ++i) {
			MachineStackColumn* column;
			SwitchUi* rv;

			column = machinestackstate_column(self->state, i);
			rv = (SwitchUi*)malloc(sizeof(SwitchUi));
			if (rv) {				
				if (column) {
					switchui_init(rv, &self->component, &self->component,
						&column->masterroute.machineparam, self->skin);							
				} else {
					switchui_init(rv, &self->component, &self->component, NULL,
						self->skin);						
				}
				rv->component.deallocate = TRUE;
				psy_ui_component_setminimumsize(&rv->component,
					psy_ui_size_make(self->state->size.width, psy_ui_value_makeeh(1.0)));
				psy_ui_component_setalign(&rv->component, psy_ui_ALIGN_LEFT);				
			}			
		}		
	}	
}

// MachineStackPane
// prototypes
static void machinestackpane_ondestroy(MachineStackPane*);
static psy_ui_Component* machinestackpane_insert(MachineStackPane*,
	uintptr_t slot, psy_ui_Component* trackpane);

void machinestackpane_init(MachineStackPane* self, psy_ui_Component* parent,
	MachineStackState* state, MachineViewSkin* skin,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);	
	self->workspace = workspace;
	self->skin = skin;
	self->state = state;	
	self->vudrawupdate = FALSE;
	self->opcount = 0;	
	machinestackpane_updateskin(self);	
	machinestackpane_build(self);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void machinestackpane_build(MachineStackPane* self)
{
	psy_ui_component_clear(&self->component);
	if (self->state->machines) {
		uintptr_t i;
		uintptr_t maxnumcolumns;

		maxnumcolumns = machinestackstate_maxnumcolumns(self->state);
		for (i = 0; i < maxnumcolumns; ++i) {
			MachineStackPaneTrack* trackpane;

			trackpane = (MachineStackPaneTrack*)malloc(sizeof(MachineStackPaneTrack));
			if (trackpane) {
				MachineStackColumn* column;
				bool insert;

				insert = FALSE;				
				machinestackpanetrack_init(trackpane, &self->component, i, &self->component,
					self->state, self->workspace);
				trackpane->component.deallocate = TRUE;				
				psy_ui_component_setalign(&trackpane->component, psy_ui_ALIGN_LEFT);
				column = machinestackstate_column(self->state, i);
				if (column) {
					psy_List* p;
					
					for (p = column->chain; p != NULL; psy_list_next(&p)) {
						uintptr_t slot;
						psy_audio_Machine* machine;

						slot = (uintptr_t)psy_list_entry(p);
						machine = psy_audio_machines_at(self->state->machines,
							slot);
						if (machine && psy_audio_machine_mode(machine) ==
								psy_audio_MACHMODE_FX) {
							machinestackpane_insert(self, slot,
								&trackpane->component);
							insert = TRUE;
						}
					}
				} 
				if (!insert) {
					psy_ui_component_setminimumsize(&trackpane->component,
						self->state->size);
					psy_ui_component_setalign(&trackpane->component,
						psy_ui_ALIGN_LEFT);
				}					
				psy_ui_component_align(&trackpane->component);
			}
		}		
		psy_ui_component_align(&self->component);		
	} 
}

psy_ui_Component* machinestackpane_insert(MachineStackPane* self, uintptr_t slot,
	psy_ui_Component* trackpane)
{	
	psy_ui_Component* rv;

	rv =  machineui_create(
		psy_audio_machines_at(self->state->machines, slot), slot, self->skin,
		trackpane, &self->component, NULL, FALSE, self->workspace);
	if (rv) {
		psy_ui_component_setminimumsize(rv, self->state->size);
		rv->deallocate = TRUE;
		psy_ui_component_setalign(rv, psy_ui_ALIGN_TOP);
		return rv;
	}	
	return NULL;
}

void machinestackpane_updateskin(MachineStackPane* self)
{
	psy_ui_component_setbackgroundcolour(&self->component, self->skin->colour);
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
	MachineStackState* state, ParamSkin* skin)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_doublebuffer(&self->component);
	self->state = state;	
	self->skin = skin;
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);	
}

void machinestackvolumes_build(MachineStackVolumes* self)
{
	uintptr_t i;
	uintptr_t maxnumcolumns;

	assert(self);
	
	psy_ui_component_clear(&self->component);
	maxnumcolumns = machinestackstate_maxnumcolumns(self->state);
	for (i = 0; i < maxnumcolumns; ++i) {
		MachineStackColumn* column;				
		psy_ui_Component* component;

		column = machinestackstate_column(self->state,  i);
		component = (psy_ui_Component*)malloc(sizeof(psy_ui_Component));
		if (component) {
			psy_ui_component_init(component, &self->component,
				&self->component);		
			if (column && machinestackcolumn_wire(column)) {
				SliderUi* slider;
				LevelUi* level;
				LabelUi* label;

				slider = (SliderUi*)malloc(sizeof(SliderUi));
				if (slider) {
					sliderui_init(slider, component, &self->component,
						&column->wirevolume->machineparam, self->skin);
					slider->component.deallocate = TRUE;
					psy_ui_component_setalign(&slider->component,
						psy_ui_ALIGN_LEFT);
				}
				level = (LevelUi*)malloc(sizeof(LevelUi));
				if (level) {
					levelui_init(level, component, &self->component,
						&column->level_param.machineparam, self->skin);
					level->component.deallocate = TRUE;
					psy_ui_component_setalign(&level->component,
						psy_ui_ALIGN_LEFT);
				}
				label = (LabelUi*)malloc(sizeof(LabelUi));
				if (label) {
					labelui_init(label, component, &self->component,
						&column->wirevolume->machineparam, self->skin);
					label->component.deallocate = TRUE;
					psy_ui_component_setalign(&label->component,
						psy_ui_ALIGN_BOTTOM);
				}
			}		
			component->deallocate = TRUE;
			psy_ui_component_setalign(component, psy_ui_ALIGN_LEFT);
			psy_ui_component_setminimumsize(component,
				psy_ui_size_make(self->state->size.width,
					psy_ui_value_makepx(182.0)));
			psy_ui_component_setmaximumsize(component,
				psy_ui_size_make(self->state->size.width,
					psy_ui_value_makepx(182.0)));
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
	psy_ui_component_setvtable(&self->component,
		machinestackpanetrack_vtable_init(self));	
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
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
static void machinestackview_build(MachineStackView*);
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
	psy_ui_Component* tabbarparent, MachineViewSkin* skin,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_setvtable(&self->component, vtable_init(self));
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
	psy_ui_component_setalign(&self->columns, psy_ui_ALIGN_VCLIENT);
	psy_ui_component_setalign(&self->scroller_columns.component,
		psy_ui_ALIGN_CLIENT);
	machinestackinputs_init(&self->inputs, &self->columns,
		&self->state, skin, workspace);
	psy_ui_component_setalign(&self->inputs.component, psy_ui_ALIGN_TOP);
	machinestackpane_init(&self->pane, &self->columns, &self->state, skin,
		workspace);	
	psy_ui_component_setalign(&self->pane.component, psy_ui_ALIGN_CLIENT);	
	machinestackvolumes_init(&self->volumes, &self->columns, &self->state,
		machineparamconfig_skin(
			psycleconfig_macparam(workspace_conf(workspace))));
	psy_ui_component_setalign(&self->volumes.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setminimumsize(&self->volumes.component,
		psy_ui_size_make(self->state.size.width, psy_ui_value_makepx(200.0)));
	machinestackoutputs_init(&self->outputs, &self->columns, &self->state,
		machineparamconfig_skin(
			psycleconfig_macparam(workspace_conf(self->workspace))));
	psy_ui_component_setalign(&self->outputs.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&workspace->signal_songchanged, self,
		machinestackview_onsongchanged);	
	machinestackview_setmachines(self, workspace_song(workspace)
		? &workspace_song(workspace)->machines : NULL);
	psy_ui_component_setfont(&self->desc.component, &skin->font);
	psy_ui_component_setfont(&self->inputs.component, &skin->font);
	psy_ui_component_setfont(&self->outputs.component, &skin->font);
	psy_ui_component_setfont(&self->pane.component, &skin->font);
	psy_ui_component_setfont(&self->volumes.component, &skin->font);
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
	}
	machinestackstate_setmachines(&self->state, machines);	
	machinestackview_build(self);
}

void machinestackview_onmachineinsert(MachineStackView* self,
	psy_audio_Machines* sender, uintptr_t slot)
{	
	machinestackstate_rebuildview(&self->state);
}

void machinestackview_onmachineremoved(MachineStackView* self,
	psy_audio_Machines* sender, uintptr_t slot)
{	
	machinestackstate_rebuildview(&self->state);
}

void machinestackview_onconnected(MachineStackView* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{	
	machinestackstate_rebuildview(&self->state);
}

void machinestackview_ondisconnected(MachineStackView* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{
	machinestackstate_rebuildview(&self->state);
}

void machinestackview_ontimer(MachineStackView* self, uintptr_t timerid)
{
	if (machinestackstate_rebuildingview(&self->state)) {
		machinestackview_build(self);
	}
	if (psy_ui_component_drawvisible(&self->volumes.component)) {
		psy_ui_component_invalidate(&self->volumes.component);
		machinestackinputs_updatevus(&self->inputs);
		machinestackpane_updatevus(&self->pane);
	}
}

void machinestackview_build(MachineStackView* self)
{
	psy_audio_exclusivelock_enter();
	machinestackstate_buildcolumns(&self->state);
	machinestackpane_build(&self->pane);
	machinestackinputs_build(&self->inputs);
	machinestackoutputs_build(&self->outputs);
	machinestackvolumes_build(&self->volumes);
	psy_ui_component_align(&self->desc.component);
	psy_ui_component_align(&self->outputs.component);
	psy_ui_component_align(&self->columns);
	psy_ui_component_align(&self->component);
	machinestackstate_endviewbuild(&self->state);
	psy_audio_exclusivelock_leave();
}

void machinestackview_addeffect(MachineStackView* self,
	const psy_audio_MachineInfo* machineinfo)
{
	if (machinestackstate_selectedcolumn(&self->state)) {
		psy_audio_machines_select(self->state.machines,
			machinestackcolumn_append_effect(
				machinestackstate_selectedcolumn(&self->state),
				psy_audio_machinefactory_make_info(
					&self->workspace->machinefactory, machineinfo)));
	}
}
