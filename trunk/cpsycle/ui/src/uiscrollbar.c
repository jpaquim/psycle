// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiscrollbar.h"
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

// psy_ui_ScrollBarPane
// prototypes
static void psy_ui_scrollbarpane_ondestroy(psy_ui_ScrollBarPane*,
	psy_ui_Component* sender);
static void psy_ui_scrollbarpane_ondraw(psy_ui_ScrollBarPane*,
	psy_ui_Graphics*);
static void psy_ui_scrollbarpane_onmousedown(psy_ui_ScrollBarPane*,
	psy_ui_MouseEvent*);
static void psy_ui_scrollbarpane_onmouseup(psy_ui_ScrollBarPane*,
	psy_ui_MouseEvent*);
static void psy_ui_scrollbarpane_onmousemove(psy_ui_ScrollBarPane*,
	psy_ui_MouseEvent*);
static double psy_ui_scrollbarpane_step(psy_ui_ScrollBarPane*);
static void psy_ui_scrollbarpane_setthumbposition(psy_ui_ScrollBarPane*,
	double pos);
static void psy_ui_scrollbarpane_enableinput(psy_ui_ScrollBarPane*);
static void psy_ui_scrollbarpane_preventinput(psy_ui_ScrollBarPane*);
// vtable
static psy_ui_ComponentVtable psy_ui_scrollbarpane_vtable;
static bool psy_ui_scrollbarpane_vtable_initialized = FALSE;

static void psy_ui_scrollbarpane_vtable_init(psy_ui_ScrollBarPane* self)
{
	if (!psy_ui_scrollbarpane_vtable_initialized) {
		psy_ui_scrollbarpane_vtable = *(self->component.vtable);
		psy_ui_scrollbarpane_vtable.ondraw = (psy_ui_fp_component_ondraw)
			psy_ui_scrollbarpane_ondraw;
		psy_ui_scrollbarpane_vtable.onmousedown =
			(psy_ui_fp_component_onmousedown)psy_ui_scrollbarpane_onmousedown;
		psy_ui_scrollbarpane_vtable.onmousemove =
			(psy_ui_fp_component_onmousemove)psy_ui_scrollbarpane_onmousemove;
		// psy_ui_scrollbarpane_vtable.onmousewheel =
		//	(psy_ui_fp_component_onmousewheel)
		//	psy_ui_scrollbarpane_onmousewheel;
		psy_ui_scrollbarpane_vtable.onmouseup =
			(psy_ui_fp_component_onmouseup)psy_ui_scrollbarpane_onmouseup;		
		psy_ui_scrollbarpane_vtable.enableinput =
			(psy_ui_fp_component_enableinput)psy_ui_scrollbarpane_enableinput;
		psy_ui_scrollbarpane_vtable.preventinput =
			(psy_ui_fp_component_preventinput)
			psy_ui_scrollbarpane_preventinput;
		psy_ui_scrollbarpane_vtable_initialized = TRUE;
	}
}
// implementation
void psy_ui_scrollbarpane_init(psy_ui_ScrollBarPane* self,
	psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_scrollbarpane_vtable_init(self);
	self->component.vtable = &psy_ui_scrollbarpane_vtable;
	psy_ui_component_preventalign(&self->component);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setbackgroundcolour(&self->component,
		psy_ui_colour_make(0x00292929));
	psy_ui_component_setcolour(&self->component,
		psy_ui_colour_make(0x00666666));
	self->pos = 0;
	self->drag = 0;
	self->screenpos = 0;	
	self->orientation = psy_ui_VERTICAL;
	self->scrollmin = 0;
	self->scrollmax = 0;
	self->screenpos = 0;
	self->enabled = TRUE;
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_clicked);
	psy_signal_connect(&self->component.signal_destroy, self,
		psy_ui_scrollbarpane_ondestroy);
}

void psy_ui_scrollbarpane_ondestroy(psy_ui_ScrollBarPane* self,
	psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_clicked);
}

void psy_ui_scrollbarpane_setorientation(psy_ui_ScrollBarPane* self,
	psy_ui_Orientation orientation)
{
	self->orientation = orientation;
}

void psy_ui_scrollbarpane_ondraw(psy_ui_ScrollBarPane* self,
	psy_ui_Graphics* g)
{
	if (self->enabled) {
		psy_ui_RealSize size;
		psy_ui_RealRectangle r;

		size = psy_ui_component_sizepx(&self->component);
		if (self->orientation == psy_ui_HORIZONTAL) {
			r = psy_ui_realrectangle_make(
				psy_ui_realpoint_make(self->screenpos, 2.0),
				psy_ui_realsize_make(20, size.height - 4));
		} else {
			r = psy_ui_realrectangle_make(
				psy_ui_realpoint_make(2.0, self->screenpos),
				psy_ui_realsize_make(size.width - 4, 20));
		}
		psy_ui_drawsolidrectangle(g, r,
			psy_ui_component_colour(&self->component));
	}
}

void psy_ui_scrollbarpane_onmousedown(psy_ui_ScrollBarPane* self,
	psy_ui_MouseEvent* ev)
{
	if (self->orientation == psy_ui_HORIZONTAL) {
		self->dragoffset = ev->pt.x - self->screenpos;
	} else if (self->orientation == psy_ui_VERTICAL) {
		self->dragoffset = ev->pt.y - self->screenpos;
	}
	self->drag = 1;
	psy_ui_component_capture(&self->component);
}

void psy_ui_scrollbarpane_onmouseup(psy_ui_ScrollBarPane* self,
	psy_ui_MouseEvent* ev)
{
	psy_ui_scrollbarpane_setthumbposition(self, self->pos);
	self->drag = 0;
	psy_ui_component_releasecapture(&self->component);	
}

void psy_ui_scrollbarpane_onmousemove(psy_ui_ScrollBarPane* self,
	psy_ui_MouseEvent* ev)
{
	if (self->drag) {
		double step;
		double pos;
		psy_ui_RealSize size;		
		
		size = psy_ui_component_sizepx(&self->component);
		if (self->orientation == psy_ui_HORIZONTAL) {
			self->screenpos = max(0, min(ev->pt.x - self->dragoffset,
				size.width - 20));
		} else {
			self->screenpos = max(0, min(ev->pt.y - self->dragoffset,
				size.height - 20));
		}
		psy_ui_component_invalidate(&self->component);
		psy_ui_component_update(&self->component);
		pos = self->scrollmin;
		step = psy_ui_scrollbarpane_step(self);
		pos = ((self->screenpos) * step) + self->scrollmin;		
		if (pos < self->scrollmin) {
			pos = self->scrollmin;
		} else if (pos > self->scrollmax) {
			pos = self->scrollmax;
		}
		if (pos != self->pos) {
			self->pos = pos;
			psy_signal_emit(&self->signal_changed, self, 0);
		}		
	}
}

void psy_ui_scrollbarpane_setthumbposition(psy_ui_ScrollBarPane* self,
	double pos)
{
	double step;

	if (pos < self->scrollmin) {
		pos = self->scrollmin;
	}
	if (pos > self->scrollmax) {
		pos = self->scrollmax;
	}
	step = psy_ui_scrollbarpane_step(self);
	if (step != 0.0) {
		self->screenpos = (1.0 / step) * floor(pos - self->scrollmin);
		self->pos = pos;
		psy_ui_component_invalidate(&self->component);
	}
}

double psy_ui_scrollbarpane_step(psy_ui_ScrollBarPane* self)
{
	double rv;
	psy_ui_RealSize size;	
	psy_ui_RealSize panesize;
	
	panesize = psy_ui_component_sizepx(&self->component);
	size.width = 20;
	size.height = 20;
	if (self->orientation == psy_ui_HORIZONTAL) {
		rv = (self->scrollmax - self->scrollmin) /
			(panesize.width - size.width);
	} else if (self->orientation == psy_ui_VERTICAL) {
		rv = (self->scrollmax - self->scrollmin) /
			(panesize.height - size.height);
	} else {
		rv = 0.0;
	}
	return rv;
}

void psy_ui_scrollbarpane_enableinput(psy_ui_ScrollBarPane* self)
{
	self->enabled = TRUE;
	psy_ui_component_setbackgroundcolour(&self->component,
		psy_ui_colour_make(0x00292929));
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_scrollbarpane_preventinput(psy_ui_ScrollBarPane* self)
{
	self->enabled = FALSE;
	psy_ui_component_setbackgroundcolour(&self->component,
		psy_ui_defaults()->style_common.backgroundcolour);
	psy_ui_component_invalidate(&self->component);
}

// psy_ui_ScrollBar
// prototypes
static void psy_ui_scrollbar_ondestroy(psy_ui_ScrollBar*,
	psy_ui_Component* sender);
static void psy_ui_scrollbar_onless(psy_ui_ScrollBar*,
	psy_ui_Component* sender);
static void psy_ui_scrollbar_onmore(psy_ui_ScrollBar*,
	psy_ui_Component* sender);
static void psy_ui_scrollbar_onscrollpanechanged(psy_ui_ScrollBar*,
	psy_ui_ScrollBarPane* sender);
static void psy_ui_scrollbar_onscrollpaneclicked(psy_ui_ScrollBar*,
	psy_ui_ScrollBarPane* sender);
// static void psy_ui_scrollbar_onmousewheel(psy_ui_ScrollBar*,
//		psy_ui_Component* sender, psy_ui_MouseEvent*);
// implementation
void psy_ui_scrollbar_init(psy_ui_ScrollBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_destroy, self,
		psy_ui_scrollbar_ondestroy);
	// Less Button
	psy_ui_button_init_connect(&self->less, &self->component,
		self, psy_ui_scrollbar_onless);
	psy_ui_button_seticon(&self->less, psy_ui_ICON_UP);
	psy_ui_button_setcharnumber(&self->less, 2);
	psy_ui_component_setalign(&self->less.component, psy_ui_ALIGN_TOP);	
	// More Button
	psy_ui_button_init_connect(&self->more, &self->component,
		self, psy_ui_scrollbar_onmore);
	psy_ui_button_seticon(&self->more, psy_ui_ICON_DOWN);
	psy_ui_button_setcharnumber(&self->more, 2);
	psy_ui_component_setalign(&self->more.component, psy_ui_ALIGN_BOTTOM);
	// Pane
	psy_ui_scrollbarpane_init(&self->sliderpane, &self->component);
	psy_ui_component_setalign(&self->sliderpane.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setpreferredsize(&self->sliderpane.component,
		psy_ui_size_make(
			psy_ui_value_makeew(2.0),
			psy_ui_value_makeeh(1.0)));	
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_clicked);
	psy_signal_connect(&self->sliderpane.signal_changed, self,
		psy_ui_scrollbar_onscrollpanechanged);	
	psy_signal_connect(&self->sliderpane.signal_clicked, self,
		psy_ui_scrollbar_onscrollpaneclicked);
}

void psy_ui_scrollbar_setorientation(psy_ui_ScrollBar* self,
	psy_ui_Orientation orientation)
{
	psy_ui_scrollbarpane_setorientation(&self->sliderpane, orientation);
	if (orientation == psy_ui_HORIZONTAL) {
		psy_ui_button_seticon(&self->less, psy_ui_ICON_LESS);
		psy_ui_component_setalign(&self->less.component, psy_ui_ALIGN_LEFT);
		psy_ui_button_seticon(&self->more, psy_ui_ICON_MORE);
		psy_ui_component_setalign(&self->more.component, psy_ui_ALIGN_RIGHT);
		psy_ui_component_setpreferredsize(&self->sliderpane.component,
			psy_ui_size_makeem(1.0, 1.0));				
	} else if (orientation == psy_ui_VERTICAL) {
		psy_ui_button_seticon(&self->less, psy_ui_ICON_UP);
		psy_ui_component_setalign(&self->less.component, psy_ui_ALIGN_TOP);
		psy_ui_button_seticon(&self->more, psy_ui_ICON_DOWN);
		psy_ui_component_setalign(&self->more.component, psy_ui_ALIGN_BOTTOM);
		psy_ui_component_setpreferredsize(&self->sliderpane.component,
			psy_ui_size_makeem(2.5, 1.0));				
	}
}

void psy_ui_scrollbar_ondestroy(psy_ui_ScrollBar* self,
	psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_clicked);
}

void psy_ui_scrollbar_onmousewheel(psy_ui_ScrollBar* self,
	psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{
}

double psy_ui_scrollbar_position(psy_ui_ScrollBar* self)
{	
	return self->sliderpane.pos;
}

void psy_ui_scrollbar_setscrollrange(psy_ui_ScrollBar* self, double scrollmin,
	double scrollmax)
{
	self->sliderpane.scrollmin = scrollmin;
	self->sliderpane.scrollmax = scrollmax;
	self->sliderpane.pos = scrollmin;
}

void psy_ui_scrollbar_scrollrange(psy_ui_ScrollBar* self, double* scrollmin,
	double* scrollmax)
{
	*scrollmin = self->sliderpane.scrollmin;
	*scrollmax = self->sliderpane.scrollmax;
}

void psy_ui_scrollbar_onless(psy_ui_ScrollBar* self, psy_ui_Component* sender)
{
	psy_ui_scrollbar_setthumbposition(self,
		psy_ui_scrollbar_position(self) - 1);
	psy_signal_emit(&self->signal_changed, self, 0);
}

void psy_ui_scrollbar_onmore(psy_ui_ScrollBar* self, psy_ui_Component* sender)
{
	psy_ui_scrollbar_setthumbposition(self,
		psy_ui_scrollbar_position(self) + 1);
	psy_signal_emit(&self->signal_changed, self, 0);
}

void psy_ui_scrollbar_onscrollpanechanged(psy_ui_ScrollBar* self,
	psy_ui_ScrollBarPane* sender)
{
	psy_signal_emit(&self->signal_changed, self, 0);
}

void psy_ui_scrollbar_onscrollpaneclicked(psy_ui_ScrollBar* self,
	psy_ui_ScrollBarPane* sender)
{
	psy_signal_emit(&self->signal_clicked, self, 0);
}

void psy_ui_scrollbar_setthumbposition(psy_ui_ScrollBar* self, double pos)
{
	psy_ui_scrollbarpane_setthumbposition(&self->sliderpane, pos);
}
