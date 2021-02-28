// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinestackview.h"
// host
#include "skingraphics.h"
#include "paramview.h"
#include "wireview.h"
// audio
#include <exclusivelock.h>
// std
#include <math.h>
// ui
#include <uiviewcomponentimp.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// MachineStackLabels
void machinestackdesc_init(MachineStackDesc* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);	
	psy_ui_label_init_text(&self->inputs, &self->component, "Inputs");
	psy_ui_component_setalign(&self->inputs.component, psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->effects, &self->component, "Effects");
	psy_ui_component_setalign(&self->effects.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setcolour(&self->component,
		psy_ui_colour_make(0x00AABBBB));
}

// MachineStackInputs
// prototypes
static void machinestackinputs_onpreferredsize(MachineStackInputs*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void machinestackinputs_buildmachineuis(MachineStackInputs*);
static void machinestackinputs_ondraw(MachineStackInputs*, psy_ui_Graphics*);
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
		machinestackinputs_vtable_initialized = TRUE;
	}
	return &machinestackinputs_vtable;
}
// implementation
void machinestackinputs_init(MachineStackInputs* self, psy_ui_Component* parent,
	psy_audio_Machines* machines, MachineViewSkin* skin, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component, machinestackinputs_vtable_init(self));
	self->component.vtable = &machinestackinputs_vtable;
	self->machines = machines;
	self->skin = skin;
	self->workspace = workspace;
}

void machinestackinputs_onpreferredsize(MachineStackInputs* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_size_setpx(rv, 10 * 170.0, 70.0);
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
		MachineList* leafs;
		MachineList* p;
		uintptr_t track;		
		
		leafs = psy_audio_machines_leafs(self->machines);
		for (track = 0, p = leafs; p != NULL; psy_list_next(&p), ++track) {
			uintptr_t slot;

			slot = (uintptr_t)psy_list_entry(p);
			psy_ui_Component* rv;

			rv = machineui_create(
				psy_audio_machines_at(self->machines, slot),
				slot, self->skin, &self->component, &self->component, NULL,
				FALSE, self->workspace);
			psy_ui_component_setalign(rv, psy_ui_ALIGN_LEFT);
			//psy_ui_component_move(rv,
			//	psy_ui_point_makepx(
			//		(double)track * 158.0,
			//		0.0));
		}
		psy_list_free(leafs);
	}
	psy_ui_component_align(&self->component);
}


// MachineStackPane
// prototypes
static void machinestackpane_ondestroy(MachineStackPane*);
static void machinestackpane_buildmachineuis(MachineStackPane*);
static void machinestackpane_buildoutchain(MachineStackPane*, uintptr_t slot,
	uintptr_t track, psy_ui_Component* trackpane, uintptr_t* line);
static void machinestackpane_removeall(MachineStackPane*);
static void machinestackpane_onsongchanged(MachineStackPane*, Workspace*,
	int flag, psy_audio_Song*);
static void machinestackpane_setmachines(MachineStackPane*,
	psy_audio_Machines*);
static psy_ui_Component* machinestackpane_insert(MachineStackPane*, uintptr_t slot,
	uintptr_t track, uintptr_t line, psy_ui_Component* trackpane);
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
static void machinestackpane_onmousedown(MachineStackPane*,
	psy_ui_MouseEvent*);
static void machinestackpane_onmousemove(MachineStackPane*,
	psy_ui_MouseEvent*);
static void machinestackpane_onmouseup(MachineStackPane*,
	psy_ui_MouseEvent*);
static void machinestackpane_onmousedoubleclick(MachineStackPane*,
	psy_ui_MouseEvent*);
static void machinestackpane_ontimer(MachineStackPane*, uintptr_t timerid);
static void machinestackpane_hittest(MachineStackPane* self,
	double x, double y, uintptr_t* track, uintptr_t* line);
// vtable
static psy_ui_ComponentVtable machinestackpane_vtable;
static bool machinestackpane_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackpane_vtable_init(MachineStackPane* self)
{
	if (!machinestackpane_vtable_initialized) {
		machinestackpane_vtable = *(self->component.vtable);
		machinestackpane_vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			machinestackpane_ondestroy;
		machinestackpane_vtable.ontimer = (psy_ui_fp_component_ontimer)
			machinestackpane_ontimer;
		machinestackpane_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			machinestackpane_onmousedown;
		machinestackpane_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)
			machinestackpane_onmouseup;
		machinestackpane_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)
			machinestackpane_onmousemove;
		machinestackpane_vtable.onmousedoubleclick = (psy_ui_fp_component_onmouseevent)
			machinestackpane_onmousedoubleclick;
		/*machinestackpane_vtable.onkeydown = (psy_ui_fp_component_onkeydown)
			machinewireview_onkeydown;*/
		machinestackpane_vtable_initialized = TRUE;
	}
	return &machinestackpane_vtable;
}

void machinestackpane_init(MachineStackPane* self, psy_ui_Component* parent,
	MachineViewSkin* skin, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component,
		machinestackpane_vtable_init(self));
	self->component.vtable = &machinestackpane_vtable;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	self->workspace = workspace;
	self->skin = skin;	
	self->dragmachineui = NULL;
	self->vudrawupdate = FALSE;
	self->opcount = 0;
	machinestackpane_updateskin(self);	
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);
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
}

void machinestackpane_buildmachineuis(MachineStackPane* self)
{
	if (self->machines) {
		MachineList* leafs;
		MachineList* p;
		uintptr_t track;	
		uintptr_t line;
				
		psy_ui_component_clear(&self->component);		
		leafs = psy_audio_machines_leafs(self->machines);		
		for (track = 0, p = leafs; p != NULL; psy_list_next(&p), ++track) {
			MachineStackPaneTrack* trackpane;
			line = 0;

			trackpane = (MachineStackPaneTrack*)malloc(sizeof(MachineStackPaneTrack));
			if (trackpane) {
				machinestackpanetrack_init(trackpane, &self->component, &self->component);				
				trackpane->component.deallocate = TRUE;
				psy_ui_component_setalign(&trackpane->component, psy_ui_ALIGN_LEFT);
				machinestackpane_buildoutchain(self, (uintptr_t)(p->entry), track,
					&trackpane->component, &line);
				psy_ui_component_align(&trackpane->component);
			}			
		}		
		psy_list_free(leafs);
		psy_ui_component_align(&self->component);
	} 
}

void machinestackpane_buildoutchain(MachineStackPane* self, uintptr_t slot, uintptr_t track,
	psy_ui_Component* trackpane,
	uintptr_t* line)
{
	psy_audio_MachineSockets* sockets;
	
	if (*line > 0) {
		machinestackpane_insert(self, slot, track, *line, trackpane);
	}
	sockets = psy_audio_connections_at(&self->machines->connections, slot);
	if (sockets) {
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->outputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (socket->slot != psy_INDEX_INVALID) {						
				++(*line);
				machinestackpane_buildoutchain(self, socket->slot, track,
					trackpane, line);
			}
		}
	}
}

psy_ui_Component* machinestackpane_insert(MachineStackPane* self, uintptr_t slot,
	uintptr_t track, uintptr_t line, psy_ui_Component* trackpane)
{	
	psy_ui_Component* rv;

	rv =  machineui_create(
		psy_audio_machines_at(self->machines, slot),
		slot, self->skin, trackpane,
		&self->component, NULL,
		FALSE, self->workspace);
	if (rv) {		
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
		// psy_signal_connect(&self->machines->signal_wireselected, self,
		// 	machinewireview_onwireselected);
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

void machinestackpane_onmousedown(MachineStackPane* self, psy_ui_MouseEvent* ev)
{	
}

void machinestackpane_onmousemove(MachineStackPane* self, psy_ui_MouseEvent* ev)
{	
	if (self->dragmachineui != NULL) {			
		self->dragmachineui->vtable->onmousemove(self->dragmachineui, ev);		
		if (!ev->bubble) {
			return;		
		}
	}
}

void machinestackpane_onmouseup(MachineStackPane* self, psy_ui_MouseEvent* ev)
{	
	self->dragmachineui = NULL;
}

void machinestackpane_onmousedoubleclick(MachineStackPane* self,
	psy_ui_MouseEvent* ev)
{
}

void machinestackpane_hittest(MachineStackPane* self, double x, double y,
	uintptr_t* track, uintptr_t* line)
{
	assert(self);

	*track = psy_INDEX_INVALID;
	*line = psy_INDEX_INVALID;
}

void machinestackpane_updateskin(MachineStackPane* self)
{
	psy_ui_component_setbackgroundcolour(&self->component,
		self->skin->colour);
}

void machinestackpane_ontimer(MachineStackPane* self, uintptr_t timerid)
{	
/*	bool updatevus;
	uintptr_t i;
	
	self->vudrawupdate = TRUE;	
	updatevus = psy_ui_component_drawvisible(&self->component);
	if (updatevus) {
		psy_ui_component_setbackgroundmode(&self->component,
			psy_ui_BACKGROUND_NONE);		
		for (i = 0; i < machineuimatrix_numtracks(&self->matrix); ++i) {
			uintptr_t j;
			for (j = 0; j < machineuimatrix_numlines(&self->matrix) + 1; ++j) {
				psy_ui_Component* machineui;

				machineui = machineuimatrix_at(&self->matrix, i, j);				
				if (machineui && updatevus) {
					psy_ui_component_invalidate(machineui);				
				}
			}
		}	
		psy_ui_component_update(&self->component);
		psy_ui_component_setbackgroundmode(&self->component,
			psy_ui_BACKGROUND_SET);
		machineui_endvuupdate();
		self->vudrawupdate = FALSE;
	}
	if (self->machines && self->opcount != self->machines->opcount) {
		psy_ui_component_invalidate(&self->component);
		self->opcount = self->machines->opcount;
	}	*/
}

// MachineStackVolumes
// prototypes
void machinestackvolumes_init(MachineStackVolumes* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
}

// MachineStackPaneTracke
void machinestackpanetrack_init(MachineStackPaneTrack* self, psy_ui_Component* parent, psy_ui_Component* view)
{
	self->component.imp = (psy_ui_ComponentImp*)
		psy_ui_viewcomponentimp_allocinit(
			&self->component, view->imp, view, "", 0, 0, 100, 100, 0, 0);
	psy_ui_component_init_imp(&self->component, view,
		self->component.imp);
}

// MachineStackView
// prototypes
static void machinestackview_onsongchanged(MachineStackView*, Workspace*,
	int flag, psy_audio_Song*);
// implementation
void machinestackview_init(MachineStackView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent,
	MachineViewSkin* skin, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	machinestackdesc_init(&self->desc, &self->component);
	psy_ui_component_setalign(&self->desc.component, psy_ui_ALIGN_LEFT);
	machinestackinputs_init(&self->inputs, &self->component,
		(workspace->song)
		? &workspace->song->machines
		: NULL,
		skin, workspace);
	psy_ui_component_setalign(&self->inputs.component, psy_ui_ALIGN_TOP);
	machinestackpane_init(&self->pane, &self->component, skin, workspace);
	psy_ui_scroller_init(&self->scroller, &self->pane.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	machinestackvolumes_init(&self->volumes, &self->component);
	psy_ui_component_setalign(&self->volumes.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&workspace->signal_songchanged, self,
		machinestackview_onsongchanged);
	psy_ui_component_setfont(&self->desc.component, &skin->font);
	psy_ui_component_setfont(&self->inputs.component, &skin->font);
	psy_ui_component_setfont(&self->pane.component, &skin->font);
}

void machinestackview_onsongchanged(MachineStackView* self, Workspace* workspace,
	int flag, psy_audio_Song* song)
{
	if (song) {
		machinestackinputs_setmachines(&self->inputs,
			psy_audio_song_machines(song));
	} else {
		machinestackinputs_setmachines(&self->inputs, NULL);
	}		
}
