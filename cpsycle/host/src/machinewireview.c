/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "machinewireview.h"
/* host */
#include "wireview.h"
#include "machineview.h"
#include "machineviewbar.h"
#include "machineui.h"
#include "styles.h"
/* audio */
#include <exclusivelock.h>
/* platform */
#include "../../detail/portable.h"


/* MachineWirePaneUis */

/* implementation */
void machinewireviewuis_init(MachineWirePaneUis* self, psy_ui_Component* view,
	ParamViews* paramviews, Workspace* workspace)
{
	assert(view);

	psy_table_init(&self->machineuis);
	self->view = view;
	self->machines = NULL;	
	self->paramviews = paramviews;
	self->workspace = workspace;
}

void machinewireviewuis_dispose(MachineWirePaneUis* self)
{
	assert(self);
	
	psy_table_dispose(&self->machineuis);
}

psy_ui_Component* machinewireviewuis_at(MachineWirePaneUis* self,
	uintptr_t slot)
{
	assert(self);
	
	return (psy_ui_Component*)psy_table_at(&self->machineuis, slot);
}

void machinewireviewuis_remove(MachineWirePaneUis* self, uintptr_t slot)
{
	psy_ui_Component* machineui;

	assert(self);
	
	machineui = machinewireviewuis_at(self, slot);
	if (machineui) {
		psy_table_remove(&self->machineuis, slot);
		psy_ui_component_remove(self->view, machineui);
	}
}

psy_ui_Component* machinewireviewuis_insert(MachineWirePaneUis* self,
	uintptr_t slot)
{
	assert(self);
	
	if (!self->machines) {
		return NULL;
	}
	if (psy_audio_machines_at(self->machines, slot)) {
		psy_ui_Component* newui;

		if (psy_table_exists(&self->machineuis, slot)) {
			machinewireviewuis_remove(self, slot);
		}
		newui = machineui_create(
			psy_audio_machines_at(self->machines, slot),
			self->view, self->paramviews, TRUE,
			self->machines);
		if (newui) {
			psy_ui_component_set_align(newui, psy_ui_ALIGN_NONE);
			psy_table_insert(&self->machineuis, slot, newui);
			return newui;
		}
	}
	return NULL;
}

void machinewireviewuis_remove_all(MachineWirePaneUis* self)
{
	assert(self);
	
	psy_ui_component_clear(self->view);
	psy_table_clear(&self->machineuis);
}


/* MachineWirePane */

/* prototypes */
static void machinewirepane_on_destroyed(MachineWirePane*);
static void machinewirepane_set_machines(MachineWirePane*, psy_audio_Machines*);
static void machinewirepane_on_draw(MachineWirePane*, psy_ui_Graphics*);
static void machinewirepane_drawdragwire(MachineWirePane*, psy_ui_Graphics*);
static bool machinewirepane_wiredragging(const MachineWirePane*);
static void machinewirepane_draw_wires(MachineWirePane*, psy_ui_Graphics*);
static void machinewirepane_draw_wire(MachineWirePane*, psy_ui_Graphics*,
	uintptr_t slot);
static void machinewirepane_draw_wire_arrow(MachineWirePane*, psy_ui_Graphics*,
	psy_ui_RealPoint p1, psy_ui_RealPoint p2);
static psy_ui_RealPoint rotate_point(psy_ui_RealPoint, double phi);
static psy_ui_RealPoint move_point(psy_ui_RealPoint pt, psy_ui_RealPoint d);
static void machinewirepane_on_mouse_down(MachineWirePane*, psy_ui_MouseEvent*);
static void machinewirepane_on_mouse_up(MachineWirePane*, psy_ui_MouseEvent*);
static void machinewirepane_on_mouse_move(MachineWirePane*, psy_ui_MouseEvent*);
static bool machinewirepane_move_machine(MachineWirePane*, uintptr_t slot,
	double dx, double dy);
static void machinewirepane_on_mouse_double_click(MachineWirePane*,
	psy_ui_MouseEvent*);
static void machinewirepane_on_key_down(MachineWirePane*, psy_ui_KeyboardEvent*);
static uintptr_t machinewirepane_hittest(const MachineWirePane*);
static psy_audio_Wire machinewirepane_hit_test_wire(MachineWirePane*,
	psy_ui_RealPoint);
static bool machinewirepane_dragging_machine(const MachineWirePane*);
static bool machinewirepane_dragging_connection(const MachineWirePane*);
static bool machinewirepane_dragging_newconnection(const MachineWirePane*);
static void machinewirepane_on_machine_selected(MachineWirePane*,
	psy_audio_Machines*, uintptr_t slot);
static void machinewirepane_on_wire_selected(MachineWirePane*,
	psy_audio_Machines* sender);
static void machinewirepane_on_machine_insert(MachineWirePane*,
	psy_audio_Machines*, uintptr_t slot);
static void machinewirepane_on_machine_removed(MachineWirePane*,
	psy_audio_Machines*, uintptr_t slot);
static void machinewirepane_on_connected(MachineWirePane*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machinewirepane_on_disconnected(MachineWirePane*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machinewirepane_on_song_changed(MachineWirePane*,
	psy_audio_Player* sender);
static void machinewirepane_build(MachineWirePane*);
static psy_ui_RealRectangle machinewirepane_update_rect(MachineWirePane*,
	uintptr_t slot);
static void machinewirepane_on_preferred_size(MachineWirePane*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static psy_ui_RealPoint  machinewirepane_centerposition(psy_ui_RealRectangle);
static bool machinewirepane_drag_machine(MachineWirePane*, uintptr_t slot,
	double x, double y);
static void machinewirepane_setdragstatus(MachineWirePane*, uintptr_t slot);
static void machinewirepane_on_align(MachineWirePane*);
static void machinewirepane_on_virtual_generators(MachineWirePane*,
	psy_Property* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MachineWirePane* self)
{
	assert(self);
	
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			machinewirepane_on_destroyed;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			machinewirepane_on_draw;
		vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			machinewirepane_on_mouse_down;
		vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			machinewirepane_on_mouse_up;
		vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			machinewirepane_on_mouse_move;
		vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			machinewirepane_on_mouse_double_click;
		vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			machinewirepane_on_key_down;
		vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			machinewirepane_on_preferred_size;
		vtable.onalign =
			(psy_ui_fp_component)
			machinewirepane_on_align;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);	
}

/* implementation */
void machinewirepane_init(MachineWirePane* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, ParamViews* paramviews,
	MachineMenu* machine_menu, Workspace* workspace)
{
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);	
	psy_ui_component_set_style_type(&self->component, STYLE_MV_WIRES);	
	machinewireviewuis_init(&self->machineuis, &self->component, paramviews,
		workspace);	
	self->centermaster = TRUE;
	psy_ui_component_set_scroll_step(&self->component,
		psy_ui_size_make_px(10.0, 1.0));	
	self->machines = NULL;
	self->paramviews = paramviews;
	self->workspace = workspace;
	self->machines = &workspace->song->machines;
	self->machine_menu = machine_menu;
	wireframes_init(&self->wireframes, self->workspace, &self->component);	
	self->randominsert = 1;	
	self->showwirehover = FALSE;
	self->drawvirtualgenerators = FALSE;
	self->dragslot = psy_INDEX_INVALID;
	self->dragmode = MACHINEVIEW_DRAG_MACHINE;
	self->selectedslot = psy_audio_MASTER_INDEX;
	psy_audio_wire_init(&self->dragwire);	
	psy_ui_component_set_wheel_scroll(&self->component, 4);	
	psy_ui_component_set_overflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	psy_audio_wire_init(&self->hoverwire);
	psy_signal_connect(&workspace->player.signal_song_changed, self,
		machinewirepane_on_song_changed);	
	if (workspace_song(workspace)) {
		machinewirepane_set_machines(self,
			psy_audio_song_machines(workspace_song(workspace)));
	} else {
		machinewirepane_set_machines(self, NULL);
	}
	machineviewconfig_connect(&workspace->config.visual.macview,
		"drawvirtualgenerators", self, machinewirepane_on_virtual_generators);
}

void machinewirepane_set_machines(MachineWirePane* self,
	psy_audio_Machines* machines)
{	
	assert(self);
	
	self->machines = machines;
	self->machineuis.machines = machines;
	if (self->machines) {
		psy_signal_connect(&self->machines->signal_slotchange, self,
			machinewirepane_on_machine_selected);
		psy_signal_connect(&self->machines->signal_wireselected, self,
			machinewirepane_on_wire_selected);		
		psy_signal_connect(&self->machines->signal_insert, self,
			machinewirepane_on_machine_insert);
		psy_signal_connect(&self->machines->signal_removed, self,
			machinewirepane_on_machine_removed);
		psy_signal_connect(&self->machines->connections.signal_connected, self,
			machinewirepane_on_connected);
		psy_signal_connect(&self->machines->connections.signal_disconnected,
			self, machinewirepane_on_disconnected);
	}
	machinewirepane_build(self);
}

void machinewirepane_on_destroyed(MachineWirePane* self)
{	
	machinewireviewuis_dispose(&self->machineuis);
	wireframes_dispose(&self->wireframes);	
}

void machinewirepane_on_draw(MachineWirePane* self, psy_ui_Graphics* g)
{		
	psy_List* p;

	assert(self);
	
	if (!self->machines) {
		return;
	}
	machinewirepane_draw_wires(self, g);					
	for (p = self->machines->selection.entries; p != NULL; p = p->next) {
		psy_audio_MachineIndex* index;
		psy_ui_Component* machineui;

		index = (psy_audio_MachineIndex*)p->entry;
		machineui = (psy_ui_Component*)machinewireviewuis_at(
			&self->machineuis, index->macid);
		if (machineui) {			
			psy_ui_RealRectangle position;
			psy_ui_Style* style;

			style = psy_ui_style(STYLE_MV_WIRE);
			position = psy_ui_component_position(machineui);			
			psy_ui_setcolour(g, style->colour);
			machineui_drawhighlight(g, position);
		}
	}		
	machinewirepane_drawdragwire(self, g);	
}

void machinewirepane_draw_wires(MachineWirePane* self, psy_ui_Graphics* g)
{
	psy_TableIterator it;
	
	assert(self);
	
	for (it = psy_table_begin(&self->machineuis.machineuis); 
			!psy_tableiterator_equal(&it, psy_table_end()); 
			psy_tableiterator_inc(&it)) {
		machinewirepane_draw_wire(self, g, psy_tableiterator_key(&it));
	}
}

void machinewirepane_draw_wire(MachineWirePane* self, psy_ui_Graphics* g,
	uintptr_t slot)
{		
	psy_audio_MachineSockets* sockets;
	
	assert(self);
	
	sockets	= psy_audio_connections_at(&self->machines->connections,
		slot);
	if (sockets) {
		psy_TableIterator it;
		psy_ui_Component* machineui;
		psy_audio_Wire selectedwire;

		machineui = (psy_ui_Component*)machinewireviewuis_at(&self->machineuis,
			slot);
		selectedwire = psy_audio_machines_selectedwire(self->machines);
		for (it = psy_audio_wiresockets_begin(&sockets->outputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);		
			if (socket->slot != psy_INDEX_INVALID) {
				psy_ui_Component* inmachineui;
								
				inmachineui = (psy_ui_Component*)machinewireviewuis_at(
					&self->machineuis, socket->slot);
				if (inmachineui && machineui) {
					psy_ui_RealPoint out;
					psy_ui_RealPoint in;
					psy_ui_Style* style;
										
					if (self->hoverwire.src == slot &&
							self->hoverwire.dst == socket->slot) {						
						style = psy_ui_style(STYLE_MV_WIRE_HOVER);
					} else if (selectedwire.src == slot &&
							selectedwire.dst == socket->slot) {
						style = psy_ui_style(STYLE_MV_WIRE_SELECT);
					} else {
						style = psy_ui_style(STYLE_MV_WIRE);
					}
					psy_ui_setcolour(g, style->colour);
					out = machinewirepane_centerposition(
							psy_ui_component_position(machineui));
					in = machinewirepane_centerposition(
							psy_ui_component_position(inmachineui));
					psy_ui_drawline(g, out, in);
					machinewirepane_draw_wire_arrow(self, g, out, in);						
				}
			}
		}
	}
}

psy_ui_RealPoint  machinewirepane_centerposition(psy_ui_RealRectangle r)
{
	return psy_ui_realpoint_make(
		r.left + psy_ui_realrectangle_width(&r) / 2,
		r.top + psy_ui_realrectangle_height(&r) / 2);
}


void machinewirepane_draw_wire_arrow(MachineWirePane* self, psy_ui_Graphics* g,
	psy_ui_RealPoint p1, psy_ui_RealPoint p2)
{			
	psy_ui_RealPoint center;
	psy_ui_RealPoint a, b, c;	
	psy_ui_RealPoint tri[4];
	double polysize2;
	float deltaColR;
	float deltaColG;
	float deltaColB;
	psy_ui_Colour polycolour;
	psy_ui_Colour polyInnards;
	psy_ui_Style* style;
	double phi;
	
	assert(self);
	
	style = psy_ui_style(STYLE_MV_WIRE_POLY);
	polycolour = style->colour;
	deltaColR = ((polycolour.r) / 510.0f) + .45f;
	deltaColG = ((polycolour.g) / 510.0f) + .45f;
	deltaColB = ((polycolour.b) / 510.0f) + .45f;
	polyInnards = psy_ui_colour_make_rgb((uint8_t)(192 * deltaColR),
		(uint8_t)(192 * deltaColG), (uint8_t)(192 * deltaColB));
			
	center.x = (p2.x - p1.x) / 2 + p1.x;
	center.y = (p2.y - p1.y) / 2 + p1.y;
	
	polysize2 = style->background.size.width / 2;
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
	
	psy_ui_drawsolidpolygon(g, tri, 4, psy_ui_colour_colorref(&polyInnards),
		psy_ui_colour_colorref(&style->colour));
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

void machinewirepane_drawdragwire(MachineWirePane* self, psy_ui_Graphics* g)
{
	assert(self);
	
	if (machinewirepane_wiredragging(self)) {
		psy_ui_Component* machineui;

		machineui = (psy_ui_Component*)machinewireviewuis_at(
			&self->machineuis, self->dragslot);
		if (machineui) {			
			psy_ui_Style* style;

			assert(self);
			
			style = psy_ui_style(STYLE_MV_WIRE);			
			psy_ui_setcolour(g, style->colour);			
			psy_ui_drawline(g, 
				machinewirepane_centerposition(
					psy_ui_component_position(machineui)),
				self->dragpt);
		}
	}
}

bool machinewirepane_wiredragging(const MachineWirePane* self)
{
	assert(self);
	
	return self->dragslot != psy_INDEX_INVALID &&
		self->dragmode >= MACHINEVIEW_DRAG_NEWCONNECTION &&
		self->dragmode <= MACHINEVIEW_DRAG_RIGHTCONNECTION;
}

void machinewirepane_center_master(MachineWirePane* self)
{
	psy_ui_Component* machineui;	

	assert(self);
	
	machineui = machinewireviewuis_at(&self->machineuis,
		psy_audio_MASTER_INDEX);
	if (machineui) {		
		psy_ui_RealSize machinesize;
		psy_ui_RealSize size;
						
		size = psy_ui_component_scroll_size_px(psy_ui_component_parent(&self->component));
		machinesize = psy_ui_component_scroll_size_px(machineui);
		psy_ui_component_move(machineui,
			psy_ui_point_make(
				psy_ui_value_make_px((size.width - machinesize.width) / 2),
				psy_ui_value_make_px((size.height - machinesize.height) / 2)));
		psy_ui_component_invalidate(machinewirepane_base(self));
	}
}

void machinewirepane_on_mouse_double_click(MachineWirePane* self,
	psy_ui_MouseEvent* ev)
{	
	assert(self);
	
	if (psy_ui_mouseevent_button(ev) == 1) {
		self->dragpt = psy_ui_mouseevent_pt(ev);
		self->dragslot = machinewirepane_hittest(self);		
		if (self->dragslot == psy_INDEX_INVALID) {
			psy_audio_Wire selectedwire;
			
			selectedwire = machinewirepane_hit_test_wire(self, psy_ui_mouseevent_pt(ev));
			if (psy_audio_wire_valid(&selectedwire)) {
				psy_audio_machines_selectwire(self->machines, selectedwire);
				wireframes_show(&self->wireframes, selectedwire);
				psy_ui_component_invalidate(&self->component);
			} else {				
				self->randominsert = 0;
				return;
			}
		} else if (machinewireviewuis_at(&self->machineuis, self->dragslot)) {			
			machinewireviewuis_at(&self->machineuis, self->dragslot)->vtable->on_mouse_double_click(
				machinewireviewuis_at(&self->machineuis, self->dragslot), ev);			
		}
		self->dragslot = psy_INDEX_INVALID;
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void machinewirepane_on_mouse_down(MachineWirePane* self, psy_ui_MouseEvent* ev)
{	
	assert(self);
	
	if (!psy_ui_component_has_focus(&self->component)) {
		psy_ui_component_set_focus(&self->component);
	}
	self->dragpt = psy_ui_mouseevent_pt(ev);
	self->mousemoved = FALSE;		
	self->dragmode = MACHINEVIEW_DRAG_NONE;
	self->dragslot = machinewirepane_hittest(self);
	self->dragmachineui = machinewireviewuis_at(&self->machineuis, self->dragslot);
	if (psy_ui_mouseevent_button(ev) == 1) {
		if (self->dragslot != psy_audio_MASTER_INDEX) {			
			psy_audio_machines_selectwire(self->machines, 
				psy_audio_wire_make(psy_INDEX_INVALID, psy_INDEX_INVALID));
			self->selectedslot = self->dragslot;			
		}
		if (self->dragmachineui) {			
			if (psy_ui_event_bubbles(&ev->event)) {
				psy_ui_RealRectangle position;

				self->dragmode = MACHINEVIEW_DRAG_MACHINE;
				position = psy_ui_component_position(self->dragmachineui);				
				psy_ui_realpoint_floor(
					psy_ui_realpoint_move(&self->dragpt,
						psy_ui_mouseevent_pt(ev).x - position.left,
						psy_ui_mouseevent_pt(ev).y - position.top));
				psy_ui_component_capture(&self->component);				
			}
		} else {
			psy_audio_Wire selectedwire;
			
			selectedwire = machinewirepane_hit_test_wire(self, psy_ui_mouseevent_pt(ev));
			psy_audio_machines_selectwire(self->machines,
				selectedwire);
			if (psy_audio_wire_valid(&selectedwire) && psy_ui_mouseevent_shift_key(ev)) {
				self->dragmode = MACHINEVIEW_DRAG_LEFTCONNECTION;
				self->dragslot = selectedwire.src;
			}
			if (psy_audio_wire_valid(&selectedwire) && psy_ui_mouseevent_ctrl_key(ev)) {
				self->dragmode = MACHINEVIEW_DRAG_RIGHTCONNECTION;
				self->dragslot = selectedwire.dst;
			}
			psy_ui_component_invalidate(&self->component);
		}
		psy_ui_mouseevent_stop_propagation(ev);
	} else if (psy_ui_mouseevent_button(ev) == 2) {
		psy_audio_Machine* machine;

		machine = psy_audio_machines_at(self->machines, self->dragslot);
		if (machine && psy_audio_machine_numoutputs(machine) > 0) {
			self->dragmode = MACHINEVIEW_DRAG_NEWCONNECTION;
			psy_ui_component_capture(&self->component);
			psy_ui_mouseevent_stop_propagation(ev);
		} else {
			self->dragslot = psy_INDEX_INVALID;
		}
	}
}

uintptr_t machinewirepane_hittest(const MachineWirePane* self)
{	
	uintptr_t rv;
	psy_TableIterator it;
	
	assert(self);
		
	rv = psy_INDEX_INVALID;
	for (it = psy_table_begin(&(((MachineWirePane*)self)->machineuis.machineuis));
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

void machinewirepane_on_mouse_move(MachineWirePane* self, psy_ui_MouseEvent* ev)
{		
	assert(self);
	
	if (!self->mousemoved) {
		psy_ui_RealPoint pt;

		pt = psy_ui_mouseevent_pt(ev);
		if (!psy_ui_realpoint_equal(&self->dragpt, &pt)) {
			self->mousemoved = TRUE;
		} else {
			return;
		}
	}	
	if (self->dragslot != psy_INDEX_INVALID) {		
		if (!psy_ui_event_bubbles(&ev->event)) {
			return;
		}		
		if (machinewirepane_dragging_machine(self)) {
			if (!machinewirepane_drag_machine(self, self->dragslot,
				psy_ui_mouseevent_pt(ev).x - self->dragpt.x,
				psy_ui_mouseevent_pt(ev).y - self->dragpt.y)) {
				self->dragmode = MACHINEVIEW_DRAG_NONE;
			}			
		} else if (machinewirepane_dragging_connection(self)) {
			self->dragpt = psy_ui_mouseevent_pt(ev);
			psy_ui_component_invalidate(&self->component);			
		}		
	} else if (self->showwirehover) {
		psy_audio_Wire hoverwire;
		
		hoverwire = machinewirepane_hit_test_wire(self, psy_ui_mouseevent_pt(ev));
		if (psy_audio_wire_valid(&hoverwire)) {
			psy_ui_Component* machineui;

			machineui = machinewireviewuis_at(&self->machineuis,
				hoverwire.dst);
			if (machineui) {				
				psy_ui_RealRectangle r;

				r = psy_ui_component_position(machineui);
				if (psy_ui_realrectangle_intersect(&r, psy_ui_mouseevent_pt(ev))) {
					psy_audio_wire_invalidate(&self->hoverwire);
					psy_ui_component_invalidate(&self->component);					
					return;
				}
			}
			machineui = machinewireviewuis_at(&self->machineuis,
				hoverwire.src);
			if (machineui) {				
				psy_ui_RealRectangle r;

				r = psy_ui_component_position(machineui);
				if (psy_ui_realrectangle_intersect(&r, psy_ui_mouseevent_pt(ev))) {
					psy_audio_wire_invalidate(&self->hoverwire);
					psy_ui_component_invalidate(&self->component);					
					return;
				}
			}
		}
		if (!psy_audio_wire_equal(&hoverwire, &self->hoverwire)) {
			self->hoverwire = hoverwire;			
			psy_ui_component_invalidate(&self->component);
		}		
	}
}

bool machinewirepane_dragging_machine(const MachineWirePane* self)
{
	return self->dragmode == MACHINEVIEW_DRAG_MACHINE;
}

bool machinewirepane_dragging_connection(const MachineWirePane* self)
{
	assert(self);
	
	return (self->dragmode >= MACHINEVIEW_DRAG_NEWCONNECTION &&
		self->dragmode <= MACHINEVIEW_DRAG_RIGHTCONNECTION);
}

bool machinewirepane_dragging_newconnection(const MachineWirePane* self)
{
	return (self->dragmode == MACHINEVIEW_DRAG_NEWCONNECTION);
}

bool machinewirepane_move_machine(MachineWirePane* self, uintptr_t slot,
	double dx, double dy)
{
	psy_ui_Component* machineui;

	assert(self);
	
	machineui = machinewireviewuis_at(&self->machineuis, slot);
	if (machineui) {
		return machinewirepane_drag_machine(self, slot,
			psy_ui_component_position(machineui).left + dx,
			psy_ui_component_position(machineui).top + dy);
	}
	return FALSE;
}

bool machinewirepane_drag_machine(MachineWirePane* self, uintptr_t slot,
	double x, double y)
{
	psy_ui_Component* machineui;	

	assert(self);
	
	machineui = machinewireviewuis_at(&self->machineuis, slot);
	if (machineui) {		
		psy_ui_RealRectangle r_old;
		psy_ui_RealRectangle r_new;
	
		r_old = machinewirepane_update_rect(self, self->dragslot);
		psy_ui_realrectangle_expand_all(&r_old,
			PSYCLE_MACHINE_BORDER_DISTANCE);		
		psy_ui_component_move(machineui, psy_ui_point_make_px(
			psy_max(0.0, x), psy_max(0.0, y)));
		machinewirepane_setdragstatus(self, slot);		
		r_new = machinewirepane_update_rect(self, self->dragslot);
		psy_ui_realrectangle_union(&r_new, &r_old);
		psy_ui_realrectangle_expand_all(&r_new,
			PSYCLE_MACHINE_BORDER_DISTANCE);
		psy_ui_component_invalidate_rect(&self->component, r_new);		
		return TRUE;
	}
	return FALSE;
}

void machinewirepane_setdragstatus(MachineWirePane* self, uintptr_t slot)
{
	assert(self);
	
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
		workspace_output_status(self->workspace, txt);
	}
}

void machinewirepane_on_mouse_up(MachineWirePane* self, psy_ui_MouseEvent* ev)
{	
	assert(self);
	
	psy_ui_component_release_capture(&self->component);
	if (self->dragslot != psy_INDEX_INVALID) {
		if (machinewirepane_dragging_machine(self)) {
			psy_ui_component_updateoverflow(&self->component);
			workspace_mark_song_modified(self->workspace);
			psy_ui_mouseevent_stop_propagation(ev);
		} else if (machinewirepane_dragging_connection(self)) {
			if (self->mousemoved) {
				uintptr_t slot;
				
				slot = self->dragslot;
				self->dragslot = machinewirepane_hittest(self);
				if (self->dragslot != psy_INDEX_INVALID) {
					if (!machinewirepane_dragging_newconnection(self)) {						
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
				psy_ui_mouseevent_stop_propagation(ev);
			} else if (psy_ui_mouseevent_button(ev) == 2) {	
				machinemenu_select(self->machine_menu, self->dragslot);			
				psy_ui_component_toggle_visibility(
					&self->machine_menu->component);
				psy_ui_mouseevent_stop_propagation(ev);
			}			
		}
	} else {		 
		if (psy_ui_component_visible(&self->machine_menu->component)) {
			machinemenu_hide(self->machine_menu);
			psy_ui_mouseevent_stop_propagation(ev);
		}
	}
	self->dragslot = psy_INDEX_INVALID;
	psy_ui_component_invalidate(&self->component);
}

void machinewirepane_on_key_down(MachineWirePane* self,
	psy_ui_KeyboardEvent* ev)
{		
	psy_audio_Wire selectedwire;

	assert(self);
	
	selectedwire = psy_audio_machines_selectedwire(self->machines);
	if (psy_ui_keyboardevent_ctrlkey(ev)) {
		if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_B) {
			self->dragwire.src = self->selectedslot;
		} else if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_E) {
			if (self->dragwire.src != psy_INDEX_INVALID &&
					self->selectedslot != psy_INDEX_INVALID) {
				self->dragwire.dst = self->selectedslot;
				if (!psy_audio_machines_connected(self->machines,
						self->dragwire)) {				
					psy_audio_machines_connect(self->machines, self->dragwire);
				} else {
					psy_audio_machines_selectwire(self->machines,
						self->dragwire);					
					psy_ui_component_invalidate(&self->component);
				}
			}
		}
	} else if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_DELETE &&
			psy_audio_wire_valid(&selectedwire)) {
		psy_audio_exclusivelock_enter();
		psy_audio_machines_disconnect(self->machines, selectedwire);
		psy_audio_exclusivelock_leave();
	} else if ((psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_DELETE) &&
			(self->selectedslot != - 1) &&
			(self->selectedslot != psy_audio_MASTER_INDEX)) {
		psy_audio_exclusivelock_enter();
		psy_audio_machines_remove(self->machines, self->selectedslot, TRUE);		
		self->selectedslot = psy_INDEX_INVALID;
		psy_audio_exclusivelock_leave();
	} else if (psy_ui_keyboardevent_repeat(ev)) {
		psy_ui_keyboardevent_stop_propagation(ev);
	}
}

psy_audio_Wire machinewirepane_hit_test_wire(MachineWirePane* self,
	psy_ui_RealPoint pt)
{		
	psy_audio_Wire rv;
	psy_TableIterator it;
	const psy_ui_TextMetric* tm;

	assert(self);
	
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

					inmachineui = (psy_ui_Component*)machinewireviewuis_at(
						&self->machineuis, socket->slot);
					outmachineui = (psy_ui_Component*)machinewireviewuis_at(
						&self->machineuis, slot);
					if (inmachineui && outmachineui) {
						psy_ui_RealRectangle r;
						psy_ui_RealRectangle inposition;
						psy_ui_RealRectangle outposition;
						psy_ui_RealSize out;
						psy_ui_RealSize in;
						double d = 4;						

						inposition = psy_ui_component_position(inmachineui);
						outposition = psy_ui_component_position(outmachineui);
						out = psy_ui_component_scroll_size_px(outmachineui);
						in = psy_ui_component_scroll_size_px(inmachineui);
						r = psy_ui_realrectangle_make(
							psy_ui_realpoint_make(pt.x - d, pt.y - d),
							psy_ui_realsize_make(2 * d, 2 * d));
						if (psy_ui_realrectangle_intersect_segment(&r,
							outposition.left + out.width / 2,
							outposition.top + out.height / 2,
							inposition.left + in.width / 2,
							inposition.top + in.height / 2)) {
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

void machinewirepane_on_machine_selected(MachineWirePane* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	assert(self);
	
	self->selectedslot = slot;	
	psy_audio_machines_selectwire(self->machines,
		psy_audio_wire_make(psy_INDEX_INVALID, psy_INDEX_INVALID));	
	psy_ui_component_invalidate(&self->component);
	psy_ui_component_set_focus(&self->component);
}

void machinewirepane_on_wire_selected(MachineWirePane* self,
	psy_audio_Machines* sender)
{	
	assert(self);
	
	psy_ui_component_invalidate(&self->component);
}

void machinewirepane_on_machine_insert(MachineWirePane* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	psy_audio_Machine* machine;

	assert(self);
	
	machine = psy_audio_machines_at(sender, slot);
	if (machine && 
			(!psy_audio_machines_isvirtualgenerator(self->machines, slot) ||
			self->drawvirtualgenerators)) {
		psy_ui_Component* machineui;

		machineui = (psy_ui_Component*)machinewireviewuis_insert(
			&self->machineuis, slot);
		if (machineui && !self->randominsert) {
			psy_ui_RealSize size;			

			size = psy_ui_component_scroll_size_px(machineui);
			psy_ui_component_move(machineui,
				psy_ui_point_make(
					psy_ui_value_make_px(psy_max(0.0, self->dragpt.x - size.width / 2)),
					psy_ui_value_make_px(psy_max(0.0, self->dragpt.y - size.height / 2))));
		}
		psy_ui_component_updateoverflow(&self->component);
		psy_ui_component_invalidate(&self->component);		
		self->randominsert = 1;
	}
}

void machinewirepane_on_machine_removed(MachineWirePane* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	assert(self);
	
	machinewireviewuis_remove(&self->machineuis, slot);
	paramviews_remove(self->paramviews, slot);
	psy_ui_component_updateoverflow(&self->component);
	psy_ui_component_invalidate(&self->component);	
}

void machinewirepane_on_connected(MachineWirePane* self,
	psy_audio_Connections* sender, uintptr_t src, uintptr_t dst)
{
	assert(self);
	
	psy_ui_component_invalidate(&self->component);	
}

void machinewirepane_on_disconnected(MachineWirePane* self,
	psy_audio_Connections* sender, uintptr_t src, uintptr_t dst)
{
	assert(self);
	
	psy_ui_component_invalidate(&self->component);	
}

void machinewirepane_build(MachineWirePane* self)
{
	assert(self);
	
	if (self->machines) {
		psy_TableIterator it;

		machinewireviewuis_remove_all(&self->machineuis);
		for (it = psy_audio_machines_begin(self->machines);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			if (self->drawvirtualgenerators ||
					!(psy_tableiterator_key(&it) > 0x80 &&
					psy_tableiterator_key(&it) <= 0xFE)) {
				psy_audio_Machine* machine;

				machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
				machinewireviewuis_insert(&self->machineuis,
					psy_tableiterator_key(&it));
			}
		}
		if (psy_audio_machines_size(self->machines) == 1) {
			// if only master exists, center
			machinewirepane_center_master(self);
		}
	}	
}

void machinewirepane_on_song_changed(MachineWirePane* self,
	psy_audio_Player* sender)
{	
	assert(self);
	
	if (sender->song) {
		machinewirepane_set_machines(self, psy_audio_song_machines(sender->song));
	} else {		
		machinewirepane_set_machines(self, NULL);
	}	
	psy_ui_component_set_scroll(&self->component, psy_ui_point_zero());
	self->centermaster = !workspace_song_has_file(self->workspace);
	if (psy_ui_component_draw_visible(&self->component)) {		
		psy_ui_component_invalidate(&self->component);
	}	
	psy_ui_component_align(&self->component);
	psy_ui_component_updateoverflow(&self->component);	
}

void machinewirepane_idle(MachineWirePane* self)
{				
	wireframes_destroy(&self->wireframes);	
}

void machinewirepane_show_virtual_generators(MachineWirePane* self)
{
	assert(self);
	
	if (!self->drawvirtualgenerators) {
		self->drawvirtualgenerators = TRUE;
		machinewirepane_build(self);
		psy_ui_component_invalidate(&self->component);
	}
}

void machinewirepane_hide_virtual_generators(MachineWirePane* self)
{
	assert(self);
	
	if (self->drawvirtualgenerators) {
		self->drawvirtualgenerators = FALSE;
		machinewirepane_build(self);
		psy_ui_component_invalidate(&self->component);
	}
}

psy_ui_RealRectangle machinewirepane_update_rect(MachineWirePane* self,
	uintptr_t slot)
{
	psy_ui_RealRectangle rv;
	psy_ui_Component* machineui;

	assert(self);
	
	machineui = machinewireviewuis_at(&self->machineuis, slot);
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

					inmachineui = machinewireviewuis_at(&self->machineuis,
						socket->slot);
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

					outmachineui = machinewireviewuis_at(&self->machineuis,
						socket->slot);
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

void machinewirepane_on_preferred_size(MachineWirePane* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_RealRectangle bounds;	
	psy_ui_RealRectangle zero;

	assert(self);
	
	bounds = psy_ui_component_bounds(&self->component);
	zero = psy_ui_realrectangle_zero();
	psy_ui_realrectangle_union(&bounds, &zero);
	psy_ui_realrectangle_expand(&bounds, 0.0, 10.0, 10.0, 0.0);
	psy_ui_size_setpx(rv, bounds.right, bounds.bottom);
	if (limit) {
		*rv = psy_ui_max_size(*rv, *limit,
			psy_ui_component_textmetric(&self->component), NULL);
	}
}

void machinewirepane_on_align(MachineWirePane* self)
{
	psy_TableIterator it;

	assert(self);
			
	if (self->centermaster) {
		if (psy_ui_component_draw_visible(&self->component)) {			
			machinewirepane_center_master(self);		
			self->centermaster = FALSE;
		}
	}
	for (it = psy_table_begin(&self->machineuis.machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_ui_Component* component;
		psy_ui_Size componentsize;		
		psy_audio_Machine* machine;
		
		component = (psy_ui_Component*)psy_tableiterator_value(&it);
		componentsize = psy_ui_component_preferred_size(component, NULL);
		machine = psy_audio_machines_at(self->machines,
			psy_tableiterator_key(&it));
		if (machine) {
			double x;
			double y;

			psy_audio_machine_position(machine, &x, &y);
			psy_ui_component_setposition(component, psy_ui_rectangle_make(
				psy_ui_point_make_px(x, y), componentsize));
		}
	}
}

void machinewirepane_on_virtual_generators(MachineWirePane* self,
	psy_Property* sender)
{
	assert(self);
	
	if (psy_property_item_bool(sender)) {
		machinewirepane_show_virtual_generators(self);		
	} else {
		machinewirepane_hide_virtual_generators(self);		
	}	
}


/* MachineWireView */

/* implementation */
void machinewireview_init(MachineWireView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, ParamViews* paramviews,
	MachineMenu* machine_menu, Workspace* workspace)
{
	assert(self);
		
	psy_ui_scroller_init(&self->scroller, parent, NULL, NULL);
	psy_ui_component_set_id(machinewireview_base(self),
		SECTION_ID_MACHINEVIEW_WIRES);
	machinewirepane_init(&self->pane, machinewireview_base(self),
		tabbarparent, workspace->paramviews, machine_menu, workspace);
	psy_ui_scroller_set_client(&self->scroller,
		machinewirepane_base(&self->pane));	
	psy_ui_component_set_align(machinewirepane_base(&self->pane),
		psy_ui_ALIGN_FIXED);	
}
