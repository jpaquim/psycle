/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiscroller.h"
/* local */
#include "uiapp.h"
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static void psy_ui_scrollanimate_init(psy_ui_ScrollAnimate* self)
{
	self->counter = 0;
	self->speed = 1.0;
}

static void psy_ui_scrollanimate_reset(psy_ui_ScrollAnimate* self)
{
	self->speed = 1.0;
}

static void psy_ui_scrollanimate_calcstep(psy_ui_ScrollAnimate* self,
	double startpx, double targetpx)
{
	double diff;

	diff = targetpx - startpx;	
	self->targetpx = targetpx;
	if (diff != 0.0) {
		self->steppx = diff / fabs(diff) * self->speed;
		self->counter = (uintptr_t)floor(fabs(diff) / fabs(self->steppx));
	} else {
		self->steppx = 0;
		self->counter = 0;
	}
}

static double psy_ui_scrollanimate_currposition(const psy_ui_ScrollAnimate* self)
{
	double rv;

	if (self->counter > 0) {
		rv = (self->targetpx) - self->counter * self->steppx;
		if (self->steppx < 0) {
			rv = psy_max(rv, self->targetpx);
		} else {
			rv = psy_min(rv, self->targetpx);
		}		
	} else {
		rv = self->targetpx;		
	}
	return rv;
}

static bool psy_ui_scrollanimate_tick(psy_ui_ScrollAnimate* self)
{
	if (self->counter > 0) {
		--self->counter;
		self->speed *= 2;
		psy_ui_scrollanimate_calcstep(self,
			psy_ui_scrollanimate_currposition(self),
			self->targetpx);
	}
	return self->counter == 0;
}

/* psy_ui_Scroller */

/* prototypes */
static void psy_ui_scroller_on_pane_size(psy_ui_Scroller*,
	psy_ui_Component* sender);
static void psy_ui_scroller_on_scroll(psy_ui_Scroller*,
	psy_ui_Component* sender);
static void psy_ui_scroller_horizontal_onchanged(psy_ui_Scroller*,
	psy_ui_ScrollBar* sender);
static void psy_ui_scroller_vertical_onchanged(psy_ui_Scroller*,
	psy_ui_ScrollBar* sender);
static void psy_ui_scroller_scroll_range_changed(psy_ui_Scroller*,
	psy_ui_Component* sender, psy_ui_Orientation);
static void psy_ui_scroller_on_mouse_down(psy_ui_Scroller*, psy_ui_MouseEvent*);
static void psy_ui_scroller_on_timer(psy_ui_Scroller*, uintptr_t timerid);
static void psy_ui_scroller_connect_client(psy_ui_Scroller*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_Scroller* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			psy_ui_scroller_on_timer;
		vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_scroller_on_mouse_down;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(psy_ui_scroller_base(self), &vtable);
}

/* implementation */
void psy_ui_scroller_init(psy_ui_Scroller* self,
	psy_ui_Component* parent, psy_ui_ScrollBar* hscroll,
	psy_ui_ScrollBar* vscroll)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);		
	self->vscroll_autohide = TRUE;
	self->hscroll_autohide = TRUE;	
	/* pane */
	psy_ui_component_init(&self->pane, &self->component, NULL);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_CLIENT);
	/* horizontal scrollbar */
	if (!hscroll) {
		psy_ui_scrollbar_init(&self->hscroll_intern, &self->component);		
		psy_ui_component_set_align(&self->hscroll_intern.component,
			psy_ui_ALIGN_BOTTOM);
		self->hscroll = &self->hscroll_intern;

	} else {
		self->hscroll = hscroll;		
	}
	psy_ui_scrollbar_setorientation(self->hscroll, psy_ui_HORIZONTAL);
	psy_ui_component_hide(&self->hscroll->component);
	/* vertical scrollbar */
	if (!vscroll) {
		psy_ui_scrollbar_init(&self->vscroll_intern, &self->component);				
		psy_ui_component_set_align(&self->vscroll_intern.component,
			psy_ui_ALIGN_RIGHT);		
		self->vscroll = &self->vscroll_intern;
	} else {
		self->vscroll = vscroll;		
	}
	psy_ui_scrollbar_setorientation(self->vscroll, psy_ui_VERTICAL);
	psy_ui_component_hide(&self->vscroll->component);
	self->thumbmove = FALSE;	
	/* scroll animate */
	self->smooth = FALSE;
	psy_ui_scrollanimate_init(&self->hanimate);
	psy_ui_scrollanimate_init(&self->vanimate);		
	self->client = NULL;	
	psy_signal_connect(&self->vscroll->signal_changed, self,
		psy_ui_scroller_vertical_onchanged);
	psy_signal_connect(&self->hscroll->signal_changed, self,
		psy_ui_scroller_horizontal_onchanged);
	psy_ui_scrollbar_setscrollrange(self->vscroll,
		psy_ui_intpoint_make(0, 100));	
}

void psy_ui_scroller_set_client(psy_ui_Scroller* self, psy_ui_Component* client)
{		
	self->client = client;
	if (!client) {
		return;
	}	
	/* reparent client */
	psy_ui_component_set_parent(client, &self->pane);	
	psy_ui_scroller_connect_client(self);
}

void psy_ui_scroller_connect_client(psy_ui_Scroller* self)
{
	psy_signal_connect(&self->client->signal_scrollrangechanged,
		self, psy_ui_scroller_scroll_range_changed);		
	psy_signal_connect(&self->pane.signal_size,
		self, psy_ui_scroller_on_pane_size);
	psy_signal_connect(&self->client->signal_scrolled,
		self, psy_ui_scroller_on_scroll);		
	psy_ui_component_set_align(self->client, psy_ui_ALIGN_FIXED);
}

void psy_ui_scroller_on_pane_size(psy_ui_Scroller* self, psy_ui_Component* sender)
{
	if (self->client) {
		psy_ui_component_updateoverflow(self->client);						
		psy_ui_scrollbar_setthumbposition(self->vscroll,
			floor(psy_ui_component_scroll_top_px(self->client) /
				psy_ui_component_scroll_step_height_px(self->client)));		
		psy_ui_scrollbar_setthumbposition(self->hscroll,
			floor(psy_ui_component_scroll_left_px(self->client) /
				psy_ui_component_scrollstep_width_px(self->client)));
	}
}

void psy_ui_scroller_horizontal_onchanged(psy_ui_Scroller* self, psy_ui_ScrollBar* sender)
{
	double iPos;
	double nPos;
	double scrollsteppx;
	const psy_ui_TextMetric* tm;
	double scrollleftpx;
	double diff;
	
	assert(self->client);

	tm = psy_ui_component_textmetric(self->client);
	scrollsteppx = psy_ui_component_scrollstep_width_px(self->client);
	scrollleftpx = psy_ui_component_scroll_left_px(self->client);	
	iPos = scrollleftpx / scrollsteppx;	
	nPos = psy_ui_scrollbar_position(sender);
	diff = -scrollsteppx * floor(iPos - nPos);	
	if (self->smooth) {	
		if (self->hanimate.counter == 0) {
			psy_ui_scrollanimate_reset(&self->hanimate);
			psy_ui_scrollanimate_calcstep(&self->hanimate, scrollleftpx,
				floor((scrollleftpx + diff) / scrollsteppx) * scrollsteppx);
			psy_ui_component_start_timer(psy_ui_scroller_base(self), 0, 50);
		} else {
			self->hanimate.targetpx =
				floor((scrollleftpx + diff) / scrollsteppx) * scrollsteppx;			
		}
	} else {	
		self->thumbmove = TRUE;
		self->client->blitscroll = TRUE;
		psy_ui_component_set_scroll_left(self->client,
			psy_ui_value_make_px(
				floor((scrollleftpx + diff) / scrollsteppx) * scrollsteppx));		
		self->client->blitscroll = FALSE;
		self->thumbmove = FALSE;
	}
}

void psy_ui_scroller_vertical_onchanged(psy_ui_Scroller* self,
	psy_ui_ScrollBar* sender)
{
	double iPos;
	double nPos;
	double scrollstepy_px;
	double diff;
	const psy_ui_TextMetric* tm;
	double scrolltoppx;	

	assert(self->client);

	tm = psy_ui_component_textmetric(self->client);
	scrollstepy_px = psy_ui_component_scroll_step_height_px(self->client);
	scrolltoppx = psy_ui_component_scroll_top_px(self->client);
	iPos = scrolltoppx / scrollstepy_px;	
	nPos = psy_ui_scrollbar_position(sender);
	if (self->vanimate.counter > 0 && self->vanimate.steppx / (iPos - nPos) > 0) {
		psy_ui_component_set_scroll_top(self->client,
			psy_ui_value_make_px(self->vanimate.targetpx));
	}
	diff = -scrollstepy_px * floor(iPos - nPos);
	if (self->smooth) {		
		if (self->vanimate.counter == 0) {
			psy_ui_scrollanimate_reset(&self->vanimate);
			psy_ui_scrollanimate_calcstep(&self->vanimate, scrolltoppx,
				floor((scrolltoppx + diff) / scrollstepy_px) * scrollstepy_px);
			psy_ui_component_start_timer(psy_ui_scroller_base(self), 1, 50);
		} else {
			self->vanimate.targetpx =
				floor((scrolltoppx + diff) / scrollstepy_px) * scrollstepy_px;
		}
	} else {
		self->thumbmove = TRUE;
		self->client->blitscroll = TRUE;
		psy_ui_component_set_scroll_top(self->client,
			psy_ui_value_make_px(
				floor((scrolltoppx + diff) / scrollstepy_px) * scrollstepy_px));		
		self->client->blitscroll = FALSE;
		self->thumbmove = FALSE;
	}
}

void psy_ui_scroller_on_timer(psy_ui_Scroller* self, uintptr_t timerid)
{	
	if (timerid == 0) {
		if (psy_ui_scrollanimate_tick(&self->hanimate)) {
			psy_ui_component_stop_timer(psy_ui_scroller_base(self), 0);
		}
		psy_ui_component_set_scroll_left(self->client,
			psy_ui_value_make_px(psy_ui_scrollanimate_currposition(
				&self->hanimate)));
	} else if (timerid == 1) {
		if (psy_ui_scrollanimate_tick(&self->vanimate)) {
			psy_ui_component_stop_timer(psy_ui_scroller_base(self), 1);
		}
		psy_ui_component_set_scroll_top(self->client,
			psy_ui_value_make_px(psy_ui_scrollanimate_currposition(
				&self->vanimate)));
	}
}

void psy_ui_scroller_on_scroll(psy_ui_Scroller* self, psy_ui_Component* sender)
{
	if (!self->thumbmove) {
		double pos;
		const psy_ui_TextMetric* tm;

		tm = psy_ui_component_textmetric(self->client);
		/* vertical */
		pos = floor(psy_ui_component_scroll_top_px(self->client) /
			psy_ui_component_scroll_step_height_px(self->client));
		psy_ui_scrollbar_setthumbposition(self->vscroll, pos);
		/* horizontal */
		pos = floor(psy_ui_component_scroll_left_px(self->client) /
			psy_ui_component_scrollstep_width_px(self->client));
		psy_ui_scrollbar_setthumbposition(self->hscroll, pos);
	}
}

void psy_ui_scroller_scroll_range_changed(psy_ui_Scroller* self, psy_ui_Component* sender,
	psy_ui_Orientation orientation)
{	
	if (orientation == psy_ui_VERTICAL) {
		psy_ui_IntPoint vrange;		
				
		vrange = psy_ui_component_verticalscrollrange(sender);
		psy_ui_scrollbar_setscrollrange(self->vscroll,
			psy_ui_component_verticalscrollrange(sender));
		vrange = psy_ui_component_verticalscrollrange(sender);
				
		if (vrange.y - vrange.x <= 0) {
			if (psy_ui_component_visible(&self->vscroll->component)) {
				if (self->vscroll_autohide && !self->vscroll->visible_state_change) {
					psy_ui_component_hide(&self->vscroll->component);
					psy_ui_component_align(&self->component);
					psy_ui_component_invalidate(&self->component);
					if (psy_ui_component_visible(&self->hscroll->component)) {
						psy_ui_component_set_margin(&self->hscroll->component,
							psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.0));
					}
				}
			}
		} else if (!psy_ui_component_visible(&self->vscroll->component)) {
			if (self->vscroll_autohide) {
				self->vscroll->visible_state_change = TRUE;
				psy_ui_component_show(&self->vscroll->component);
				if (psy_ui_component_visible(&self->hscroll->component)) {
					psy_ui_component_set_margin(&self->hscroll->component,
						psy_ui_margin_make_em(0.0, 2.0, 0.0, 0.0));
				}
				psy_ui_component_align(&self->component);		
				psy_ui_component_invalidate(&self->component);				
				self->vscroll->visible_state_change = FALSE;
			}
		}
	} else if (orientation == psy_ui_HORIZONTAL) {
		psy_ui_IntPoint hrange;

		hrange = psy_ui_component_horizontalscrollrange(sender);
		psy_ui_scrollbar_setscrollrange(self->hscroll, hrange);
		hrange = psy_ui_component_horizontalscrollrange(sender);
		if ((hrange.y - hrange.x) <= 0) {
			if (self->hscroll_autohide) {
				if (!self->hscroll->visible_state_change &&
						psy_ui_component_visible(&self->hscroll->component)) {
					if (psy_ui_component_visible(&self->vscroll->component)) {
						psy_ui_component_set_margin(&self->hscroll->component,
							psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.0));
					}
					psy_ui_component_hide(&self->hscroll->component);
					psy_ui_component_align(&self->component);
					psy_ui_component_invalidate(&self->component);
				}
			}
		} else if (!psy_ui_component_visible(&self->hscroll->component)) {
			if (self->hscroll_autohide) {
				if (psy_ui_component_visible(&self->vscroll->component)) {
					psy_ui_component_set_margin(&self->hscroll->component,
						psy_ui_margin_make_em(0.0, 2.0, 0.0, 0.0));
				}
				self->hscroll->visible_state_change = TRUE;
				psy_ui_component_show(&self->hscroll->component);				
				psy_ui_component_align(&self->component);
				psy_ui_component_invalidate(&self->component);
				self->hscroll->visible_state_change = FALSE;
			}
		}
	}
}

void psy_ui_scroller_on_mouse_down(psy_ui_Scroller* self, psy_ui_MouseEvent* ev)
{
	if (self->client) {		
		psy_ui_component_set_focus(self->client);		
		ev->event.prevent_focus = TRUE;		
	}
	psy_ui_event_stop_propagation(psy_ui_mouseevent_base(ev));
}
