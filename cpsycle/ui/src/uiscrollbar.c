/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiscrollbar.h"
/* local */
#include "uiapp.h"
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"

#define REPEAT_TIMEINTERVAL 50
#define REPEAT_DELAYCOUNT 15
#define REPEAT_DELAYFIRSTCOUNT 5

/* psy_ui_ScrollBarState */
void psy_ui_scrollbarstate_init(psy_ui_ScrollBarState* self)
{
	assert(self);

	self->dragthumb = FALSE;
}

/* psy_ui_ScrollBarThumb */
/* prototypes */
static void psy_ui_scrollbarthumb_onmousedown(psy_ui_ScrollBarThumb*,
	psy_ui_MouseEvent*);
static void psy_ui_scrollbarthumb_onmouseenter(psy_ui_ScrollBarThumb*);
static void psy_ui_scrollbarthumb_onmouseleave(psy_ui_ScrollBarThumb*);
/* vtable */
static psy_ui_ComponentVtable psy_ui_scrollbarthumb_vtable;
static bool psy_ui_scrollbarthumb_vtable_initialized = FALSE;

static void psy_ui_scrollbarthumb_vtableinit_init(psy_ui_ScrollBarThumb* self)
{
	assert(self);

	if (!psy_ui_scrollbarthumb_vtable_initialized) {
		psy_ui_scrollbarthumb_vtable = *(self->component.vtable);
		psy_ui_scrollbarthumb_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			psy_ui_scrollbarthumb_onmousedown;
		psy_ui_scrollbarthumb_vtable_initialized = TRUE;
	}
	self->component.vtable = &psy_ui_scrollbarthumb_vtable;
}
/* implementation */
void psy_ui_scrollbarthumb_init(psy_ui_ScrollBarThumb* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_ui_ScrollBarState* state)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, view ? view : parent);
	psy_ui_scrollbarthumb_vtableinit_init(self);
	self->state = state;
	psy_ui_component_setstyletypes(psy_ui_scrollbarthumb_base(self),
		psy_ui_STYLE_SCROLLTHUMB,
		psy_ui_STYLE_SCROLLTHUMB_HOVER,
		psy_ui_STYLE_SCROLLTHUMB_HOVER,
		psy_INDEX_INVALID);	
	psy_ui_component_setpreferredsize(psy_ui_scrollbarthumb_base(self),
		psy_ui_size_make_em(1.0, 1.0));
}

void psy_ui_scrollbarthumb_onmousedown(psy_ui_ScrollBarThumb* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	self->state->dragthumb = TRUE;
}

/* psy_ui_ScrollBarPane */
/* prototypes */
static void psy_ui_scrollbarpane_ondestroy(psy_ui_ScrollBarPane*,
	psy_ui_Component* sender);
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
static void psy_ui_scrollbarpane_updatethumbposition(psy_ui_ScrollBarPane*);
static void psy_ui_scrollbarpane_ontimer(psy_ui_ScrollBarPane*, uintptr_t id);
/* vtable */
static psy_ui_ComponentVtable psy_ui_scrollbarpane_vtable;
static bool psy_ui_scrollbarpane_vtable_initialized = FALSE;

static void psy_ui_scrollbarpane_vtable_init(psy_ui_ScrollBarPane* self)
{
	assert(self);

	if (!psy_ui_scrollbarpane_vtable_initialized) {
		psy_ui_scrollbarpane_vtable = *(self->component.vtable);
		psy_ui_scrollbarpane_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			psy_ui_scrollbarpane_onmousedown;
		psy_ui_scrollbarpane_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			psy_ui_scrollbarpane_onmousedown;
		psy_ui_scrollbarpane_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			psy_ui_scrollbarpane_onmousemove;
		psy_ui_scrollbarpane_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			psy_ui_scrollbarpane_onmouseup;
		psy_ui_scrollbarpane_vtable.enableinput =
			(psy_ui_fp_component_enableinput)
			psy_ui_scrollbarpane_enableinput;
		psy_ui_scrollbarpane_vtable.preventinput =
			(psy_ui_fp_component_preventinput)
			psy_ui_scrollbarpane_preventinput;
		psy_ui_scrollbarpane_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			psy_ui_scrollbarpane_ontimer;
		psy_ui_scrollbarpane_vtable_initialized = TRUE;
	}
}
/* implementation */
void psy_ui_scrollbarpane_init(psy_ui_ScrollBarPane* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_ui_ScrollBarState* state)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, view);
	psy_ui_scrollbarpane_vtable_init(self);
	self->component.vtable = &psy_ui_scrollbarpane_vtable;
	psy_ui_component_preventalign(&self->component);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setstyletypes(&self->component,
		psy_ui_STYLE_SCROLLPANE, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);
	psy_ui_scrollbarthumb_init(&self->thumb, &self->component, view, state);
	self->pos = 0;
	self->screenpos = 0;
	self->orientation = psy_ui_VERTICAL;
	psy_ui_intpoint_init(&self->scrollrange);
	self->enabled = TRUE;
	self->state = state;
	self->repeat = 0;
	self->repeatdelaycounter = 0;
	psy_ui_scrollbarpane_updatethumbposition(self);
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_clicked);
	psy_signal_connect(&self->component.signal_destroy, self,
		psy_ui_scrollbarpane_ondestroy);
}

void psy_ui_scrollbarpane_ondestroy(psy_ui_ScrollBarPane* self,
	psy_ui_Component* sender)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_clicked);
}

void psy_ui_scrollbarpane_setorientation(psy_ui_ScrollBarPane* self,
	psy_ui_Orientation orientation)
{
	assert(self);

	self->orientation = orientation;
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
	updateposition = psy_ui_component_position(&self->thumb.component);
	psy_ui_component_setposition(psy_ui_scrollbarthumb_base(&self->thumb),
		psy_ui_rectangle_make_px(&newposition));
	psy_ui_realrectangle_union(&updateposition, &newposition);
	psy_ui_component_invalidaterect(&self->component, updateposition);
}

void psy_ui_scrollbarpane_onmousedown(psy_ui_ScrollBarPane* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_capture(&self->component);
	if (self->state->dragthumb) {
		if (self->orientation == psy_ui_HORIZONTAL) {
			self->dragoffset = ev->pt.x - self->screenpos;
		} else if (self->orientation == psy_ui_VERTICAL) {
			self->dragoffset = ev->pt.y - self->screenpos;
		}
	} else {
		psy_ui_RealRectangle thumbposition;

		thumbposition = psy_ui_component_position(
			psy_ui_scrollbarthumb_base(&self->thumb));
		if (self->orientation == psy_ui_HORIZONTAL) {
			if (ev->pt.x > thumbposition.right) {
				self->repeat = 1;
			} else {
				self->repeat = -1;
			}
		} else if (self->orientation == psy_ui_VERTICAL) {
			if (ev->pt.y > thumbposition.bottom) {
				self->repeat = 1;
			} else {
				self->repeat = -1;
			}
		}
		psy_ui_scrollbarpane_setthumbposition(self,
			self->pos + self->repeat);
		psy_signal_emit(&self->signal_changed, self, 0);
		self->repeatdelaycounter = REPEAT_DELAYCOUNT;
		psy_ui_component_starttimer(&self->component, 0,
			REPEAT_TIMEINTERVAL);
	}
}

void psy_ui_scrollbarpane_onmouseup(psy_ui_ScrollBarPane* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_releasecapture(&self->component);
	if (self->state->dragthumb) {
		psy_ui_scrollbarpane_setthumbposition(self, self->pos);
		self->state->dragthumb = FALSE;
	}
	if (self->repeat != 0) {
		psy_ui_component_stoptimer(&self->component, 0);
		self->repeat = 0;
	}
}

void psy_ui_scrollbarpane_onmousemove(psy_ui_ScrollBarPane* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	if (self->state->dragthumb) {
		double step;
		double pos;
		psy_ui_RealSize size;
		psy_ui_RealSize thumbsize;

		size = psy_ui_component_scrollsize_px(&self->component);
		thumbsize = psy_ui_component_scrollsize_px(
			psy_ui_scrollbarthumb_base(&self->thumb));
		if (self->orientation == psy_ui_HORIZONTAL) {
			self->screenpos = psy_max(0, psy_min(ev->pt.x - self->dragoffset,
				size.width - thumbsize.width));
		} else {
			self->screenpos = psy_max(0, psy_min(ev->pt.y - self->dragoffset,
				size.height - thumbsize.height));
		}
		psy_ui_scrollbarpane_updatethumbposition(self);		
		pos = (double)self->scrollrange.x;
		step = psy_ui_scrollbarpane_step(self);
		pos = ((self->screenpos) * step) + self->scrollrange.x;
		if (pos < self->scrollrange.x) {
			pos = (double)self->scrollrange.x;
		} else if (pos > self->scrollrange.y) {
			pos = (double)self->scrollrange.y;
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

	assert(self);

	if (pos < self->scrollrange.x) {
		pos = (double)self->scrollrange.y;
	}
	if (pos > self->scrollrange.y) {
		pos = (double)self->scrollrange.y;
	}
	step = psy_ui_scrollbarpane_step(self);
	if (step != 0.0) {
		self->screenpos = (1.0 / step) * floor(pos - self->scrollrange.x);
		self->pos = pos;
		psy_ui_scrollbarpane_updatethumbposition(self);
	}
}

double psy_ui_scrollbarpane_step(psy_ui_ScrollBarPane* self)
{
	double rv;
	psy_ui_RealSize size;
	psy_ui_RealSize panesize;

	assert(self);

	panesize = psy_ui_component_scrollsize_px(&self->component);
	size = psy_ui_component_scrollsize_px(psy_ui_scrollbarthumb_base(&self->thumb));
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

void psy_ui_scrollbarpane_enableinput(psy_ui_ScrollBarPane* self)
{
	assert(self);

	self->enabled = TRUE;
	psy_ui_component_setbackgroundcolour(&self->component,
		psy_ui_colour_make(0x00292929));
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_scrollbarpane_preventinput(psy_ui_ScrollBarPane* self)
{
	assert(self);

	self->enabled = FALSE;
	psy_ui_component_setbackgroundcolour(&self->component,
		psy_ui_style_const(psy_ui_STYLE_ROOT)->backgroundcolour);
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_scrollbarpane_ontimer(psy_ui_ScrollBarPane* self, uintptr_t id)
{
	assert(self);

	if (self->repeatdelaycounter == 0 ||
			self->repeatdelaycounter == REPEAT_DELAYFIRSTCOUNT) {
		psy_ui_scrollbarpane_setthumbposition(self,
			self->pos + self->repeat);
		psy_signal_emit(&self->signal_changed, self, 0);
	}
	if (self->repeatdelaycounter > 0) {
		--self->repeatdelaycounter;
	}
}

/* psy_ui_ScrollBar */
/* prototypes */
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
/* static void psy_ui_scrollbar_onmousewheel(psy_ui_ScrollBar*,
		psy_ui_Component* sender, psy_ui_MouseEvent*); */
/* implementation */
void psy_ui_scrollbar_init(psy_ui_ScrollBar* self, psy_ui_Component* parent,
	psy_ui_Component* view)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, view);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	psy_signal_connect(&self->component.signal_destroy, self,
		psy_ui_scrollbar_ondestroy);
	/* Less Button */
	psy_ui_button_init_connect(&self->less,
		&self->component, view, self, psy_ui_scrollbar_onless);
	psy_ui_button_seticon(&self->less, psy_ui_ICON_UP);
	psy_ui_button_setcharnumber(&self->less, 2);
	psy_ui_component_setalign(psy_ui_button_base(&self->less),
		psy_ui_ALIGN_TOP);
	psy_ui_component_setstyletypes(psy_ui_button_base(&self->less),
		psy_ui_STYLE_SCROLLBUTTON, psy_ui_STYLE_SCROLLBUTTON_HOVER,
		psy_INDEX_INVALID, psy_INDEX_INVALID);
	psy_ui_component_setstyletype_active(psy_ui_button_base(&self->less),
		psy_ui_STYLE_SCROLLBUTTON_ACTIVE);
	/* More Button */
	psy_ui_button_init_connect(&self->more, &self->component, view,
		self, psy_ui_scrollbar_onmore);
	psy_ui_button_seticon(&self->more, psy_ui_ICON_DOWN);
	psy_ui_button_setcharnumber(&self->more, 2);
	psy_ui_component_setalign(psy_ui_button_base(&self->more),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setstyletypes(psy_ui_button_base(&self->more),
		psy_ui_STYLE_SCROLLBUTTON, psy_ui_STYLE_SCROLLBUTTON_HOVER,
		psy_INDEX_INVALID, psy_INDEX_INVALID);
	psy_ui_component_setstyletype_active(psy_ui_button_base(&self->more),
		psy_ui_STYLE_SCROLLBUTTON_ACTIVE);
	/* state */
	psy_ui_scrollbarstate_init(&self->state);
	/* Pane */
	psy_ui_scrollbarpane_init(&self->pane, &self->component, view,
		&self->state);
	psy_ui_component_setalign(psy_ui_scrollbarpane_base(&self->pane),
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setpreferredsize(
		psy_ui_scrollbarpane_base(&self->pane),
		psy_ui_size_make_em(2.0, 1.0));			
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_clicked);
	psy_signal_connect(&self->pane.signal_changed, self,
		psy_ui_scrollbar_onscrollpanechanged);
	psy_signal_connect(&self->pane.signal_clicked, self,
		psy_ui_scrollbar_onscrollpaneclicked);
	psy_ui_scrollbar_setorientation(self, psy_ui_VERTICAL);
}

void psy_ui_scrollbar_setorientation(psy_ui_ScrollBar* self,
	psy_ui_Orientation orientation)
{
	assert(self);

	psy_ui_scrollbarpane_setorientation(&self->pane, orientation);
	if (orientation == psy_ui_HORIZONTAL) {
		psy_ui_button_seticon(&self->less, psy_ui_ICON_LESS);
		psy_ui_component_setalign(psy_ui_button_base(&self->less),
			psy_ui_ALIGN_LEFT);
		psy_ui_component_setpreferredsize(
			&self->less.component,
			psy_ui_size_make_em(2.7, 1.0));
		psy_ui_button_seticon(&self->more, psy_ui_ICON_MORE);
		psy_ui_component_setpreferredsize(
			&self->more.component,
			psy_ui_size_make_em(2.7, 1.0));
		psy_ui_component_setalign(psy_ui_button_base(&self->more),
			psy_ui_ALIGN_RIGHT);
		psy_ui_component_setpreferredsize(
			psy_ui_scrollbarpane_base(&self->pane),
			psy_ui_size_make_em(2.7, 1.0));
		psy_ui_scrollbarpane_updatethumbposition(&self->pane);
	} else if (orientation == psy_ui_VERTICAL) {
		psy_ui_button_seticon(&self->less, psy_ui_ICON_UP);
		psy_ui_component_setalign(psy_ui_button_base(&self->less),
			psy_ui_ALIGN_TOP);
		psy_ui_component_setpreferredsize(
			psy_ui_button_base(&self->less),
			psy_ui_size_make_em(2.7, 1.0));
		psy_ui_button_seticon(&self->more, psy_ui_ICON_DOWN);
		psy_ui_component_setalign(psy_ui_button_base(&self->more),
			psy_ui_ALIGN_BOTTOM);
		psy_ui_component_setpreferredsize(
			psy_ui_button_base(&self->more),
			psy_ui_size_make_em(2.7, 1.0));
		psy_ui_component_setpreferredsize(
			psy_ui_scrollbarpane_base(&self->pane),
			psy_ui_size_make_em(2.7, 1.0));	
		psy_ui_scrollbarpane_updatethumbposition(&self->pane);
	}
}

void psy_ui_scrollbar_ondestroy(psy_ui_ScrollBar* self,
	psy_ui_Component* sender)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_clicked);
}

void psy_ui_scrollbar_onmousewheel(psy_ui_ScrollBar* self,
	psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{
}

double psy_ui_scrollbar_position(psy_ui_ScrollBar* self)
{
	assert(self);

	return self->pane.pos;
}

void psy_ui_scrollbar_setscrollrange(psy_ui_ScrollBar* self, psy_ui_IntPoint range)
{
	assert(self);

	self->pane.scrollrange = range;
	self->pane.pos = (double)range.x;
}

psy_ui_IntPoint psy_ui_scrollbar_scrollrange(const psy_ui_ScrollBar* self)
{
	assert(self);

	return self->pane.scrollrange;
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

void psy_ui_scrollbar_onscrollpaneclicked(psy_ui_ScrollBar* self,
	psy_ui_ScrollBarPane* sender)
{
	assert(self);

	psy_signal_emit(&self->signal_clicked, self, 0);
}

void psy_ui_scrollbar_setthumbposition(psy_ui_ScrollBar* self, double pos)
{
	assert(self);

	psy_ui_scrollbarpane_setthumbposition(&self->pane, pos);
}
