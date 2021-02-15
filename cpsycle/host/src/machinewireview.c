// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinewireview.h"
// host
#include "resources/resource.h"
#include "skingraphics.h"
#include "wireview.h"
#include "machineview.h"
#include "machineviewbar.h"
#include "miniview.h"
// audio
#include <exclusivelock.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// MachineWireView
// prototypes
static void machinewireview_ondestroy(MachineWireView*);
static psy_ui_RealRectangle machinewireview_bounds(MachineWireView*);
static void machinewireview_setmachines(MachineWireView*, psy_audio_Machines*);
static void machinewireview_ondraw(MachineWireView*, psy_ui_Graphics*);
static void machinewireview_drawdragwire(MachineWireView*, psy_ui_Graphics*);
static bool machinewireview_wiredragging(const MachineWireView*);
static void machinewireview_drawwires(MachineWireView*, psy_ui_Graphics*);
static void machinewireview_drawwire(MachineWireView*, psy_ui_Graphics*,
	MachineUi*);
static void machinewireview_drawwirearrow(MachineWireView*, psy_ui_Graphics*,
	double x1, double y1, double x2, double y2);
static psy_ui_RealPoint rotate_point(psy_ui_RealPoint, double phi);
static psy_ui_RealPoint move_point(psy_ui_RealPoint pt, psy_ui_RealPoint d);
static void machinewireview_onmousedown(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onmouseup(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onmousemove(MachineWireView*, psy_ui_MouseEvent*);
static bool machinewireview_movemachine(MachineWireView*, uintptr_t slot,
	double dx, double dy);
static void machinewireview_onmousedoubleclick(MachineWireView*,
	psy_ui_MouseEvent*);
static void machinewireview_onkeydown(MachineWireView*, psy_ui_KeyEvent*);
static uintptr_t machinewireview_machineleft(MachineWireView*, uintptr_t src);
static uintptr_t machinewireview_machineright(MachineWireView*, uintptr_t src);
static uintptr_t machinewireview_machineup(MachineWireView*, uintptr_t src);
static uintptr_t machinewireview_machinedown(MachineWireView*, uintptr_t src);
static uintptr_t machinewireview_hittest(const MachineWireView*);
static int machinewireview_hittestpan(MachineWireView*, double x, double y,
	uintptr_t slot, double* dx);
static bool machinewireview_hittestcoord(MachineWireView*, double x, double y,
	int mode, SkinCoord*);
static psy_audio_Wire machinewireview_hittestwire(MachineWireView*,
	double x, double y);
static int machinewireview_hittesteditname(MachineWireView*, psy_ui_RealPoint,
	uintptr_t slot);
static bool machinewireview_dragging_machine(const MachineWireView*);
static bool machinewireview_dragging_connection(const MachineWireView*);
static bool machinewireview_dragging_newconnection(const MachineWireView*);
static void machinewireview_onmachineselected(MachineWireView*,
	psy_audio_Machines*, uintptr_t slot);
static void machinewireview_onwireselected(MachineWireView*,
	psy_audio_Machines* sender, psy_audio_Wire);
static void machinewireview_onmachineinsert(MachineWireView*,
	psy_audio_Machines*, uintptr_t slot);
static void machinewireview_onmachineremoved(MachineWireView*,
	psy_audio_Machines*, uintptr_t slot);
static void machinewireview_onconnected(MachineWireView*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machinewireview_ondisconnected(MachineWireView*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machinewireview_onsongchanged(MachineWireView*, Workspace*,
	int flag, psy_audio_Song*);
static void machinewireview_buildmachineuis(MachineWireView*);
static void machinewireview_onshowparameters(MachineWireView*, Workspace*,
	uintptr_t slot);
static void machinewireview_onmasterworked(MachineWireView*,
	psy_audio_Machine*, uintptr_t slot, psy_audio_BufferContext*);
static void machinewireview_ontimer(MachineWireView*, uintptr_t timerid);
static void machinewireview_destroywireframes(MachineWireView*);
static void machinewireview_showwireview(MachineWireView*, psy_audio_Wire);
static void machinewireview_onwireframedestroyed(MachineWireView*,
	psy_ui_Component* sender);
static WireFrame* machinewireview_wireframe(MachineWireView*, psy_audio_Wire);
static psy_ui_RealRectangle machinewireview_updaterect(MachineWireView*,
	uintptr_t slot);
static void machinewireview_onpreferredsize(MachineWireView*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static MachineUi* machineuis_insert(MachineWireView*, uintptr_t slot);
static MachineUi* machineuis_at(MachineWireView*, uintptr_t slot);
static void machineuis_remove(MachineWireView*, uintptr_t slot);
static void machineuis_removeall(MachineWireView*);

static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(MachineWireView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			machinewireview_ondestroy;
		vtable.ondraw = (psy_ui_fp_component_ondraw)machinewireview_ondraw;
		vtable.ontimer = (psy_ui_fp_component_ontimer)machinewireview_ontimer;
		vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			machinewireview_onmousedown;
		vtable.onmouseup = (psy_ui_fp_component_onmouseevent)
			machinewireview_onmouseup;
		vtable.onmousemove = (psy_ui_fp_component_onmouseevent)
			machinewireview_onmousemove;
		vtable.onmousedoubleclick = (psy_ui_fp_component_onmouseevent)
			machinewireview_onmousedoubleclick;
		vtable.onkeydown = (psy_ui_fp_component_onkeyevent)
			machinewireview_onkeydown;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			machinewireview_onpreferredsize;
		vtable_initialized = TRUE;
	}
	return &vtable;
}

void machinewireview_init(MachineWireView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, MachineViewSkin* skin,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component, vtable_init(self));	
	self->component.vtable = &vtable;	
	self->opcount = 0;
	self->component.scrollstepx = psy_ui_value_makepx(10);
	self->component.scrollstepy = psy_ui_value_makepx(10);
	self->statusbar = NULL;
	self->machines = NULL;
	self->workspace = workspace;
	self->machines = &workspace->song->machines;	
	self->wireframes = 0;
	self->randominsert = 1;
	self->addeffect = 0;
	self->vudrawupdate = FALSE;
	self->showwirehover = FALSE;
	self->drawvirtualgenerators = FALSE;	
	self->skin = skin;	
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	psy_table_init(&self->machineuis);	
	psy_ui_component_setfont(&self->component, &self->skin->font);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	self->dragslot = psy_INDEX_INVALID;
	self->dragmode = MACHINEVIEW_DRAG_MACHINE;
	self->selectedslot = psy_audio_MASTER_INDEX;	
	psy_audio_wire_init(&self->selectedwire);
	psy_audio_wire_init(&self->hoverwire);
	psy_signal_connect(&workspace->signal_songchanged, self,
		machinewireview_onsongchanged);	
	if (workspace_song(workspace)) {
		machinewireview_setmachines(self,
			psy_audio_song_machines(workspace_song(workspace)));
	} else {
		machinewireview_setmachines(self, NULL);
	}
	psy_signal_connect(&workspace->signal_showparameters, self,
		machinewireview_onshowparameters);	
	psy_ui_edit_init(&self->editname, &self->component);
	psy_ui_component_hide(&self->editname.component);
	machinewireview_updateskin(self);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void machinewireview_setmachines(MachineWireView* self,
	psy_audio_Machines* machines)
{	
	self->machines = machines;
	if (self->machines) {
		psy_signal_connect(&self->machines->signal_slotchange, self,
			machinewireview_onmachineselected);
		psy_signal_connect(&self->machines->signal_wireselected, self,
			machinewireview_onwireselected);		
		psy_signal_connect(&self->machines->signal_insert, self,
			machinewireview_onmachineinsert);
		psy_signal_connect(&self->machines->signal_removed, self,
			machinewireview_onmachineremoved);
		psy_signal_connect(&self->machines->connections.signal_connected, self,
			machinewireview_onconnected);
		psy_signal_connect(&self->machines->connections.signal_disconnected, self,
			machinewireview_ondisconnected);
		psy_signal_connect(
			&psy_audio_machines_master(self->machines)->signal_worked, self,
			machinewireview_onmasterworked);
	}
	machinewireview_buildmachineuis(self);
}

void machinewireview_ondestroy(MachineWireView* self)
{	
	if (self->machines) {
		psy_signal_disconnect_context(
			&psy_audio_machines_master(self->machines)->signal_worked, self);
	}	
	psy_table_disposeall(&self->machineuis, (psy_fp_disposefunc)
		machineui_dispose);
	psy_list_deallocate(&self->wireframes, (psy_fp_disposefunc)
		psy_ui_component_destroy);	
}

void machinewireview_configure(MachineWireView* self, MachineViewConfig* config)
{	
	self->skin->drawvumeters = machineviewconfig_vumeters(config);
	self->skin->drawmachineindexes = machineviewconfig_machineindexes(config);
	self->showwirehover = machineviewconfig_wirehover(config);	
	if (machineviewconfig_virtualgenerators(config)) {
		machinewireview_showvirtualgenerators(self);
	} else {
		machinewireview_hidevirtualgenerators(self);
	}
}

void machinewireview_updateskin(MachineWireView* self)
{
	psy_ui_component_setbackgroundcolour(&self->component,
		self->skin->colour);
	machinewireview_buildmachineuis(self);
}

void machinewireview_ondraw(MachineWireView* self, psy_ui_Graphics* g)
{	
	if (!self->vudrawupdate) {
		machinewireview_drawwires(self, g);
	}
	machinewireview_drawmachines(self, g);
	if (!self->vudrawupdate) {
		machinewireview_drawdragwire(self, g);
	}
}

void machinewireview_drawwires(MachineWireView* self, psy_ui_Graphics* g)
{
	psy_TableIterator it;
	
	for (it = psy_table_begin(&self->machineuis); 
			!psy_tableiterator_equal(&it, psy_table_end()); 
			psy_tableiterator_inc(&it)) {
		machinewireview_drawwire(self, g,
			(MachineUi*)psy_tableiterator_value(&it));
	}	
}

void machinewireview_drawwire(MachineWireView* self, psy_ui_Graphics* g,
	MachineUi* machineui)
{		
	psy_audio_MachineSockets* sockets;	
	
	sockets	= psy_audio_connections_at(&self->machines->connections,
		machineui->slot);
	if (sockets) {
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->outputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);		
			if (socket->slot != psy_INDEX_INVALID) {
				MachineUi* inmachineui;				

				inmachineui = machineuis_at(self, socket->slot);
				if (inmachineui && machineui) {
					psy_ui_RealPoint out;
					psy_ui_RealPoint in;
										
					if (self->hoverwire.src == machineui->slot &&
							self->hoverwire.dst == socket->slot) {
						psy_ui_setcolour(g, self->skin->hoverwirecolour);
					} else if (self->selectedwire.src == machineui->slot &&
							self->selectedwire.dst == socket->slot) {
						psy_ui_setcolour(g, self->skin->selwirecolour);
					} else {
						psy_ui_setcolour(g, self->skin->wirecolour);
					}
					out = machineui_centerposition(machineui);
					in = machineui_centerposition(inmachineui);
					psy_ui_drawline(g, out, in);
					machinewireview_drawwirearrow(self, g, out.x, out.y,
						in.x, in.y);
				}
			}
		}
	}
}

void machinewireview_drawwirearrow(MachineWireView* self, psy_ui_Graphics* g,
	double x1, double y1, double x2, double y2)
{			
	psy_ui_RealPoint center;
	psy_ui_RealPoint a, b, c;	
	psy_ui_RealPoint tri[4];
	double polysize;
	float deltaColR = ((self->skin->polycolour.value     & 0xFF) / 510.0f) + .45f;
	float deltaColG = ((self->skin->polycolour.value >>8  & 0xFF) / 510.0f) + .45f;
	float deltaColB = ((self->skin->polycolour.value>>16 & 0xFF) / 510.0f) + .45f;
	unsigned int polyInnards;
	double phi;
	
	polyInnards = psy_ui_colour_make_rgb((uint8_t)(192 * deltaColR),
		(uint8_t)(192 * deltaColG), (uint8_t)(192 * deltaColB)).value;
			
	center.x = (x2 - x1) / 2 + x1;
	center.y = (y2 - y1) / 2 + y1;

	polysize = self->skin->triangle_size;
	a.x = -polysize/2;
	a.y = polysize/2;
	b.x = polysize/2;
	b.y = polysize/2;
	c.x = 0;
	c.y = -polysize/2;

	phi = atan2(x2 - x1, y1 - y2);
	
	tri[0] = move_point(rotate_point(a, phi), center);
	tri[1] = move_point(rotate_point(b, phi), center);
	tri[2] = move_point(rotate_point(c, phi), center);
	tri[3] = tri[0];
	
	psy_ui_drawsolidpolygon(g, tri, 4, polyInnards, self->skin->wireaacolour.value);
}

psy_ui_RealPoint rotate_point(psy_ui_RealPoint pt, double phi)
{	
	return psy_ui_realpoint_make(
		cos(phi) * pt.x - sin(phi) * pt.y,
		sin(phi) * pt.x + cos(phi) * pt.y);	
}

psy_ui_RealPoint move_point(psy_ui_RealPoint pt, psy_ui_RealPoint d)
{
	return psy_ui_realpoint_make(pt.x + d.x, pt.y + d.y);	
}

void machinewireview_drawdragwire(MachineWireView* self, psy_ui_Graphics* g)
{
	if (machinewireview_wiredragging(self)) {
		MachineUi* machineui;

		machineui = machineuis_at(self, self->dragslot);
		if (machineui) {			
			psy_ui_setcolour(g, self->skin->wirecolour);
			psy_ui_drawline(g, machineui_centerposition(machineui),
				self->dragpt);
		}
	}
}

bool machinewireview_wiredragging(const MachineWireView* self)
{
	return self->dragslot != psy_INDEX_INVALID &&
		self->dragmode >= MACHINEVIEW_DRAG_NEWCONNECTION &&
		self->dragmode <= MACHINEVIEW_DRAG_RIGHTCONNECTION;
}

void machinewireview_drawmachines(MachineWireView* self, psy_ui_Graphics* g)
{
	psy_TableIterator it;
	
	for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {				
		MachineUi* machineui;
		const psy_ui_RealRectangle* position;

		machineui = (MachineUi*)psy_tableiterator_value(&it);
		position = machineui_position(machineui);
		if (psy_ui_realrectangle_intersect_rectangle(&g->clip, position)) {
			psy_ui_setorigin(g, psy_ui_realpoint_make(-position->left,
				-position->top));
			machineui_draw(machineui, g, psy_tableiterator_key(&it),
				self->vudrawupdate);
			psy_ui_resetorigin(g);
			if (self->selectedwire.src == psy_INDEX_INVALID &&
					self->selectedslot == psy_tableiterator_key(&it)) {
				machineui_drawhighlight(machineui, g);
			}
		}
	}
}


void machinewireview_centermaster(MachineWireView* self)
{
	MachineUi* machineui;	

	machineui = machineuis_at(self, psy_audio_MASTER_INDEX);
	if (machineui) {		
		psy_ui_RealSize machinesize;
		psy_ui_RealSize size;
						
		size = psy_ui_component_sizepx(&self->component);
		machinesize = machineui_size(machineui);
		machineui_move(machineui,
			psy_ui_realpoint_make(
				(size.width - machinesize.width) / 2,
				(size.height - machinesize.height) / 2));
		psy_ui_component_invalidate(machinewireview_base(self));
	}
}

void machinewireview_onmousedoubleclick(MachineWireView* self,
	psy_ui_MouseEvent* ev)
{	
	if (ev->button == 1) {		
		self->dragpt = psy_ui_realpoint_make(ev->pt.x, ev->pt.y);
		self->dragslot = machinewireview_hittest(self);		
		if (self->dragslot == psy_INDEX_INVALID) {
			self->selectedwire = machinewireview_hittestwire(self, ev->pt.x,
				ev->pt.y);
			if (psy_audio_wire_valid(&self->selectedwire)) {
				machinewireview_showwireview(self, self->selectedwire);
				psy_ui_component_invalidate(&self->component);
			} else {				
				self->randominsert = 0;
				return;
			}
		} else if (machinewireview_hittesteditname(self,
				psy_ui_realpoint_make(ev->pt.x, ev->pt.y), self->dragslot)) {
			if (machineuis_at(self, self->dragslot)) {								
				machineui_editname(machineuis_at(self, self->dragslot),
					&self->editname,					
					psy_ui_component_scrollpx(&self->component));				
			}
		} else if (machinewireview_hittestcoord(self, ev->pt.x, ev->pt.y,
				MACHMODE_GENERATOR, &self->skin->generator.solo) ||
			machinewireview_hittestcoord(self, ev->pt.x, ev->pt.y, MACHMODE_FX,
				&self->skin->effect.bypass) ||
			machinewireview_hittestcoord(self, ev->pt.x, ev->pt.y,
				MACHMODE_GENERATOR, &self->skin->generator.mute) ||
			machinewireview_hittestcoord(self, ev->pt.x, ev->pt.y, MACHMODE_FX,
				&self->skin->effect.mute) ||
			machinewireview_hittestpan(self, ev->pt.x, ev->pt.y,
				self->dragslot, &self->dragpt.x)) {
		} else {
			workspace_showparameters(self->workspace, self->dragslot);
		}
		self->dragslot = psy_INDEX_INVALID;
		psy_ui_mouseevent_stoppropagation(ev);
	}
}

void machinewireview_onmousedown(MachineWireView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_hide(&self->editname.component);
	psy_ui_component_setfocus(&self->component);
	self->dragpt = psy_ui_realpoint_make(ev->pt.x, ev->pt.y);
	self->mousemoved = FALSE;		
	self->dragmode = MACHINEVIEW_DRAG_NONE;
	self->dragslot = machinewireview_hittest(self);
	self->dragmachineui = machineuis_at(self, self->dragslot);
	if (ev->button == 1) {
		if (self->dragslot != psy_audio_MASTER_INDEX) {
			self->selectedslot = self->dragslot;
			psy_audio_wire_invalidate(&self->selectedwire);
			psy_audio_machines_select(self->machines, self->selectedslot);
		}
		if (self->dragmachineui) {
			machineui_onmousedown(self->dragmachineui, ev);
			if (ev->bubble) {
				self->dragmode = MACHINEVIEW_DRAG_MACHINE;
				self->dragpt = psy_ui_realpoint_make(
					ev->pt.x - machineui_position(self->dragmachineui)->left,
					ev->pt.y - machineui_position(self->dragmachineui)->top);
				psy_ui_component_capture(&self->component);				
			}
		} else {			
			self->selectedwire = machinewireview_hittestwire(self, ev->pt.x,
				ev->pt.y);			
			if (psy_audio_wire_valid(&self->selectedwire) && ev->shift) {
				self->dragmode = MACHINEVIEW_DRAG_LEFTCONNECTION;
				self->dragslot = self->selectedwire.src;
			}
			if (psy_audio_wire_valid(&self->selectedwire) && ev->ctrl) {
				self->dragmode = MACHINEVIEW_DRAG_RIGHTCONNECTION;
				self->dragslot = self->selectedwire.dst;
			}
			psy_ui_component_invalidate(&self->component);
		}
		psy_ui_mouseevent_stoppropagation(ev);
	} else if (ev->button == 2) {
		if (self->dragmachineui &&
				psy_audio_machine_numoutputs(self->dragmachineui->machine) > 0) {
			self->dragmode = MACHINEVIEW_DRAG_NEWCONNECTION;
			psy_ui_component_capture(&self->component);
			psy_ui_mouseevent_stoppropagation(ev);
		} else {
			self->dragslot = psy_INDEX_INVALID;
		}
	}
}

int machinewireview_hittestpan(MachineWireView* self, double x, double y,
	uintptr_t slot, double* dx)
{	
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui && machineui->coords && machineui->machine) {
		double mx;
		double my;
		psy_ui_RealRectangle r;
		double offset;

		psy_audio_machine_position(machineui->machine, &mx, &my);
		offset = psy_audio_machine_panning(machineui->machine) *
			machineui->coords->pan.range;
		r = skincoord_destposition(&machineui->coords->pan);
		psy_ui_realrectangle_move(&r, offset, 0);		
		*dx = x - mx - r.left;
		return psy_ui_realrectangle_intersect(&r,
			psy_ui_realpoint_make(x - mx, y - my));
	}
	return FALSE;
}

int machinewireview_hittesteditname(MachineWireView* self, psy_ui_RealPoint pt,
	uintptr_t slot)
{		
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui && machineui->coords) {
		psy_ui_RealRectangle r;

		r = machineui_coordposition(machineui, &machineui->coords->name);		
		return psy_ui_realrectangle_intersect(&r, pt);
	}
	return FALSE;
}

bool machinewireview_hittestcoord(MachineWireView* self, double x, double y,
	int mode, SkinCoord* coord)
{
	MachineUi* machineui;

	machineui = machineuis_at(self, self->dragslot);
	if (machineui && machineui->mode == mode) {		
		psy_ui_RealRectangle r;

		r = machineui_coordposition(machineui, coord);		
		return psy_ui_realrectangle_intersect(&r, psy_ui_realpoint_make(x, y));
	}
	return FALSE;
}

uintptr_t machinewireview_hittest(const MachineWireView* self)
{	
	uintptr_t rv;
	psy_TableIterator it;
		
	rv = psy_INDEX_INVALID;
	for (it = psy_table_begin(&(((MachineWireView*)self)->machineuis));
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {		
		if (psy_ui_realrectangle_intersect(machineui_position((MachineUi*)
				psy_tableiterator_value(&it)), self->dragpt)) {
			rv = psy_tableiterator_key(&it);
			break;	
		}
	}
	return rv;
}

void machinewireview_onmousemove(MachineWireView* self, psy_ui_MouseEvent* ev)
{		
	if (!self->mousemoved) {
		if (self->dragpt.x != ev->pt.x || self->dragpt.y != ev->pt.y) {
			self->mousemoved = TRUE;
		} else {
			return;
		}
	}	
	if (self->dragslot != psy_INDEX_INVALID) {
		MachineUi* machineui;
		machineui = machineuis_at(self, self->dragslot);
		if (machineui) {
			machineui_onmousemove(machineui, ev);
			++self->component.opcount;
			if (!ev->bubble) {
				return;
			}
		}
		if (machinewireview_dragging_machine(self)) {
			if (!machinewireview_dragmachine(self, self->dragslot,
					ev->pt.x - self->dragpt.x, ev->pt.y - self->dragpt.y)) {
				self->dragmode = MACHINEVIEW_DRAG_NONE;
			}			
		} else if (machinewireview_dragging_connection(self)) {
			self->dragpt.x = ev->pt.x;
			self->dragpt.y = ev->pt.y;
			psy_ui_component_invalidate(&self->component);
			++self->component.opcount;
		}		
	} else if (self->showwirehover) {
		psy_audio_Wire hoverwire;
		
		hoverwire = machinewireview_hittestwire(self, ev->pt.x, ev->pt.y);
		if (psy_audio_wire_valid(&hoverwire)) {
			MachineUi* machineui;

			machineui = machineuis_at(self, hoverwire.dst);
			if (machineui) {				
				if (psy_ui_realrectangle_intersect(machineui_position(machineui),
					psy_ui_realpoint_make(ev->pt.x, ev->pt.y))) {
					psy_audio_wire_invalidate(&self->hoverwire);
					psy_ui_component_invalidate(&self->component);
					++self->component.opcount;
					return;
				}
			}
			machineui = machineuis_at(self, hoverwire.src);
			if (machineui) {				
				if (psy_ui_realrectangle_intersect(machineui_position(machineui),
						psy_ui_realpoint_make(ev->pt.x, ev->pt.y))) {
					psy_audio_wire_invalidate(&self->hoverwire);
					psy_ui_component_invalidate(&self->component);
					++self->component.opcount;
					return;
				}
			}
		}
		if (!psy_audio_wire_equal(&hoverwire, &self->hoverwire)) {
			self->hoverwire = hoverwire;
			++self->component.opcount;
			psy_ui_component_invalidate(&self->component);
		}		
	}
}

bool machinewireview_dragging_machine(const MachineWireView* self)
{
	return self->dragmode == MACHINEVIEW_DRAG_MACHINE;
}

bool machinewireview_dragging_connection(const MachineWireView* self)
{
	return (self->dragmode >= MACHINEVIEW_DRAG_NEWCONNECTION &&
		self->dragmode <= MACHINEVIEW_DRAG_RIGHTCONNECTION);
}

bool machinewireview_dragging_newconnection(const MachineWireView* self)
{
	return (self->dragmode == MACHINEVIEW_DRAG_NEWCONNECTION);
}

bool machinewireview_movemachine(MachineWireView* self, uintptr_t slot,
	double dx, double dy)
{
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		return machinewireview_dragmachine(self, slot,
			machineui_position(machineui)->left + dx,
			machineui_position(machineui)->top + dy);
	}
	return FALSE;
}

bool machinewireview_dragmachine(MachineWireView* self, uintptr_t slot,
	double x, double y)
{
	MachineUi* machineui;
	psy_ui_RealRectangle r_old;
	psy_ui_RealRectangle r_new;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		r_old = *machineui_position(machineui);
		psy_ui_realrectangle_expand(&r_old, 10.0, 10.0, 10.0, 10.0);
		machineui_move(machineui,
			psy_ui_realpoint_make(
				psy_max(0.0, x), psy_max(0.0, y)));
		if (self->statusbar && machineui->machine) {
			static char txt[128];
			double mx;
			double my;

			psy_audio_machine_position(machineui->machine, &mx, &my);
			psy_snprintf(txt, 128, "%s (%d, %d)",
				(psy_audio_machine_editname(machineui->machine))
				? psy_audio_machine_editname(machineui->machine)
				: "",
				(int)mx, (int)my);
			machineviewbar_settext(self->statusbar, txt);
		}
		r_new = machinewireview_updaterect(self, self->dragslot);
		psy_ui_realrectangle_union(&r_new, &r_old);
		psy_ui_realrectangle_expand(&r_new, 10.0, 10.0, 10.0, 10.0);
		psy_ui_component_invalidaterect(&self->component, r_new);
		return TRUE;
	}
	return FALSE;
}

void machinewireview_onmouseup(MachineWireView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_releasecapture(&self->component);
	if (self->dragslot != psy_INDEX_INVALID) {
		if (machinewireview_dragging_machine(self)) {
			psy_ui_component_updateoverflow(&self->component);
			psy_ui_mouseevent_stoppropagation(ev);
		} else if (machinewireview_dragging_connection(self)) {
			if (self->mousemoved) {
				uintptr_t slot = self->dragslot;
				self->dragslot = machinewireview_hittest(self);
				if (self->dragslot != psy_INDEX_INVALID) {
					if (!machinewireview_dragging_newconnection(self)) {
						psy_audio_machines_disconnect(self->machines,
							self->selectedwire);						
					}
					if (self->dragmode < MACHINEVIEW_DRAG_RIGHTCONNECTION) {
						if (psy_audio_machines_valid_connection(self->machines,
							psy_audio_wire_make(slot, self->dragslot))) {
							psy_audio_machines_connect(self->machines,
								psy_audio_wire_make(slot, self->dragslot));							
						}
					} else if (psy_audio_machines_valid_connection(
						self->machines, psy_audio_wire_make(self->dragslot,
							slot))) {
						psy_audio_machines_connect(self->machines,
							psy_audio_wire_make(self->dragslot, slot));						
					}
				}
				psy_ui_mouseevent_stoppropagation(ev);
			} else if (ev->button == 2) {
				if (!self->workspace->gearvisible) {
					workspace_togglegear(self->workspace);					
				}
				psy_ui_mouseevent_stoppropagation(ev);
			}			
		}
	}
	self->dragslot = psy_INDEX_INVALID;
	psy_ui_component_invalidate(&self->component);
}

void machinewireview_onkeydown(MachineWireView* self, psy_ui_KeyEvent* ev)
{		
	if (ev->ctrl) {
		if (ev->keycode == psy_ui_KEY_B) {
			self->dragwire.src = self->selectedslot;
		} else if (ev->keycode == psy_ui_KEY_E) {
			if (self->dragwire.src != psy_INDEX_INVALID &&
					self->selectedslot != psy_INDEX_INVALID) {
				self->dragwire.dst = self->selectedslot;
				if (!psy_audio_machines_connected(self->machines,
						self->dragwire)) {				
					psy_audio_machines_connect(self->machines, self->dragwire);
				} else {
					self->selectedwire = self->dragwire;
					psy_ui_component_invalidate(&self->component);
				}
			}
		}
	} else if (ev->keycode == psy_ui_KEY_UP) {
		if (ev->shift) {
			machinewireview_movemachine(self, self->selectedslot, 0, -10);
		} else {
			uintptr_t index;

			index = machinewireview_machineup(self, self->selectedslot);
			if (index != psy_INDEX_INVALID) {
				psy_audio_machines_select(self->machines, index);
			}
		}
	} else if (ev->keycode == psy_ui_KEY_DOWN) {
		if (ev->shift) {
			machinewireview_movemachine(self, self->selectedslot, 0, 10);
		} else {
			uintptr_t index;

			index = machinewireview_machinedown(self, self->selectedslot);
			if (index != psy_INDEX_INVALID) {
				psy_audio_machines_select(self->machines, index);
			}
		}
	} else if (ev->keycode == psy_ui_KEY_LEFT) {
		if (ev->shift) {
			machinewireview_movemachine(self, self->selectedslot, -10, 0);
		} else {
			uintptr_t index;

			index = machinewireview_machineleft(self, self->selectedslot);
			if (index != psy_INDEX_INVALID) {
				psy_audio_machines_select(self->machines, index);
			}
		}
	} else if (ev->keycode == psy_ui_KEY_RIGHT) {
		if (ev->shift) {
			machinewireview_movemachine(self, self->selectedslot, 10, 0);
		} else {
			uintptr_t index;

			index = machinewireview_machineright(self, self->selectedslot);
			if (index != psy_INDEX_INVALID) {
				psy_audio_machines_select(self->machines, index);
			}
		}
	} else if (ev->keycode == psy_ui_KEY_DELETE &&
			psy_audio_wire_valid(&self->selectedwire)) {
		psy_audio_machines_disconnect(self->machines, self->selectedwire);
	} else if (ev->keycode == psy_ui_KEY_DELETE && self->selectedslot != - 1 &&
			self->selectedslot != psy_audio_MASTER_INDEX) {
		psy_audio_machines_remove(self->machines, self->selectedslot);		
		self->selectedslot = psy_INDEX_INVALID;
	} else if (ev->repeat) {
		psy_ui_keyevent_stoppropagation(ev);
	}
}

uintptr_t machinewireview_machineleft(MachineWireView* self, uintptr_t src)
{
	uintptr_t rv;
	MachineUi* srcmachineui;
	double currpos;	

	rv = psy_INDEX_INVALID;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		double srcpos;

		srcpos = machineui_position(srcmachineui)->left;
		currpos = (double)INTPTR_MAX;
		for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			MachineUi* machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			if (machineui_position(machineui)->left < srcpos) {
				if (currpos == INTPTR_MAX ||
						currpos < machineui_position(machineui)->left) {
					rv = psy_tableiterator_key(&it);
					currpos = machineui_position(machineui)->left;
				}
			}
		}
	}
	return rv;
}

uintptr_t machinewireview_machineright(MachineWireView* self, uintptr_t src)
{
	uintptr_t rv;
	MachineUi* srcmachineui;
	double currpos;

	rv = psy_INDEX_INVALID;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		double srcpos;

		srcpos = machineui_position(srcmachineui)->left;
		currpos = (double)INTPTR_MIN;
		for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			MachineUi* machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			if (machineui_position(machineui)->left > srcpos) {
				if (currpos == (double)INTPTR_MIN ||
						currpos > machineui_position(machineui)->left) {
					rv = psy_tableiterator_key(&it);
					currpos = machineui_position(machineui)->left;
				}
			}
		}
	}
	return rv;
}

uintptr_t machinewireview_machineup(MachineWireView* self, uintptr_t src)
{
	uintptr_t rv;
	MachineUi* srcmachineui;
	double currpos;

	rv = psy_INDEX_INVALID;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		double srcpos;

		srcpos = machineui_position(srcmachineui)->top;
		currpos = (double)INTPTR_MAX;
		for (it = psy_table_begin(&self->machineuis);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			MachineUi* machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			if (machineui_position(machineui)->top < srcpos) {
				if (currpos == INTPTR_MAX ||
					currpos < machineui_position(machineui)->top) {
					rv = psy_tableiterator_key(&it);
					currpos = machineui_position(machineui)->top;
				}
			}
		}
	}
	return rv;
}

uintptr_t machinewireview_machinedown(MachineWireView* self, uintptr_t src)
{
	uintptr_t rv;
	MachineUi* srcmachineui;
	double currpos;

	rv = psy_INDEX_INVALID;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		double srcpos;

		srcpos = machineui_position(srcmachineui)->bottom;
		currpos = INTPTR_MIN;
		for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			MachineUi* machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			if (machineui_position(machineui)->bottom > srcpos) {
				if (currpos == (double)INTPTR_MIN ||
					currpos > machineui_position(machineui)->bottom) {
					rv = psy_tableiterator_key(&it);
					currpos = machineui_position(machineui)->bottom;
				}
			}
		}
	}
	return rv;
}

psy_audio_Wire machinewireview_hittestwire(MachineWireView* self, double x,
	double y)
{		
	psy_audio_Wire rv;
	psy_TableIterator it;
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);	
	psy_audio_wire_init(&rv);
	for (it = psy_audio_machines_begin(self->machines); it.curr != 0; 
			psy_tableiterator_inc(&it)) {
		psy_audio_MachineSockets* sockets;			
		uintptr_t slot = it.curr->key;
	
		sockets	= psy_audio_connections_at(&self->machines->connections, slot);
		if (sockets) {
			psy_TableIterator it;

			for (it = psy_audio_wiresockets_begin(&sockets->outputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
				psy_audio_WireSocket* socket;

				socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);											
				if (socket->slot != psy_INDEX_INVALID) {
					MachineUi* inmachineui;
					MachineUi* outmachineui;

					inmachineui = machineuis_at(self, socket->slot);
					outmachineui = machineuis_at(self, slot);
					if (inmachineui && outmachineui) {
						psy_ui_RealRectangle r;
						psy_ui_RealSize out;
						psy_ui_RealSize in;
						double d = 4;
						double mxout;
						double myout;
						double mxin;
						double myin;

						psy_audio_machine_position(outmachineui->machine,
							&mxout, &myout);
						psy_audio_machine_position(inmachineui->machine,
							&mxin, &myin);

						out = machineui_size(outmachineui);
						in = machineui_size(inmachineui);
						psy_ui_setrectangle(&r, x - d, y - d, 2 * d, 2 * d);
						if (psy_ui_realrectangle_intersect_segment(&r,
								mxout + out.width / 2, myout + out.height / 2,
								mxin + in.width / 2, myin + in.height / 2)) {
							psy_audio_wire_set(&rv, slot, socket->slot);
						}						
					}
				}
				if (psy_audio_wire_valid(&rv)) {
					break;
				}				
			}
		}
	}
	return rv;
}

void machinewireview_onmachineselected(MachineWireView* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	self->selectedslot = slot;
	psy_audio_wire_invalidate(&self->selectedwire);	
	psy_ui_component_invalidate(&self->component);
	psy_ui_component_setfocus(&self->component);
}

void machinewireview_onwireselected(MachineWireView* self,
	psy_audio_Machines* sender, psy_audio_Wire wire)
{
	self->selectedwire = wire;
	psy_ui_component_invalidate(&self->component);
}

void machinewireview_onmachineinsert(MachineWireView* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(sender, slot);
	if (machine && 
			(!psy_audio_machines_isvirtualgenerator(self->machines, slot) ||
			self->drawvirtualgenerators)) {
		MachineUi* machineui;

		machineui = machineuis_insert(self, slot);
		if (machineui && !self->randominsert) {
			psy_ui_RealSize size;			

			size = machineui_size(machineui);				
			machineui_move(machineui,
				psy_ui_realpoint_make(
					psy_max(0.0, self->dragpt.x - size.width / 2),
					psy_max(0.0, self->dragpt.y - size.height / 2)));
		}
		psy_ui_component_updateoverflow(&self->component);
		psy_ui_component_invalidate(&self->component);
		++self->component.opcount;
		self->randominsert = 1;
	}
}

void machinewireview_onmachineremoved(MachineWireView* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	machineuis_remove(self, slot);
	psy_ui_component_updateoverflow(&self->component);
	psy_ui_component_invalidate(&self->component);
	++self->component.opcount;
}

void machinewireview_onconnected(MachineWireView* self,
	psy_audio_Connections* sender, uintptr_t src, uintptr_t dst)
{
	psy_ui_component_invalidate(&self->component);
	++self->component.opcount;
}

void machinewireview_ondisconnected(MachineWireView* self,
	psy_audio_Connections* sender, uintptr_t src, uintptr_t dst)
{
	psy_ui_component_invalidate(&self->component);
	++self->component.opcount;
}

void machinewireview_buildmachineuis(MachineWireView* self)
{
	if (self->machines) {
		psy_TableIterator it;

		machineuis_removeall(self);
		for (it = psy_audio_machines_begin(self->machines);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			if (self->drawvirtualgenerators ||
					!(psy_tableiterator_key(&it) > 0x80 &&
					psy_tableiterator_key(&it) <= 0xFE)) {
				psy_audio_Machine* machine;

				machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
				machineuis_insert(self, psy_tableiterator_key(&it));
			}
		}
		if (psy_audio_machines_size(self->machines) == 1) {
			// if only master exists, center
			machinewireview_centermaster(self);
		}
	}
	++self->component.opcount;
}

void machinewireview_onsongchanged(MachineWireView* self, Workspace* sender,
	int flag, psy_audio_Song* song)
{	
	self->machines = &sender->song->machines;	
	if (song) {
		machinewireview_setmachines(self, psy_audio_song_machines(song));
	} else {
		machinewireview_setmachines(self, NULL);
	}	
	psy_ui_component_setscroll(&self->component, psy_ui_point_zero());	
	psy_ui_component_updateoverflow(&self->component);
	psy_ui_component_invalidate(&self->component);
	++self->component.opcount;
}

void machinewireview_onshowparameters(MachineWireView* self, Workspace* sender,
	uintptr_t slot)
{	
	if (machineuis_at(self, slot)) {
		machineui_showparameters(machineuis_at(self, slot), &self->component);
	}
}

void machinewireview_onmasterworked(MachineWireView* self,
	psy_audio_Machine* machine, uintptr_t slot, psy_audio_BufferContext* bc)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		MachineUi* machineui;
		
		machineui = psy_tableiterator_value(&it);
		if (machineui->slot != psy_audio_MASTER_INDEX) {		
			machineui_updatevolumedisplay(machineui);
		}
	}
}

void machinewireview_ontimer(MachineWireView* self, uintptr_t timerid)
{	
	psy_TableIterator it;
	bool updatevus;
	
	machinewireview_destroywireframes(self);
	self->vudrawupdate = TRUE;
	updatevus = psy_ui_component_drawvisible(&self->component) &&
		self->skin->drawvumeters;
	if (updatevus) {
		psy_ui_component_setbackgroundmode(&self->component,
			psy_ui_BACKGROUND_NONE);
	}	
	for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		MachineUi* machineui;

		machineui = psy_tableiterator_value(&it);		
		if (updatevus && machineui->slot != psy_audio_MASTER_INDEX) {
			machineui_invalidate(machineui, TRUE);
		}
	}	
	if (updatevus) {
		psy_ui_component_update(machinewireview_base(self));		
		psy_ui_component_setbackgroundmode(machinewireview_base(self),
			psy_ui_BACKGROUND_SET);
	}
	if (self->machines && self->opcount != self->machines->opcount) {
		psy_ui_component_invalidate(&self->component);
		self->opcount = self->machines->opcount;
	}
	self->vudrawupdate = FALSE;
}

void machinewireview_destroywireframes(MachineWireView* self)
{
	psy_List* p;
	psy_List* q;

	for (p = self->wireframes; p != NULL; p = q) {
		WireFrame* frame;

		frame = (WireFrame*)psy_list_entry(p);
		q = p->next;
		if (!wireview_wireexists(&frame->wireview)) {			
			psy_ui_component_deallocate(&frame->component);		
			psy_list_remove(&self->wireframes, p);
		}
	}
}

void machinewireview_showwireview(MachineWireView* self, psy_audio_Wire wire)
{	
	if (workspace_song(self->workspace) && psy_audio_wire_valid(&wire)) {
		WireFrame* wireframe;
		
		wireframe = machinewireview_wireframe(self, wire);
		if (!wireframe) {		
			wireframe = wireframe_allocinit(&self->component, wire,
				self->workspace);
			if (wireframe) {
				psy_list_append(&self->wireframes, wireframe);
				psy_signal_connect(&wireframe->component.signal_destroyed,
					self, machinewireview_onwireframedestroyed);
			}
		}
		if (wireframe != NULL) {
			psy_ui_component_show(&wireframe->component);
		}
	}
}

void machinewireview_onwireframedestroyed(MachineWireView* self,
	psy_ui_Component* sender)
{
	psy_List* p;
	psy_List* q;

	for (p = self->wireframes; p != NULL; p = q) {
		WireFrame* frame;

		frame = (WireFrame*)psy_list_entry(p);
		q = p->next;
		if (&frame->component == sender) {
			psy_list_remove(&self->wireframes, p);
		}
	}
}

WireFrame* machinewireview_wireframe(MachineWireView* self,
	psy_audio_Wire wire)
{	
	WireFrame* rv;
	psy_List* p;

	rv = NULL;
	p = self->wireframes;
	while (p != NULL) {
		WireFrame* frame;

		frame = (WireFrame*)psy_list_entry(p->entry);
		if (psy_audio_wire_equal(wireframe_wire(frame), &wire)) {
			rv = frame;
			break;
		}
		psy_list_next(&p);
	}
	return rv;
}

MachineUi* machineuis_insert(MachineWireView* self, uintptr_t slot)
{	
	if (psy_audio_machines_at(self->machines, slot)) {
		MachineUi* rv;

		if (psy_table_exists(&self->machineuis, slot)) {
			machineuis_remove(self, slot);
		}	
		rv = (MachineUi*)malloc(sizeof(MachineUi));
		if (rv) {
			machineui_init(rv, slot, self->skin, &self->component,
				self->workspace);
			psy_table_insert(&self->machineuis, slot, rv);
		}
		return rv;
	}
	return NULL;
}

MachineUi* machineuis_at(MachineWireView* self, uintptr_t slot)
{
	return psy_table_at(&self->machineuis, slot);
}

void machineuis_remove(MachineWireView* self, uintptr_t slot)
{
	MachineUi* machineui;

	machineui = (MachineUi*) psy_table_at(&self->machineuis, slot);
	if (machineui) {
		machineui_dispose(machineui);
		free(machineui);
		psy_table_remove(&self->machineuis, slot);
	}
}

void machineuis_removeall(MachineWireView* self)
{
	psy_table_disposeall(&self->machineuis, (psy_fp_disposefunc)
		machineui_dispose);	
	psy_table_init(&self->machineuis);
}

void machinewireview_showvirtualgenerators(MachineWireView* self)
{
	if (!self->drawvirtualgenerators) {
		self->drawvirtualgenerators = TRUE;
		machinewireview_buildmachineuis(self);
		psy_ui_component_invalidate(&self->component);
	}
}

void machinewireview_hidevirtualgenerators(MachineWireView* self)
{
	if (self->drawvirtualgenerators) {
		self->drawvirtualgenerators = FALSE;
		machinewireview_buildmachineuis(self);
		psy_ui_component_invalidate(&self->component);
	}
}

psy_ui_RealRectangle machinewireview_updaterect(MachineWireView* self,
	uintptr_t slot)
{
	psy_ui_RealRectangle rv;
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		psy_audio_MachineSockets* sockets;

		rv = *machineui_position(machineui);
		sockets = psy_audio_connections_at(&self->machines->connections, slot);
		if (sockets) {
			psy_TableIterator it;

			for (it = psy_audio_wiresockets_begin(&sockets->outputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
				psy_audio_WireSocket* socket;

				socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
				if (socket->slot != UINTPTR_MAX) {
					MachineUi* inmachineui;

					inmachineui = machineuis_at(self, socket->slot);
					if (inmachineui && machineui) {
						psy_ui_realrectangle_union(&rv,
							machineui_position(inmachineui));
					}
				}
			}
			for (it = psy_audio_wiresockets_begin(&sockets->inputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
				psy_audio_WireSocket* socket;

				socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
				if (socket->slot != psy_INDEX_INVALID) {
					MachineUi* outmachineui;

					outmachineui = machineuis_at(self, socket->slot);
					if (outmachineui && machineui) {
						psy_ui_realrectangle_union(&rv,
							machineui_position(outmachineui));
					}
				}
			}
		}
		return rv;
	}
	return psy_ui_realrectangle_zero();
}

void machinewireview_onpreferredsize(MachineWireView* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_RealRectangle bounds;

	bounds = machinewireview_bounds(self);
	psy_ui_size_setpx(rv, bounds.right, bounds.bottom);
}

psy_ui_RealRectangle machinewireview_bounds(MachineWireView* self)
{
	psy_ui_RealRectangle rv;
	psy_TableIterator it;

	rv = psy_ui_realrectangle_zero();
	for (it = psy_table_begin(&self->machineuis);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_ui_realrectangle_union(&rv, machineui_position(
			((MachineUi*)psy_tableiterator_value(&it))));
	}
	psy_ui_realrectangle_expand(&rv, 0.0, 10.0, 10.0, 0.0);
	return rv;
}

void machinewireview_onminiviewcustomdraw(MachineWireView* self, MiniView* miniview,
	psy_ui_Graphics* g)
{
	psy_TableIterator it;
	psy_ui_Bitmap bitmap;
	psy_ui_Graphics mem;	
	psy_ui_Size preferredsize;
	psy_ui_RealSize preferredsizepx;
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;
	
	tm = psy_ui_component_textmetric(&self->component);
	preferredsize = psy_ui_component_preferredsize(&self->component, NULL);
	size = psy_ui_component_sizepx(&self->component);
	preferredsizepx.width = psy_ui_value_px(&preferredsize.width, tm);
	preferredsizepx.height = psy_ui_value_px(&preferredsize.height, tm);
	if (preferredsizepx.width < size.width) {
		preferredsizepx.width = size.width;
	}
	if (preferredsizepx.height < size.height) {
		preferredsizepx.height = size.height;
	}	
	psy_ui_bitmap_init_size(&bitmap, preferredsizepx);	
	psy_ui_graphics_init_bitmap(&mem, &bitmap);
	psy_ui_drawsolidrectangle(&mem,
		psy_ui_realrectangle_make(psy_ui_realpoint_zero(),
			preferredsizepx),
		psy_ui_component_backgroundcolour(&self->component));
	for (it = psy_table_begin(&self->machineuis);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		MachineUi* machineui;
		const psy_ui_RealRectangle* position;

		machineui = (MachineUi*)psy_tableiterator_value(&it);
		position = machineui_position(machineui);		
		psy_ui_setorigin(&mem, psy_ui_realpoint_make(-position->left,
			-position->top));
		machineui_draw(machineui, &mem, psy_tableiterator_key(&it), FALSE);
		psy_ui_resetorigin(&mem);
		if (self->selectedwire.src == psy_INDEX_INVALID &&
			self->selectedslot == psy_tableiterator_key(&it)) {
			machineui_drawhighlight(machineui, &mem);
		}		
	}
	psy_ui_graphics_dispose(&mem);
	psy_ui_drawstretchedbitmap(g, &bitmap,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_zero(),
			psy_ui_component_sizepx(&miniview->component)),
		psy_ui_realpoint_zero(),
		preferredsizepx);
	psy_ui_bitmap_dispose(&bitmap);		
}
