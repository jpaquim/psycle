// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "miniview.h"
// platform
#include "../../detail/portable.h"
#ifdef DIVERSALIS__OS__MICROSOFT
// thread for bitmap update
#include <windows.h>
#include <process.h>
#endif


enum {
	MINIVIEW_DRAG_NONE,
	MINIVIEW_DRAG_LEFT,
	MINIVIEW_DRAG_RIGHT,
	MINIVIEW_DRAG_MOVE
};

static void miniview_customdraw(MiniView*, psy_ui_Graphics*);
static void miniview_drawborder(MiniView*, psy_ui_Graphics*);
static void miniview_ondestroy(MiniView*, psy_ui_Component* sender);
static void miniview_ondraw(MiniView*, psy_ui_Graphics*);
static void miniview_onmousedown(MiniView*, psy_ui_MouseEvent*);
static void miniview_onmouseup(MiniView*, psy_ui_MouseEvent*);
static void miniview_onmousemove(MiniView*, psy_ui_MouseEvent*);
static void miniview_onpreferredsize(MiniView*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void miniview_updatebitmap(MiniView*);
static void miniview_startupdate(MiniView*);
static void miniview_ontimer(MiniView*, uintptr_t id);

static psy_ui_ComponentVtable miniview_vtable;
static bool miniview_vtable_initialized = FALSE;

static void miniviewvtable_init(MiniView* self)
{
	if (!miniview_vtable_initialized) {
		miniview_vtable = *(self->component.vtable);		
		miniview_vtable.ondraw = (psy_ui_fp_component_ondraw)miniview_ondraw;
		miniview_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			miniview_onmousedown;
		miniview_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)
			miniview_onmousemove;
		miniview_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)
			miniview_onmouseup;
		miniview_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			miniview_onpreferredsize;
		miniview_vtable.ontimer = (psy_ui_fp_component_ontimer)
			miniview_ontimer;
		miniview_vtable_initialized = TRUE;
	}
	self->component.vtable = &miniview_vtable;
}

void miniview_init(MiniView* self, psy_ui_Component* parent)
{		
	psy_ui_component_init(&self->component, parent);
	miniviewvtable_init(self);
	//psy_ui_component_setpreferredsize(&self->component,
		//psy_ui_size_makeem(10.0, 2.0));
	//psy_ui_component_preventalign(&self->component);
	psy_ui_component_doublebuffer(&self->component);
	self->start = 0.0;
	self->end = 1.0;
	self->dragmode = MINIVIEW_DRAG_NONE;
	self->dragoffset = 0;
	self->opcount = UINTPTR_MAX;
	psy_signal_init(&self->signal_customdraw);
	psy_signal_init(&self->signal_zoom);	
	psy_signal_connect(&self->component.signal_destroy, self,
		miniview_ondestroy);
	self->view = NULL;
	self->threadrunning = FALSE;	
}

void miniview_ondestroy(MiniView* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_customdraw);
	psy_signal_dispose(&self->signal_zoom);	
	if (self->view) {
		psy_ui_bitmap_dispose(&self->scaledbitmap);
		psy_ui_bitmap_dispose(&self->bitmap);
	}	
}

void miniview_ondraw(MiniView* self, psy_ui_Graphics* g)
{	
	psy_ui_RealSize size;	
	double zoomleftpx;
	double zoomrightpx;

	size = psy_ui_component_sizepx(&self->component);	
	zoomleftpx = size.width * self->start;
	zoomrightpx = size.width * self->end;
	if (zoomleftpx == zoomrightpx) {
		++zoomrightpx;
	}	
	psy_ui_setcolour(g, psy_ui_colour_make(0x00666666));		
	psy_signal_emit(&self->signal_customdraw, self, 1, g);
	if (self->view) {
		miniview_customdraw(self, g);	
	}
	miniview_drawborder(self, g);
}

void miniview_customdraw(MiniView* self, psy_ui_Graphics* g)
{		
	if (!self->threadrunning && self->opcount != self->view->opcount) {		
		miniview_startupdate(self);
	}
	psy_ui_drawfullbitmap(g, &self->scaledbitmap, psy_ui_realpoint_zero());
}

void miniview_updatebitmap(MiniView* self)
{
	psy_ui_Graphics scaledmem;	
	psy_ui_Graphics mem;
	psy_ui_Size preferredsize;
	psy_ui_RealSize preferredsizepx;
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(self->view);
	preferredsize = psy_ui_component_preferredsize(self->view, NULL);
	size = psy_ui_component_sizepx(self->view);
	preferredsizepx.width = psy_ui_value_px(&preferredsize.width, tm);
	preferredsizepx.height = psy_ui_value_px(&preferredsize.height, tm);
	if (preferredsizepx.width < size.width) {
		preferredsizepx.width = size.width;
	}
	if (preferredsizepx.height < size.height) {
		preferredsizepx.height = size.height;
	}	
	psy_ui_graphics_init_bitmap(&mem, &self->bitmap);
	psy_ui_drawsolidrectangle(&mem,
		psy_ui_realrectangle_make(psy_ui_realpoint_zero(),
			preferredsizepx),
		psy_ui_component_backgroundcolour(&self->component));
	if (self->view->vtable->ondraw) {
		self->view->vtable->ondraw(self->view, &mem);
	}
	psy_ui_graphics_dispose(&mem);
	psy_ui_bitmap_init_size(&self->scaledbitmap, psy_ui_component_sizepx(self->view));
	psy_ui_graphics_init_bitmap(&scaledmem, &self->scaledbitmap);
	psy_ui_drawstretchedbitmap(&scaledmem, &self->bitmap,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_zero(),
			psy_ui_component_sizepx(&self->component)),
		psy_ui_realpoint_zero(),
		preferredsizepx);
	psy_ui_graphics_dispose(&scaledmem);	
	self->threadrunning = FALSE;
}

void miniview_startupdate(MiniView* self)
{
	assert(self);
	
#ifdef DIVERSALIS__OS__MICROSOFT
	self->threadrunning = TRUE;
	self->opcount = self->view->opcount;
	_beginthread(miniview_updatebitmap, 0, self);	
#else	
#endif
}


void miniview_drawborder(MiniView* self, psy_ui_Graphics* g)
{	
	psy_ui_RealRectangle r;
	psy_ui_RealSize size;
	double radius;

	size = psy_ui_component_sizepx(&self->component);
	r = psy_ui_realrectangle_make(psy_ui_realpoint_zero(), size);
	radius = 5.0;
	psy_ui_drawroundrectangle(g, r, psy_ui_size_makepx(radius, radius));
	psy_ui_setcolour(g, psy_ui_component_backgroundcolour(&self->component));
	psy_ui_drawline(g, psy_ui_realpoint_make(radius, 0.0),
		psy_ui_realpoint_make(size.width - radius, 0.0));
	psy_ui_drawline(g, psy_ui_realpoint_make(radius, size.height - 1.0),
		psy_ui_realpoint_make(size.width - radius, size.height - 1.0));
	psy_ui_drawline(g, psy_ui_realpoint_make(0.0, radius),
		psy_ui_realpoint_make(0.0, size.height - radius));
	psy_ui_drawline(g, psy_ui_realpoint_make(size.width - 1.0, radius),
		psy_ui_realpoint_make(size.width - 1.0, size.height - radius));
}

void miniview_onmousedown(MiniView* self, psy_ui_MouseEvent* ev)
{
	psy_ui_RealSize size;	
	double zoomleftpx;
	double zoomrightpx;
	
	size = psy_ui_component_sizepx(&self->component);	
	zoomrightpx = size.width * self->end;
	if (ev->pt.x >= zoomrightpx - 5.0 && ev->pt.x < zoomrightpx + 5.0) {
		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		self->dragmode = MINIVIEW_DRAG_RIGHT;
		self->dragoffset = ev->pt.x - zoomrightpx;
	} else {	
		zoomleftpx = size.width * self->start;
		if (ev->pt.x >= zoomleftpx - 5.0 && ev->pt.x < zoomleftpx + 5.0) {
			psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			self->dragmode = MINIVIEW_DRAG_LEFT;
			self->dragoffset = ev->pt.x - zoomleftpx;
		} else
		if (ev->pt.x > zoomleftpx && ev->pt.x < zoomrightpx) {
			self->dragmode = MINIVIEW_DRAG_MOVE;
			self->dragoffset = ev->pt.x - zoomleftpx;
		}
	}
	psy_ui_component_capture(&self->component);
}

void miniview_onmousemove(MiniView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_RealSize size;

	size = psy_ui_component_sizepx(&self->component);	
	if (self->dragmode == MINIVIEW_DRAG_NONE) {
		double zoomleftpx;
		double zoomrightpx;

		zoomleftpx = size.width * self->start;
		if (ev->pt.x >= zoomleftpx - 5 && ev->pt.x < zoomleftpx + 5) {
			psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		} else {
			zoomrightpx = size.width * self->end;
			if (ev->pt.x >= zoomrightpx - 5 && ev->pt.x < zoomrightpx + 5) {
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			}
		}
	} else if (self->dragmode == MINIVIEW_DRAG_LEFT) {
		double zoomold;

		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		zoomold = self->start;
		self->start = (ev->pt.x - self->dragoffset) / size.width;
		if (self->start > self->end) {
			self->start = self->end;
		} else if (self->start < 0.0) {
			self->start = 0.0;
		} else if (self->start > 1.0) {
			self->start = 1.0;
		}
		if (zoomold != self->end) {
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_update(&self->component);
			psy_signal_emit(&self->signal_zoom, self, 0);
		}
	} else if (self->dragmode == MINIVIEW_DRAG_RIGHT) {
		double zoomold;

		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
		zoomold = self->end;
		self->end = (ev->pt.x - self->dragoffset) / size.width;
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
	} else if (self->dragmode == MINIVIEW_DRAG_MOVE) {
		double zoomold;
		double length;
		
		zoomold = self->start;
		length = self->end - self->start;
		self->start = (ev->pt.x - self->dragoffset) / size.width;
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

void miniview_onmouseup(MiniView* self, psy_ui_MouseEvent* ev)
{
	self->dragmode = MINIVIEW_DRAG_NONE;
	psy_ui_component_releasecapture(&self->component);
}

void miniview_onpreferredsize(MiniView* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	*rv = psy_ui_size_makeem(8.0, 1.5);
}

void miniview_ontimer(MiniView* self, uintptr_t id)
{
	psy_ui_component_invalidate(&self->component);
}

void miniview_setview(MiniView* self, psy_ui_Component* view)
{
	if (self->view) {
		psy_ui_bitmap_dispose(&self->scaledbitmap);
		psy_ui_bitmap_dispose(&self->bitmap);
	}
	self->view = view;
	self->opcount = UINTPTR_MAX;
	if (self->view) {		
		psy_ui_Size preferredsize;
		psy_ui_RealSize preferredsizepx;
		psy_ui_RealSize size;
		const psy_ui_TextMetric* tm;

		tm = psy_ui_component_textmetric(self->view);
		preferredsize = psy_ui_component_preferredsize(self->view, NULL);
		size = psy_ui_component_sizepx(self->view);
		preferredsizepx.width = psy_ui_value_px(&preferredsize.width, tm);
		preferredsizepx.height = psy_ui_value_px(&preferredsize.height, tm);
		if (preferredsizepx.width < size.width) {
			preferredsizepx.width = size.width;
		}
		if (preferredsizepx.height < size.height) {
			preferredsizepx.height = size.height;
		}
		psy_ui_bitmap_init_size(&self->bitmap, preferredsizepx);
		psy_ui_bitmap_init_size(&self->scaledbitmap,
			psy_ui_component_sizepx(self->view));
	}
	if (self->view) {
		psy_ui_component_starttimer(&self->component, 0, 200);
	} else {
		psy_ui_component_invalidate(&self->component);
		psy_ui_component_stoptimer(&self->component, 0, 200);
	}
}