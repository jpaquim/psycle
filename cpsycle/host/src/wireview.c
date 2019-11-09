// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "wireview.h"
#include "math.h"
#include "resources/resource.h"

static void wireview_ondraw(WireView*, ui_component* sender, ui_graphics*);
static void wireview_onsongchanged(WireView*, Workspace*);
static void wireview_connectmachinessignals(WireView*, Workspace*);
static void wireview_onsliderchanged(WireView*, ui_component* sender);
static void wireview_ontimer(WireView*, ui_component* sender, int timerid);

static void wireframe_ondestroy(WireFrame*, ui_component* frame);
static void wireframe_onsize(WireFrame*, ui_component* sender, ui_size* size);

void wireview_init(WireView* self, ui_component* parent, uintptr_t src,
	uintptr_t dst, Workspace* workspace)
{					
	self->src = src;
	self->dst = dst;
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	ui_component_enablealign(&self->component);
	ui_slider_init(&self->volslider, &self->component);
	ui_component_resize(&self->volslider.component, 20, 0);
	ui_component_setalign(&self->volslider.component, UI_ALIGN_RIGHT);
	signal_connect(&self->component.signal_timer, self, wireview_ontimer);
	ui_slider_showvertical(&self->volslider);
	signal_connect(&self->volslider.signal_changed, self, wireview_onsliderchanged);
	tabbar_init(&self->tabbar, &self->component);
	ui_component_setalign(&self->tabbar.component, UI_ALIGN_TOP);
	tabbar_append(&self->tabbar, "Channel Mapping");
	ui_notebook_init(&self->notebook, &self->component);	
	ui_component_setalign(&self->notebook.component, UI_ALIGN_CLIENT);	
	signal_connect(&self->component.signal_draw, self, wireview_ondraw);	
	wireview_connectmachinessignals(self, workspace);
	channelmappingview_init(&self->channelmappingview, &self->notebook.component,
		src, dst, workspace);
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	tabbar_select(&self->tabbar, 0);
	ui_component_starttimer(&self->component, 800, 50);
}

void wireview_ondraw(WireView* self, ui_component* sender, ui_graphics* g)
{	
	ui_rectangle r;	
	ui_size size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);	
	ui_drawsolidrectangle(g, r, 0x003E3E3E);
}

void wireview_onsongchanged(WireView* self, Workspace* workspace)
{		
	wireview_connectmachinessignals(self, workspace);
}

void wireview_onsliderchanged(WireView* self, ui_component* sender)
{
	Connections* connections;
	WireSocketEntry* input;

	connections = &self->workspace->song->machines.connections;
	input = connection_input(connections, self->src, self->dst);
	if (input) {
		input->volume = self->volslider.value * self->volslider.value * 4;
	}
}

void wireview_ontimer(WireView* self, ui_component* sender, int timerid)
{
	Connections* connections;
	WireSocketEntry* input;	

	connections = &self->workspace->song->machines.connections;
	input = connection_input(connections, self->src, self->dst);
	if (input) {
		float oldvalue;

		oldvalue = self->volslider.value;
		self->volslider.value = sqrt(input->volume) * 0.5;
		if (oldvalue != self->volslider.value) {
			ui_invalidate(&self->volslider.component);
		}
	}
}

void wireview_connectmachinessignals(WireView* self, Workspace* workspace)
{	
}

void wireframe_init(WireFrame* self, ui_component* parent, WireView* view)
{
	self->wireview = view;
	ui_frame_init(&self->component, parent);
	ui_component_seticonressource(&self->component, IDI_MACPARAM);
	ui_component_move(&self->component, 200, 150);
	ui_component_resize(&self->component, 400, 400);
	signal_connect(&self->component.signal_destroy, self, wireframe_ondestroy);
	signal_connect(&self->component.signal_size, self, wireframe_onsize);
}

void wireframe_ondestroy(WireFrame* self, ui_component* frame)
{
	self->component.hwnd = 0;
}

void wireframe_onsize(WireFrame* self, ui_component* sender, ui_size* size)
{
	if (self->wireview) {
		ui_component_resize(&self->wireview->component, size->width, size->height);
	}
}
