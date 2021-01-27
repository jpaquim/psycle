// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinedock.h"
// audio
#include <songio.h>
// platform
#include "../../detail/portable.h"

// ParamRackBox
// prototypes
static void paramrackbox_onmousedoubleclick(ParamRackBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void paramrackbox_onthemechanged(ParamRackBox*, MachineViewConfig*,
	psy_Property* theme);
static void paramrackbox_onaddeffect(ParamRackBox*, psy_ui_Button* sender);
// implementation
void paramrackbox_init(ParamRackBox* self, psy_ui_Component* parent,
	uintptr_t slot, Workspace* workspace)
{
	psy_Property* theme;
	psy_audio_Machine* machine;
	psy_ui_Margin margin;

	machine = psy_audio_machines_at(&workspace->song->machines, slot);
	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;
	self->slot = slot;
	theme = workspace->config.macparam.theme;
	if (theme) {
		psy_ui_component_setbackgroundcolour(&self->component, psy_ui_colour_make(
			psy_property_at_colour(theme, "machineguititlecolour", 0x00292929)));		
	}
	psy_ui_component_init(&self->header, &self->component);
	psy_ui_component_setalign(&self->header, psy_ui_ALIGN_TOP);
	if (theme) {
		psy_ui_component_setcolour(&self->header, psy_ui_colour_make(
			psy_property_at_colour(theme, "machineguititlefontcolour", 0x00B4B4B4)));
	}
	// title label
	psy_ui_label_init(&self->title, &self->header);
	psy_ui_label_preventtranslation(&self->title);	
	if (machine) {
		psy_ui_label_settext(&self->title, psy_audio_machine_editname(machine));
	}
	psy_ui_component_setalign(&self->title.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->title.component.signal_mousedoubleclick, self,
		paramrackbox_onmousedoubleclick);
	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(0.0), psy_ui_value_makeew(0.0),
		psy_ui_value_makeeh(0.5), psy_ui_value_makeew(0.5));
	//psy_ui_component_setspacing(&self->title.component, &margin);
	// Insert Effect
	if (machine && slot != psy_audio_MASTER_INDEX) {
		psy_ui_button_init_text(&self->inserteffect, &self->header, "+");
		psy_ui_component_setalign(&self->inserteffect.component, psy_ui_ALIGN_RIGHT);
		psy_signal_connect(&self->inserteffect.signal_clicked, self,
			paramrackbox_onaddeffect);
	}
	// Parameter List
	parameterlistbox_init(&self->parameters, &self->component,
		psy_audio_machines_at(&workspace->song->machines, slot), workspace);
	psy_ui_component_setalign(&self->parameters.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(
		&psycleconfig_macview(workspace_conf(workspace))->signal_themechanged,
		self, paramrackbox_onthemechanged);
	self->nextbox = NULL;
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
	workspace_showparameters(self->workspace, self->slot);	
}

void paramrackbox_onthemechanged(ParamRackBox* self, MachineViewConfig* config,
	psy_Property* theme)
{
	if (theme) {
		psy_ui_component_setbackgroundcolour(&self->component, psy_ui_colour_make(
			psy_property_at_colour(theme, "machineguititlecolour", 0x00292929)));	
		psy_ui_component_setcolour(&self->header, psy_ui_colour_make(
			psy_property_at_colour(theme, "machineguititlefontcolour", 0x00B4B4B4)));
		psy_ui_component_setbackgroundcolour(&self->parameters.listbox.component,
			psy_ui_colour_make(
			psy_property_at_colour(theme, "machineguititlecolour", 0x00292929)));		
	}
	psy_ui_component_invalidate(&self->component);
}

void paramrackbox_onaddeffect(ParamRackBox* self, psy_ui_Button* sender)
{
	if (self->workspace && workspace_song(self->workspace)) {
		if (self->nextbox) {
			psy_audio_machines_selectwire(&self->workspace->song->machines,
				psy_audio_wire_make(self->slot, self->nextbox->slot));
			workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
				SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_ADDEFFECT);
		} else {
			workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
				SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_APPEND);
		}
	}
}

// ParamRackPane
// implementation
static void paramrackpane_ondestroy(ParamRackPane*);
static void paramrackpane_build(ParamRackPane*);
static void paramrackpane_buildall(ParamRackPane*);
static void paramrackpane_buildinputs(ParamRackPane*);
static void paramrackpane_buildoutputs(ParamRackPane*);
static void paramrackpane_buildinchain(ParamRackPane*);
static void paramrackpane_buildoutchain(ParamRackPane*, uintptr_t slot);
static void paramrackpane_insertbox(ParamRackPane*, uintptr_t slot);
static void paramrackpane_removebox(ParamRackPane*, uintptr_t slot);
static void paramrackpane_onsongchanged(ParamRackPane*, Workspace* sender,
	int flag, psy_audio_SongFile*);
static void paramrackpane_connectsong(ParamRackPane*);
static void paramrackpane_onmachineselected(ParamRackPane*,
	psy_audio_Machines*, uintptr_t slot);
static void paramrackpane_onmachinesinsert(ParamRackPane*,
	psy_audio_Machines*, uintptr_t slot);
static void paramrackpane_onmachinesremoved(ParamRackPane*,
	psy_audio_Machines*, uintptr_t slot);
static void paramrackpane_onconnected(ParamRackPane*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void paramrackpane_ondisconnected(ParamRackPane*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);


// ParamRackPane
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ParamRackPane* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)paramrackpane_ondestroy;		
		vtable_initialized = TRUE;
	}
}
// implementation
void paramrackpane_init(ParamRackPane* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);	
	vtable_init(self);
	self->component.vtable = &vtable;
	self->lastselected = psy_INDEX_INVALID;
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_margin_make(
		psy_ui_value_makeeh(0.0), psy_ui_value_makeew(0.1),
		psy_ui_value_makeeh(0.0), psy_ui_value_makeew(0.0)));	
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setscrollstepx(&self->component, psy_ui_value_makepx(100));
	if (workspace_song(workspace)) {
		self->machines = &workspace->song->machines;
	}
	self->mode = PARAMRACK_OUTCHAIN;
	self->workspace = workspace;
	psy_signal_connect(&workspace->signal_songchanged, self,
		paramrackpane_onsongchanged);
	paramrackpane_connectsong(self);
	psy_table_init(&self->boxes);
	self->lastinserted = NULL;
	paramrackpane_build(self);
}

void paramrackpane_ondestroy(ParamRackPane* self)
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

void paramrackpane_onsongchanged(ParamRackPane* self, Workspace* sender,
	int flag, psy_audio_SongFile* songfile)
{
	if (workspace_song(sender)) {
		self->machines = &sender->song->machines;
		self->lastselected = psy_audio_machines_selected(self->machines);		
		paramrackpane_connectsong(self);		
		paramrackpane_build(self);		
	} else {
		self->machines = NULL;
		self->lastselected = psy_INDEX_INVALID;
		psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
			psy_ui_component_destroy);
		psy_table_init(&self->boxes);		
	}
}

void paramrackpane_build(ParamRackPane* self)
{
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
			psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
				psy_ui_component_destroy);
			paramrackpane_buildoutchain(self, self->lastselected);
			break; }
		// Fallthrough
		case PARAMRACK_ALL:
		default:
			paramrackpane_buildall(self);
			break;
	}
	psy_ui_component_align(&self->component);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}

void paramrackpane_buildall(ParamRackPane* self)
{
	psy_TableIterator it;	

	psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
		psy_ui_component_destroy);
	psy_table_init(&self->boxes);
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

	psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
		psy_ui_component_destroy);
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

	psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
		psy_ui_component_destroy);
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
	psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
		psy_ui_component_destroy);
	if (self->machines) {
		MachineList* path;
			
		path = psy_audio_compute_path(self->machines, self->lastselected, FALSE);
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

 void paramrackpane_insertbox(ParamRackPane* self, uintptr_t slot)
{
	if (psy_audio_machines_at(self->machines, slot)) {
		ParamRackBox* box;

		if (psy_table_exists(&self->boxes, slot)) {
			paramrackpane_removebox(self, slot);
		}
		box = (ParamRackBox*)malloc(sizeof(ParamRackBox));
		if (box) {			
			paramrackbox_init(box, &self->component, slot, self->workspace);
			if (self->lastinserted) {
				self->lastinserted->nextbox = box;
			}
			self->lastinserted = box;
			psy_ui_component_setalign(&box->component, psy_ui_ALIGN_LEFT);
			psy_table_insert(&self->boxes, slot, box);			
		}		
	}
}

void paramrackpane_removebox(ParamRackPane* self, uintptr_t slot)
{
	if (psy_table_exists(&self->boxes, slot)) {
		ParamRackBox* box;

		box = psy_table_at(&self->boxes, slot);
		psy_ui_component_deallocate(&box->component);		
		psy_table_remove(&self->boxes, slot);		
	}
}

void paramrackpane_connectsong(ParamRackPane* self)
{
	if (self->machines) {
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
	}
}


void paramrackpane_onmachineselected(ParamRackPane* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	if (self->mode == PARAMRACK_ALL) {
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
	} else if (self->lastselected != slot) {
		self->lastselected = slot;
		paramrackpane_build(self);
	}
}

void paramrackpane_onmachinesinsert(ParamRackPane* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	paramrackpane_insertbox(self, slot);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}

void paramrackpane_onmachinesremoved(ParamRackPane* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	paramrackpane_removebox(self, slot);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
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

// ParamRack
// prototypes
static void paramrack_horizontal_onchanged(ParamRack*, psy_ui_ScrollBar* sender);
static void paramrack_scrollrangechanged(ParamRack*, psy_ui_Component* sender,
	psy_ui_Orientation orientation);
static void paramrack_onmodeselected(ParamRack*, TabBar* sender, intptr_t index);
static void paramrack_onalign(ParamRack*, psy_ui_Component* sender);
// implementation
void paramrack_init(ParamRack* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	// Bottom
	psy_ui_component_init_align(&self->bottom, &self->component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_scrollbar_init(&self->hscroll, &self->bottom);
	psy_ui_scrollbar_setorientation(&self->hscroll, psy_ui_HORIZONTAL);
	psy_ui_component_setalign(psy_ui_scrollbar_base(&self->hscroll),
		psy_ui_ALIGN_CLIENT);	
	tabbar_init(&self->modeselector, &self->bottom);
	tabbar_append_tabs(&self->modeselector,
		"All", "Inputs", "Outputs", "Inchain", "Outchain", NULL);	
	psy_signal_connect(&self->modeselector.signal_change, self,
		paramrack_onmodeselected);
	psy_ui_component_setalign(tabbar_base(&self->modeselector),
		psy_ui_ALIGN_RIGHT);
	// Pane
	paramrackpane_init(&self->pane, &self->component, workspace);		
	psy_ui_component_setalign(&self->pane.component,
		psy_ui_ALIGN_VCLIENT);
	psy_ui_component_setmaximumsize(&self->component,
		psy_ui_size_makeem(0.0, 10.0));
	psy_ui_component_setminimumsize(&self->component,
		psy_ui_size_makeem(0.0, 10.0));	
	// connect scrollbar
	psy_signal_connect(&self->hscroll.signal_changed, self,
		paramrack_horizontal_onchanged);
	psy_ui_scrollbar_setscrollrange(&self->hscroll, 0, 100);
	psy_signal_connect(&self->pane.component.signal_scrollrangechanged, self,
		paramrack_scrollrangechanged);
	psy_signal_connect(&self->component.signal_align, self,
		paramrack_onalign);	
	tabbar_select(&self->modeselector, (uintptr_t)self->pane.mode);
}

void paramrack_horizontal_onchanged(ParamRack* self, psy_ui_ScrollBar* sender)
{
	psy_ui_RealRectangle position;
	double nPos;
	psy_ui_Value step;
	double steppx;
		
	nPos = psy_ui_scrollbar_position(sender);
	position = psy_ui_component_position(&self->pane.component);
	step = psy_ui_component_scrollstepx(&self->pane.component);
	steppx = psy_ui_value_px(&step,
		psy_ui_component_textmetric(&self->component));
	psy_ui_component_move(&self->pane.component,		
		psy_ui_point_makepx(floor(-nPos) * steppx, 0));
}

void paramrack_scrollrangechanged(ParamRack* self, psy_ui_Component* sender,
	psy_ui_Orientation orientation)
{	
	double nPos;

	nPos = psy_ui_scrollbar_position(&self->hscroll);	
	psy_ui_scrollbar_setscrollrange(&self->hscroll,
		(double)sender->hscrollrange.x,
		(double)sender->hscrollrange.y);
	if (nPos > sender->hscrollrange.y) {
		psy_ui_scrollbar_setthumbposition(&self->hscroll, sender->hscrollrange.y);
	}	
}

void paramrack_onmodeselected(ParamRack* self, TabBar* sender,
	intptr_t index)
{	
	paramrackpane_setmode(&self->pane, (ParamRackMode)index);
}

void paramrack_onalign(ParamRack* self, psy_ui_Component* sender)
{
	psy_ui_Size limit;
	psy_ui_Size preferredsize;
	double widthpx;
	double visipx;
	const psy_ui_TextMetric* tm;
	psy_ui_Value step;
	double steppx;
	double steps;	

	limit = psy_ui_component_size(&self->component);

	if (psy_ui_component_at(&self->pane.component, 0)) {
		psy_ui_Size preferredboxsize;
		
		preferredboxsize = psy_ui_component_preferredsize(
			psy_ui_component_at(&self->pane.component, 0), &limit);
		psy_ui_component_setscrollstepx(&self->pane.component,
			preferredboxsize.width);
	}
	preferredsize = psy_ui_component_preferredsize(&self->pane.component, &limit);
	tm = psy_ui_component_textmetric(&self->pane.component);
	widthpx = psy_ui_value_px(&preferredsize.width, tm);
	visipx = psy_ui_value_px(&limit.width, tm);
	step = psy_ui_component_scrollstepx(&self->pane.component);
	steppx =
		psy_ui_value_px(&step,
		psy_ui_component_textmetric(&self->pane.component));
	steps = psy_max(0.0, ((widthpx - visipx) / steppx + 0.5));	
	psy_ui_component_sethorizontalscrollrange(&self->pane.component, 0, (intptr_t)steps);	
	if (steps == 0) {
		psy_ui_component_preventinput(&self->hscroll.component, TRUE);
	} else {
		psy_ui_component_enableinput(&self->hscroll.component, TRUE);
	}
}
