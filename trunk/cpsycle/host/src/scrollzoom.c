// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "scrollzoom.h"
#include "../../detail/portable.h"

static void scrollzoom_ondestroy(ScrollZoom*, psy_ui_Component* sender);
static void scrollzoom_ondraw(ScrollZoom*, psy_ui_Component* sender,
	psy_ui_Graphics*);
static void scrollzoom_onmousedown(ScrollZoom*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void scrollzoom_onmouseup(ScrollZoom*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void scrollzoom_onmousemove(ScrollZoom*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);

enum {
	SCROLLZOOM_DRAG_NONE,
	SCROLLZOOM_DRAG_LEFT,
	SCROLLZOOM_DRAG_RIGHT,
	SCROLLZOOM_DRAG_MOVE
};

void scrollzoom_init(ScrollZoom* self, psy_ui_Component* parent)
{	
	self->zoomleft = 0.f;
	self->zoomright = 1.f;	
	self->dragmode = SCROLLZOOM_DRAG_NONE;
	self->dragoffset = 00;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_doublebuffer(&self->component);
	psy_signal_init(&self->signal_customdraw);
	psy_signal_init(&self->signal_zoom);	
	psy_signal_connect(&self->component.signal_destroy, self,
		scrollzoom_ondestroy);
	psy_signal_connect(&self->component.signal_draw, self,
		scrollzoom_ondraw);
	psy_signal_connect(&self->component.signal_mousedown, self,
		scrollzoom_onmousedown);
	psy_signal_connect(&self->component.signal_mouseup, self,
		scrollzoom_onmouseup);
	psy_signal_connect(&self->component.signal_mousemove, self,
		scrollzoom_onmousemove);
	psy_ui_component_resize(&self->component, 100, 50);
}

void scrollzoom_ondestroy(ScrollZoom* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_customdraw);
	psy_signal_dispose(&self->signal_zoom);	
}

void scrollzoom_ondraw(ScrollZoom* self, psy_ui_Component* sender,
	psy_ui_Graphics* g)
{
	psy_ui_Rectangle r;
	psy_ui_Size size;
	int zoomleftx;
	int zoomrightx;

	size = psy_ui_component_size(&self->component);
	zoomleftx = (int)(size.width * self->zoomleft);
	zoomrightx = (int)(size.width * self->zoomright);
	if (zoomleftx == zoomrightx) {
		++zoomrightx;
	}	
	psy_ui_setcolor(g, 0x00666666);
	psy_ui_setrectangle(&r, zoomleftx, 0, zoomrightx - zoomleftx, size.height);
	psy_ui_drawrectangle(g, r);
	psy_signal_emit(&self->signal_customdraw, self, 1, g);
}

void scrollzoom_onmousedown(ScrollZoom* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_ui_Size size;
	int zoomleftx;
	int zoomrightx;
	
	size = psy_ui_component_size(&self->component);
	zoomrightx = (int)(size.width * self->zoomright);
	if (ev->x >= zoomrightx - 5 && ev->x < zoomrightx + 5) {
		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		self->dragmode = SCROLLZOOM_DRAG_RIGHT;
		self->dragoffset = ev->x - zoomrightx;
	} else {	
		zoomleftx = (int)(size.width * self->zoomleft);
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

void scrollzoom_onmousemove(ScrollZoom* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	psy_ui_Size size;	

	size = psy_ui_component_size(&self->component);
	if (self->dragmode == SCROLLZOOM_DRAG_NONE) {
		int zoomleftx;
		int zoomrightx;

		zoomleftx = (int)(size.width * self->zoomleft);
		if (ev->x >= zoomleftx - 5 && ev->x < zoomleftx + 5) {
			psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		} else {
			zoomrightx = (int)(size.width * self->zoomright);
			if (ev->x >= zoomrightx - 5 && ev->x < zoomrightx + 5) {
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			}
		}
	} else 
	if (self->dragmode == SCROLLZOOM_DRAG_LEFT) {
		float zoomold;

		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		zoomold = self->zoomleft;
		self->zoomleft = (ev->x - self->dragoffset) / (float) size.width;
		if (self->zoomleft > self->zoomright) {
			self->zoomleft = self->zoomright;
		} else
		if (self->zoomleft < 0.f) {
			self->zoomleft = 0.f;
		} else
		if (self->zoomleft > 1.f) {
			self->zoomleft = 1.f;
		}
		if (zoomold != self->zoomright) {
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_update(&self->component);
			psy_signal_emit(&self->signal_zoom, self, 0);
		}
	} else
	if (self->dragmode == SCROLLZOOM_DRAG_RIGHT) {
		float zoomold;

		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		zoomold = self->zoomright;
		self->zoomright = (ev->x - self->dragoffset) / (float) size.width;
		if (self->zoomright < self->zoomleft) {
			self->zoomright = self->zoomleft;
		} else
		if (self->zoomright < 0.f) {
			self->zoomright = 0.f;
		} else
		if (self->zoomright > 1.f) {
			self->zoomright = 1.f;
		}
		if (zoomold != self->zoomright) {
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_update(&self->component);
			psy_signal_emit(&self->signal_zoom, self, 0);
		}
	} else
	if (self->dragmode == SCROLLZOOM_DRAG_MOVE) {
		float zoomold;
		float length;
		
		zoomold = self->zoomleft;
		length = self->zoomright - self->zoomleft;
		self->zoomleft = (ev->x - self->dragoffset) / (float) size.width;
		if (self->zoomleft < 0.f) {
			self->zoomleft = 0.f;			
		}
		if (self->zoomleft + length > 1.f) {
			self->zoomleft = 1.f - length;
			if (self->zoomleft < 0.f) {
				self->zoomleft = 0.f;			
			}
		}
		if (self->zoomleft != zoomold) {
			self->zoomright = self->zoomleft + length;
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_update(&self->component);
			psy_signal_emit(&self->signal_zoom, self, 0);
		}
	}
}

void scrollzoom_onmouseup(ScrollZoom* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	self->dragmode = SCROLLZOOM_DRAG_NONE;
	psy_ui_component_releasecapture(&self->component);
}
