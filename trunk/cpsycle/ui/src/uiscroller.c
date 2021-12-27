/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net
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
	self->steppx = diff / fabs(diff) * self->speed;
	self->counter = (uintptr_t)floor(fabs(diff) / fabs(self->steppx));
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
static void psy_ui_scroller_onpanesize(psy_ui_Scroller*, psy_ui_Component* sender);
static void psy_ui_scroller_onpanedraw(psy_ui_Scroller*, psy_ui_Component* sender, psy_ui_Graphics* g);
static void psy_ui_scroller_onscroll(psy_ui_Scroller*, psy_ui_Component* sender);
static void psy_ui_scroller_onscrollbarclicked(psy_ui_Scroller*, psy_ui_Component* sender);
static void psy_ui_scroller_horizontal_onchanged(psy_ui_Scroller*, psy_ui_ScrollBar* sender);
static void psy_ui_scroller_vertical_onchanged(psy_ui_Scroller*, psy_ui_ScrollBar* sender);
static void psy_ui_scroller_scrollrangechanged(psy_ui_Scroller*, psy_ui_Component* sender,
	psy_ui_Orientation);
static void psy_ui_scroller_onfocus(psy_ui_Scroller*, psy_ui_Component* sender);
static void psy_ui_scroller_ontimer(psy_ui_Scroller*, uintptr_t timerid);
static void psy_ui_scroller_onupdatestyles(psy_ui_Scroller*);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_Scroller* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			psy_ui_scroller_ontimer;
		vtable.onupdatestyles =
			(psy_ui_fp_component_event)
			psy_ui_scroller_onupdatestyles;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}

/* implementation */
void psy_ui_scroller_init(psy_ui_Scroller* self, psy_ui_Component* client,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);	
	/* bottom */
	psy_ui_component_init(&self->bottom, &self->component, NULL);
	psy_ui_component_setalign(&self->bottom, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(&self->bottom);
	/* spacer */
	psy_ui_component_init(&self->spacer, &self->bottom, NULL);
	psy_ui_component_setalign(&self->spacer, psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->spacer);
	psy_ui_component_setpreferredsize(&self->spacer,
		psy_ui_size_make_em(2.5, 1.0));
	psy_ui_component_preventalign(&self->spacer);
	/* horizontal scrollbar */
	psy_ui_scrollbar_init(&self->hscroll, &self->bottom);
	psy_ui_scrollbar_setorientation(&self->hscroll, psy_ui_HORIZONTAL);	
	psy_ui_component_setalign(&self->hscroll.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&self->hscroll.signal_clicked, self,
		psy_ui_scroller_onscrollbarclicked);
	/* vertical scrollbar */
	psy_ui_scrollbar_init(&self->vscroll, &self->component);
	psy_ui_scrollbar_setorientation(&self->vscroll, psy_ui_VERTICAL);
	psy_ui_component_setalign(&self->vscroll.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->vscroll.component);
	psy_signal_connect(&self->hscroll.signal_clicked, self,
		psy_ui_scroller_onscrollbarclicked);	
	self->thumbmove = FALSE;
	/* pane */
	psy_ui_component_init(&self->pane, &self->component, NULL);
	psy_ui_component_setbackgroundmode(&self->pane, psy_ui_NOBACKGROUND);
	psy_ui_component_setalign(&self->pane, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->pane.signal_draw, self,
		psy_ui_scroller_onpanedraw);
	/* scroll animate */
	self->smooth = FALSE;
	psy_ui_scrollanimate_init(&self->hanimate);
	psy_ui_scrollanimate_init(&self->vanimate);	
	/* reparent client */
	self->client = client;
	if (self->client) {
		const psy_ui_Style* style;

		psy_ui_component_setparent(client, &self->pane);		
		psy_ui_component_setalign(client, psy_ui_ALIGN_FIXED);
		style = psy_ui_app_style_const(psy_ui_app(),
			psy_ui_componentstyle_currstyleid(&self->client->style));
		if (style) {
			psy_ui_component_setborder(&self->component, &style->border);
		}
	}
	psy_signal_connect(&self->vscroll.signal_changed, self,
		psy_ui_scroller_vertical_onchanged);
	psy_signal_connect(&self->hscroll.signal_changed, self,
		psy_ui_scroller_horizontal_onchanged);
	psy_ui_scrollbar_setscrollrange(&self->vscroll, psy_ui_intpoint_make(0, 100));
	if (self->client) {
		psy_ui_scroller_connectclient(self);
	}
	psy_signal_connect(&self->component.signal_focus, self,
		psy_ui_scroller_onfocus);	
}

void psy_ui_scroller_connectclient(psy_ui_Scroller* self)
{
	psy_signal_connect(&self->client->signal_scrollrangechanged, self,
		psy_ui_scroller_scrollrangechanged);		
	psy_signal_connect(&self->pane.signal_size, self,
		psy_ui_scroller_onpanesize);
	psy_signal_connect(&self->client->signal_scroll, self,
		psy_ui_scroller_onscroll);		
	psy_ui_component_setalign(self->client, psy_ui_ALIGN_FIXED);	
}

void psy_ui_scroller_setbackgroundmode(psy_ui_Scroller* self,
	psy_ui_BackgroundMode scroller, psy_ui_BackgroundMode pane)
{
	psy_ui_component_setbackgroundmode(&self->component, scroller);
	psy_ui_component_setbackgroundmode(&self->pane, pane);
}

void psy_ui_scroller_onpanesize(psy_ui_Scroller* self, psy_ui_Component* sender)
{
	if (self->client) {
		double nPosX;
		double nPosY;
		double scrollstepx_px;
		double scrollstepy_px;
		const psy_ui_TextMetric* tm;

		psy_ui_component_updateoverflow(self->client);
		tm = psy_ui_component_textmetric(self->client);
		scrollstepy_px = psy_ui_component_scrollstep_height_px(self->client);
		nPosY = floor(psy_ui_component_scrolltop_px(self->client) / scrollstepy_px);
		psy_ui_scrollbar_setthumbposition(&self->vscroll, nPosY);
		scrollstepx_px = scrollstepy_px = psy_ui_component_scrollstep_width_px(self->client);
		nPosX = floor(psy_ui_component_scrollleftpx(self->client) / scrollstepx_px);
		psy_ui_scrollbar_setthumbposition(&self->hscroll, nPosX);
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
	scrollleftpx = psy_ui_component_scrollleftpx(self->client);	
	iPos = scrollleftpx / scrollsteppx;	
	nPos = psy_ui_scrollbar_position(sender);
	diff = -scrollsteppx * floor(iPos - nPos);	
	if (self->smooth) {	
		if (self->hanimate.counter == 0) {
			psy_ui_scrollanimate_reset(&self->hanimate);
			psy_ui_scrollanimate_calcstep(&self->hanimate, scrollleftpx,
				floor((scrollleftpx + diff) / scrollsteppx) * scrollsteppx);
			psy_ui_component_starttimer(psy_ui_scroller_base(self), 0, 50);
		} else {
			self->hanimate.targetpx =
				floor((scrollleftpx + diff) / scrollsteppx) * scrollsteppx;			
		}
	} else {	
		self->thumbmove = TRUE;
		psy_ui_component_setscrollleft(self->client,
			psy_ui_value_make_px(
				floor((scrollleftpx + diff) / scrollsteppx) * scrollsteppx));		
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
	scrollstepy_px = psy_ui_component_scrollstep_height_px(self->client);
	scrolltoppx = psy_ui_component_scrolltop_px(self->client);
	iPos = scrolltoppx / scrollstepy_px;	
	nPos = psy_ui_scrollbar_position(sender);
	if (self->vanimate.counter > 0 && self->vanimate.steppx / (iPos - nPos) > 0) {
		psy_ui_component_setscrolltop(self->client,
			psy_ui_value_make_px(self->vanimate.targetpx));
	}
	diff = -scrollstepy_px * floor(iPos - nPos);
	if (self->smooth) {		
		if (self->vanimate.counter == 0) {
			psy_ui_scrollanimate_reset(&self->vanimate);
			psy_ui_scrollanimate_calcstep(&self->vanimate, scrolltoppx,
				floor((scrolltoppx + diff) / scrollstepy_px) * scrollstepy_px);
			psy_ui_component_starttimer(psy_ui_scroller_base(self), 1, 50);
		} else {
			self->vanimate.targetpx =
				floor((scrolltoppx + diff) / scrollstepy_px) * scrollstepy_px;
		}
	} else {
		self->thumbmove = TRUE;
		psy_ui_component_setscrolltop(self->client,
			psy_ui_value_make_px(
				floor((scrolltoppx + diff) / scrollstepy_px) * scrollstepy_px));		
		self->thumbmove = FALSE;
	}
}

void psy_ui_scroller_ontimer(psy_ui_Scroller* self, uintptr_t timerid)
{	
	if (timerid == 0) {
		if (psy_ui_scrollanimate_tick(&self->hanimate)) {
			psy_ui_component_stoptimer(psy_ui_scroller_base(self), 0);
		}
		psy_ui_component_setscrollleft(self->client,
			psy_ui_value_make_px(psy_ui_scrollanimate_currposition(
				&self->hanimate)));
	} else if (timerid == 1) {
		if (psy_ui_scrollanimate_tick(&self->vanimate)) {
			psy_ui_component_stoptimer(psy_ui_scroller_base(self), 1);
		}
		psy_ui_component_setscrolltop(self->client,
			psy_ui_value_make_px(psy_ui_scrollanimate_currposition(
				&self->vanimate)));
	}
}

void psy_ui_scroller_onscroll(psy_ui_Scroller* self, psy_ui_Component* sender)
{
	if (!self->thumbmove) {
		double pos;
		const psy_ui_TextMetric* tm;

		tm = psy_ui_component_textmetric(self->client);
		/* vertical */
		pos = floor(psy_ui_component_scrolltop_px(self->client) /
			psy_ui_component_scrollstep_height_px(self->client));
		psy_ui_scrollbar_setthumbposition(&self->vscroll, pos);
		/* horizontal */
		pos = floor(psy_ui_component_scrollleftpx(self->client) /
			psy_ui_component_scrollstep_width_px(self->client));
		psy_ui_scrollbar_setthumbposition(&self->hscroll, pos);
	}
}

void psy_ui_scroller_scrollrangechanged(psy_ui_Scroller* self, psy_ui_Component* sender,
	psy_ui_Orientation orientation)
{	
	if (orientation == psy_ui_VERTICAL) {
		psy_ui_IntPoint vrange;		
		
		if (self->component.id == 50) {
			self = self;
		}
		vrange = psy_ui_component_verticalscrollrange(sender);
		psy_ui_scrollbar_setscrollrange(&self->vscroll,
			psy_ui_component_verticalscrollrange(sender));
		vrange = psy_ui_component_verticalscrollrange(sender);
				
		if (vrange.y - vrange.x <= 0) {
			if (psy_ui_component_visible(&self->vscroll.component)) {
				psy_ui_component_hide(&self->vscroll.component);
				psy_ui_component_hide(&self->spacer);
				psy_ui_component_align(&self->component);
				psy_ui_component_align(&self->bottom);
			}
		} else if (!psy_ui_component_visible(&self->vscroll.component)) {
			psy_ui_component_show(&self->vscroll.component);
			if (psy_ui_component_visible(&self->hscroll.component)) {
				psy_ui_component_show(&self->spacer);
			}
			psy_ui_component_align(&self->component);
			psy_ui_component_align(&self->bottom);
		}
	} else if (orientation == psy_ui_HORIZONTAL) {
		psy_ui_IntPoint hrange;

		hrange = psy_ui_component_horizontalscrollrange(sender);
		psy_ui_scrollbar_setscrollrange(&self->hscroll,
			hrange);
		hrange = psy_ui_component_horizontalscrollrange(sender);
		if ((hrange.y - hrange.x) <= 0) {
			if (psy_ui_component_visible(&self->bottom)) {
				psy_ui_component_hide(&self->bottom);
				psy_ui_component_hide(&self->spacer);
				psy_ui_component_align(&self->component);				
			}
		} else if (!psy_ui_component_visible(&self->bottom)) {
			psy_ui_component_show(&self->bottom);
			if (psy_ui_component_visible(&self->vscroll.component)) {
				psy_ui_component_show(&self->spacer);
			}
			psy_ui_component_align(&self->component);
			psy_ui_component_align(&self->bottom);
		} else if (psy_ui_component_visible(&self->vscroll.component)
				&& psy_ui_component_visible(&self->bottom) &&
				!psy_ui_component_visible(&self->spacer)) {
			psy_ui_component_show(&self->spacer);
			psy_ui_component_align(&self->bottom);
		}
	}
}

void psy_ui_scroller_onscrollbarclicked(psy_ui_Scroller* self,
	psy_ui_Component* sender)
{
	if (self->client) {
		psy_ui_component_setfocus(self->client);
	}
}

void psy_ui_scroller_onfocus(psy_ui_Scroller* self, psy_ui_Component* sender)
{
	if (self->client) {
		psy_ui_component_setfocus(self->client);
	}
}

void psy_ui_scroller_onupdatestyles(psy_ui_Scroller* self)
{
	if (self->client) {		
		const psy_ui_Style* style;
		
		style = psy_ui_app_style_const(psy_ui_app(),
			psy_ui_componentstyle_currstyleid(&self->client->style));
		if (style) {
			psy_ui_component_setborder(&self->component, &style->border);			
		}
	}
}

void psy_ui_scroller_onpanedraw(psy_ui_Scroller* self,
	psy_ui_Component* sender, psy_ui_Graphics* g)
{
	if (self->client && self->pane.backgroundmode == psy_ui_NOBACKGROUND) {
		psy_ui_RealRectangle r;
		psy_ui_RealSize size;
		psy_ui_RealRectangle client_position;				

		size = psy_ui_component_scrollsize_px(sender);
		client_position = psy_ui_component_position(self->client);
		if (client_position.bottom < size.height) {
			r = psy_ui_realrectangle_make(
				psy_ui_realpoint_make(0.0, client_position.bottom),
				psy_ui_realsize_make(
					size.width,
					psy_max(0.0, size.height - client_position.bottom)));
			psy_ui_drawsolidrectangle(g, r, psy_ui_component_backgroundcolour(sender));
		}
		if (client_position.bottom < size.height) {
			r = psy_ui_realrectangle_make(
				psy_ui_realpoint_make(client_position.right, 0.0),
				psy_ui_realsize_make(					
					psy_max(size.width - client_position.right, 0.0),
					size.height));
			psy_ui_drawsolidrectangle(g, r, psy_ui_component_backgroundcolour(sender));
		}		
	}
}
