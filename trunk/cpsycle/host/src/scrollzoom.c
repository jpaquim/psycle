// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "scrollzoom.h"

#include "../../detail/portable.h"

enum {
	SCROLLZOOM_DRAG_NONE,
	SCROLLZOOM_DRAG_LEFT,
	SCROLLZOOM_DRAG_RIGHT,
	SCROLLZOOM_DRAG_MOVE
};

static void scrollzoom_on_destroy(ScrollZoom*, psy_ui_Component* sender);
static void scrollzoom_ondraw(ScrollZoom*, psy_ui_Graphics*);
static void scrollzoom_on_mouse_down(ScrollZoom*, psy_ui_MouseEvent*);
static void scrollzoom_on_mouse_up(ScrollZoom*, psy_ui_MouseEvent*);
static void scrollzoom_onmousemove(ScrollZoom*, psy_ui_MouseEvent*);

static psy_ui_ComponentVtable scrollzoom_vtable;
static bool scrollzoom_vtable_initialized = FALSE;

static void scrollzoomvtable_init(ScrollZoom* self)
{
	if (!scrollzoom_vtable_initialized) {
		scrollzoom_vtable = *(self->component.vtable);		
		scrollzoom_vtable.ondraw = (psy_ui_fp_component_ondraw)scrollzoom_ondraw;
		scrollzoom_vtable.on_mouse_down = (psy_ui_fp_component_on_mouse_event)
			scrollzoom_on_mouse_down;
		scrollzoom_vtable.on_mouse_move = (psy_ui_fp_component_on_mouse_event)
			scrollzoom_onmousemove;
		scrollzoom_vtable.on_mouse_up = (psy_ui_fp_component_on_mouse_event)
			scrollzoom_on_mouse_up;
		scrollzoom_vtable_initialized = TRUE;
	}
	self->component.vtable = &scrollzoom_vtable;
}

void scrollzoom_init(ScrollZoom* self, psy_ui_Component* parent)
{		
	psy_ui_component_init(&self->component, parent, NULL);
	scrollzoomvtable_init(self);
	psy_ui_component_preventalign(&self->component);	
	self->start = 0.0;
	self->end = 1.0;
	self->dragmode = SCROLLZOOM_DRAG_NONE;
	self->dragoffset = 0;	
	psy_signal_init(&self->signal_customdraw);
	psy_signal_init(&self->signal_zoom);	
	psy_signal_connect(&self->component.signal_destroy, self,
		scrollzoom_on_destroy);
}

void scrollzoom_on_destroy(ScrollZoom* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_customdraw);
	psy_signal_dispose(&self->signal_zoom);	
}

void scrollzoom_ondraw(ScrollZoom* self, psy_ui_Graphics* g)
{	
	psy_ui_RealSize size;	
	double zoomleftpx;
	double zoomrightpx;

	size = psy_ui_component_scroll_size_px(&self->component);	
	zoomleftpx = size.width * self->start;
	zoomrightpx = size.width * self->end;
	if (zoomleftpx == zoomrightpx) {
		++zoomrightpx;
	}	
	psy_ui_setcolour(g, psy_ui_colour_make(0x00666666));	
	psy_ui_drawrectangle(g, psy_ui_realrectangle_make(
		psy_ui_realpoint_make(zoomleftpx, 0.0),
		psy_ui_realsize_make(zoomrightpx - zoomleftpx, size.height)));
	psy_signal_emit(&self->signal_customdraw, self, 1, g);
}

void scrollzoom_on_mouse_down(ScrollZoom* self, psy_ui_MouseEvent* ev)
{
	psy_ui_RealSize size;	
	double zoomleftpx;
	double zoomrightpx;
	
	size = psy_ui_component_scroll_size_px(&self->component);	
	zoomrightpx = size.width * self->end;
	if (psy_ui_mouseevent_pt(ev).x >= zoomrightpx - 5.0 && psy_ui_mouseevent_pt(ev).x < zoomrightpx + 5.0) {
		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		self->dragmode = SCROLLZOOM_DRAG_RIGHT;
		self->dragoffset = psy_ui_mouseevent_pt(ev).x - zoomrightpx;
	} else {	
		zoomleftpx = size.width * self->start;
		if (psy_ui_mouseevent_pt(ev).x >= zoomleftpx - 5.0 && psy_ui_mouseevent_pt(ev).x < zoomleftpx + 5.0) {
			psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			self->dragmode = SCROLLZOOM_DRAG_LEFT;
			self->dragoffset = psy_ui_mouseevent_pt(ev).x - zoomleftpx;
		} else
		if (psy_ui_mouseevent_pt(ev).x > zoomleftpx && psy_ui_mouseevent_pt(ev).x < zoomrightpx) {
			self->dragmode = SCROLLZOOM_DRAG_MOVE;
			self->dragoffset = psy_ui_mouseevent_pt(ev).x - zoomleftpx;
		}
	}
	psy_ui_component_capture(&self->component);
}

void scrollzoom_onmousemove(ScrollZoom* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_RealSize size;

	size = psy_ui_component_scroll_size_px(&self->component);	
	if (self->dragmode == SCROLLZOOM_DRAG_NONE) {
		double zoomleftpx;
		double zoomrightpx;

		zoomleftpx = size.width * self->start;
		if (psy_ui_mouseevent_pt(ev).x >= zoomleftpx - 5 && psy_ui_mouseevent_pt(ev).x < zoomleftpx + 5) {
			psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		} else {
			zoomrightpx = size.width * self->end;
			if (psy_ui_mouseevent_pt(ev).x >= zoomrightpx - 5 && psy_ui_mouseevent_pt(ev).x < zoomrightpx + 5) {
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			}
		}
	} else if (self->dragmode == SCROLLZOOM_DRAG_LEFT) {
		double zoomold;

		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		zoomold = self->start;
		self->start = (psy_ui_mouseevent_pt(ev).x - self->dragoffset) / size.width;
		if (self->start > self->end) {
			self->start = self->end;
		} else if (self->start < 0.0) {
			self->start = 0.0;
		} else if (self->start > 1.0) {
			self->start = 1.0;
		}
		if (zoomold != self->end) {
			psy_ui_component_invalidate(&self->component);			
			psy_signal_emit(&self->signal_zoom, self, 0);
		}
	} else if (self->dragmode == SCROLLZOOM_DRAG_RIGHT) {
		double zoomold;

		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		zoomold = self->end;
		self->end = (psy_ui_mouseevent_pt(ev).x - self->dragoffset) / size.width;
		if (self->end < self->start) {
			self->end = self->start;
		} else if (self->end < 0.0) {
			self->end = 0.0;
		} else if (self->end > 1.0) {
			self->end = 1.0;
		}
		if (zoomold != self->end) {
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_update(&self->component);
			psy_signal_emit(&self->signal_zoom, self, 0);
		}
	} else if (self->dragmode == SCROLLZOOM_DRAG_MOVE) {
		double zoomold;
		double length;
		
		zoomold = self->start;
		length = self->end - self->start;
		self->start = (psy_ui_mouseevent_pt(ev).x - self->dragoffset) / size.width;
		if (self->start < 0.0) {
			self->start = 0.0;			
		}
		if (self->start + length > 1.0) {
			self->start = 1.0 - length;
			if (self->start < 0.0) {
				self->start = 0.0;			
			}
		}
		if (self->start != zoomold) {
			self->end = self->start + length;
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_update(&self->component);
			psy_signal_emit(&self->signal_zoom, self, 0);
		}
	}
}

void scrollzoom_on_mouse_up(ScrollZoom* self, psy_ui_MouseEvent* ev)
{
	self->dragmode = SCROLLZOOM_DRAG_NONE;
	psy_ui_component_release_capture(&self->component);
}
