// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiscrollbar.h"

#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"
#include "../../detail/trace.h"

static void psy_ui_scrollbar_ondestroy(psy_ui_ScrollBar*, psy_ui_Component* sender);
static void psy_ui_scrollbar_onmousedown(psy_ui_ScrollBar*, psy_ui_Component* sender, psy_ui_MouseEvent*);
static void psy_ui_scrollbar_onmouseup(psy_ui_ScrollBar*, psy_ui_Component* sender, psy_ui_MouseEvent*);
static void psy_ui_scrollbar_onmousemove(psy_ui_ScrollBar*, psy_ui_Component* sender, psy_ui_MouseEvent*);
static void psy_ui_scrollbar_onless(psy_ui_ScrollBar*, psy_ui_Component* sender);
static void psy_ui_scrollbar_onmore(psy_ui_ScrollBar*, psy_ui_Component* sender);
static void psy_ui_scrollbar_onmousewheel(psy_ui_ScrollBar*, psy_ui_Component* sender, psy_ui_MouseEvent*);
static double psy_ui_scrollbar_step(psy_ui_ScrollBar*);


void psy_ui_scrollbar_init(psy_ui_ScrollBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);		
	psy_ui_component_enablealign(&self->component);	
	psy_signal_connect(&self->component.signal_destroy, self,
		psy_ui_scrollbar_ondestroy);
	// Less Button
	psy_ui_button_init(&self->less, &self->component);
	psy_ui_button_seticon(&self->less, psy_ui_ICON_UP);
	psy_ui_button_setcharnumber(&self->less, 2);
	psy_ui_component_setalign(&self->less.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->less.signal_clicked, self, psy_ui_scrollbar_onless);
	// More Button
	psy_ui_button_init(&self->more, &self->component);
	psy_ui_button_seticon(&self->more, psy_ui_ICON_DOWN);
	psy_ui_button_setcharnumber(&self->more, 2);
	psy_ui_component_setalign(&self->more.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->more.signal_clicked, self, psy_ui_scrollbar_onmore);
	// Pane
	psy_ui_component_init(&self->sliderpane, &self->component);
	psy_ui_component_setalign(&self->sliderpane, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setpreferredsize(&self->sliderpane,
		psy_ui_size_make(
			psy_ui_value_makeew(2.0),
			psy_ui_value_makeeh(1.0)));
	//psy_ui_component_doublebuffer(&self->sliderpane);
	psy_ui_component_setbackgroundcolor(&self->sliderpane, psy_ui_color_make(0x00444444));
	// Slider
	psy_ui_component_init(&self->slider, &self->sliderpane);
	psy_ui_component_resize(&self->slider,
		psy_ui_size_make(
			psy_ui_value_makeew(2.0),
			psy_ui_value_makeeh(1.0)));
	//psy_ui_component_doublebuffer(&self->slider);
	psy_ui_component_setbackgroundcolor(&self->slider, psy_ui_color_make(0x00999999));
	psy_signal_connect(&self->slider.signal_mousedown, self,
		psy_ui_scrollbar_onmousedown);
	psy_signal_connect(&self->slider.signal_mousemove, self,
		psy_ui_scrollbar_onmousemove);
	psy_signal_connect(&self->slider.signal_mouseup, self,
		psy_ui_scrollbar_onmouseup);
	self->drag = 0;
	self->scrollmin = 0;
	self->scrollmax = 0;
	self->pos = 0;
	psy_signal_init(&self->signal_changed);
}

void psy_ui_scrollbar_setorientation(psy_ui_ScrollBar* self, psy_ui_Orientation orientation)
{
	self->orientation = orientation;
	if (orientation == psy_ui_HORIZONTAL) {
		psy_ui_button_seticon(&self->less, psy_ui_ICON_LESS);
		psy_ui_component_setalign(&self->less.component, psy_ui_ALIGN_LEFT);
		psy_ui_button_seticon(&self->more, psy_ui_ICON_MORE);
		psy_ui_component_setalign(&self->more.component, psy_ui_ALIGN_RIGHT);
		psy_ui_component_setpreferredsize(&self->sliderpane,
			psy_ui_size_make(
				psy_ui_value_makeew(1.0),
				psy_ui_value_makeeh(1.0)));
		psy_ui_component_resize(&self->slider,
			psy_ui_size_make(
				psy_ui_value_makeew(2.0),
				psy_ui_value_makeeh(1.0)));
	} else if (orientation == psy_ui_VERTICAL) {
		psy_ui_button_seticon(&self->less, psy_ui_ICON_UP);
		psy_ui_component_setalign(&self->less.component, psy_ui_ALIGN_TOP);
		psy_ui_button_seticon(&self->more, psy_ui_ICON_DOWN);
		psy_ui_component_setalign(&self->more.component, psy_ui_ALIGN_BOTTOM);
		psy_ui_component_setpreferredsize(&self->sliderpane,
			psy_ui_size_make(
				psy_ui_value_makeew(2.5),
				psy_ui_value_makeeh(1.0)));
		psy_ui_component_resize(&self->slider,
			psy_ui_size_make(
				psy_ui_value_makeew(2.5),
				psy_ui_value_makeeh(1.0)));
	}
}

void psy_ui_scrollbar_ondestroy(psy_ui_ScrollBar* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
}

void psy_ui_scrollbar_onmousedown(psy_ui_ScrollBar* self, psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{
	psy_ui_Rectangle position;

	position = psy_ui_component_position(&self->slider);
	if (self->orientation == psy_ui_HORIZONTAL) {
		self->dragoffset = ev->x;
	} else if (self->orientation == psy_ui_VERTICAL) {
		self->dragoffset = ev->y;
	}
	self->drag = 1;
	psy_ui_component_capture(&self->slider);
}

void psy_ui_scrollbar_onmousemove(psy_ui_ScrollBar* self, psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{
	if (self->drag) {
		psy_ui_Rectangle position;
		psy_ui_Size size;
		psy_ui_Rectangle paneposition;
		double step;
		intptr_t pos = self->scrollmin;

		position = psy_ui_component_position(&self->slider);
		size = psy_ui_component_size(&self->slider);
		paneposition = psy_ui_component_position(&self->sliderpane);
		step = psy_ui_scrollbar_step(self);
		if (self->orientation == psy_ui_HORIZONTAL) {			
			pos = (intptr_t)((position.left + ev->x - self->dragoffset) * step) + self->scrollmin;
		} else if (self->orientation == psy_ui_VERTICAL) {			
			pos = (intptr_t)((position.top + ev->y - self->dragoffset) * step) + self->scrollmin;
		}
		if (pos < self->scrollmin) {
			pos = self->scrollmin;
		} else if (pos > self->scrollmax) {
			pos = self->scrollmax;
		}		
		if (self->orientation == psy_ui_HORIZONTAL) {
			psy_ui_component_move(&self->slider,
				psy_ui_point_make(
					psy_ui_value_makepx(
						max(0,
							min(paneposition.right - paneposition.left - (position.right - position.left),
								position.left + ev->x - self->dragoffset))),
					psy_ui_value_makepx(position.top)));
		} if (self->orientation == psy_ui_VERTICAL) {
			psy_ui_component_move(&self->slider,
				psy_ui_point_make(
					psy_ui_value_makepx(position.left),
					psy_ui_value_makepx(
						max(0,
							min(paneposition.bottom - paneposition.top - (position.bottom - position.top),
								position.top + ev->y - self->dragoffset)))));
		}
		psy_ui_component_invalidate(&self->sliderpane);
		if (pos != self->pos) {
			self->pos = pos;
			psy_signal_emit(&self->signal_changed, self, 0);
		}
	}
}

void psy_ui_scrollbar_onmouseup(psy_ui_ScrollBar* self, psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{
	psy_ui_scrollbar_setthumbposition(self, self->pos);
	self->drag = 0;
	psy_ui_component_releasecapture(&self->slider);
}

void psy_ui_scrollbar_onmousewheel(psy_ui_ScrollBar* self, psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{

}

intptr_t psy_ui_scrollbar_position(psy_ui_ScrollBar* self)
{	
	return self->pos;	
}

void psy_ui_scrollbar_setscrollrange(psy_ui_ScrollBar* self, intptr_t scrollmin, intptr_t scrollmax)
{
	self->scrollmin = scrollmin;
	self->scrollmax = scrollmax;
	self->pos = scrollmin;
}

void psy_ui_scrollbar_scrollrange(psy_ui_ScrollBar* self, intptr_t* scrollmin, intptr_t* scrollmax)
{
	*scrollmin = self->scrollmin;
	*scrollmax = self->scrollmax;
}

void psy_ui_scrollbar_setthumbposition(psy_ui_ScrollBar* self, intptr_t pos)
{
	psy_ui_Rectangle position;
	psy_ui_Rectangle paneposition;
	double step;

	if (pos < self->scrollmin) {
		pos = self->scrollmin;
	} else
	if (pos > self->scrollmax) {
		pos = self->scrollmax;
	}
	position = psy_ui_component_position(&self->slider);
	paneposition = psy_ui_component_position(&self->sliderpane);
	step = psy_ui_scrollbar_step(self);	
	if (self->orientation == psy_ui_HORIZONTAL) {
		psy_ui_component_move(&self->slider,
			psy_ui_point_make(				
				psy_ui_value_makepx((intptr_t)(1 / step * (pos - self->scrollmin))),
				psy_ui_value_makepx(position.top)));
	} if (self->orientation == psy_ui_VERTICAL) {
		psy_ui_component_move(&self->slider,
			psy_ui_point_make(
				psy_ui_value_makepx(position.left),
				psy_ui_value_makepx((intptr_t)(1 / step * (pos - self->scrollmin)))));
	}
	self->pos = pos;
	psy_ui_component_invalidate(&self->sliderpane);
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

double psy_ui_scrollbar_step(psy_ui_ScrollBar* self)
{
	double rv;
	psy_ui_Rectangle position;
	psy_ui_Size size;
	psy_ui_Rectangle paneposition;	

	position = psy_ui_component_position(&self->slider);
	size = psy_ui_component_size(&self->slider);
	paneposition = psy_ui_component_position(&self->sliderpane);
	if (self->orientation == psy_ui_HORIZONTAL) {
		rv = (double)(self->scrollmax - self->scrollmin) /
			(paneposition.right - paneposition.left - (position.right - position.left));		
	} else if (self->orientation == psy_ui_VERTICAL) {
		rv = (double)(self->scrollmax - self->scrollmin) /
			(paneposition.bottom - paneposition.top - (position.bottom - position.top));
	} else {
		rv = 0.0;
	}
	return rv;
}