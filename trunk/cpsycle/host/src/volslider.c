// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "volslider.h"
#include <uiapp.h>
#include <stdio.h>
#include <math.h>
#include <portable.h>

static int TIMERID_VOLSLIDER = 700;

static void volslider_ondraw(VolSlider*, psy_ui_Component* sender,
	psy_ui_Graphics*);
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

void volslider_init(VolSlider* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	self->value = 0.f;
	self->dragx = -1;
	self->machines = &workspace->song->machines;	
	ui_component_init(&self->component, parent);	
	psy_signal_connect(&self->component.signal_draw, self,
		volslider_ondraw);
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
	ui_component_starttimer(&self->component, TIMERID_VOLSLIDER, 50);
}

void volslider_ondraw(VolSlider* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{
	ui_rectangle r;
	ui_size size;
	int sliderwidth;
	extern psy_ui_App app;

	size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_setcolor(g, ui_defaults_bordercolor(&app.defaults));
	ui_drawrectangle(g, r);
	sliderwidth = 6;	
	ui_setrectangle(&r, (int)((size.width - sliderwidth) * self->value), 
		2, sliderwidth, size.height - 4);
	ui_drawsolidrectangle(g, r, ui_defaults_color(&app.defaults));
	
}

void volslider_onmousedown(VolSlider* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	ui_size size;
	size = ui_component_size(&self->component);
	self->dragx = ev->x - (int)(self->value * (size.width - 6));
	ui_component_capture(&self->component);
}

void volslider_onmousemove(VolSlider* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->dragx != -1) {
		ui_size size;

		size = ui_component_size(&self->component);
		self->value = max(0.f, 
			min(1.f, (ev->x - self->dragx) / (float)(size.width - 6)));
		volslider_onsliderchanged(self, sender);
		ui_component_invalidate(&self->component);
	}
}

void volslider_onmouseup(VolSlider* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	self->dragx = -1;
	ui_component_releasecapture(&self->component);
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
			ui_component_invalidate(&self->component);
		}
	}
}
