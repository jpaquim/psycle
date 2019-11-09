// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "volslider.h"
#include <stdio.h>
#include <math.h>

static int TIMERID_VOLSLIDER = 700;

static void volslider_ondraw(VolSlider*, ui_component* sender, ui_graphics*);
static void volslider_onmousedown(VolSlider*, ui_component* sender, int x, int y, int button);
static void volslider_onmouseup(VolSlider*, ui_component* sender, int x, int y, int button);
static void volslider_onmousemove(VolSlider*, ui_component* sender,int x, int y, int button);
static void volslider_ontimer(VolSlider*, ui_component* sender, int timerid);
static void volslider_onsliderchanged(VolSlider*, ui_component* sender);
static void volslider_onsongchanged(VolSlider*, Workspace*);

void volslider_init(VolSlider* self, ui_component* parent, Workspace* workspace)
{		
	self->value = 0.f;
	self->dragx = -1;
	self->machines = &workspace->song->machines;	
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, volslider_ondraw);
	signal_connect(&self->component.signal_mousedown, self, volslider_onmousedown);
	signal_connect(&self->component.signal_mousemove, self, volslider_onmousemove);
	signal_connect(&self->component.signal_mouseup, self, volslider_onmouseup);
	signal_connect(&self->component.signal_timer, self, volslider_ontimer);	
	signal_connect(&workspace->signal_songchanged, self, volslider_onsongchanged);
	ui_component_starttimer(&self->component, TIMERID_VOLSLIDER, 50);
}

void volslider_ondraw(VolSlider* self, ui_component* sender, ui_graphics* g)
{
	ui_rectangle r;
	ui_size size;
	int sliderwidth;	

	size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_setcolor(g, 0x00333333);
	ui_drawrectangle(g, r);
	sliderwidth = 6;	
	ui_setrectangle(&r, (int)((size.width - sliderwidth) * self->value), 
		2, sliderwidth, size.height - 4);
	ui_drawsolidrectangle(g, r, 0x00CACACA);
	
}

void volslider_onmousedown(VolSlider* self, ui_component* sender, int x, int y,
	int button)
{
	ui_size size;
	size = ui_component_size(&self->component);
	self->dragx = x - (int)(self->value * (size.width - 6));
	ui_component_capture(&self->component);
}

void volslider_onmousemove(VolSlider* self, ui_component* sender,int x, int y,
	int button)
{
	if (self->dragx != -1) {
		ui_size size;

		size = ui_component_size(&self->component);
		self->value = max(0.f, 
			min(1.f, (x - self->dragx) / (float)(size.width - 6)));
		volslider_onsliderchanged(self, sender);
		ui_invalidate(&self->component);
	}
}

void volslider_onmouseup(VolSlider* self, ui_component* sender, int x, int y,
	int button)
{
	self->dragx = -1;
	ui_component_releasecapture(&self->component);
}

void volslider_onsliderchanged(VolSlider* self, ui_component* sender)
{	
	if (self->machines) {
		machines_setvolume(self->machines, self->value * self->value * 4.f);
	}
}

void volslider_onsongchanged(VolSlider* self, Workspace* workspace)
{
	self->machines = &workspace->song->machines;
}

void volslider_ontimer(VolSlider* self, ui_component* sender, int timerid)
{		
	if (self->machines) {
		float oldvalue;

		oldvalue = self->value;
		self->value = (float)(sqrt(machines_volume(self->machines)) * 0.5f);
		if (oldvalue != self->value) {
			ui_invalidate(&self->component);
		}
	}
}
