// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "envelopeview.h"

#include <math.h>
// platform
#include "../../detail/portable.h"

static void envelopebox_ondraw(EnvelopeBox*, psy_ui_Graphics*);
static void envelopebox_drawgrid(EnvelopeBox*, psy_ui_Graphics*);
static void envelopebox_drawpoints(EnvelopeBox*, psy_ui_Graphics*);
static void envelopebox_drawlines(EnvelopeBox*, psy_ui_Graphics*);
static void envelopebox_drawruler(EnvelopeBox*, psy_ui_Graphics*);
static void envelopebox_onsize(EnvelopeBox*, const psy_ui_Size*);
static void envelopebox_ondestroy(EnvelopeBox*, psy_ui_Component* sender);
static void envelopebox_onmousedown(EnvelopeBox*, psy_ui_MouseEvent*);
static void envelopebox_onmousemove(EnvelopeBox*, psy_ui_MouseEvent*);
static void envelopebox_onmouseup(EnvelopeBox*, psy_ui_MouseEvent*);
static psy_List* envelopebox_hittestpoint(EnvelopeBox* self, double x, double y);
static void envelopebox_shiftsuccessors(EnvelopeBox* self, double timeshift);
static double envelopebox_pxvalue(EnvelopeBox*, double value);
static double envelopebox_pxtime(EnvelopeBox*, psy_dsp_big_seconds_t t);
static psy_dsp_big_seconds_t envelopebox_pxtotime(EnvelopeBox*, double px);
static psy_dsp_EnvelopePoint envelopebox_pxtopoint(EnvelopeBox*, double x, double y);
static psy_dsp_big_seconds_t envelopebox_displaymaxtime(EnvelopeBox*);

static void checkadjustpointrange(psy_List* pointnode);

static psy_ui_ComponentVtable envelopebox_vtable;
static bool envelopebox_vtable_initialized = FALSE;

static void envelopebox_vtable_init(EnvelopeBox* self)
{
	if (!envelopebox_vtable_initialized) {
		envelopebox_vtable = *(self->component.vtable);
		envelopebox_vtable.onsize = (psy_ui_fp_component_onsize) envelopebox_onsize;
		envelopebox_vtable.ondraw = (psy_ui_fp_component_ondraw) envelopebox_ondraw;		
		envelopebox_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			envelopebox_onmousedown;
		envelopebox_vtable.onmousemove = (psy_ui_fp_component_onmousemove)
			envelopebox_onmousemove;
		envelopebox_vtable.onmouseup = (psy_ui_fp_component_onmouseup)
			envelopebox_onmouseup;
		envelopebox_vtable_initialized = TRUE;
	}
}

void envelopebox_init(EnvelopeBox* self, psy_ui_Component* parent)
{				
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_preventalign(&self->component);
	envelopebox_vtable_init(self);
	self->component.vtable = &envelopebox_vtable;
	self->zoomleft = 0.f;
	self->zoomright = 1.f;
	self->modamount = 1.f;
	self->cx = 0;
	self->cy = 0;
	self->dragpoint = NULL;
	self->settings = NULL;
	self->sustainstage = 2;
	self->dragrelative = 1;	
	self->dragpointindex = psy_INDEX_INVALID;
	psy_ui_margin_init_all(&self->spacing,
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5));
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(20),
			psy_ui_value_makeeh(15)));	
	psy_ui_component_doublebuffer(&self->component);	
	psy_signal_connect(&self->component.signal_destroy, self,
		envelopebox_ondestroy);	
	psy_signal_init(&self->signal_tweaked);
}

void envelopebox_setenvelope(EnvelopeBox* self,
	psy_dsp_Envelope* settings)
{	
	self->settings = settings;
	self->modamount = 1.f;
	psy_ui_component_invalidate(&self->component);
}

void envelopebox_ondestroy(EnvelopeBox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_tweaked);
}

void envelopebox_ondraw(EnvelopeBox* self, psy_ui_Graphics* g)
{	
	envelopebox_drawgrid(self, g);
	envelopebox_drawruler(self, g);
	envelopebox_drawlines(self, g);	
	envelopebox_drawpoints(self, g);	
}

void envelopebox_drawgrid(EnvelopeBox* self, psy_ui_Graphics* g)
{
	psy_dsp_big_seconds_t i;
	psy_dsp_big_seconds_t smallstep;
	psy_dsp_big_seconds_t step;

	if (self->settings && self->settings->timemode == psy_dsp_ENVELOPETIME_TICK) {
		smallstep = 1;
		step = 10;
	} else {
		smallstep = 0.1;
		step = 0.5;
	}	
	psy_ui_setcolour(g, psy_ui_colour_make(0x00333333));
	for (i = 0; i <= 1.0; i += smallstep ) {
		double cpy;

		cpy = envelopebox_pxvalue(self, i);
		psy_ui_drawline(g,
			self->spacing.left.quantity.px, cpy,
			self->spacing.left.quantity.px + self->cx, cpy);
	}	
	for (i = 0; i <= envelopebox_displaymaxtime(self); i += step) {
		psy_ui_drawline(g,
			envelopebox_pxtime(self, i),
			self->spacing.top.quantity.px,
			envelopebox_pxtime(self, i),
			self->cy - self->spacing.bottom.quantity.px);
	}
}

void envelopebox_drawpoints(EnvelopeBox* self, psy_ui_Graphics* g)
{
	psy_List* p;
	const psy_ui_TextMetric* tm;
	psy_ui_Size ptsize;
	psy_ui_Size ptsize2;
	psy_dsp_EnvelopePoint* q = 0;
	psy_List* points;

	if (self->settings) {
		points = self->settings->points;
	} else {
		return;
	}
	tm = psy_ui_component_textmetric(&self->component);
	ptsize = psy_ui_size_make(psy_ui_value_makepx(5), psy_ui_value_makepx(5));
	ptsize2 = psy_ui_size_make(
		psy_ui_value_makepx(psy_ui_value_px(&ptsize.width, tm) / 2),
		psy_ui_value_makepx(psy_ui_value_px(&ptsize.height, tm) / 2));
	for (p = points; p != 0; p = p->next) {
		psy_ui_Rectangle r;
		psy_dsp_EnvelopePoint* pt;

		pt = (psy_dsp_EnvelopePoint*)p->entry;
		psy_ui_setrectangle(&r,
			envelopebox_pxtime(self, pt->time) - psy_ui_value_px(&ptsize2.width, tm),
			envelopebox_pxvalue(self, pt->value * self->modamount) -
				psy_ui_value_px(&ptsize2.height, tm),
			psy_ui_value_px(&ptsize.width, tm),
			psy_ui_value_px(&ptsize.height, tm));
		psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00B1C8B0));
		q = pt;
	}
}

void envelopebox_drawlines(EnvelopeBox* self, psy_ui_Graphics* g)
{
	psy_List* p;
	psy_dsp_EnvelopePoint* q = 0;
	uintptr_t count = 0;
	psy_List* points;

	if (self->settings) {
		points = self->settings->points;
	} else {
		return;
	}

	psy_ui_setcolour(g, psy_ui_colour_make(0x00B1C8B0));
	for (p = points; p !=0; p = p->next, ++count) {		
		psy_dsp_EnvelopePoint* pt;

		pt = (psy_dsp_EnvelopePoint*)p->entry;			
		if (q) {
			psy_ui_drawline(g,
				envelopebox_pxtime(self, q->time),
				envelopebox_pxvalue(self, q->value * self->modamount),
				envelopebox_pxtime(self, pt->time),
				envelopebox_pxvalue(self, pt->value * self->modamount));
		}
		q = pt;
		if (count == self->sustainstage) {
			psy_ui_setcolour(g, psy_ui_colour_make(0x00516850));
			psy_ui_drawline(g,
				envelopebox_pxtime(self, q->time),
				self->spacing.top.quantity.px,
				envelopebox_pxtime(self, q->time),
				self->spacing.top.quantity.px + self->cy);
			psy_ui_setcolour(g, psy_ui_colour_make(0x00B1C8B0));
		}
	}	
}

void envelopebox_drawruler(EnvelopeBox* self, psy_ui_Graphics* g)
{
	psy_dsp_big_seconds_t t;
	psy_dsp_big_seconds_t maxtime;
	const psy_ui_TextMetric* tm;
	int step;
	int numsteps;

	maxtime = envelopebox_displaymaxtime(self);
	psy_ui_setcolour(g, psy_ui_defaults()->style_common.border.colour_top);
	psy_ui_drawline(g, 0, self->cy, self->cx, self->cy);
	tm = psy_ui_component_textmetric(envelopebox_base(self));
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00434343));
	if (self->settings && self->settings->timemode == psy_dsp_ENVELOPETIME_TICK) {
		numsteps = (int)(maxtime);
	} else {
		numsteps = (int)(maxtime / 0.1);
	}
	for (step = 0; step  <= numsteps; ++step) {
		double cpx;
		int res;
		
		if (self->settings &&
				self->settings->timemode == psy_dsp_ENVELOPETIME_TICK) {
			t = step;
			res = 20;
		} else {
			res = 10;
			t = step * 0.1;
		}
		cpx = envelopebox_pxtime(self, t);
		psy_ui_drawline(g, cpx, self->cy - 5, cpx, self->cy);
		if ((step % res) == 0.0) {
			char text[64];

			if (self->settings && self->settings->timemode == psy_dsp_ENVELOPETIME_TICK) {
				psy_snprintf(text, 64, "%dt", (int)t);
			} else {
				psy_snprintf(text, 64, "%ds", (int)t);
			}
			if (step < numsteps) {
				psy_ui_textout(g, cpx + 4, self->cy - (tm->tmHeight * 1.5),
					text, strlen(text));
			} else {
				psy_ui_textout(g,
					cpx - 3.5 * (double)tm->tmAveCharWidth, self->cy - (tm->tmHeight * 1.5),
					text, strlen(text));
			}
			psy_ui_setcolour(g, psy_ui_colour_make(0x00434343));
			psy_ui_drawline(g, cpx, self->spacing.top.quantity.px,
				cpx, self->cy - self->spacing.bottom.quantity.px);
			psy_ui_setcolour(g, psy_ui_defaults()->style_common.border.colour_top);
		}
	}
}

void envelopebox_onsize(EnvelopeBox* self, const psy_ui_Size* size)
{
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);
	self->cx = psy_ui_value_px(&size->width, tm) - psy_ui_value_px(&self->spacing.left, tm) -
		psy_ui_value_px(&self->spacing.right, tm);
	self->cy = psy_ui_value_px(&size->height, tm) - psy_ui_value_px(&self->spacing.top, tm) -
		psy_ui_value_px(&self->spacing.bottom, tm);
}

void envelopebox_onmousedown(EnvelopeBox* self, psy_ui_MouseEvent* ev)
{	
	self->dragpoint = envelopebox_hittestpoint(self, ev->x, ev->y);
	self->dragpointindex = psy_INDEX_INVALID;
	if (ev->button == 1) {
		if (!self->dragpoint && self->settings) {
			psy_dsp_EnvelopePoint pt_new;
			psy_dsp_EnvelopePoint* pt_insert;
			psy_List* p;

			pt_new = envelopebox_pxtopoint(self, ev->x, ev->y);
			p = NULL;
			if (self->settings->points) {
				for (p = self->settings->points->tail; p != NULL; p = p->prev) {
					psy_dsp_EnvelopePoint* pt;

					pt = (psy_dsp_EnvelopePoint*)p->entry;
					if (pt->time < pt_new.time) {
						break;
					}
				}
			}
			pt_insert = psy_dsp_envelopepoint_alloc();
			if (pt_insert) {				
				if (p == NULL) { // first?
					*pt_insert = psy_dsp_envelopepoint_make_all(
						0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
				} else {
					*pt_insert = pt_new;
					pt_insert->maxtime = 65535.f;
					pt_insert->maxvalue = 1.f;
				}
				if (p && p->next == NULL) {
					psy_dsp_EnvelopePoint* pt;

					pt = (psy_dsp_EnvelopePoint*)p->entry;
					pt->maxvalue = 1.f;
					pt_insert->maxvalue = 0.f;
					pt_insert->value = 0.f; // last point
				}
				psy_list_insert(&self->settings->points, p, pt_insert);
				psy_ui_component_invalidate(&self->component);
				self->dragpointindex = psy_list_entry_index(
					self->settings->points, pt_insert);
				psy_signal_emit(&self->signal_tweaked, self, 1, self->dragpointindex);
			}
		} else  if (self->settings && self->settings->points) {
			self->dragpointindex = psy_list_entry_index(
				self->settings->points, self->dragpoint->entry);
			psy_signal_emit(&self->signal_tweaked, self, 1, self->dragpointindex);
		}
		psy_ui_component_capture(&self->component);
	} else if (self->settings && self->dragpoint) {
		psy_List* p;
		bool first;

		first = self->settings->points == self->dragpoint;
		p = psy_list_remove(&self->settings->points, self->dragpoint);
		self->dragpoint = NULL;
		if (first && self->settings->points) {
			psy_dsp_EnvelopePoint* pt;

			pt = (psy_dsp_EnvelopePoint*)(psy_list_entry(self->settings->points));
			*pt = psy_dsp_envelopepoint_make_start();
			psy_signal_emit(&self->signal_tweaked, self, 1, self->dragpointindex);
		} else if (p == NULL && self->settings->points) {
			psy_dsp_EnvelopePoint* pt;

			pt = (psy_dsp_EnvelopePoint*)(psy_list_entry(psy_list_last(self->settings->points)));			
			pt->maxvalue = 0.f;
			pt->value = 0.f;
			psy_signal_emit(&self->signal_tweaked, self, 1, self->dragpointindex);
		}		
		psy_ui_component_invalidate(&self->component);
	}	
}

void envelopebox_setzoom(EnvelopeBox* self, double zoomleft, double zoomright)
{
	self->zoomleft = zoomleft;
	self->zoomright = zoomright;
	if (fabs(self->zoomleft - self->zoomright) < 0.001) {
		self->zoomright = self->zoomleft + 0.001f;
	}
	psy_ui_component_invalidate(&self->component);
}

void envelopebox_onmousemove(EnvelopeBox* self, psy_ui_MouseEvent* ev)
{		
	if (self->dragpoint) {		
		psy_dsp_EnvelopePoint* pt;
		double oldtime;

		pt = (psy_dsp_EnvelopePoint*)self->dragpoint->entry;
		oldtime = pt->time;
		pt->value = (psy_dsp_amp_t)((1.0 - ((ev->y - self->spacing.top.quantity.px) /
			self->cy)) / (double)(self->modamount));
		pt->time = (psy_dsp_beat_t)envelopebox_pxtotime(self, ev->x);
		checkadjustpointrange(self->dragpoint);
		envelopebox_shiftsuccessors(self, pt->time - oldtime);		
		psy_ui_component_invalidate(&self->component);
		psy_signal_emit(&self->signal_tweaked, self, 1, self->dragpointindex);
	}
}

void envelopebox_shiftsuccessors(EnvelopeBox* self, double timeshift)
{	
	if (self->dragrelative) {
		psy_List* p;
		for (p = self->dragpoint->next; p != NULL; p = p->next) {		
			psy_dsp_EnvelopePoint* pt;		

			pt = (psy_dsp_EnvelopePoint*)p->entry;
			pt->time += (float)timeshift;
			checkadjustpointrange(p);
		}
	}
}

void checkadjustpointrange(psy_List* p)
{
	psy_dsp_EnvelopePoint* pt;	

	pt = (psy_dsp_EnvelopePoint*) p->entry;	
	if (p->prev) {
		psy_dsp_EnvelopePoint* ptprev;

		ptprev = (psy_dsp_EnvelopePoint*) p->prev->entry;
		if (pt->time < ptprev->time) {
			pt->time = ptprev->time;
		}
	}
	if (pt->value > pt->maxvalue) {
		pt->value = pt->maxvalue;
	} else
	if (pt->value < pt->minvalue) {
		pt->value = pt->minvalue;
	}
	if (pt->time > pt->maxtime) {
		pt->time = pt->maxtime;
	} else
	if (pt->time < pt->mintime) {
		pt->time = pt->mintime;
	}	
}

void envelopebox_onmouseup(EnvelopeBox* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_releasecapture(&self->component);
	self->dragpoint = NULL;	
}

psy_List* envelopebox_hittestpoint(EnvelopeBox* self, double x, double y)
{
	psy_List* p;
	psy_List* points;

	points = (self->settings)
		? self->settings->points
		: NULL;		
	if (!points) {
		return NULL;
	}
	for (p = points->tail; p != NULL; p = p->prev) {		
		psy_dsp_EnvelopePoint* pt;		

		pt = (psy_dsp_EnvelopePoint*)p->entry;			
		if (abs((int)(envelopebox_pxtime(self, pt->time) - x)) < 5 &&
				abs((int)(envelopebox_pxvalue(self,
					pt->value * self->modamount) - y)) < 5) {
			break;
		}
	}	
	return p;
}

psy_dsp_EnvelopePoint envelopebox_pxtopoint(EnvelopeBox* self, double x,
	double y)
{	
	return psy_dsp_envelopepoint_make(
		(float)envelopebox_pxtotime(self, x),
		(psy_dsp_amp_t)(1.0 - (y - self->spacing.top.quantity.px) / self->cy));
}

double envelopebox_pxvalue(EnvelopeBox* self, double value)
{
	return (self->cy - value * self->cy) +
		self->spacing.top.quantity.px;
}

psy_dsp_big_seconds_t envelopebox_pxtime(EnvelopeBox* self, psy_dsp_big_seconds_t t)
{
	psy_dsp_big_seconds_t offsetstep;
	
	offsetstep= envelopebox_displaymaxtime(self)
		/ self->cx * (self->zoomright - self->zoomleft);
	return ((t - (envelopebox_displaymaxtime(self) *
		self->zoomleft)) / offsetstep) + self->spacing.left.quantity.px;
}

psy_dsp_big_seconds_t envelopebox_pxtotime(EnvelopeBox* self, double px)
{
	psy_dsp_big_seconds_t t;

	double offsetstep = envelopebox_displaymaxtime(self)
		/ self->cx * (self->zoomright - self->zoomleft);
	t = (offsetstep * (px - self->spacing.left.quantity.px)) +
		(envelopebox_displaymaxtime(self) * self->zoomleft);
	if (t < 0) {
		t = 0;
	} else if (t > envelopebox_displaymaxtime(self)) {
		t = envelopebox_displaymaxtime(self);
	}
	return t;
}

psy_dsp_big_seconds_t envelopebox_displaymaxtime(EnvelopeBox* self)
{
	if (self->settings &&
			self->settings->timemode == psy_dsp_ENVELOPETIME_TICK) {
		return 300.0;
	}
	return 5.f;	
}

psy_dsp_EnvelopePoint* allocpoint(psy_dsp_seconds_t time, psy_dsp_amp_t value,
	psy_dsp_seconds_t mintime, psy_dsp_seconds_t maxtime,
	psy_dsp_amp_t minvalue, psy_dsp_amp_t maxvalue)
{
	psy_dsp_EnvelopePoint* rv;

	rv = psy_dsp_envelopepoint_alloc();
	rv->time = time;
	rv->value = value;
	rv->mintime = mintime;
	rv->maxtime = maxtime;
	rv->minvalue = minvalue;
	rv->maxvalue = maxvalue;
	return rv;
}
 
void envelopebox_update(EnvelopeBox* self)
{
	if (self->settings && self->settings->points && !self->dragpoint) {		
		psy_ui_component_invalidate(&self->component);
	}
}

// EnvelopeBar
// prototypes
void envelopebar_enablemillisecs(EnvelopeBar*);
void envelopebar_enableticks(EnvelopeBar*);
void envelopebar_onmillisecs(EnvelopeBar*, psy_ui_Component* sender);
void envelopebar_onticks(EnvelopeBar*, psy_ui_Component* sender);
// implementation
void envelopebar_init(EnvelopeBar* self, psy_ui_Component* parent)
{
	psy_ui_Margin tab;
	tab = psy_ui_defaults_hmargin(psy_ui_defaults());
	tab.right = psy_ui_value_makeew(4.0);
	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_setdefaultalign(envelopebar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_checkbox_init_text(&self->enabled, &self->component, "Envelope");	
	psy_ui_component_setmargin(psy_ui_checkbox_base(&self->enabled), &tab);
	psy_ui_checkbox_init_text(&self->carry, &self->component, "Carry (continue)");	
	psy_ui_component_setmargin(psy_ui_checkbox_base(&self->carry), &tab);
	psy_ui_button_init_text_connect(&self->millisec, &self->component, "Millisecs",
		self, envelopebar_onmillisecs);	
	psy_ui_button_init_text_connect(&self->ticks, &self->component, "Ticks",
		self, envelopebar_onticks);
	psy_ui_component_setmargin(&self->ticks.component, &tab);
	psy_ui_button_init_text(&self->adsr, &self->component, "ADSR");	
	psy_ui_button_allowrightclick(&self->adsr);
	envelopebar_enablemillisecs(self);
}

void envelopebar_settext(EnvelopeBar* self, const char* text)
{
	psy_ui_checkbox_settext(&self->enabled, text);	
}

void envelopebar_enablemillisecs(EnvelopeBar* self)
{
	psy_ui_button_highlight(&self->millisec);
	psy_ui_button_disablehighlight(&self->ticks);
}

void envelopebar_enableticks(EnvelopeBar* self)
{
	psy_ui_button_disablehighlight(&self->millisec);
	psy_ui_button_highlight(&self->ticks);
}

void envelopebar_onmillisecs(EnvelopeBar* self, psy_ui_Component* sender)
{
	envelopebar_enablemillisecs(self);
}

void envelopebar_onticks(EnvelopeBar* self, psy_ui_Component* sender)
{
	envelopebar_enableticks(self);
}

// EnvelopeView
static void envelopeview_ondestroy(EnvelopeView*);
static void envelopeview_onzoom(EnvelopeView*, ScrollZoom* sender);
static void envelopeview_onpredefs(EnvelopeView* self, psy_ui_Button* sender);
static void envelopeview_onenable(EnvelopeView*, psy_ui_CheckBox* sender);
static void envelopeview_oncarry(EnvelopeView*, psy_ui_CheckBox* sender);
static void envelopeview_onmillisecs(EnvelopeView*, psy_ui_Button* sender);
static void envelopeview_onticks(EnvelopeView*, psy_ui_Button* sender);
static void envelopeview_ontweaked(EnvelopeView*, EnvelopeBox* sender,
	int pointindex);

static psy_ui_ComponentVtable envelopeview_vtable;
static int envelopeview_vtable_initialized = 0;

static psy_ui_ComponentVtable* envelopeview_vtable_init(EnvelopeView* self)
{
	if (!envelopeview_vtable_initialized) {
		envelopeview_vtable = *(self->component.vtable);
		envelopeview_vtable.destroy = (psy_ui_fp_component_destroy)
			envelopeview_ondestroy;
		envelopeview_vtable_initialized = 1;
	}
	return &envelopeview_vtable;
}

void envelopeview_init(EnvelopeView* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_component_init(envelopeview_base(self), parent);
	psy_ui_component_setvtable(envelopeview_base(self),
		envelopeview_vtable_init(self));	
	self->workspace = workspace;
	envelopebar_init(&self->bar, envelopeview_base(self));
	psy_ui_component_setalign(envelopebar_base(&self->bar), psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(envelopebar_base(&self->bar),
		psy_ui_defaults_pvmargin(psy_ui_defaults()));
	envelopebox_init(&self->envelopebox, envelopeview_base(self));
	psy_ui_component_setalign(envelopebox_base(&self->envelopebox),
		psy_ui_ALIGN_CLIENT);
	scrollzoom_init(&self->zoom, envelopeview_base(self));
	psy_ui_component_setalign(scrollzoom_base(&self->zoom), psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setpreferredsize(scrollzoom_base(&self->zoom),
		psy_ui_size_make(psy_ui_value_makepx(0),
		psy_ui_value_makeeh(1)));
	psy_signal_connect(&self->zoom.signal_zoom, self,
		envelopeview_onzoom);
	psy_signal_connect(&self->bar.adsr.signal_clicked, self,
		envelopeview_onpredefs);
	psy_signal_connect(&self->bar.enabled.signal_clicked, self,
		envelopeview_onenable);
	psy_signal_connect(&self->bar.carry.signal_clicked, self,
		envelopeview_oncarry);
	psy_signal_connect(&self->bar.millisec.signal_clicked, self,
		envelopeview_onmillisecs);
	psy_signal_connect(&self->bar.ticks.signal_clicked, self,
		envelopeview_onticks);
	psy_signal_init(&self->signal_tweaked);
	psy_signal_connect(&self->envelopebox.signal_tweaked, self,
		envelopeview_ontweaked);	
}

void envelopeview_ondestroy(EnvelopeView* self)
{
	psy_signal_dispose(&self->signal_tweaked);
}

void envelopeview_settext(EnvelopeView* self, const char* text)
{
	envelopebar_settext(&self->bar, text);	
}

void envelopeview_setenvelope(EnvelopeView* self,
	psy_dsp_Envelope* settings)
{
	envelopebox_setenvelope(&self->envelopebox, settings);
	if (settings && psy_dsp_envelope_isenabled(settings)) {
		psy_ui_checkbox_check(&self->bar.enabled);
	} else {
		psy_ui_checkbox_disablecheck(&self->bar.enabled);
	}
	if (settings && psy_dsp_envelope_iscarry(settings)) {
		psy_ui_checkbox_check(&self->bar.carry);
	} else {
		psy_ui_checkbox_disablecheck(&self->bar.carry);
	}
	if (settings && settings->timemode == psy_dsp_ENVELOPETIME_TICK) {
		psy_ui_button_highlight(&self->bar.ticks);
	} else {
		psy_ui_button_disablehighlight(&self->bar.ticks);
	}
	if (!settings || (settings && settings->timemode == psy_dsp_ENVELOPETIME_SECONDS)) {
		psy_ui_button_highlight(&self->bar.millisec);
	} else {
		psy_ui_button_disablehighlight(&self->bar.millisec);
	}
}

void envelopeview_update(EnvelopeView* self)
{
	envelopebox_update(&self->envelopebox);
}

void envelopeview_onzoom(EnvelopeView* self, ScrollZoom* sender)
{
	envelopebox_setzoom(&self->envelopebox, scrollzoom_start(sender),
		scrollzoom_end(sender));
}

void envelopeview_onenable(EnvelopeView* self, psy_ui_CheckBox* sender)
{
	if (self->envelopebox.settings) {
		psy_dsp_envelope_setenabled(self->envelopebox.settings,
			psy_ui_checkbox_checked(sender));
	}
}

void envelopeview_oncarry(EnvelopeView* self, psy_ui_CheckBox* sender)
{
	if (self->envelopebox.settings) {
		psy_dsp_envelope_setcarry(self->envelopebox.settings,
			psy_ui_checkbox_checked(sender));
	}
}

void envelopeview_onmillisecs(EnvelopeView* self, psy_ui_Button* sender)
{
	if (self->envelopebox.settings) {
		self->envelopebox.settings->timemode = psy_dsp_ENVELOPETIME_SECONDS;
		envelopeview_update(self);
		psy_signal_emit(&self->signal_tweaked, self, 1, 0);
	}
}

void envelopeview_onticks(EnvelopeView* self, psy_ui_Button* sender)
{
	if (self->envelopebox.settings) {
		self->envelopebox.settings->timemode = psy_dsp_ENVELOPETIME_TICK;
		envelopeview_update(self);
		psy_signal_emit(&self->signal_tweaked, self, 1, 0);
	}
}

void envelopeview_onpredefs(EnvelopeView* self, psy_ui_Button* sender)
{		
	/*if (self->envelopebox.settings && psy_ui_button_clickstate(sender) == 1) {
		predefsconfig_predef(&self->workspace->config.predefs,
			index, self->envelopebox.settings);		
		psy_ui_component_invalidate(envelopebox_base(&self->envelopebox));
	} else {
		predefsconfig_storepredef(&self->workspace->config.predefs,
			index, self->envelopebox.settings);		
	}*/
}

void envelopeview_ontweaked(EnvelopeView* self, EnvelopeBox* sender, int pointindex)
{
	psy_signal_emit(&self->signal_tweaked, self, 1, pointindex);
}