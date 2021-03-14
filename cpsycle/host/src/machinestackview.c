// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinestackview.h"
// host
#include "knobui.h"
#include "machineviewbar.h"
#include "slidergroupui.h"
// audio
#include <exclusivelock.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// RouteMachineParam
static void outputrouteparam_tweak(OutputRouteParam*, float val);
static float outputrouteparam_normvalue(OutputRouteParam*);
static int outputrouteparam_describe(OutputRouteParam* self, char* text);
static int outputrouteparam_name(OutputRouteParam*, char* text);
static int outputrouteparam_type(OutputRouteParam*);
static void outputrouteparam_range(OutputRouteParam*,
	intptr_t* minval, intptr_t* maxval);
static psy_audio_Wire outputrouteparam_wire(OutputRouteParam*);
static uintptr_t outputrouteparam_busid(OutputRouteParam*, psy_audio_Wire);
// vtable
static MachineParamVtable outputrouteparam_vtable;
static bool outputrouteparam_vtable_initialized = FALSE;

static void outputrouteparam_vtable_init(OutputRouteParam* self)
{
	if (!outputrouteparam_vtable_initialized) {
		outputrouteparam_vtable = *(self->machineparam.vtable);
		outputrouteparam_vtable.normvalue = (fp_machineparam_normvalue)
			outputrouteparam_normvalue;
		outputrouteparam_vtable.tweak = (fp_machineparam_tweak)
			outputrouteparam_tweak;
		outputrouteparam_vtable.describe = (fp_machineparam_describe)
			outputrouteparam_describe;
		outputrouteparam_vtable.type = (fp_machineparam_type)
			outputrouteparam_type;
		outputrouteparam_vtable.range = (fp_machineparam_range)
			outputrouteparam_range;
		outputrouteparam_vtable.name = (fp_machineparam_name)
			outputrouteparam_name;
		outputrouteparam_vtable_initialized = TRUE;
	}
}
// implementation
void outputrouteparam_init(OutputRouteParam* self, psy_audio_Machines* machines,
	uintptr_t column, MachineStackState* state)
{
	psy_audio_machineparam_init(&self->machineparam);
	outputrouteparam_vtable_init(self);
	self->machineparam.vtable = &outputrouteparam_vtable;	
	self->column = column;
	self->machines = machines;
	self->state = state;
}

void outputrouteparam_dispose(OutputRouteParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
}

void outputrouteparam_tweak(OutputRouteParam* self, float value)
{
	intptr_t minval;
	intptr_t maxval;
	intptr_t selidx;
	psy_audio_Wire wire;

	psy_audio_exclusivelock_enter();
	self->state->preventrebuild = TRUE;
	outputrouteparam_range(self, &minval, &maxval);
	selidx = (intptr_t)(value * (maxval - minval) + 0.5f) + minval;	
	wire = outputrouteparam_wire(self);
	if (selidx == 0) { // No Connection
		MachineStackColumn* column;
		uintptr_t src;

		column = machinestackstate_column(self->state, self->column);
		src = machinestackcolumn_lastbeforemaster(column);
		if (wire.dst != psy_INDEX_INVALID) {
			psy_audio_machines_disconnect(self->state->machines,
				psy_audio_wire_make(src, wire.dst));	
		}
	} else if (selidx == 1) { // Master Connection
		if (wire.dst != psy_audio_MASTER_INDEX) {
			if (wire.dst != psy_INDEX_INVALID) {
				psy_audio_Machine* machine;

				machine = psy_audio_machines_at(self->state->machines,
					wire.dst);
				if (machine && psy_audio_machine_isbus(machine)) {
					psy_audio_machines_disconnect(self->state->machines, wire);
				}
			}
			if (wire.dst != psy_INDEX_INVALID) {
				psy_audio_machines_connect(self->state->machines,
					psy_audio_wire_make(wire.dst, psy_audio_MASTER_INDEX));
			} else { // Bus Connection
				MachineStackColumn* column;

				column = machinestackstate_column(self->state, self->column);
				if (column) {
					uintptr_t src;
					
					src = machinestackcolumn_at(column, 0);
					psy_audio_machines_connect(self->state->machines,
						psy_audio_wire_make(src, psy_audio_MASTER_INDEX));
				}
			}
		}					
	} else {
		MachineStackColumn* column;

		column = machinestackstate_column(self->state, self->column);
		if (psy_audio_wire_valid(&wire)) {
			psy_audio_machines_disconnect(self->state->machines, wire);
		}				
		if (column) {
			uintptr_t src;			
			psy_List* buses;

			src = machinestackcolumn_lastbeforemaster(column);
			buses = machinestackstate_buses(self->state);
			if (buses) {
				psy_List* p;

				p = psy_list_at(buses, selidx - 2);
				if (p) {
					uintptr_t dest;

					dest = (uintptr_t)psy_list_entry(p);
					psy_audio_machines_connect(self->state->machines,
						psy_audio_wire_make(src, dest));
				}
			}
		}
	}
	psy_audio_exclusivelock_leave();
	self->state->preventrebuild = FALSE;	
}

float outputrouteparam_normvalue(OutputRouteParam* self)
{		
	psy_audio_Wire wire;
	uintptr_t selidx;
	intptr_t minval;
	intptr_t maxval;
	
	outputrouteparam_range(self, &minval, &maxval);
	selidx = 0;
	wire = outputrouteparam_wire(self);
	if (wire.dst == psy_INDEX_INVALID) {
		selidx = 0;
	} else if (wire.dst == psy_audio_MASTER_INDEX) {
		selidx = 1;
	} else {
		selidx = outputrouteparam_busid(self, wire);
		if (selidx == psy_INDEX_INVALID) {
			selidx = 0;
		} else {
			selidx += 2;
		}
	}
	return ((maxval - minval) != 0)
		? (selidx - minval) / (float)(maxval - minval)
		: 0.f;	
}

psy_audio_Wire outputrouteparam_wire(OutputRouteParam* self)
{
	MachineStackColumn* column;

	column = machinestackstate_column(self->state, self->column);
	if (column) {
		return machinestackcolumn_outputwire(column);
	}
	return psy_audio_wire_make(psy_INDEX_INVALID, psy_INDEX_INVALID);
}

uintptr_t outputrouteparam_busid(OutputRouteParam* self, psy_audio_Wire wire)
{
	return psy_list_entry_index(machinestackstate_buses(self->state),
		(void*)wire.dst);	
}

int outputrouteparam_describe(OutputRouteParam* self, char* text)
{	
	psy_audio_Wire wire;

	wire = outputrouteparam_wire(self);
	if (psy_audio_wire_valid(&wire)) {
		if (wire.dst == psy_audio_MASTER_INDEX) {
			psy_snprintf(text, 128, "%s", "Master");
		} else {
			psy_audio_Machine* machine;

			machine = psy_audio_machines_at(self->state->machines,
				wire.dst);
			if (psy_audio_machine_isbus(machine)) {
				psy_snprintf(text, 128, "Bus %.2X", (int)wire.dst);
			} else {
				psy_snprintf(text, 128, "End at %.2X", (int)wire.dst);
			}
		}
	} else {
		psy_snprintf(text, 128, "%s", "None");
	}
	return 1;	
}

int outputrouteparam_name(OutputRouteParam* self, char* text)
{
	psy_snprintf(text, 128, "%s", "Output");
	return 1;
}

int outputrouteparam_type(OutputRouteParam* self)
{
	return MPF_STATE | MPF_SMALL;
}

void outputrouteparam_range(OutputRouteParam* self,
	intptr_t* minval, intptr_t* maxval)
{
	psy_List* buses;
		
	*minval = 0;
	*maxval = 1;
	buses = machinestackstate_buses(self->state);
	if (buses) {
		*maxval += psy_list_size(buses);
	}
}

// MachineStackColumn
// prototypes
static void  machinestackcolumn_level_normvalue(MachineStackColumn*,
	psy_audio_IntMachineParam* sender, float* rv);
// implementation
void machinestackcolumn_init(MachineStackColumn* self, uintptr_t column,
	uintptr_t inputroute, MachineStackState* state)
{
	self->state = state;
	self->column = column;
	self->inputroute = inputroute;
	self->chain = NULL;
	self->wirevolume = NULL;	
	outputrouteparam_init(&self->outputroute, state->machines, column, state);
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
	outputrouteparam_dispose(&self->outputroute);
}

void machinestackcolumn_setwire(MachineStackColumn* self, psy_audio_Wire wire)
{
	psy_audio_Machine* machine;	

	if (self->wirevolume) {
		psy_audio_wiremachineparam_deallocate(self->wirevolume);
		self->wirevolume = NULL;
	}	
	machine = psy_audio_machines_at(self->state->machines, wire.src);
	if (machine) {
		self->wirevolume = psy_audio_wiremachineparam_allocinit(
			psy_audio_machine_editname(machine), wire, self->state->machines);
	}
}

psy_audio_Wire machinestackcolumn_outputwire(MachineStackColumn* self)
{
	if (self->wirevolume) {
		return self->wirevolume->wire;
	}
	return psy_audio_wire_make(psy_INDEX_INVALID, psy_INDEX_INVALID);
}

void machinestackcolumn_append(MachineStackColumn* self, uintptr_t macid)
{
	psy_list_append(&self->chain, (void*)macid);	
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

uintptr_t machinestackcolumn_at(const MachineStackColumn* self, uintptr_t index)
{
	if (self->chain && index != psy_INDEX_INVALID) {
		const psy_List* p;

		p = psy_list_at(self->chain, index);
		if (p) {
			return (uintptr_t)psy_list_entry_const(p);
		}
	}
	return psy_INDEX_INVALID;
}

uintptr_t machinestackcolumn_last(const MachineStackColumn* self)
{
	if (self->chain) {
		const psy_List* p;

		p = psy_list_last(self->chain);
		if (p) {
			return (uintptr_t)psy_list_entry_const(p);
		}
	}
	return psy_INDEX_INVALID;
}

uintptr_t machinestackcolumn_lastbeforeindex(const MachineStackColumn* self, uintptr_t index)
{
	if (self->chain && index > 0) {
		const psy_List* last;

		last = psy_list_at(self->chain, index - 1);
		if (last) {
			return (uintptr_t)psy_list_entry_const(last);			
		}
	}
	return psy_INDEX_INVALID;
}

uintptr_t machinestackcolumn_nextindex(const MachineStackColumn* self, uintptr_t index)
{
	if (self->chain && index > 0) {
		const psy_List* next;

		next = psy_list_at(self->chain, index + 1);
		if (next) {
			return (uintptr_t)psy_list_entry_const(next);
		}
	}
	return psy_INDEX_INVALID;
}

bool machinestackcolumn_connectedtomaster(const MachineStackColumn* self)
{	
	return (psy_audio_machineparam_normvalue(&self->outputroute.machineparam) >
		0.f);
}

void  machinestackcolumn_level_normvalue(MachineStackColumn* self,
	psy_audio_IntMachineParam* sender, float* rv)
{	
	psy_audio_Machine* machine;
	psy_audio_Buffer* memory;

	*rv = 0.f;	
	machine = psy_audio_machines_at(self->state->machines,
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
	uintptr_t slot, uintptr_t prevslot, uintptr_t columnindex, bool first);
// implementation
void machinestackstate_init(MachineStackState* self, MachineViewBar* statusbar)
{
	psy_table_init(&self->columns);
	self->statusbar = statusbar;
	self->machines = NULL;	
	self->selected = psy_INDEX_INVALID;
	self->effectsize = psy_ui_size_makepx(138.0, 52.0);
	self->columnselected = FALSE;
	// 20: right margin of column
	self->columnsize = psy_ui_size_makepx(138.0 + 20, 52.0);
	self->update = FALSE;
	self->effectinsertpos = psy_INDEX_INVALID;
	self->effectinsertright = FALSE;
	self->preventrebuild = FALSE;
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
	uintptr_t columnindex, uintptr_t inputroute)
{
	MachineStackColumn* column;
	
	column = machinestackstate_column(self, columnindex);
	if (column) {
		machinestackcolumn_dispose(column);
		free(column);
	}
	column = (MachineStackColumn*)malloc(sizeof(MachineStackColumn));
	machinestackcolumn_init(column, columnindex, inputroute, self);
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
			if (psy_audio_machine_mode(machine) == psy_audio_MACHMODE_GENERATOR ||
					psy_audio_machine_isbus(machine)) {
				psy_list_append(&rv, (void*)psy_tableiterator_key(&it));
			}
		}
		return rv;
	}
	return NULL;
}

psy_List* machinestackstate_buses(MachineStackState* self)
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
			if (psy_audio_machine_isbus(machine)) {
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
			machinestackstate_buildcolumnoutchain(self, psy_INDEX_INVALID, 0, psy_INDEX_INVALID, TRUE);
		} else {
			columnindex = 0;
			for (p = inputs; p != NULL; psy_list_next(&p)) {
				columnindex = machinestackstate_buildcolumnoutchain(self,
					(uintptr_t)(p->entry), psy_INDEX_INVALID,
					columnindex, TRUE) + 1;
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
	uintptr_t prevslot, uintptr_t columnindex, bool first)
{
	uintptr_t rv;
	MachineStackColumn* column;
	psy_audio_MachineSockets* sockets;
	psy_audio_Machine* machine;

	if (slot == psy_INDEX_INVALID) {
		return columnindex;
	}
	rv = columnindex;
	column = machinestackstate_column(self, columnindex);
	if (!machinestackstate_column(self, columnindex)) {
		column = machinestackstate_insertcolumn(self, columnindex, prevslot);
	}
	machinestackcolumn_append(column, slot);
	machine = psy_audio_machines_at(self->machines, slot);
	if (!first && psy_audio_machine_isbus(machine)) {
		return rv;
	}
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
				rv = machinestackstate_buildcolumnoutchain(self, socket->slot, slot,
					rv + c, FALSE);
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
static void machinestackdesc_onalign(MachineStackDesc*);
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
	volumesizepx.height = psy_max(182.0, volumesizepx.height);
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
static void machinestackinputs_onmouseenter(MachineStackInputs*);	
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
		machinestackinputs_vtable.onmouseenter =
			(psy_ui_fp_component_onmouseenter)
			machinestackinputs_onmouseenter;
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
	*rv = self->state->columnsize;
	// +1 : empty space to add new generator
	psy_ui_value_mul_real(&rv->width, 
		psy_max(1.0, machinestackstate_maxnumcolumns(self->state) + 1));
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
				psy_audio_Machine* machine;
				bool isbus;
				bool isgen;

				inputmacid = (uintptr_t)psy_list_entry(column->chain);
				isbus = FALSE;
				isgen = FALSE;
				machine = psy_audio_machines_at(self->state->machines, inputmacid);
				if (machine) {
					isbus = psy_audio_machine_isbus(machine);
					isgen = psy_audio_machine_mode(machine) == psy_audio_MACHMODE_GENERATOR;
				}
				if (!isgen && !isbus) {
					inputmacid = psy_INDEX_INVALID;
				}
			}
			if (inputmacid != psy_INDEX_INVALID) {
				component = machineui_create(
					psy_audio_machines_at(self->state->machines, inputmacid),
					inputmacid, self->skin, &self->component, &self->component, NULL,
					FALSE, self->workspace);
			} else {
				component = psy_ui_component_allocinit(&self->component,
					&self->component);
				psy_ui_component_setbackgroundmode(component,
					psy_ui_NOBACKGROUND);
			}
			if (component) {				
				component->deallocate = TRUE;				
				psy_ui_component_setminimumsize(component,
					self->state->columnsize);
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

void machinestackinputs_onmouseenter(MachineStackInputs* self)
{	
	// machineviewbar_settext(self->state->statusbar,
	//	"Double click mouse to insert a generator");	
}

void machinestackinputs_updatevus(MachineStackInputs* self)
{	
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);
	machineui_beginvuupdate();
	psy_ui_component_invalidate(&self->component);
	machineui_endvuupdate();
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_SETBACKGROUND);
	psy_ui_component_update(&self->component);
}

// MachineStackOutputs
// implementation
void machinestackoutputs_init(MachineStackOutputs* self,
	psy_ui_Component* parent, MachineStackState* state,
	ParamSkin* skin)
{
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);		
	psy_ui_component_setminimumsize(&self->component,
		psy_ui_size_makeem(10.0, 2.0));
	self->skin = skin;	
	self->state = state;
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
			KnobUi* knobui;

			column = machinestackstate_column(self->state, i);			
			knobui = knobui_allocinit(&self->component, NULL,
				(column)
				? &column->outputroute.machineparam
				: NULL,
				self->skin);
			if (knobui) {
				psy_ui_component_setminimumsize(&knobui->component,
					psy_ui_size_make(self->state->columnsize.width,
						psy_ui_value_makeeh(1.0)));
				psy_ui_component_setalign(&knobui->component,
					psy_ui_ALIGN_LEFT);
			}
		}		
	}	
}

// MachineStackPaneTrack
static void machinestackpanetrack_onmousedoubleclick(MachineStackPaneTrack*,
	psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable machinestackpanetrack_vtable;
static bool machinestackpanetrack_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackpanetrack_vtable_init(
	MachineStackPaneTrack* self)
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
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_TOP,
		psy_ui_margin_make(
			psy_ui_value_makepx(20.0), psy_ui_value_makepx(20.0),
			psy_ui_value_makepx(0.0), psy_ui_value_makepx(0.0)));
	self->column = column;
	self->state = state;
	self->workspace = workspace;
}

void machinestackpanetrack_onmousedoubleclick(MachineStackPaneTrack* self,
	psy_ui_MouseEvent* ev)
{
	MachineStackColumn* column;

	self->state->columnselected = TRUE;
	column = machinestackstate_column(self->state, self->column);
	if (column && column->chain) {
		psy_List* p;
		psy_List* q;
		uintptr_t effect;
		uintptr_t c;

		q = psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE);
		effect = psy_INDEX_INVALID;
		for (p = q, c = 0; p != NULL; psy_list_next(&p), ++c) {
			psy_ui_Component* component;
			psy_ui_RealRectangle position;

			component = (psy_ui_Component*)psy_list_entry(p);
			position = psy_ui_component_position(component);
			if (position.bottom > ev->pt.y) {
				break;
			}
			if (psy_ui_realrectangle_intersect(&position, ev->pt)) {
				effect = c;
				break;
			}
		}
		if (effect == psy_INDEX_INVALID) {
			self->state->effectinsertpos = c;
			self->state->effectinsertright =
				(ev->pt.x > psy_ui_value_px(&self->state->effectsize.width,
					psy_ui_component_textmetric(&self->component)));
			self->state->selected = self->column;
			workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
				SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_ADDEFFECTSTACK);
			psy_ui_mouseevent_stoppropagation(ev);
		} else {
			self->state->effectinsertpos = psy_INDEX_INVALID;
			self->state->effectinsertright = FALSE;
		}
	}
}

// MachineStackPane
static void machinestackpane_onmousedoubleclick(MachineStackPane* self,
	psy_ui_MouseEvent* ev);
static psy_ui_Component* machinestackpane_insert(MachineStackPane*,
	uintptr_t slot, psy_ui_Component* trackpane);
// vtable
static psy_ui_ComponentVtable machinestackpane_vtable;
static bool machinestackpane_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackpane_vtable_init(MachineStackPane* self)
{
	if (!machinestackpane_vtable_initialized) {
		machinestackpane_vtable = *(self->component.vtable);
		machinestackpane_vtable.onmousedoubleclick =			
			(psy_ui_fp_component_onmouseevent)
			machinestackpane_onmousedoubleclick;
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
								psy_audio_MACHMODE_FX && !psy_audio_machine_isbus(machine)) {
							machinestackpane_insert(self, slot,
								&trackpane->component);
							insert = TRUE;
						}
					}
				} 
				if (!insert) {
					psy_ui_component_setminimumsize(&trackpane->component,
						self->state->columnsize);
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
		psy_ui_component_setminimumsize(rv, self->state->effectsize);
		rv->deallocate = TRUE;
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
		psy_ui_NOBACKGROUND);
	q = psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE);
	for (p = q; p != NULL; psy_list_next(&p)) {
		psy_ui_Component* trackpane;
		int restorebgmode;
		
		trackpane = (psy_ui_Component*)psy_list_entry(p);
		restorebgmode = trackpane->backgroundmode;
		psy_ui_component_setbackgroundmode(trackpane,
			psy_ui_NOBACKGROUND);
		machineui_beginvuupdate();
		psy_ui_component_invalidate(trackpane);
		machineui_endvuupdate();
		psy_ui_component_setbackgroundmode(trackpane,
			restorebgmode);
	}
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_SETBACKGROUND);
	psy_ui_component_update(&self->component);	
}

void machinestackpane_onmousedoubleclick(MachineStackPane* self,
	psy_ui_MouseEvent* ev)
{
	if (!self->state->columnselected) {
		machineviewbar_settext(self->state->statusbar,
			"Add first input (Double click in Inputs)");
	}	
	self->state->columnselected = FALSE;
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
		psy_ui_Component* component;
		MachineStackColumn* column;				
				
		component = NULL;
		column = machinestackstate_column(self->state,  i);			
		if (column && machinestackcolumn_wire(column)) {
			SliderGroupUi* slidergroup;			

			slidergroup = slidergroupui_allocinit(&self->component,
				&self->component,
				&column->wirevolume->machineparam,
				&column->level_param.machineparam,
				self->skin);
			if (slidergroup) {				
				component = &slidergroup->component;				
			}
		} else {
			component = psy_ui_component_allocinit(&self->component,
				&self->component);
			psy_ui_component_setbackgroundmode(component, psy_ui_NOBACKGROUND);
		}
		if (component) {
			psy_ui_Margin margin;

			psy_ui_component_setalign(component, psy_ui_ALIGN_LEFT);
			psy_ui_component_setminimumsize(component, psy_ui_size_make(
				psy_ui_value_makepx(138.0 + 19),
				psy_ui_value_makepx(182.0)));
			psy_ui_component_setmaximumsize(component, psy_ui_size_make(
				psy_ui_value_zero(),
				psy_ui_value_makepx(182.0)));
			margin = psy_ui_margin_make(
				psy_ui_value_makepx(0.0), psy_ui_value_makepx(1.0),
				psy_ui_value_makepx(0.0), psy_ui_value_makepx(0.0));
			psy_ui_component_setmargin(component, &margin);	
		}
	}
	psy_ui_component_align(&self->component);
}

// MachineStackView
// prototypes
static void machinestackview_destroy(MachineStackView*);
static void machinestackview_onsongchanged(MachineStackView*, Workspace*,
	int flag, psy_audio_Song*);
static void machinestackview_onbuschanged(MachineStackView*, Workspace*,
	psy_audio_Machine*);
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
	MachineViewBar* statusbar, Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_setvtable(&self->component, vtable_init(self));
	self->workspace = workspace;
	machinestackstate_init(&self->state, statusbar);
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
		psy_ui_size_make(self->state.columnsize.width, psy_ui_value_makepx(182.0)));
	machinestackoutputs_init(&self->outputs, &self->columns, &self->state,
		machineparamconfig_skin(
			psycleconfig_macparam(workspace_conf(self->workspace))));
	psy_ui_component_setalign(&self->outputs.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&workspace->signal_songchanged, self,
		machinestackview_onsongchanged);	
	psy_signal_connect(&workspace->signal_buschanged, self,
		machinestackview_onbuschanged);
	machinestackview_setmachines(self, workspace_song(workspace)
		? &workspace_song(workspace)->machines : NULL);
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

void machinestackview_onbuschanged(MachineStackView* self, Workspace* sender,
	psy_audio_Machine* machine)
{
	machinestackstate_rebuildview(&self->state);
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
		//psy_ui_component_invalidate(&self->outputs.component);
		machinestackinputs_updatevus(&self->inputs);
		machinestackpane_updatevus(&self->pane);
	}
}

void machinestackview_build(MachineStackView* self)
{
	if (!self->state.preventrebuild) {
		psy_audio_exclusivelock_enter();
		machinestackstate_buildcolumns(&self->state);
		machinestackpane_build(&self->pane);
		machinestackinputs_build(&self->inputs);
		machinestackoutputs_build(&self->outputs);
		machinestackvolumes_build(&self->volumes);
		psy_ui_app()->alignvalid = FALSE;
		psy_ui_component_align(&self->component);
		// reset to normal align
		psy_ui_app()->alignvalid = TRUE;
		psy_ui_component_invalidate(&self->component);		
		psy_audio_exclusivelock_leave();
	}
	machinestackstate_endviewbuild(&self->state);
}

void machinestackview_addeffect(MachineStackView* self,
	const psy_audio_MachineInfo* machineinfo)
{
	psy_audio_Machine* effect;	
	
	effect = psy_audio_machinefactory_make_info(
		&self->workspace->machinefactory, machineinfo);	
	if (effect) {
		uintptr_t macid;
		MachineStackColumn* insertcolumn;

		macid = psy_audio_machines_append(self->state.machines, effect);		
		insertcolumn = machinestackstate_selectedcolumn(&self->state);
		if (insertcolumn) {
			uintptr_t firstmacid;
			bool hasinput;
			uintptr_t insertpos;
			psy_audio_Machine* input;
			uintptr_t prevmacid;
			uintptr_t nextmacid;

			firstmacid = machinestackcolumn_at(insertcolumn, 0);
			prevmacid = psy_INDEX_INVALID;
			nextmacid = psy_INDEX_INVALID;
			input = psy_audio_machines_at(self->state.machines, firstmacid);
			hasinput = (input && (psy_audio_machine_isbus(input) ||
				psy_audio_machine_mode(input) == psy_audio_MACHMODE_GENERATOR));
			insertpos = self->state.effectinsertpos;
			if (!hasinput) {
				if (self->state.effectinsertpos > 0) {
					insertpos -= 1;
				} else {
					insertpos = psy_INDEX_INVALID;
				}
			}
			if (insertpos != psy_INDEX_INVALID) {
				prevmacid = machinestackcolumn_at(insertcolumn, insertpos);
			} else {
				prevmacid = insertcolumn->inputroute;
				nextmacid = machinestackcolumn_at(insertcolumn, 0);
			}
			if (self->state.effectinsertright) {
				nextmacid = psy_INDEX_INVALID;
			} else if (insertpos != psy_INDEX_INVALID) {
				nextmacid = machinestackcolumn_at(insertcolumn, insertpos + 1);				
			}
			if (prevmacid != psy_INDEX_INVALID) {
				psy_audio_exclusivelock_enter();
				psy_audio_machines_connect(self->state.machines,
					psy_audio_wire_make(prevmacid, macid));								
				psy_audio_exclusivelock_leave();
			}
			if (nextmacid != psy_INDEX_INVALID) {
				psy_audio_exclusivelock_enter();
				psy_audio_machines_disconnect(self->state.machines,
					psy_audio_wire_make(prevmacid, nextmacid));
				psy_audio_machines_connect(self->state.machines,
					psy_audio_wire_make(macid, nextmacid));
				psy_audio_exclusivelock_leave();
			}
		}		
	}
}
