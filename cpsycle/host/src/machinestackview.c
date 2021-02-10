// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinestackview.h"
// host
#include "skingraphics.h"
#include "wireview.h"
// audio
#include <exclusivelock.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// MachineUiMatrix
void machineuimatrix_init(MachineUiMatrix* self)
{
	psy_table_init(&self->tracks);
	self->maxlines = 0;
}

void machineuimatrix_dispose(MachineUiMatrix* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->tracks);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		MachineUiTrack* track;

		track = (MachineUiTrack*)psy_tableiterator_value(&it);
		psy_table_disposeall(track, (psy_fp_disposefunc)
			machineui_dispose);
		free(track);
	}
	psy_table_dispose(&self->tracks);
}

void machineuimatrix_clear(MachineUiMatrix* self)
{
	machineuimatrix_dispose(self);
	machineuimatrix_init(self);
	self->maxlines = 0;
}

void machineuimatrix_insert(MachineUiMatrix* self, uintptr_t trackidx, uintptr_t line,
	MachineUi* machineui)
{
	MachineUiTrack* track;
	
	if (!psy_table_exists(&self->tracks, trackidx)) {		
		track = (MachineUiTrack*)malloc(sizeof(MachineUiTrack));
		psy_table_init(track);
		psy_table_insert(&self->tracks, trackidx, track);
	} else {
		track = (MachineUiTrack*)psy_table_at(&self->tracks, trackidx);
	}
	if (self->maxlines < line) {
		self->maxlines = line;
	}
	psy_table_insert(track, line, (void*)machineui);
}

MachineUi* machineuimatrix_at(MachineUiMatrix* self, uintptr_t trackidx, uintptr_t line)
{	
	MachineUiTrack* track;

	if (!psy_table_exists(&self->tracks, trackidx)) {
		return NULL;
	}
	track = (MachineUiTrack*)psy_table_at(&self->tracks, trackidx);
	if (!psy_table_exists(track, line)) {
		return NULL;
	}
	return (MachineUi*)psy_table_at(track, line);
}

uintptr_t machineuimatrix_numtracks(const MachineUiMatrix* self)
{
	return psy_table_size(&self->tracks);
}

uintptr_t machineuimatrix_numlines(const MachineUiMatrix* self)
{
	return self->maxlines;
}

// MachineStackView
// prototypes
static void machinestackview_ondestroy(MachineStackView*);
static void machinestackview_ondraw(MachineStackView*, psy_ui_Graphics*);
static void machinestackview_computeoutchainlevels(MachineStackView*,
	uintptr_t slot, uintptr_t track, uintptr_t* level);
static void machinestackview_computemaxlevels(MachineStackView*);
static void machinestackview_buildmachineuis(MachineStackView*);
static void machinestackview_buildoutchain(MachineStackView*, uintptr_t slot,
	uintptr_t track, uintptr_t* line);
static void machinestackview_removeall(MachineStackView*);
static void machinestackview_onsongchanged(MachineStackView*, Workspace*,
	int flag, psy_audio_Song*);
static void machinestackview_setmachines(MachineStackView*,
	psy_audio_Machines*);
static void  machinestackview_drawmachines(MachineStackView*,
	psy_ui_Graphics* g);
static MachineUi* machinestackview_insert(MachineStackView*, uintptr_t slot,
	uintptr_t track, uintptr_t line);
static MachineUi* machinestackview_at(MachineStackView*, uintptr_t slot);
static void machinestackview_remove(MachineStackView*, uintptr_t slot);
static void machinestackview_onmachineinsert(MachineStackView*,
	psy_audio_Machines*, uintptr_t slot);
static void machinestackview_onmachineremoved(MachineStackView*,
	psy_audio_Machines*, uintptr_t slot);
static void machinestackview_onconnected(MachineStackView*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machinestackview_ondisconnected(MachineStackView*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machinestackview_onpreferredsize(MachineStackView*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void machinestackview_onmousedown(MachineStackView*,
	psy_ui_MouseEvent*);
static void machinestackview_onmousemove(MachineStackView*,
	psy_ui_MouseEvent*);
static void machinestackview_onmouseup(MachineStackView*,
	psy_ui_MouseEvent*);
static void machinestackview_ontimer(MachineStackView*, uintptr_t timerid);
static void machinestackview_hittest(MachineStackView* self,
	double x, double y, uintptr_t* track, uintptr_t* line);
// vtable
static psy_ui_ComponentVtable machinestackview_vtable;
static bool machinestackview_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackview_vtable_init(MachineStackView* self)
{
	if (!machinestackview_vtable_initialized) {
		machinestackview_vtable = *(self->component.vtable);
		machinestackview_vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			machinestackview_ondestroy;
		machinestackview_vtable.ondraw = (psy_ui_fp_component_ondraw)
			machinestackview_ondraw;
		machinestackview_vtable.ontimer = (psy_ui_fp_component_ontimer)
			machinestackview_ontimer;
		machinestackview_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			machinestackview_onmousedown;
		machinestackview_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)
			machinestackview_onmouseup;
		machinestackview_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)
			machinestackview_onmousemove;
		/*machinestackview_vtable.onmousedoubleclick = (psy_ui_fp_component_onmousedoubleclick)
			machinewireview_onmousedoubleclick;
		machinestackview_vtable.onkeydown = (psy_ui_fp_component_onkeydown)
			machinewireview_onkeydown;*/
		machinestackview_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			machinestackview_onpreferredsize;
		machinestackview_vtable_initialized = TRUE;
	}
	return &machinestackview_vtable;
}

void machinestackview_init(MachineStackView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_ui_Scroller* scroller,
	MachineViewSkin* skin, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component, machinestackview_vtable_init(self));
	self->component.vtable = &machinestackview_vtable;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	machineuimatrix_init(&self->matrix);
	psy_table_init(&self->maxlevels);
	self->workspace = workspace;
	self->skin = skin;
	self->dragslot = psy_INDEX_INVALID;
	self->dragmachineui = NULL;
	self->vudrawupdate = FALSE;
	self->opcount = 0;
	machinestackview_updateskin(self);
	psy_ui_component_setfont(&self->component, &self->skin->font);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);
	if (workspace_song(workspace)) {
		machinestackview_setmachines(self,
			psy_audio_song_machines(workspace_song(workspace)));
	} else {
		machinestackview_setmachines(self, NULL);
	}
	machinestackview_buildmachineuis(self);
	psy_signal_connect(&workspace->signal_songchanged, self,
		machinestackview_onsongchanged);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void machinestackview_ondestroy(MachineStackView* self)
{
	psy_table_dispose(&self->maxlevels);
	machineuimatrix_dispose(&self->matrix);
}

void machinestackview_ondraw(MachineStackView* self, psy_ui_Graphics* g)
{
	if (self->machines) {
		machinestackview_drawmachines(self, g);
	}
}

void  machinestackview_drawmachines(MachineStackView* self, psy_ui_Graphics* g)
{	
	uintptr_t i;

	for (i = 0; i < machineuimatrix_numtracks(&self->matrix); ++i) {
		uintptr_t j;
		for (j = 0; j < machineuimatrix_numlines(&self->matrix) + 1; ++j) {
			MachineUi* machineui;

			machineui = (MachineUi*)machineuimatrix_at(&self->matrix, i, j);
			if (machineui) {
				const psy_ui_RealRectangle* position;

				position = machineui_position(machineui);
				if (psy_ui_realrectangle_intersect_rectangle(&g->clip, position)) {
					psy_ui_setorigin(g, psy_ui_realpoint_make(-position->left, -position->top));
					machineui_draw(machineui, g, machineui->slot, FALSE);
					psy_ui_resetorigin(g);
				}
			}
		}
	}
	//machineui_drawhighlight(machineui, g);			
}

void machinestackview_computemaxlevels(MachineStackView* self)
{
	if (self->machines) {
		MachineList* leafs;
		MachineList* p;
		uintptr_t track;
		uintptr_t level;		

		psy_table_clear(&self->maxlevels);
		leafs = psy_audio_machines_leafs(self->machines);
		for (track = 0, p = leafs; p != NULL; psy_list_next(&p), ++track) {
			level = 0;
			machinestackview_computeoutchainlevels(self, (uintptr_t)(p->entry), track, &level);			
		}
		if (psy_table_exists(&self->maxlevels, psy_audio_MASTER_INDEX)) {
			level = (uintptr_t)psy_table_at(&self->maxlevels, psy_audio_MASTER_INDEX);
			psy_table_insert(&self->maxlevels, psy_audio_MASTER_INDEX, (void*)(level + 1));
		}
		psy_list_free(leafs);
	}
}

void machinestackview_computeoutchainlevels(MachineStackView* self,
	uintptr_t slot, uintptr_t track, uintptr_t* level)
{
	psy_audio_MachineSockets* sockets;	
	
	if (!psy_table_exists(&self->maxlevels, slot)) {		
		psy_table_insert(&self->maxlevels, slot, (void*)(*level));		
	} else {
		uintptr_t currlevel;

		currlevel = (uintptr_t)psy_table_at(&self->maxlevels, slot);
		if (currlevel < *level) {
			psy_table_insert(&self->maxlevels, slot, (void*)*level);
		}
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
				++(*level);
				machinestackview_computeoutchainlevels(self, socket->slot, track,
					level);
				--(*level);
			}
		}
	}
}

void machinestackview_buildmachineuis(MachineStackView* self)
{
	if (self->machines) {
		MachineList* leafs;
		MachineList* p;
		uintptr_t track;	
		uintptr_t line;
		
		machineuimatrix_clear(&self->matrix);
		psy_table_clear(&self->maxlevels);		
		machinestackview_computemaxlevels(self);
		leafs = psy_audio_machines_leafs(self->machines);		
		for (track = 0, p = leafs; p != NULL; psy_list_next(&p), ++track) {
			line = 0;
			machinestackview_buildoutchain(self, (uintptr_t)(p->entry), track,
				&line);
		}		
		psy_list_free(leafs);
	} 
}

void machinestackview_buildoutchain(MachineStackView* self, uintptr_t slot, uintptr_t track,
	uintptr_t* line)
{
	psy_audio_MachineSockets* sockets;
	
	machinestackview_insert(self, slot, track, *line);
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
				machinestackview_buildoutchain(self, socket->slot, track, line);								
			}
		}
	}
}

MachineUi* machinestackview_insert(MachineStackView* self, uintptr_t slot,
	uintptr_t track, uintptr_t line)
{	
//	if (psy_audio_machines_at(self->machines, slot)) {
		MachineUi* rv;

		//if (psy_table_exists(&self->machineuis, slot)) {
			//machinestackview_remove(self, slot);
		//}
		rv = (MachineUi*)malloc(sizeof(MachineUi));
		if (rv) {
			double lineheight;
			double trackwidth;
			uintptr_t maxlevel;
			
			machineui_init(rv, slot, self->skin, &self->component, self->workspace);
			lineheight = 70;
			trackwidth = 150;
			rv->machinepos = FALSE;
			maxlevel = (uintptr_t)psy_table_at(&self->maxlevels, slot);
			machineui_move(rv, psy_ui_realpoint_make(
				(double)track * trackwidth, (double)line * lineheight));
			machineuimatrix_insert(&self->matrix, track, line, rv);
		}
		return rv;
	//}
	//return NULL;
}

MachineUi* machinestackview_at(MachineStackView* self, uintptr_t slot)
{
	return 0; // psy_list_at(&self->machineuis, slot);
}

void machinestackview_remove(MachineStackView* self, uintptr_t slot)
{
	//MachineUi* machineui;

	//machineui = (MachineUi*)psy_table_at(&self->machineuis, slot);
	//if (machineui) {
	//	machineui_dispose(machineui);
	//	free(machineui);
	//	psy_table_remove(&self->machineuis, slot);
	//}
}

void machinestackview_removeall(MachineStackView* self)
{
	machineuimatrix_clear(&self->matrix);
}

void machinestackview_onsongchanged(MachineStackView* self, Workspace* workspace,
	int flag, psy_audio_Song* song)
{
	if (song) {
		machinestackview_setmachines(self, psy_audio_song_machines(song));
	} else {
		machinestackview_setmachines(self, NULL);
	}
	machinestackview_buildmachineuis(self);
}

void machinestackview_setmachines(MachineStackView* self,
	psy_audio_Machines* machines)
{	
	self->machines = machines;	
	if (self->machines) {
		// psy_signal_connect(&self->machines->signal_slotchange, self,
		//	machinewireview_onmachineselected);
		// psy_signal_connect(&self->machines->signal_wireselected, self,
		// 	machinewireview_onwireselected);
		psy_signal_connect(&self->machines->signal_insert, self,
			machinestackview_onmachineinsert);
		psy_signal_connect(&self->machines->signal_removed, self,
			machinestackview_onmachineremoved);
		psy_signal_connect(&self->machines->connections.signal_connected, self,
			machinestackview_onconnected);
		psy_signal_connect(&self->machines->connections.signal_disconnected, self,
			machinestackview_ondisconnected);
		//psy_signal_connect(
			//&psy_audio_machines_master(self->machines)->signal_worked, self,
			//machinewireview_onmasterworked);
	}
}

void machinestackview_onmachineinsert(MachineStackView* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	machinestackview_buildmachineuis(self);
}

void machinestackview_onmachineremoved(MachineStackView* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	machinestackview_buildmachineuis(self);
}

void machinestackview_onconnected(MachineStackView* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{
	machinestackview_buildmachineuis(self);
}

void machinestackview_ondisconnected(MachineStackView* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{
	machinestackview_buildmachineuis(self);
}

void machinestackview_onpreferredsize(MachineStackView* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_size_setpx(rv,
		(double)machineuimatrix_numtracks(&self->matrix) * 150.0,
		(double)machineuimatrix_numlines(&self->matrix) * 70.0);
}

void machinestackview_onmousedown(MachineStackView* self, psy_ui_MouseEvent* ev)
{	
	uintptr_t track;
	uintptr_t line;

	line = psy_INDEX_INVALID;
	track = psy_INDEX_INVALID;
	self->dragslot = psy_INDEX_INVALID;
	self->dragmachineui = NULL;
	machinestackview_hittest(self, ev->pt.x, ev->pt.y, &track, &line);
	if (line == psy_INDEX_INVALID) {
	} else if (ev->button == 1) {		
		MachineUi* machineui;

		machineui = machineuimatrix_at(&self->matrix, track, line);
		if (machineui && machineui->slot != psy_audio_MASTER_INDEX) {				
			machineui_onmousedown(machineui, ev);			
			self->dragslot = machineui->slot;
			self->dragmachineui = machineui;
		}
	}
}

void machinestackview_onmousemove(MachineStackView* self, psy_ui_MouseEvent* ev)
{	
	if (self->dragmachineui != NULL) {			
		machineui_onmousemove(self->dragmachineui, ev);
		if (!ev->bubble) {
			return;		
		}
	}
}

void machinestackview_onmouseup(MachineStackView* self, psy_ui_MouseEvent* ev)
{
	self->dragslot = psy_INDEX_INVALID;
	self->dragmachineui = NULL;
}

void machinestackview_hittest(MachineStackView* self, double x, double y,
	uintptr_t* track, uintptr_t* line)
{	
	uintptr_t i;

	*track = psy_INDEX_INVALID;
	*line = psy_INDEX_INVALID;
	for (i = 0; i < machineuimatrix_numtracks(&self->matrix); ++i) {
		uintptr_t j;
		for (j = 0; j < machineuimatrix_numlines(&self->matrix) + 1; ++j) {
			MachineUi* machineui;

			machineui = (MachineUi*)machineuimatrix_at(&self->matrix, i, j);			
			if (machineui && psy_ui_realrectangle_intersect(
					machineui_position(machineui),
					psy_ui_realpoint_make(x, y))) {
				*track = i;
				*line = j;
				break;
			}
		}
		if (*line != psy_INDEX_INVALID) {
			break;
		}
	}	
}

void machinestackview_updateskin(MachineStackView* self)
{
	psy_ui_component_setbackgroundcolour(&self->component,
		self->skin->colour);
}

void machinestackview_ontimer(MachineStackView* self, uintptr_t timerid)
{	
	bool updatevus;
	uintptr_t i;
	
	self->vudrawupdate = TRUE;
	updatevus = psy_ui_component_drawvisible(&self->component);
	if (updatevus) {
		psy_ui_component_setbackgroundmode(&self->component,
			psy_ui_BACKGROUND_NONE);
	}	
	for (i = 0; i < machineuimatrix_numtracks(&self->matrix); ++i) {
		uintptr_t j;
		for (j = 0; j < machineuimatrix_numlines(&self->matrix) + 1; ++j) {
			MachineUi* machineui;

			machineui = (MachineUi*)machineuimatrix_at(&self->matrix, i, j);
			if (machineui && machineui->slot != psy_audio_MASTER_INDEX) {
				if (updatevus) {
					machineui_updatevolumedisplay(machineui);					
					machineui_invalidate(machineui, TRUE);
				}
			}
		}
	}
	if (updatevus) {
		psy_ui_component_update(&self->component);
		psy_ui_component_setbackgroundmode(&self->component,
			psy_ui_BACKGROUND_SET);
	}
	if (self->machines && self->opcount != self->machines->opcount) {
		psy_ui_component_invalidate(&self->component);
		self->opcount = self->machines->opcount;
	}
	self->vudrawupdate = FALSE;
}
