// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "volslider.h"
#include <uiapp.h>
#include <stdio.h>
#include <math.h>
#include "../../detail/portable.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>


static int TIMERID_VOLSLIDER = 700;

static void volslider_ondraw(VolSlider*, psy_ui_Graphics*);
static void volslider_onmousedown(VolSlider*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void volslider_onmouseup(VolSlider*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void volslider_onmousemove(VolSlider*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void volslider_ontimer(VolSlider*, psy_ui_Component* sender,
	int timerid);
static void volslider_onsliderchanged(VolSlider*, psy_ui_Component* sender);
static void volslider_onsongchanged(VolSlider*, Workspace*);
static void volslider_onpreferredsize(VolSlider*, psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(VolSlider* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			volslider_onpreferredsize;
		vtable.ondraw = (psy_ui_fp_ondraw) volslider_ondraw;
	}
}

void volslider_init(VolSlider* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	self->value = 0.f;
	self->dragx = -1;
	self->machines = &workspace->song->machines;	
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;	
	psy_signal_connect(&self->component.signal_mousedown, self,
		volslider_onmousedown);
	psy_signal_connect(&self->component.signal_mousemove, self,
		volslider_onmousemove);
	psy_signal_connect(&self->component.signal_mouseup, self,
		volslider_onmouseup);
	psy_signal_connect(&self->component.signal_timer, self,
		volslider_ontimer);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		volslider_onsongchanged);
	psy_ui_component_starttimer(&self->component, TIMERID_VOLSLIDER, 50);
}

void volslider_ondraw(VolSlider* self, psy_ui_Graphics* g)
{
	psy_ui_Rectangle r;
	psy_ui_Size size;
	int sliderwidth;
	extern psy_ui_App app;

	size = psy_ui_component_size(&self->component);
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_setcolor(g, psy_ui_defaults_bordercolor(&app.defaults));
	psy_ui_drawrectangle(g, r);
	sliderwidth = 6;	
	psy_ui_setrectangle(&r, (int)((size.width - sliderwidth) * self->value), 
		2, sliderwidth, size.height - 4);
	psy_ui_drawsolidrectangle(g, r, psy_ui_defaults_color(&app.defaults));
	
}

void volslider_onmousedown(VolSlider* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_ui_Size size;
	size = psy_ui_component_size(&self->component);
	self->dragx = ev->x - (int)(self->value * (size.width - 6));
	psy_ui_component_capture(&self->component);
}

void volslider_onmousemove(VolSlider* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->dragx != -1) {
		psy_ui_Size size;

		size = psy_ui_component_size(&self->component);
		self->value = max(0.f, 
			min(1.f, (ev->x - self->dragx) / (float)(size.width - 6)));
		volslider_onsliderchanged(self, sender);
		psy_ui_component_invalidate(&self->component);
	}
}

void volslider_onmouseup(VolSlider* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	self->dragx = -1;
	psy_ui_component_releasecapture(&self->component);
}

void volslider_onsliderchanged(VolSlider* self, psy_ui_Component* sender)
{	
	if (self->machines) {
		machines_setvolume(self->machines, self->value * self->value * 4.f);
	}
}

void volslider_onsongchanged(VolSlider* self, Workspace* workspace)
{
	self->machines = &workspace->song->machines;
}

void volslider_ontimer(VolSlider* self, psy_ui_Component* sender, int timerid)
{		
	if (self->machines) {
		psy_dsp_amp_t oldvalue;

		oldvalue = self->value;
		self->value = (psy_dsp_amp_t)(sqrt(machines_volume(self->machines)) * 0.5f);
		if (oldvalue != self->value) {
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void volslider_onpreferredsize(VolSlider* self, psy_ui_Size* limit, psy_ui_Size* rv)
{	
	rv->width = 180;
	rv->height = 20;
}
