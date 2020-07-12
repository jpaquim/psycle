// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "envelopeview.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

static psy_dsp_EnvelopePoint* allocpoint(psy_dsp_seconds_t time, psy_dsp_amp_t value,
	psy_dsp_seconds_t mintime,
	psy_dsp_seconds_t maxtime, psy_dsp_amp_t minvalue, psy_dsp_amp_t maxvalue);

void adsrpointmapper_init(ADSRPointMapper* self)
{
	psy_dsp_envelopepoint_init(&self->start, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	psy_dsp_envelopepoint_init(&self->attack, 0.2f, 1.f, 0.f, 5.f, 1.f, 1.f);
	psy_dsp_envelopepoint_init(&self->decay, 0.4f, 0.5f, 0.f, 5.f, 0.f, 1.f);
	psy_dsp_envelopepoint_init(&self->release, 0.6f, 0.f, 0.f, 5.f, 0.f, 0.f);
	self->settings = 0;
}

void adsrpointmapper_updatepoints(ADSRPointMapper* self)
{	
	if (self->settings) {
		self->attack.time = self->settings->attack;
		self->decay.time = self->settings->attack +
			self->settings->decay;
		self->release.time = self->settings->attack + self->settings->decay +
			self->settings->release;
		self->decay.value = self->settings->sustain;
	} else {
		adsrpointmapper_init(self);
	}
}

void adsrpointmapper_updatesettings(ADSRPointMapper* self)
{
	if (self->settings) {
		self->settings->attack = self->attack.time;
		self->settings->decay = self->decay.time - self->attack.time;
		self->settings->sustain = self->decay.value;
		self->settings->release = self->release.time - self->decay.time;
	}
}

static void envelopebox_ondraw(EnvelopeBox*, psy_ui_Graphics*);
static void envelopebox_drawlabel(EnvelopeBox*, psy_ui_Graphics*);
static void envelopebox_drawgrid(EnvelopeBox*, psy_ui_Graphics*);
static void envelopebox_drawpoints(EnvelopeBox*, psy_ui_Graphics*);
static void envelopebox_drawlines(EnvelopeBox*, psy_ui_Graphics*);
static void envelopebox_onsize(EnvelopeBox*, const psy_ui_Size*);
static void envelopebox_ondestroy(EnvelopeBox*, psy_ui_Component* sender);
static void envelopebox_onmousedown(EnvelopeBox*, psy_ui_MouseEvent*);
static void envelopebox_onmousemove(EnvelopeBox*, psy_ui_MouseEvent*);
static void envelopebox_onmouseup(EnvelopeBox*, psy_ui_MouseEvent*);
static psy_List* envelopebox_hittestpoint(EnvelopeBox* self, int x, int y);
static void envelopebox_shiftsuccessors(EnvelopeBox* self, double timeshift);
static int envelopebox_pxvalue(EnvelopeBox*, double value);
static int envelopebox_pxtime(EnvelopeBox*, double t);
static double envelopebox_pxtotime(EnvelopeBox*, int px);
static psy_dsp_seconds_t envelopebox_displaymaxtime(EnvelopeBox*);

static void checkadjustpointrange(psy_List* p);

static psy_ui_ComponentVtable envelopebox_vtable;
static int envelopebox_vtable_initialized = 0;

static void envelopebox_vtable_init(EnvelopeBox* self)
{
	if (!envelopebox_vtable_initialized) {
		envelopebox_vtable = *(self->component.vtable);
		envelopebox_vtable.onsize = (psy_ui_fp_onsize) envelopebox_onsize;
		envelopebox_vtable.ondraw = (psy_ui_fp_ondraw) envelopebox_ondraw;		
		envelopebox_vtable.onmousedown = (psy_ui_fp_onmousedown)
			envelopebox_onmousedown;
		envelopebox_vtable.onmousemove = (psy_ui_fp_onmousemove)
			envelopebox_onmousemove;
		envelopebox_vtable.onmouseup = (psy_ui_fp_onmouseup)
			envelopebox_onmouseup;
		envelopebox_vtable_initialized = 1;
	}
}

void envelopebox_init(EnvelopeBox* self, psy_ui_Component* parent)
{				
	psy_ui_component_init(&self->component, parent);
	envelopebox_vtable_init(self);
	self->component.vtable = &envelopebox_vtable;
	self->zoomleft = 0.f;
	self->zoomright = 1.f;
	self->cx = 0;
	self->cy = 0;
	self->dragpoint = 0;
	self->points = 0;
	self->sustainstage = 2;
	self->dragrelative = 1;
	self->text = strdup("");
	psy_ui_margin_init_all(&self->spacing,
		psy_ui_value_makepx(30),
		psy_ui_value_makepx(0),
		psy_ui_value_makepx(4),
		psy_ui_value_makepx(0));
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(20),
			psy_ui_value_makeeh(15)));	
	psy_ui_component_doublebuffer(&self->component);		
	adsr_settings_init(&self->dummysettings, 0.2f, 0.4f, 0.8f, 0.3f);
	adsrpointmapper_init(&self->pointmapper);
	self->points = psy_list_create(&self->pointmapper.start);
	psy_list_append(&self->points, &self->pointmapper.attack);
	psy_list_append(&self->points, &self->pointmapper.decay);
	psy_list_append(&self->points, &self->pointmapper.release);
	psy_signal_connect(&self->component.signal_destroy, self,
		envelopebox_ondestroy);	
}

void envelopebox_setadsrenvelope(EnvelopeBox* self,
	psy_dsp_ADSRSettings* adsr_settings)
{			
	self->pointmapper.settings = adsr_settings;
	adsrpointmapper_updatepoints(&self->pointmapper);
	psy_ui_component_invalidate(&self->component);
}

void envelopebox_ondestroy(EnvelopeBox* self, psy_ui_Component* sender)
{	
	psy_list_free(self->points);
	self->points = 0;
	free(self->text);
	self->text = 0;	
}

void envelopebox_ondraw(EnvelopeBox* self, psy_ui_Graphics* g)
{		
	envelopebox_drawlabel(self, g);
	envelopebox_drawgrid(self, g);
	envelopebox_drawlines(self, g);
	envelopebox_drawpoints(self, g);
}

void envelopebox_drawlabel(EnvelopeBox* self, psy_ui_Graphics* g)
{
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolor(g, psy_ui_color_make(0x00D1C5B6));
	psy_ui_textout(g, 0, 0, self->text, strlen(self->text));
}

void envelopebox_drawgrid(EnvelopeBox* self, psy_ui_Graphics* g)
{
	double i;		

	psy_ui_setcolor(g, psy_ui_color_make(0x00333333));
	for (i = 0; i <= 1.0; i += 0.1 ) {
		psy_ui_drawline(g,
			self->spacing.left.quantity.integer,
				envelopebox_pxvalue(self, i),
			self->spacing.left.quantity.integer + self->cx,
				envelopebox_pxvalue(self, i));
	}	
	for (i = 0; i <= envelopebox_displaymaxtime(self); i += 0.5) {
		psy_ui_drawline(g,
				envelopebox_pxtime(self, i),
			self->spacing.top.quantity.integer,
				envelopebox_pxtime(self, i),
			self->spacing.top.quantity.integer + self->cy);
	}
}

void envelopebox_drawpoints(EnvelopeBox* self, psy_ui_Graphics* g)
{
	psy_List* p;
	psy_ui_TextMetric tm;
	psy_ui_Size ptsize;
	psy_ui_Size ptsize2;
	psy_dsp_EnvelopePoint* q = 0;

	tm = psy_ui_component_textmetric(&self->component);
	ptsize = psy_ui_size_make(psy_ui_value_makepx(5), psy_ui_value_makepx(5));
	ptsize2 = psy_ui_size_make(
		psy_ui_value_makepx(psy_ui_value_px(&ptsize.width, &tm) / 2),
		psy_ui_value_makepx(psy_ui_value_px(&ptsize.height, &tm) / 2));
	for (p = self->points; p !=0; p = p->next) {
		psy_ui_Rectangle r;
		psy_dsp_EnvelopePoint* pt;

		pt = (psy_dsp_EnvelopePoint*)p->entry;
		psy_ui_setrectangle(&r, 
			envelopebox_pxtime(self, pt->time) - psy_ui_value_px(&ptsize2.width, &tm),
			envelopebox_pxvalue(self, pt->value) - psy_ui_value_px(&ptsize2.height, &tm),
			psy_ui_value_px(&ptsize.width, &tm),
			psy_ui_value_px(&ptsize.height, &tm));
		psy_ui_drawsolidrectangle(g, r, psy_ui_color_make(0x00B1C8B0));
		q = pt;
	}
}

void envelopebox_drawlines(EnvelopeBox* self, psy_ui_Graphics* g)
{
	psy_List* p;
	psy_dsp_EnvelopePoint* q = 0;
	uintptr_t count = 0;

	psy_ui_setcolor(g, psy_ui_color_make(0x00B1C8B0));
	for (p = self->points; p !=0; p = p->next, ++count) {		
		psy_dsp_EnvelopePoint* pt;

		pt = (psy_dsp_EnvelopePoint*)p->entry;			
		if (q) {
			psy_ui_drawline(g,
				envelopebox_pxtime(self, q->time),
				envelopebox_pxvalue(self, q->value),
				envelopebox_pxtime(self, pt->time),
				envelopebox_pxvalue(self, pt->value));
		}
		q = pt;
		if (count == self->sustainstage) {
			psy_ui_drawline(g,
				envelopebox_pxtime(self, q->time),
				self->spacing.top.quantity.integer,
				envelopebox_pxtime(self, q->time),
				self->spacing.top.quantity.integer + self->cy);
		}
	}
}

void envelopebox_onsize(EnvelopeBox* self, const psy_ui_Size* size)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	self->cx = psy_ui_value_px(&size->width, &tm) - psy_ui_value_px(&self->spacing.left, &tm) -
		psy_ui_value_px(&self->spacing.right, &tm);
	self->cy = psy_ui_value_px(&size->height, &tm) - psy_ui_value_px(&self->spacing.top, &tm) -
		psy_ui_value_px(&self->spacing.bottom, &tm);
}

void envelopebox_onmousedown(EnvelopeBox* self, psy_ui_MouseEvent* ev)
{
	self->dragpoint = envelopebox_hittestpoint(self, ev->x, ev->y);
	psy_ui_component_capture(&self->component);
}

void envelopebox_setzoom(EnvelopeBox* self, float zoomleft, float zoomright)
{
	self->zoomleft = zoomleft;
	self->zoomright = zoomright;
	psy_ui_component_invalidate(&self->component);
}

void envelopebox_onmousemove(EnvelopeBox* self, psy_ui_MouseEvent* ev)
{		
	if (self->dragpoint) {		
		psy_dsp_EnvelopePoint* pt;
		double oldtime;

		pt = (psy_dsp_EnvelopePoint*)self->dragpoint->entry;
		oldtime = pt->time;
		pt->value = 1.f - (ev->y - self->spacing.top.quantity.integer) /
			(float)self->cy;
		pt->time = (psy_dsp_beat_t)envelopebox_pxtotime(self, ev->x);
		checkadjustpointrange(self->dragpoint);
		envelopebox_shiftsuccessors(self, pt->time - oldtime);
		adsrpointmapper_updatesettings(&self->pointmapper);
		psy_ui_component_invalidate(&self->component);
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
	self->dragpoint = 0;
	psy_ui_component_releasecapture(&self->component);
}

psy_List* envelopebox_hittestpoint(EnvelopeBox* self, int x, int y)
{
	psy_List* p;
	
	for (p = self->points->tail; p != NULL; p = p->prev) {		
		psy_dsp_EnvelopePoint* pt;		

		pt = (psy_dsp_EnvelopePoint*)p->entry;			
		if (abs(envelopebox_pxtime(self, pt->time) - x) < 5 &&
				abs(envelopebox_pxvalue(self, pt->value) - y) < 5) {
			break;
		}
	}	
	return p;
}

int envelopebox_pxvalue(EnvelopeBox* self, double value)
{
	return (int)(self->cy - value * self->cy) +
		self->spacing.top.quantity.integer;
}

int envelopebox_pxtime(EnvelopeBox* self, double t)
{
	float offsetstep = (float) (float)envelopebox_displaymaxtime(self) 
		/ self->cx * (self->zoomright - self->zoomleft);		
	return (int)((t - (envelopebox_displaymaxtime(self) *
		self->zoomleft)) / offsetstep) + self->spacing.left.quantity.integer;
}

double envelopebox_pxtotime(EnvelopeBox* self, int px)
{
	double t;
	float offsetstep = (float)envelopebox_displaymaxtime(self)
		/ self->cx * (self->zoomright - self->zoomleft);
	t = (offsetstep * px) + (envelopebox_displaymaxtime(self) *
		self->zoomleft);
	if (t < 0) {
		t = 0;
	} else
	if (t > envelopebox_displaymaxtime(self)) {
		t = envelopebox_displaymaxtime(self);
	}
	return t;
}

float envelopebox_displaymaxtime(EnvelopeBox* self)
{
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
	if (self->points && !self->dragpoint) {
		adsrpointmapper_updatepoints(&self->pointmapper);
		psy_ui_component_invalidate(&self->component);
	}
}

void envelopebox_settext(EnvelopeBox* self, const char* text)
{
	free(self->text);
	self->text = strdup(text);
}


// EnvelopeView
static void envelopeview_onzoom(EnvelopeView*, ScrollZoom* sender);

static psy_ui_ComponentVtable envelopeview_vtable;
static int envelopeview_vtable_initialized = 0;

static void envelopeview_vtable_init(EnvelopeView* self)
{
	if (!envelopeview_vtable_initialized) {
		envelopeview_vtable = *(self->component.vtable);	
		envelopeview_vtable_initialized = 1;
	}
}

void envelopeview_init(EnvelopeView* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	envelopeview_vtable_init(self);
	self->component.vtable = &envelopeview_vtable;
	psy_ui_component_enablealign(&self->component);
	envelopebox_init(&self->envelopebox, &self->component);
	psy_ui_component_setalign(&self->envelopebox.component, psy_ui_ALIGN_CLIENT);
	scrollzoom_init(&self->zoom, &self->component);
	psy_ui_component_setalign(&self->zoom.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setpreferredsize(&self->zoom.component,
		psy_ui_size_make(psy_ui_value_makepx(0),
		psy_ui_value_makeeh(1)));
	psy_signal_connect(&self->zoom.signal_zoom, self, envelopeview_onzoom);	
}

void envelopeview_settext(EnvelopeView* self, const char* text)
{
	envelopebox_settext(&self->envelopebox, text);
}

void envelopeview_setadsrenvelope(EnvelopeView* self,
	psy_dsp_ADSRSettings* adsr_settings)
{
	envelopebox_setadsrenvelope(&self->envelopebox, adsr_settings);
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
