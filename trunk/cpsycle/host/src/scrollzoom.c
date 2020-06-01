// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "scrollzoom.h"

#include "../../detail/portable.h"

enum {
	SCROLLZOOM_DRAG_NONE,
	SCROLLZOOM_DRAG_LEFT,
	SCROLLZOOM_DRAG_RIGHT,
	SCROLLZOOM_DRAG_MOVE
};

static void scrollzoom_ondestroy(ScrollZoom*, psy_ui_Component* sender);
static void scrollzoom_ondraw(ScrollZoom*, psy_ui_Graphics*);
static void scrollzoom_onmousedown(ScrollZoom*, psy_ui_MouseEvent*);
static void scrollzoom_onmouseup(ScrollZoom*, psy_ui_MouseEvent*);
static void scrollzoom_onmousemove(ScrollZoom*, psy_ui_MouseEvent*);

static psy_ui_ComponentVtable scrollzoom_vtable;
static bool scrollzoom_vtable_initialized = FALSE;

static void scrollzoomvtable_init(ScrollZoom* self)
{
	if (!scrollzoom_vtable_initialized) {
		scrollzoom_vtable = *(self->component.vtable);		
		scrollzoom_vtable.ondraw = (psy_ui_fp_ondraw)scrollzoom_ondraw;
		scrollzoom_vtable.onmousedown = (psy_ui_fp_onmousedown)
			scrollzoom_onmousedown;
		scrollzoom_vtable.onmousemove = (psy_ui_fp_onmousemove)
			scrollzoom_onmousemove;
		scrollzoom_vtable.onmouseup = (psy_ui_fp_onmouseup)
			scrollzoom_onmouseup;
		scrollzoom_vtable_initialized = TRUE;
	}
	self->component.vtable = &scrollzoom_vtable;
}

void scrollzoom_init(ScrollZoom* self, psy_ui_Component* parent)
{		
	psy_ui_component_init(&self->component, parent);
	scrollzoomvtable_init(self);
	psy_ui_component_doublebuffer(&self->component);
	self->start = 0.f;
	self->end = 1.f;
	self->dragmode = SCROLLZOOM_DRAG_NONE;
	self->dragoffset = 0;	
	psy_signal_init(&self->signal_customdraw);
	psy_signal_init(&self->signal_zoom);	
	psy_signal_connect(&self->component.signal_destroy, self,
		scrollzoom_ondestroy);
}

void scrollzoom_ondestroy(ScrollZoom* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_customdraw);
	psy_signal_dispose(&self->signal_zoom);	
}

void scrollzoom_ondraw(ScrollZoom* self, psy_ui_Graphics* g)
{
	psy_ui_Rectangle r;
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	int zoomleftx;
	int zoomrightx;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	zoomleftx = (int)(psy_ui_value_px(&size.width, &tm) * self->start);
	zoomrightx = (int)(psy_ui_value_px(&size.width, &tm) * self->end);
	if (zoomleftx == zoomrightx) {
		++zoomrightx;
	}	
	psy_ui_setcolor(g, 0x00666666);
	psy_ui_setrectangle(&r, zoomleftx, 0, zoomrightx - zoomleftx,
		psy_ui_value_px(&size.height, &tm));
	psy_ui_drawrectangle(g, r);
	psy_signal_emit(&self->signal_customdraw, self, 1, g);
}

void scrollzoom_onmousedown(ScrollZoom* self, psy_ui_MouseEvent* ev)
{
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	int zoomleftx;
	int zoomrightx;
	
	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	zoomrightx = (int)(psy_ui_value_px(&size.width, &tm) * self->end);
	if (ev->x >= zoomrightx - 5 && ev->x < zoomrightx + 5) {
		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		self->dragmode = SCROLLZOOM_DRAG_RIGHT;
		self->dragoffset = ev->x - zoomrightx;
	} else {	
		zoomleftx = (int)(psy_ui_value_px(&size.width, &tm) * self->start);
		if (ev->x >= zoomleftx - 5 && ev->x < zoomleftx + 5) {
			psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			self->dragmode = SCROLLZOOM_DRAG_LEFT;
			self->dragoffset = ev->x - zoomleftx;
		} else
		if (ev->x > zoomleftx && ev->x < zoomrightx) {
			self->dragmode = SCROLLZOOM_DRAG_MOVE;
			self->dragoffset = ev->x - zoomleftx;
		}
	}
	psy_ui_component_capture(&self->component);
}

void scrollzoom_onmousemove(ScrollZoom* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_Size size;
	psy_ui_TextMetric tm;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	if (self->dragmode == SCROLLZOOM_DRAG_NONE) {
		int zoomleftx;
		int zoomrightx;

		zoomleftx = (int)(psy_ui_value_px(&size.width, &tm) * self->start);
		if (ev->x >= zoomleftx - 5 && ev->x < zoomleftx + 5) {
			psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		} else {
			zoomrightx = (int)(psy_ui_value_px(&size.width, &tm) * self->end);
			if (ev->x >= zoomrightx - 5 && ev->x < zoomrightx + 5) {
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			}
		}
	} else 
	if (self->dragmode == SCROLLZOOM_DRAG_LEFT) {
		float zoomold;

		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		zoomold = self->start;
		self->start = (ev->x - self->dragoffset) /
			(float)psy_ui_value_px(&size.width, &tm);
		if (self->start > self->end) {
			self->start = self->end;
		} else
		if (self->start < 0.f) {
			self->start = 0.f;
		} else
		if (self->start > 1.f) {
			self->start = 1.f;
		}
		if (zoomold != self->end) {
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_update(&self->component);
			psy_signal_emit(&self->signal_zoom, self, 0);
		}
	} else
	if (self->dragmode == SCROLLZOOM_DRAG_RIGHT) {
		float zoomold;

		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		zoomold = self->end;
		self->end = (ev->x - self->dragoffset) /
			(float)psy_ui_value_px(&size.width, &tm);
		if (self->end < self->start) {
			self->end = self->start;
		} else
		if (self->end < 0.f) {
			self->end = 0.f;
		} else
		if (self->end > 1.f) {
			self->end = 1.f;
		}
		if (zoomold != self->end) {
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_update(&self->component);
			psy_signal_emit(&self->signal_zoom, self, 0);
		}
	} else
	if (self->dragmode == SCROLLZOOM_DRAG_MOVE) {
		float zoomold;
		float length;
		
		zoomold = self->start;
		length = self->end - self->start;
		self->start = (ev->x - self->dragoffset) /
			(float)psy_ui_value_px(&size.width, &tm);
		if (self->start < 0.f) {
			self->start = 0.f;			
		}
		if (self->start + length > 1.f) {
			self->start = 1.f - length;
			if (self->start < 0.f) {
				self->start = 0.f;			
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

void scrollzoom_onmouseup(ScrollZoom* self, psy_ui_MouseEvent* ev)
{
	self->dragmode = SCROLLZOOM_DRAG_NONE;
	psy_ui_component_releasecapture(&self->component);
}
