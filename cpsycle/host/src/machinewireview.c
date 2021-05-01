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
#include "machineui.h"
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
	uintptr_t slot);
static void machinewireview_drawwirearrow(MachineWireView*, psy_ui_Graphics*,
	psy_ui_RealPoint p1, psy_ui_RealPoint p2);
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
static psy_audio_Wire machinewireview_hittestwire(MachineWireView*,
	psy_ui_RealPoint);
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
static void machinewireview_destroywireframes(MachineWireView*);
static void machinewireview_showwireview(MachineWireView*, psy_audio_Wire);
static void machinewireview_onwireframedestroyed(MachineWireView*,
	psy_ui_Component* sender);
static WireFrame* machinewireview_wireframe(MachineWireView*, psy_audio_Wire);
static psy_ui_RealRectangle machinewireview_updaterect(MachineWireView*,
	uintptr_t slot);
static void machinewireview_onpreferredsize(MachineWireView*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static psy_ui_Component* machineuis_insert(MachineWireView*, uintptr_t slot);
static psy_ui_Component* machineuis_at(MachineWireView*, uintptr_t slot);
static void machineuis_remove(MachineWireView*, uintptr_t slot);
static void machineuis_removeall(MachineWireView*);
static psy_ui_RealPoint  machinewireview_centerposition(psy_ui_RealRectangle);
static bool machinewireview_dragmachine(MachineWireView*, uintptr_t slot,
	double x, double y);
static void machinewireview_setdragstatus(MachineWireView*, uintptr_t slot);
static void machinewireview_onalign(MachineWireView*);

static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(MachineWireView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			machinewireview_ondestroy;
		vtable.ondraw = (psy_ui_fp_component_ondraw)machinewireview_ondraw;
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
		vtable.onalign = (psy_ui_fp_component_onalign)
			machinewireview_onalign;
		vtable_initialized = TRUE;
	}
	return &vtable;
}

void machinewireview_init(MachineWireView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, MachineViewSkin* skin,
	ParamViews* paramviews, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setvtable(&self->component, vtable_init(self));	
	self->opcount = 0;
	self->centermaster = TRUE;
	psy_ui_component_setscrollstep(&self->component,
		psy_ui_size_make_px(10.0, 1.0));	
	self->machines = NULL;
	self->paramviews = paramviews;
	self->workspace = workspace;
	self->machines = &workspace->song->machines;	
	self->wireframes = 0;
	self->randominsert = 1;
	self->addeffect = 0;
	self->vudrawupdate = FALSE;
	self->showwirehover = FALSE;
	self->drawvirtualgenerators = FALSE;	
	self->skin = skin;
	psy_audio_wire_init(&self->dragwire);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	psy_table_init(&self->machineuis);	
	psy_ui_component_setfont(&self->component, &self->skin->font);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	self->dragslot = psy_INDEX_INVALID;
	self->dragmode = MACHINEVIEW_DRAG_MACHINE;
	self->selectedslot = psy_audio_MASTER_INDEX;	
	psy_audio_wire_init(&self->hoverwire);
	psy_signal_connect(&workspace->signal_songchanged, self,
		machinewireview_onsongchanged);	
	if (workspace_song(workspace)) {
		machinewireview_setmachines(self,
			psy_audio_song_machines(workspace_song(workspace)));
	} else {
		machinewireview_setmachines(self, NULL);
	}	
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
	}
	machinewireview_buildmachineuis(self);
}

void machinewireview_ondestroy(MachineWireView* self)
{	
	if (self->machines) {
		psy_signal_disconnect_context(
			&psy_audio_machines_master(self->machines)->signal_worked, self);
	}	
	psy_table_dispose(&self->machineuis);	
	psy_list_deallocate(&self->wireframes, (psy_fp_disposefunc)
		psy_ui_component_destroy);	
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
	if (!self->vudrawupdate) {
		machinewireview_drawdragwire(self, g);
		if (self->machines) {
			psy_ui_RealRectangle position;
			psy_ui_Component* machineui;
			uintptr_t slot;

			slot = psy_audio_machines_selected(self->machines);
			machineui = (psy_ui_Component*)machineuis_at(self, slot);
			if (machineui) {
				position = psy_ui_component_position(machineui);
				psy_ui_setcolour(g, self->skin->wirecolour);
				machineui_drawhighlight(g, position);
			}
		}		
	}
}

void machinewireview_drawwires(MachineWireView* self, psy_ui_Graphics* g)
{
	psy_TableIterator it;
	
	for (it = psy_table_begin(&self->machineuis); 
			!psy_tableiterator_equal(&it, psy_table_end()); 
			psy_tableiterator_inc(&it)) {
		machinewireview_drawwire(self, g, psy_tableiterator_key(&it));
	}	
}

void machinewireview_drawwire(MachineWireView* self, psy_ui_Graphics* g,
	uintptr_t slot)
{		
	psy_audio_MachineSockets* sockets;
	
	sockets	= psy_audio_connections_at(&self->machines->connections,
		slot);
	if (sockets) {
		psy_TableIterator it;
		psy_ui_Component* machineui;

		machineui = (psy_ui_Component*)machineuis_at(self, slot);
		for (it = psy_audio_wiresockets_begin(&sockets->outputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);		
			if (socket->slot != psy_INDEX_INVALID) {
				psy_ui_Component* inmachineui;
				psy_audio_Wire selectedwire;

				selectedwire = psy_audio_machines_selectedwire(self->machines);
				inmachineui = (psy_ui_Component*)machineuis_at(self, socket->slot);
				if (inmachineui && machineui) {
					psy_ui_RealPoint out;
					psy_ui_RealPoint in;
										
					if (self->hoverwire.src == slot &&
							self->hoverwire.dst == socket->slot) {
						psy_ui_setcolour(g, self->skin->hoverwirecolour);
					} else if (selectedwire.src == slot &&
							selectedwire.dst == socket->slot) {
						psy_ui_setcolour(g, self->skin->selwirecolour);
					} else {
						psy_ui_setcolour(g, self->skin->wirecolour);
					}
					out = machinewireview_centerposition(
							psy_ui_component_position(machineui));
					in = machinewireview_centerposition(
							psy_ui_component_position(inmachineui));
					psy_ui_drawline(g, out, in);
					machinewireview_drawwirearrow(self, g, out, in);						
				}
			}
		}
	}
}

psy_ui_RealPoint  machinewireview_centerposition(psy_ui_RealRectangle r)
{
	return psy_ui_realpoint_make(
		r.left + psy_ui_realrectangle_width(&r) / 2,
		r.top + psy_ui_realrectangle_height(&r) / 2);
}


void machinewireview_drawwirearrow(MachineWireView* self, psy_ui_Graphics* g,
	psy_ui_RealPoint p1, psy_ui_RealPoint p2)
{			
	psy_ui_RealPoint center;
	psy_ui_RealPoint a, b, c;	
	psy_ui_RealPoint tri[4];
	double polysize2;
	float deltaColR;
	float deltaColG;
	float deltaColB;
	unsigned int polyInnards;
	double phi;
	
	deltaColR = ((self->skin->polycolour.value & 0xFF) / 510.0f) + .45f;
	deltaColG = ((self->skin->polycolour.value >> 8 & 0xFF) / 510.0f) + .45f;
	deltaColB = ((self->skin->polycolour.value >> 16 & 0xFF) / 510.0f) + .45f;
	polyInnards = psy_ui_colour_make_rgb((uint8_t)(192 * deltaColR),
		(uint8_t)(192 * deltaColG), (uint8_t)(192 * deltaColB)).value;
			
	center.x = (p2.x - p1.x) / 2 + p1.x;
	center.y = (p2.y - p1.y) / 2 + p1.y;
	
	polysize2 = self->skin->triangle_size / 2;
	a.x = -polysize2;
	a.y = polysize2;
	b.x = polysize2;
	b.y = polysize2;
	c.x = 0;
	c.y = -polysize2;

	phi = atan2(p2.x - p1.x, p1.y - p2.y);
	
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
		psy_ui_Component* machineui;

		machineui = (psy_ui_Component*)machineuis_at(self, self->dragslot);
		if (machineui) {			
			psy_ui_setcolour(g, self->skin->wirecolour);
			psy_ui_drawline(g, 
				machinewireview_centerposition(
					psy_ui_component_position(machineui)),
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

void machinewireview_centermaster(MachineWireView* self)
{
	psy_ui_Component* machineui;	

	machineui = machineuis_at(self, psy_audio_MASTER_INDEX);
	if (machineui) {		
		psy_ui_RealSize machinesize;
		psy_ui_RealSize size;
						
		size = psy_ui_component_scrollsize_px(&self->component);
		machinesize = psy_ui_component_scrollsize_px(machineui);
		psy_ui_component_move(machineui,
			psy_ui_point_make(
				psy_ui_value_make_px((size.width - machinesize.width) / 2),
				psy_ui_value_make_px((size.height - machinesize.height) / 2)));
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
			psy_audio_Wire selectedwire;
			
			selectedwire = machinewireview_hittestwire(self, ev->pt);
			if (psy_audio_wire_valid(&selectedwire)) {
				psy_audio_machines_selectwire(self->machines, selectedwire);
				machinewireview_showwireview(self, selectedwire);
				psy_ui_component_invalidate(&self->component);
			} else {				
				self->randominsert = 0;
				return;
			}
		} else if (machineuis_at(self, self->dragslot)) {			
			machineuis_at(self, self->dragslot)->vtable->onmousedoubleclick(
				machineuis_at(self, self->dragslot), ev);			
		}
		self->dragslot = psy_INDEX_INVALID;
		psy_ui_mouseevent_stoppropagation(ev);
	}
}

void machinewireview_onmousedown(MachineWireView* self, psy_ui_MouseEvent* ev)
{	
	if (!psy_ui_component_hasfocus(&self->component)) {
		psy_ui_component_setfocus(&self->component);
	}
	self->dragpt = ev->pt;
	self->mousemoved = FALSE;		
	self->dragmode = MACHINEVIEW_DRAG_NONE;
	self->dragslot = machinewireview_hittest(self);
	self->dragmachineui = machineuis_at(self, self->dragslot);
	if (ev->button == 1) {
		if (self->dragslot != psy_audio_MASTER_INDEX) {			
			psy_audio_machines_selectwire(self->machines, 
				psy_audio_wire_make(psy_INDEX_INVALID, psy_INDEX_INVALID));
			self->selectedslot = self->dragslot;			
		}
		if (self->dragmachineui) {			
			if (ev->bubble) {
				psy_ui_RealRectangle position;

				self->dragmode = MACHINEVIEW_DRAG_MACHINE;
				position = psy_ui_component_position(self->dragmachineui);				
				psy_ui_realpoint_floor(
					psy_ui_realpoint_move(&self->dragpt,
						ev->pt.x - position.left,
						ev->pt.y - position.top));
				psy_ui_component_capture(&self->component);				
			}
		} else {
			psy_audio_Wire selectedwire;
			
			selectedwire = machinewireview_hittestwire(self, ev->pt);
			psy_audio_machines_selectwire(self->machines,
				selectedwire);
			if (psy_audio_wire_valid(&selectedwire) && ev->shift) {
				self->dragmode = MACHINEVIEW_DRAG_LEFTCONNECTION;
				self->dragslot = selectedwire.src;
			}
			if (psy_audio_wire_valid(&selectedwire) && ev->ctrl) {
				self->dragmode = MACHINEVIEW_DRAG_RIGHTCONNECTION;
				self->dragslot = selectedwire.dst;
			}
			psy_ui_component_invalidate(&self->component);
		}
		psy_ui_mouseevent_stoppropagation(ev);
	} else if (ev->button == 2) {
		psy_audio_Machine* machine;

		machine = psy_audio_machines_at(self->machines, self->dragslot);
		if (machine && psy_audio_machine_numoutputs(machine) > 0) {
			self->dragmode = MACHINEVIEW_DRAG_NEWCONNECTION;
			psy_ui_component_capture(&self->component);
			psy_ui_mouseevent_stoppropagation(ev);
		} else {
			self->dragslot = psy_INDEX_INVALID;
		}
	}
}

uintptr_t machinewireview_hittest(const MachineWireView* self)
{	
	uintptr_t rv;
	psy_TableIterator it;
		
	rv = psy_INDEX_INVALID;
	for (it = psy_table_begin(&(((MachineWireView*)self)->machineuis));
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {	
		psy_ui_RealRectangle r;

		r = psy_ui_component_position(((psy_ui_Component*)
			psy_tableiterator_value(&it)));
		if (psy_ui_realrectangle_intersect(&r, self->dragpt)) {
			rv = psy_tableiterator_key(&it);
			break;	
		}
	}
	return rv;
}

void machinewireview_onmousemove(MachineWireView* self, psy_ui_MouseEvent* ev)
{		
	if (!self->mousemoved) {
		if (!psy_ui_realpoint_equal(&self->dragpt, &ev->pt)) {
			self->mousemoved = TRUE;
		} else {
			return;
		}
	}	
	if (self->dragslot != psy_INDEX_INVALID) {		
		if (!ev->bubble) {
			return;
		}		
		if (machinewireview_dragging_machine(self)) {
			if (!machinewireview_dragmachine(self, self->dragslot,
					ev->pt.x - self->dragpt.x, ev->pt.y - self->dragpt.y)) {
				self->dragmode = MACHINEVIEW_DRAG_NONE;
			}			
		} else if (machinewireview_dragging_connection(self)) {
			self->dragpt = ev->pt;	
			psy_ui_component_invalidate(&self->component);
			++self->component.opcount;
		}		
	} else if (self->showwirehover) {
		psy_audio_Wire hoverwire;
		
		hoverwire = machinewireview_hittestwire(self, ev->pt);
		if (psy_audio_wire_valid(&hoverwire)) {
			psy_ui_Component* machineui;

			machineui = machineuis_at(self, hoverwire.dst);
			if (machineui) {				
				psy_ui_RealRectangle r;

				r = psy_ui_component_position(machineui);
				if (psy_ui_realrectangle_intersect(&r, ev->pt)) {
					psy_audio_wire_invalidate(&self->hoverwire);
					psy_ui_component_invalidate(&self->component);
					++self->component.opcount;
					return;
				}
			}
			machineui = machineuis_at(self, hoverwire.src);
			if (machineui) {				
				psy_ui_RealRectangle r;

				r = psy_ui_component_position(machineui);
				if (psy_ui_realrectangle_intersect(&r, ev->pt)) {
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
	psy_ui_Component* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		return machinewireview_dragmachine(self, slot,
			psy_ui_component_position(machineui).left + dx,
			psy_ui_component_position(machineui).top + dy);
	}
	return FALSE;
}

bool machinewireview_dragmachine(MachineWireView* self, uintptr_t slot,
	double x, double y)
{
	psy_ui_Component* machineui;
	psy_ui_RealRectangle r_old;
	psy_ui_RealRectangle r_new;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		psy_ui_RealPoint topleft;
		r_old = psy_ui_component_position(machineui);
		psy_ui_realrectangle_expand(&r_old, 10.0, 10.0, 10.0, 10.0);
		topleft = psy_ui_realpoint_make(psy_max(0.0, x), psy_max(0.0, y));
		psy_ui_component_move(machineui,
			psy_ui_point_make(
				psy_ui_value_make_px(topleft.x),
				psy_ui_value_make_px(topleft.y)));		
		machinewireview_setdragstatus(self, slot);		
		r_new = machinewireview_updaterect(self, self->dragslot);
		psy_ui_realrectangle_union(&r_new, &r_old);
		psy_ui_realrectangle_expand(&r_new, 10.0, 10.0, 10.0, 10.0);
		psy_ui_component_invalidaterect(&self->component, r_new);
		return TRUE;
	}
	return FALSE;
}

void machinewireview_setdragstatus(MachineWireView* self, uintptr_t slot)
{
	if (psy_audio_machines_at(self->machines, slot)) {
		psy_audio_Machine* machine;
		char txt[128];

		machine = psy_audio_machines_at(self->machines, slot);
		if (machine) {
			double x;
			double y;
			const char* editname;

			psy_audio_machine_position(machine, &x, &y);
			editname = psy_audio_machine_editname(machine);
			if (editname) {
				psy_snprintf(txt, 128, "%s (%d, %d)", editname, (int)x, (int)y);
			} else {
				psy_snprintf(txt, 128, "(%d, %d)", (int)x, (int)y);
			}
		} else {
			psy_snprintf(txt, 128, "(-, -)");
		}
		workspace_outputstatus(self->workspace, txt);
	}
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
				uintptr_t slot;
				
				slot = self->dragslot;
				self->dragslot = machinewireview_hittest(self);
				if (self->dragslot != psy_INDEX_INVALID) {
					if (!machinewireview_dragging_newconnection(self)) {						
						psy_audio_machines_disconnect(self->machines,
							psy_audio_machines_selectedwire(self->machines));
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
				//if (!self->workspace->gearvisible) {

					//workspace_togglegear(self->workspace);					
				//}
				//psy_ui_mouseevent_stoppropagation(ev);
			}			
		}
	}
	self->dragslot = psy_INDEX_INVALID;
	psy_ui_component_invalidate(&self->component);
}

void machinewireview_onkeydown(MachineWireView* self, psy_ui_KeyEvent* ev)
{		
	psy_audio_Wire selectedwire;

	selectedwire = psy_audio_machines_selectedwire(self->machines);
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
					psy_audio_machines_selectwire(self->machines, self->dragwire);					
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
			psy_audio_wire_valid(&selectedwire)) {
		psy_audio_exclusivelock_enter();
		psy_audio_machines_disconnect(self->machines, selectedwire);
		psy_audio_exclusivelock_leave();
	} else if (ev->keycode == psy_ui_KEY_DELETE && self->selectedslot != - 1 &&
			self->selectedslot != psy_audio_MASTER_INDEX) {
		psy_audio_exclusivelock_enter();
		psy_audio_machines_remove(self->machines, self->selectedslot, TRUE);		
		self->selectedslot = psy_INDEX_INVALID;
		psy_audio_exclusivelock_leave();
	} else if (ev->repeat) {
		psy_ui_keyevent_stoppropagation(ev);
	}
}

uintptr_t machinewireview_machineleft(MachineWireView* self, uintptr_t src)
{
	uintptr_t rv;
	psy_ui_Component* srcmachineui;
	double currpos;	

	rv = psy_INDEX_INVALID;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		double srcpos;

		srcpos = psy_ui_component_position(srcmachineui).left;
		currpos = (double)INTPTR_MAX;
		for (it = psy_table_begin(&self->machineuis);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_ui_Component* machineui;

			machineui = (psy_ui_Component*)psy_tableiterator_value(&it);
			if (psy_ui_component_position(machineui).left < srcpos) {
				if (currpos == INTPTR_MAX ||
						currpos < psy_ui_component_position(machineui).left) {
					rv = psy_tableiterator_key(&it);
					currpos = psy_ui_component_position(machineui).left;
				}
			}
		}
	}
	return rv;
}

uintptr_t machinewireview_machineright(MachineWireView* self, uintptr_t src)
{
	uintptr_t rv;
	psy_ui_Component* srcmachineui;
	double currpos;

	rv = psy_INDEX_INVALID;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		double srcpos;

		srcpos = psy_ui_component_position(srcmachineui).left;
		currpos = (double)INTPTR_MIN;
		for (it = psy_table_begin(&self->machineuis);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_ui_Component* machineui;

			machineui = (psy_ui_Component*)psy_tableiterator_value(&it);
			if (psy_ui_component_position(machineui).left > srcpos) {
				if (currpos == (double)INTPTR_MIN ||
						currpos > psy_ui_component_position(machineui).left) {
					rv = psy_tableiterator_key(&it);
					currpos = psy_ui_component_position(machineui).left;
				}
			}
		}
	}
	return rv;
}

uintptr_t machinewireview_machineup(MachineWireView* self, uintptr_t src)
{
	uintptr_t rv;
	psy_ui_Component* srcmachineui;
	double currpos;

	rv = psy_INDEX_INVALID;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		double srcpos;				

		srcpos = psy_ui_component_position(srcmachineui).top;
		currpos = (double)INTPTR_MAX;
		for (it = psy_table_begin(&self->machineuis);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_ui_Component* machineui;

			machineui = (psy_ui_Component*)psy_tableiterator_value(&it);
			if (psy_ui_component_position(machineui).top < srcpos) {
				if (currpos == INTPTR_MAX ||
					currpos < psy_ui_component_position(machineui).top) {
					rv = psy_tableiterator_key(&it);
					currpos = psy_ui_component_position(machineui).top;
				}
			}
		}
	}
	return rv;
}

uintptr_t machinewireview_machinedown(MachineWireView* self, uintptr_t src)
{
	uintptr_t rv;
	psy_ui_Component* srcmachineui;
	double currpos;

	rv = psy_INDEX_INVALID;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		double srcpos;

		srcpos = psy_ui_component_position(srcmachineui).bottom;
		currpos = INTPTR_MIN;
		for (it = psy_table_begin(&self->machineuis);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_ui_Component* machineui;

			machineui = (psy_ui_Component*)psy_tableiterator_value(&it);
			if (psy_ui_component_position(machineui).bottom > srcpos) {
				if (currpos == (double)INTPTR_MIN ||
					currpos > psy_ui_component_position(machineui).bottom) {
					rv = psy_tableiterator_key(&it);
					currpos = psy_ui_component_position(machineui).bottom;
				}
			}
		}
	}
	return rv;
}

psy_audio_Wire machinewireview_hittestwire(MachineWireView* self, psy_ui_RealPoint pt)
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
					psy_ui_Component* inmachineui;
					psy_ui_Component* outmachineui;

					inmachineui = (psy_ui_Component*)machineuis_at(self, socket->slot);
					outmachineui = (psy_ui_Component*)machineuis_at(self, slot);
					if (inmachineui && outmachineui) {
						psy_ui_RealRectangle r;
						psy_ui_RealRectangle inposition;
						psy_ui_RealRectangle outposition;
						psy_ui_RealSize out;
						psy_ui_RealSize in;
						double d = 4;						

						inposition = psy_ui_component_position(inmachineui);
						outposition = psy_ui_component_position(outmachineui);
						out = psy_ui_component_scrollsize_px(outmachineui);
						in = psy_ui_component_scrollsize_px(inmachineui);
						psy_ui_setrectangle(&r, pt.x - d, pt.y - d, 2 * d,
							2 * d);
						if (psy_ui_realrectangle_intersect_segment(&r,
							outposition.left + out.width / 2, outposition.top + out.height / 2,
							inposition.left + in.width / 2, inposition.top + in.height / 2)) {
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
	psy_audio_machines_selectwire(self->machines,
		psy_audio_wire_make(psy_INDEX_INVALID, psy_INDEX_INVALID));	
	psy_ui_component_invalidate(&self->component);
	psy_ui_component_setfocus(&self->component);
}

void machinewireview_onwireselected(MachineWireView* self,
	psy_audio_Machines* sender, psy_audio_Wire wire)
{	
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
		psy_ui_Component* machineui;

		machineui = (psy_ui_Component*)machineuis_insert(self, slot);
		if (machineui && !self->randominsert) {
			psy_ui_RealSize size;			

			size = psy_ui_component_scrollsize_px(machineui);
			psy_ui_component_move(machineui,
				psy_ui_point_make(
					psy_ui_value_make_px(psy_max(0.0, self->dragpt.x - size.width / 2)),
					psy_ui_value_make_px(psy_max(0.0, self->dragpt.y - size.height / 2))));
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
	paramviews_remove(self->paramviews, slot);
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

void machinewireview_idle(MachineWireView* self)
{	
	psy_TableIterator it;
	bool updatevus;
	
	machinewireview_destroywireframes(self);	
	updatevus = psy_ui_component_drawvisible(&self->component) &&
		self->skin->drawvumeters;
	if (updatevus) {
		self->vudrawupdate = TRUE;
		psy_ui_component_setbackgroundmode(&self->component,
			psy_ui_NOBACKGROUND);
		machineui_beginvuupdate();
		for (it = psy_table_begin(&self->machineuis);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {							
			psy_ui_component_invalidate((psy_ui_Component*)
				psy_tableiterator_value(&it));
		}		
		psy_ui_component_update(machinewireview_base(self));		
		psy_ui_component_setbackgroundmode(machinewireview_base(self),
			psy_ui_SETBACKGROUND);
		machineui_endvuupdate();
		self->vudrawupdate = FALSE;
	}
	if (self->machines && self->opcount != self->machines->opcount) {
		psy_ui_component_invalidate(&self->component);
		self->opcount = self->machines->opcount;
	}	
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

psy_ui_Component* machineuis_insert(MachineWireView* self, uintptr_t slot)
{	
	if (psy_audio_machines_at(self->machines, slot)) {		
		psy_ui_Component* newui;

		if (psy_table_exists(&self->machineuis, slot)) {
			machineuis_remove(self, slot);
		}		
		newui = machineui_create(
			psy_audio_machines_at(self->machines, slot),
			slot, self->skin, &self->component, &self->component,
			self->paramviews, TRUE, MACHINEUIMODE_BITMAP,
			self->workspace);
		if (newui) {			
			psy_table_insert(&self->machineuis, slot, newui);
			return newui;
		}		
	}
	return NULL;
}

psy_ui_Component* machineuis_at(MachineWireView* self, uintptr_t slot)
{
	return (psy_ui_Component*)psy_table_at(&self->machineuis, slot);
}

void machineuis_remove(MachineWireView* self, uintptr_t slot)
{
	psy_ui_Component* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		psy_table_remove(&self->machineuis, slot);
		psy_ui_component_remove(&self->component, machineui);		
	}
}

void machineuis_removeall(MachineWireView* self)
{
	psy_ui_component_clear(&self->component);	
	psy_table_clear(&self->machineuis);	
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
	psy_ui_Component* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		psy_audio_MachineSockets* sockets;

		rv = psy_ui_component_position(machineui);
		sockets = psy_audio_connections_at(&self->machines->connections, slot);
		if (sockets) {
			psy_TableIterator it;

			for (it = psy_audio_wiresockets_begin(&sockets->outputs);
					!psy_tableiterator_equal(&it, psy_table_end());
					psy_tableiterator_inc(&it)) {
				psy_audio_WireSocket* socket;

				socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
				if (socket->slot != UINTPTR_MAX) {
					psy_ui_Component* inmachineui;

					inmachineui = machineuis_at(self, socket->slot);
					if (inmachineui && machineui) {
						psy_ui_RealRectangle r;

						r = psy_ui_component_position(inmachineui);
						psy_ui_realrectangle_union(&rv, &r);
					}
				}
			}
			for (it = psy_audio_wiresockets_begin(&sockets->inputs);
					!psy_tableiterator_equal(&it, psy_table_end());
					psy_tableiterator_inc(&it)) {
				psy_audio_WireSocket* socket;

				socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
				if (socket->slot != psy_INDEX_INVALID) {
					psy_ui_Component* outmachineui;

					outmachineui = machineuis_at(self, socket->slot);
					if (outmachineui && machineui) {
						psy_ui_RealRectangle r;

						r = psy_ui_component_position(outmachineui);
						psy_ui_realrectangle_union(&rv, &r);
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
	if (limit) {
		*rv = psy_ui_max_size(*rv, *limit,
			psy_ui_component_textmetric(&self->component));
	}
}

psy_ui_RealRectangle machinewireview_bounds(MachineWireView* self)
{
	psy_ui_RealRectangle rv;
	psy_TableIterator it;

	rv = psy_ui_realrectangle_zero();
	for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_ui_RealRectangle r;

		r = psy_ui_component_position(
			((psy_ui_Component*)psy_tableiterator_value(&it)));
		psy_ui_realrectangle_union(&rv, &r);
	}
	psy_ui_realrectangle_expand(&rv, 0.0, 10.0, 10.0, 0.0);
	return rv;
}

void machinewireview_onalign(MachineWireView* self)
{
	if (self->centermaster) {
		machinewireview_centermaster(self);
		self->centermaster = FALSE;
	}
}
