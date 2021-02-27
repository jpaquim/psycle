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
			psy_ui_component_dispose);
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
	psy_ui_Component* machineui)
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

psy_ui_Component* machineuimatrix_at(MachineUiMatrix* self, uintptr_t trackidx, uintptr_t line)
{	
	MachineUiTrack* track;

	if (!psy_table_exists(&self->tracks, trackidx)) {
		return NULL;
	}
	track = (MachineUiTrack*)psy_table_at(&self->tracks, trackidx);
	if (!psy_table_exists(track, line)) {
		return NULL;
	}
	return (psy_ui_Component*)psy_table_at(track, line);
}

uintptr_t machineuimatrix_numtracks(const MachineUiMatrix* self)
{
	return psy_table_size(&self->tracks);
}

uintptr_t machineuimatrix_numlines(const MachineUiMatrix* self)
{
	return self->maxlines;
}

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
static void machinestackinputs_ondestroy(MachineStackInputs*);
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
		machinestackinputs_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			machinestackinputs_ondestroy;
		machinestackinputs_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			machinestackinputs_onpreferredsize;
		machinestackinputs_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			machinestackinputs_ondraw;
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
	self->components = NULL;	
}

void machinestackinputs_ondestroy(MachineStackInputs* self)
{
	psy_list_deallocate(&self->components, (psy_fp_disposefunc)
		psy_ui_component_dispose);
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
	psy_list_deallocate(&self->components, (psy_fp_disposefunc)
		psy_ui_component_dispose);
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
				slot, self->skin, &self->component, NULL,
				FALSE, self->workspace);
			psy_list_append(&self->components, rv);
			psy_ui_component_move(rv,
				psy_ui_point_makepx(
					(double)track * 158.0,
					0.0));
		}
		psy_list_free(leafs);
	}
}

void machinestackinputs_ondraw(MachineStackInputs* self, psy_ui_Graphics* g)
{
	psy_List* p;

	for (p = self->components; p != NULL; psy_list_next(&p)) {
		psy_ui_RealRectangle position;
		psy_ui_Component* machineui;

		machineui = (psy_ui_Component*)psy_list_entry(p);
		position = psy_ui_component_position(machineui);
		if (psy_ui_realrectangle_intersect_rectangle(&g->clip, &position)) {
			psy_ui_setorigin(g, psy_ui_realpoint_make(-position.left, -position.top));
			machineui->vtable->ondraw(machineui, g);
			psy_ui_resetorigin(g);
		}
	}
}

// MachineStackPane
// prototypes
static void machinestackpane_ondestroy(MachineStackPane*);
static void machinestackpane_ondraw(MachineStackPane*, psy_ui_Graphics*);
static void machinestackpane_computeoutchainlevels(MachineStackPane*,
	uintptr_t slot, uintptr_t track, uintptr_t* level);
static void machinestackpane_computemaxlevels(MachineStackPane*);
static void machinestackpane_buildmachineuis(MachineStackPane*);
static void machinestackpane_buildoutchain(MachineStackPane*, uintptr_t slot,
	uintptr_t track, uintptr_t* line);
static void machinestackpane_removeall(MachineStackPane*);
static void machinestackpane_onsongchanged(MachineStackPane*, Workspace*,
	int flag, psy_audio_Song*);
static void machinestackpane_setmachines(MachineStackPane*,
	psy_audio_Machines*);
static void  machinestackpane_drawmachines(MachineStackPane*,
	psy_ui_Graphics*);
static psy_ui_Component* machinestackpane_insert(MachineStackPane*, uintptr_t slot,
	uintptr_t track, uintptr_t line);
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
static void machinestackpane_onpreferredsize(MachineStackPane*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
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
		machinestackpane_vtable.ondraw = (psy_ui_fp_component_ondraw)
			machinestackpane_ondraw;
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
		machinestackpane_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			machinestackpane_onpreferredsize;
		machinestackpane_vtable_initialized = TRUE;
	}
	return &machinestackpane_vtable;
}

void machinestackpane_init(MachineStackPane* self, psy_ui_Component* parent,
	MachineViewSkin* skin, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component, machinestackpane_vtable_init(self));
	self->component.vtable = &machinestackpane_vtable;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	machineuimatrix_init(&self->matrix);
	psy_table_init(&self->maxlevels);
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
	psy_table_dispose(&self->maxlevels);
	machineuimatrix_dispose(&self->matrix);
}

void machinestackpane_ondraw(MachineStackPane* self, psy_ui_Graphics* g)
{
	if (self->machines) {
		machinestackpane_drawmachines(self, g);		
	}
}

void machinestackpane_drawmachines(MachineStackPane* self, psy_ui_Graphics* g)
{	
	uintptr_t i;
	psy_ui_Size size;
	psy_ui_RealSize sizepx;
	psy_ui_RealRectangle r;
	psy_ui_Border border;

	size = psy_ui_component_preferredsize(&self->component, NULL);
	sizepx = psy_ui_size_px(&size, psy_ui_component_textmetric(&self->component));
	psy_ui_border_init_style(&border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&border, psy_ui_colour_make(0x000151515));
	psy_ui_colour_set(&border.colour_right, psy_ui_colour_make(0x00303030));
	psy_ui_colour_set(&border.colour_bottom, psy_ui_colour_make(0x00303030));	
	for (i = 0; i < machineuimatrix_numtracks(&self->matrix); ++i) {
		uintptr_t j;		
		for (j = 0; j < machineuimatrix_numlines(&self->matrix) + 1; ++j) {
			psy_ui_Component* machineui;

			machineui = machineuimatrix_at(&self->matrix, i, j);
			if (machineui) {
				psy_ui_RealRectangle position;

				position = psy_ui_component_position(machineui); 
				if (j == 0) {
					r = psy_ui_realrectangle_make(
						psy_ui_realpoint_make(position.left - 6, position.top),
						psy_ui_realsize_make(psy_ui_realrectangle_width(&position) + 14, sizepx.height));
					psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00202020));
					psy_ui_drawborder(g, r, &border);					
				}
				if (psy_ui_realrectangle_intersect_rectangle(&g->clip, &position)) {
					psy_ui_setorigin(g, psy_ui_realpoint_make(-position.left, -position.top));										
					machineui->vtable->ondraw(machineui, g);					
					psy_ui_resetorigin(g);
				}
			}
		}
	}
	//psy_ui_settextcolour(g, psy_ui_colour_make(0x00AABBBB));	
	//machineui_drawhighlight(machineui, g);			
}

void machinestackpane_computemaxlevels(MachineStackPane* self)
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

			machinestackpane_computeoutchainlevels(self, (uintptr_t)(p->entry), track, &level);			
		}
		if (psy_table_exists(&self->maxlevels, psy_audio_MASTER_INDEX)) {
			level = (uintptr_t)psy_table_at(&self->maxlevels, psy_audio_MASTER_INDEX);
			psy_table_insert(&self->maxlevels, psy_audio_MASTER_INDEX, (void*)(level + 1));
		}
		psy_list_free(leafs);
	}
}

void machinestackpane_computeoutchainlevels(MachineStackPane* self,
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
				machinestackpane_computeoutchainlevels(self, socket->slot, track,
					level);
				--(*level);
			}
		}
	}
}

void machinestackpane_buildmachineuis(MachineStackPane* self)
{
	if (self->machines) {
		MachineList* leafs;
		MachineList* p;
		uintptr_t track;	
		uintptr_t line;
		
		machineuimatrix_clear(&self->matrix);
		psy_table_clear(&self->maxlevels);		
		machinestackpane_computemaxlevels(self);
		leafs = psy_audio_machines_leafs(self->machines);		
		for (track = 0, p = leafs; p != NULL; psy_list_next(&p), ++track) {
			line = 0;
			machinestackpane_buildoutchain(self, (uintptr_t)(p->entry), track,
				&line);
		}		
		psy_list_free(leafs);
	} 
}

void machinestackpane_buildoutchain(MachineStackPane* self, uintptr_t slot, uintptr_t track,
	uintptr_t* line)
{
	psy_audio_MachineSockets* sockets;
	
	if (*line > 0) {
		machinestackpane_insert(self, slot, track, *line);
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
				machinestackpane_buildoutchain(self, socket->slot, track, line);								
			}
		}
	}
}

psy_ui_Component* machinestackpane_insert(MachineStackPane* self, uintptr_t slot,
	uintptr_t track, uintptr_t line)
{	
	psy_ui_Component* rv;

	rv =  machineui_create(
		psy_audio_machines_at(self->machines, slot),
		slot, self->skin, &self->component, NULL,
		FALSE, self->workspace);
	if (rv) {
		double lineheight;
		double trackwidth;
		uintptr_t maxlevel;
					
		lineheight = 60;
		trackwidth = 158;		
		maxlevel = (uintptr_t)psy_table_at(&self->maxlevels, slot);
		psy_ui_component_move(rv,
			psy_ui_point_make(
				psy_ui_value_makepx((double)track * trackwidth),
				psy_ui_value_makepx((double)line * lineheight)));
		machineuimatrix_insert(&self->matrix, track, line, rv);
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
	machineuimatrix_clear(&self->matrix);
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

void machinestackpane_onpreferredsize(MachineStackPane* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_size_setpx(rv,
		(double)machineuimatrix_numtracks(&self->matrix) * 150.0,
		(double)machineuimatrix_numlines(&self->matrix) * 70.0);
}

void machinestackpane_onmousedown(MachineStackPane* self, psy_ui_MouseEvent* ev)
{	
	uintptr_t track;
	uintptr_t line;

	line = psy_INDEX_INVALID;
	track = psy_INDEX_INVALID;	
	self->dragmachineui = NULL;
	machinestackpane_hittest(self, ev->pt.x, ev->pt.y, &track, &line);
	if (line == psy_INDEX_INVALID) {
	} else if (ev->button == 1) {		
		psy_ui_Component* machineui;

		machineui = machineuimatrix_at(&self->matrix, track, line);
		if (machineui) {	
			machineui->vtable->onmousedown(machineui, ev);
			self->dragmachineui = machineui;
		}
	}
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
	uintptr_t track;
	uintptr_t line;

	line = psy_INDEX_INVALID;
	track = psy_INDEX_INVALID;	
	self->dragmachineui = NULL;
	machinestackpane_hittest(self, ev->pt.x, ev->pt.y, &track, &line);
	if (line == psy_INDEX_INVALID) {
	} else if (ev->button == 1) {
		psy_ui_Component* machineui;

		machineui = machineuimatrix_at(&self->matrix, track, line);
		if (machineui) {
			machineui->vtable->onmousedoubleclick(machineui, ev);			
			self->dragmachineui = machineui;
		}
	}
}

void machinestackpane_hittest(MachineStackPane* self, double x, double y,
	uintptr_t* track, uintptr_t* line)
{	
	uintptr_t i;

	assert(self);

	*track = psy_INDEX_INVALID;
	*line = psy_INDEX_INVALID;
	for (i = 0; i < machineuimatrix_numtracks(&self->matrix); ++i) {
		uintptr_t j;
		for (j = 0; j < machineuimatrix_numlines(&self->matrix) + 1; ++j) {
			psy_ui_Component* machineui;
			psy_ui_RealRectangle r;				

			machineui = (psy_ui_Component*)machineuimatrix_at(&self->matrix, i, j);
			if (machineui) {
				r = psy_ui_component_position(machineui);
				if (psy_ui_realrectangle_intersect(&r,					
						psy_ui_realpoint_make(x, y))) {
					*track = i;
					*line = j;
					break;
				}
			}
		}
		if (*line != psy_INDEX_INVALID) {
			break;
		}
	}	
}

void machinestackpane_updateskin(MachineStackPane* self)
{
	psy_ui_component_setbackgroundcolour(&self->component,
		self->skin->colour);
}

void machinestackpane_ontimer(MachineStackPane* self, uintptr_t timerid)
{	
	bool updatevus;
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
	}	
}

// MachineStackVolumes
// prototypes
void machinestackvolumes_init(MachineStackVolumes* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
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
