/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiscrollbar.h"
/* local */
#include "uiapp.h"
/* std */
#include <math.h>

#define REPEAT_TIMEINTERVAL 50
#define REPEAT_DELAYCOUNT 15
#define REPEAT_DELAYFIRSTCOUNT 5

/* psy_ui_ScrollBarPane */
/* prototypes */
static void psy_ui_scrollbarpane_on_destroy(psy_ui_ScrollBarPane*);
static void psy_ui_scrollbarpane_on_mouse_down(psy_ui_ScrollBarPane*,
	psy_ui_MouseEvent*);
static void psy_ui_scrollbarpane_on_mouse_up(psy_ui_ScrollBarPane*,
	psy_ui_MouseEvent*);
static void psy_ui_scrollbarpane_onmousemove(psy_ui_ScrollBarPane*,
	psy_ui_MouseEvent*);
static double psy_ui_scrollbarpane_step(psy_ui_ScrollBarPane*);
static double psy_ui_scrollbarpane_position_in_scrollrange(
	const psy_ui_ScrollBarPane*, double pos);
static void psy_ui_scrollbarpane_updatethumbposition(psy_ui_ScrollBarPane*);
static void psy_ui_scrollbarpane_on_timer(psy_ui_ScrollBarPane*, uintptr_t id);

/* vtable */
static psy_ui_ComponentVtable psy_ui_scrollbarpane_vtable;
static bool psy_ui_scrollbarpane_vtable_initialized = FALSE;

static void psy_ui_scrollbarpane_vtable_init(psy_ui_ScrollBarPane* self)
{
	assert(self);

	if (!psy_ui_scrollbarpane_vtable_initialized) {
		psy_ui_scrollbarpane_vtable = *(self->component.vtable);
		psy_ui_scrollbarpane_vtable.on_destroy =
			(psy_ui_fp_component_event)
			psy_ui_scrollbarpane_on_destroy;
		psy_ui_scrollbarpane_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_scrollbarpane_on_mouse_down;
		psy_ui_scrollbarpane_vtable.onmousedoubleclick =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_scrollbarpane_on_mouse_down;
		psy_ui_scrollbarpane_vtable.onmousemove =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_scrollbarpane_onmousemove;
		psy_ui_scrollbarpane_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_scrollbarpane_on_mouse_up;
		
		psy_ui_scrollbarpane_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			psy_ui_scrollbarpane_on_timer;
		psy_ui_scrollbarpane_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &psy_ui_scrollbarpane_vtable);
}

/* implementation */
void psy_ui_scrollbarpane_init(psy_ui_ScrollBarPane* self,
	psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_scrollbarpane_vtable_init(self);	
	psy_ui_component_init(&self->thumb, &self->component, NULL);	
	psy_signal_init(&self->signal_changed);
	psy_ui_intpoint_init(&self->scrollrange);
	self->position = 0;
	self->screenpos = 0;
	self->orientation = psy_ui_VERTICAL;	
	self->repeat = 0;
	self->repeatdelaycounter = 0;		
	psy_ui_scrollbarpane_setorientation(self, psy_ui_VERTICAL);	
}

void psy_ui_scrollbarpane_on_destroy(psy_ui_ScrollBarPane* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);	
}

void psy_ui_scrollbarpane_setorientation(psy_ui_ScrollBarPane* self,
	psy_ui_Orientation orientation)
{
	assert(self);

	self->orientation = orientation;
	if (self->orientation == psy_ui_HORIZONTAL) {
		psy_ui_component_set_style_type(&self->component, psy_ui_STYLE_HSCROLLPANE);		
		psy_ui_component_set_style_type(&self->thumb, psy_ui_STYLE_HSCROLLTHUMB);
		psy_ui_component_set_style_type_hover(&self->thumb,
			psy_ui_STYLE_HSCROLLTHUMB_HOVER);
		psy_ui_component_set_style_type_active(&self->thumb,
			psy_ui_STYLE_HSCROLLTHUMB_HOVER);
		psy_ui_component_set_preferred_size(&self->thumb,
			psy_ui_size_make_em(1.0, 1.0));
	} else {
		psy_ui_component_set_style_type(&self->component, psy_ui_STYLE_VSCROLLPANE);
		psy_ui_component_set_style_type(&self->thumb, psy_ui_STYLE_VSCROLLTHUMB);
		psy_ui_component_set_style_type_hover(&self->thumb,
			psy_ui_STYLE_VSCROLLTHUMB_HOVER);
		psy_ui_component_set_style_type_active(&self->thumb,
			psy_ui_STYLE_VSCROLLTHUMB_HOVER);
		psy_ui_component_set_preferred_size(&self->thumb,
			psy_ui_size_make_em(1.0, 1.0));
	}
	psy_ui_scrollbarpane_updatethumbposition(self);
}

void psy_ui_scrollbarpane_updatethumbposition(psy_ui_ScrollBarPane* self)
{
	psy_ui_RealRectangle newposition;
	psy_ui_RealRectangle updateposition;
	psy_ui_RealSize size;
	psy_ui_RealSize thumbsize;
	const psy_ui_TextMetric* tm;

	assert(self);

	size = psy_ui_component_scrollsize_px(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	if (self->orientation == psy_ui_HORIZONTAL) {
		thumbsize = psy_ui_realsize_make(tm->tmAveCharWidth * 2.5,
			size.height);
		newposition = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(self->screenpos, 0.0),
			thumbsize);
	} else {
		thumbsize = psy_ui_realsize_make(size.width, tm->tmHeight);
		newposition = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(0.0, self->screenpos),
			thumbsize);
	}
	updateposition = psy_ui_component_position(&self->thumb);
	psy_ui_component_setposition(&self->thumb,
		psy_ui_rectangle_make_px(&newposition));
	psy_ui_realrectangle_union(&updateposition, &newposition);
	psy_ui_component_invalidaterect(&self->component, updateposition);
}

void psy_ui_scrollbarpane_on_mouse_down(psy_ui_ScrollBarPane* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);
	
	psy_ui_component_set_focus(&self->component);
	if (psy_ui_mouseevent_target(ev) == &self->thumb) {
		psy_ui_component_capture(&self->thumb);
		if (self->orientation == psy_ui_HORIZONTAL) {
			self->dragoffset = psy_ui_mouseevent_pt(ev).x - self->screenpos;
		} else if (self->orientation == psy_ui_VERTICAL) {
			self->dragoffset = psy_ui_mouseevent_pt(ev).y - self->screenpos;
		}
	} else {
		psy_ui_RealRectangle thumbposition;

		thumbposition = psy_ui_component_position(&self->thumb);
		if (self->orientation == psy_ui_HORIZONTAL) {
			if (psy_ui_mouseevent_pt(ev).x > thumbposition.right) {
				self->repeat = 1;
			} else {
				self->repeat = -1;
			}
		} else if (self->orientation == psy_ui_VERTICAL) {
			if (psy_ui_mouseevent_pt(ev).y > thumbposition.bottom) {
				self->repeat = 1;
			} else {
				self->repeat = -1;
			}
		}
		psy_ui_scrollbarpane_setthumbposition(self,
			self->position + self->repeat);
		psy_signal_emit(&self->signal_changed, self, 0);
		self->repeatdelaycounter = REPEAT_DELAYCOUNT;
		psy_ui_component_start_timer(&self->component, 0,
			REPEAT_TIMEINTERVAL);
	}
}

void psy_ui_scrollbarpane_on_mouse_up(psy_ui_ScrollBarPane* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_releasecapture(&self->thumb);
	if (psy_ui_mouseevent_target(ev) == &self->thumb) {
		psy_ui_scrollbarpane_setthumbposition(self, self->position);
	}
	if (self->repeat != 0) {
		psy_ui_component_stop_timer(&self->component, 0);
		self->repeat = 0;
	}
}

void psy_ui_scrollbarpane_onmousemove(psy_ui_ScrollBarPane* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	if ((ev->button_ == 1) && (psy_ui_mouseevent_target(ev) == &self->thumb)) {
		double step;
		double position;
		psy_ui_RealSize size;
		psy_ui_RealSize thumbsize;

		size = psy_ui_component_scrollsize_px(&self->component);
		thumbsize = psy_ui_component_scrollsize_px(&self->thumb);
		if (self->orientation == psy_ui_HORIZONTAL) {
			self->screenpos = psy_max(0, psy_min(
				psy_ui_mouseevent_pt(ev).x - self->dragoffset,
				size.width - thumbsize.width));
		} else {
			self->screenpos = psy_max(0, psy_min(
				psy_ui_mouseevent_pt(ev).y - self->dragoffset,
				size.height - thumbsize.height));
		}
		psy_ui_scrollbarpane_updatethumbposition(self);		
		position = (double)self->scrollrange.x;
		step = psy_ui_scrollbarpane_step(self);		
		position = psy_ui_scrollbarpane_position_in_scrollrange(self,
			((self->screenpos) * step) + self->scrollrange.x);
		if (position != self->position) {
			self->position = position;
			psy_signal_emit(&self->signal_changed, self, 0);
		}
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void psy_ui_scrollbarpane_setthumbposition(psy_ui_ScrollBarPane* self,
	double position)
{
	double step;

	assert(self);
	
	step = psy_ui_scrollbarpane_step(self);
	if (step != 0.0) {
		self->position = psy_ui_scrollbarpane_position_in_scrollrange(self, position);
		self->screenpos = (1.0 / step) * floor(self->position - self->scrollrange.x);
		psy_ui_scrollbarpane_updatethumbposition(self);
	}
}

double psy_ui_scrollbarpane_position_in_scrollrange(
	const psy_ui_ScrollBarPane* self, double pos)
{
	assert(self);

	if (pos < self->scrollrange.x) {
		pos = (double)self->scrollrange.x;
	}
	if (pos > self->scrollrange.y) {
		pos = (double)self->scrollrange.y;
	}
	return pos;
}

double psy_ui_scrollbarpane_step(psy_ui_ScrollBarPane* self)
{
	double rv;
	psy_ui_RealSize size;
	psy_ui_RealSize panesize;

	assert(self);

	panesize = psy_ui_component_scrollsize_px(&self->component);
	size = psy_ui_component_scrollsize_px(&self->thumb);
	if (self->orientation == psy_ui_HORIZONTAL) {
		rv = (self->scrollrange.y - self->scrollrange.x) /
			(panesize.width - size.width);
	} else if (self->orientation == psy_ui_VERTICAL) {
		rv = (self->scrollrange.y - self->scrollrange.x) /
			(panesize.height - size.height);
	} else {
		rv = 0.0;
	}
	return rv;
}

void psy_ui_scrollbarpane_on_timer(psy_ui_ScrollBarPane* self, uintptr_t id)
{
	assert(self);

	if (self->repeatdelaycounter == 0 ||
			self->repeatdelaycounter == REPEAT_DELAYFIRSTCOUNT) {
		psy_ui_scrollbarpane_setthumbposition(self,
			self->position + self->repeat);
		psy_signal_emit(&self->signal_changed, self, 0);
	}
	if (self->repeatdelaycounter > 0) {
		--self->repeatdelaycounter;
	}
}

/* psy_ui_ScrollBar */
/* prototypes */
static void psy_ui_scrollbar_on_destroy(psy_ui_ScrollBar*);
static void psy_ui_scrollbar_onless(psy_ui_ScrollBar*,
	psy_ui_Component* sender);
static void psy_ui_scrollbar_onmore(psy_ui_ScrollBar*,
	psy_ui_Component* sender);
static void psy_ui_scrollbar_onscrollpanechanged(psy_ui_ScrollBar*,
	psy_ui_ScrollBarPane* sender);
static void psy_ui_scrollbar_onwheel(psy_ui_ScrollBar*, psy_ui_MouseEvent*);

/* vtable */
static psy_ui_ComponentVtable psy_ui_scrollbar_vtable;
static bool psy_ui_scrollbar_vtable_initialized = FALSE;

static void psy_ui_scrollbar_vtable_init(psy_ui_ScrollBar* self)
{
	assert(self);

	if (!psy_ui_scrollbar_vtable_initialized) {
		psy_ui_scrollbar_vtable = *(self->component.vtable);
		psy_ui_scrollbar_vtable.on_destroy =
			(psy_ui_fp_component_event)
			psy_ui_scrollbarpane_on_destroy;
		psy_ui_scrollbar_vtable.onmousewheel =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_scrollbar_onwheel;		
		psy_ui_scrollbar_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &psy_ui_scrollbar_vtable);
}

/* implementation */
void psy_ui_scrollbar_init(psy_ui_ScrollBar* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_scrollbar_vtable_init(self);
	self->visible_state_change = FALSE;
	/* Less Button */
	psy_ui_button_init_connect(&self->less, &self->component,
		self, psy_ui_scrollbar_onless);
	psy_ui_button_seticon(&self->less, psy_ui_ICON_UP);
	psy_ui_button_setcharnumber(&self->less, 2);
	psy_ui_component_set_align(psy_ui_button_base(&self->less),
		psy_ui_ALIGN_TOP);	
	/* More Button */
	psy_ui_button_init_connect(&self->more, &self->component,
		self, psy_ui_scrollbar_onmore);
	psy_ui_button_seticon(&self->more, psy_ui_ICON_DOWN);
	psy_ui_button_setcharnumber(&self->more, 2);
	psy_ui_component_set_align(psy_ui_button_base(&self->more),
		psy_ui_ALIGN_BOTTOM);	
	/* Scrollpane */
	psy_ui_scrollbarpane_init(&self->pane, &self->component);
	psy_ui_component_set_align(psy_ui_scrollbarpane_base(&self->pane),
		psy_ui_ALIGN_CLIENT);
	/* Orientation */
	psy_ui_scrollbar_setorientation(self, psy_ui_VERTICAL);
	/* Signals */
	psy_signal_init(&self->signal_changed);
	/* Connect pane */
	psy_signal_connect(&self->pane.signal_changed, self,
		psy_ui_scrollbar_onscrollpanechanged);	
}

void psy_ui_scrollbar_setorientation(psy_ui_ScrollBar* self,
	psy_ui_Orientation orientation)
{
	assert(self);
	
	if (orientation == psy_ui_HORIZONTAL) {
		psy_ui_button_seticon(&self->less, psy_ui_ICON_LESS);
		psy_ui_component_set_align(psy_ui_button_base(&self->less),
			psy_ui_ALIGN_LEFT);
		psy_ui_component_set_style_types(psy_ui_button_base(&self->less),
			psy_ui_STYLE_HSCROLLBUTTON, psy_ui_STYLE_HSCROLLBUTTON_HOVER,
			psy_INDEX_INVALID, psy_INDEX_INVALID);
		psy_ui_component_set_style_type_active(psy_ui_button_base(&self->less),
			psy_ui_STYLE_HSCROLLBUTTON_ACTIVE);		
		psy_ui_button_seticon(&self->more, psy_ui_ICON_MORE);		
		psy_ui_component_set_align(psy_ui_button_base(&self->more),
			psy_ui_ALIGN_RIGHT);		
		psy_ui_component_set_style_types(psy_ui_button_base(&self->more),
			psy_ui_STYLE_HSCROLLBUTTON, psy_ui_STYLE_HSCROLLBUTTON_HOVER,
			psy_INDEX_INVALID, psy_INDEX_INVALID);
		psy_ui_component_set_style_type_active(psy_ui_button_base(&self->more),
			psy_ui_STYLE_HSCROLLBUTTON_ACTIVE);
		psy_ui_scrollbarpane_updatethumbposition(&self->pane);
	} else if (orientation == psy_ui_VERTICAL) {
		psy_ui_button_seticon(&self->less, psy_ui_ICON_UP);
		psy_ui_component_set_align(psy_ui_button_base(&self->less),
			psy_ui_ALIGN_TOP);
		psy_ui_component_set_style_types(psy_ui_button_base(&self->less),
			psy_ui_STYLE_VSCROLLBUTTON, psy_ui_STYLE_VSCROLLBUTTON_HOVER,
			psy_INDEX_INVALID, psy_INDEX_INVALID);
		psy_ui_component_set_style_type_active(psy_ui_button_base(&self->less),
			psy_ui_STYLE_VSCROLLBUTTON_ACTIVE);		
		psy_ui_button_seticon(&self->more, psy_ui_ICON_DOWN);
		psy_ui_component_set_align(psy_ui_button_base(&self->more),
			psy_ui_ALIGN_BOTTOM);		
		psy_ui_component_set_style_types(psy_ui_button_base(&self->more),
			psy_ui_STYLE_VSCROLLBUTTON, psy_ui_STYLE_VSCROLLBUTTON_HOVER,
			psy_INDEX_INVALID, psy_INDEX_INVALID);
		psy_ui_component_set_style_type_active(psy_ui_button_base(&self->more),
			psy_ui_STYLE_VSCROLLBUTTON_ACTIVE);
	}
	psy_ui_scrollbarpane_setorientation(&self->pane, orientation);
}

void psy_ui_scrollbar_on_destroy(psy_ui_ScrollBar* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);	
}

void psy_ui_scrollbar_onwheel(psy_ui_ScrollBar* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_delta(ev) > 0) {
		psy_ui_scrollbar_onless(self, psy_ui_mouseevent_target(ev));
	} else {
		psy_ui_scrollbar_onmore(self, psy_ui_mouseevent_target(ev));
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void psy_ui_scrollbar_onless(psy_ui_ScrollBar* self, psy_ui_Component* sender)
{
	assert(self);

	psy_ui_scrollbar_setthumbposition(self,
		psy_ui_scrollbar_position(self) - 1);
	psy_signal_emit(&self->signal_changed, self, 0);
}

void psy_ui_scrollbar_onmore(psy_ui_ScrollBar* self, psy_ui_Component* sender)
{
	assert(self);

	psy_ui_scrollbar_setthumbposition(self,
		psy_ui_scrollbar_position(self) + 1);
	psy_signal_emit(&self->signal_changed, self, 0);
}

void psy_ui_scrollbar_onscrollpanechanged(psy_ui_ScrollBar* self,
	psy_ui_ScrollBarPane* sender)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 0);
}
