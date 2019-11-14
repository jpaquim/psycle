// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "wireview.h"
#include "math.h"
#include "resources/resource.h"
#include <portable.h>

static void wireview_ondestroy(WireView*);
static void wireview_initvolumeslider(WireView*);
static void wireview_inittabbar(WireView* self);
static void wireview_initbottomgroup(WireView*);
static void wireview_ondraw(WireView*, ui_component* sender, ui_graphics*);
static void wireview_onsongchanged(WireView*, Workspace*);
static void wireview_connectmachinessignals(WireView*, Workspace*);
static void wireview_ondescribevolume(WireView*, ui_slider*, char* txt);
static void wireview_ontweakvolume(WireView*, ui_slider*, float value);
static void wireview_onvaluevolume(WireView*, ui_slider*, float* value);
static void wireview_ondeleteconnection(WireView*, ui_component* sender);
static void wireview_ondisconnected(WireView*, Connections*, uintptr_t outputslot, uintptr_t inputslot);

static void wireframe_ondestroy(WireFrame*, ui_component* frame);
static void wireframe_onsize(WireFrame*, ui_component* sender, ui_size* size);

void wireview_init(WireView* self, ui_component* parent, Wire wire,
	Workspace* workspace)
{					
	self->wire = wire;	
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	ui_component_enablealign(&self->component);
	wireview_initvolumeslider(self);
	wireview_inittabbar(self);
	wireview_initbottomgroup(self);	
	ui_notebook_init(&self->notebook, &self->component);	
	ui_component_setalign(&self->notebook.component, UI_ALIGN_CLIENT);	
	signal_connect(&self->component.signal_destroy, self, wireview_ondestroy);
	wireview_connectmachinessignals(self, workspace);
	vuscope_init(&self->vuscope, &self->notebook.component, wire, workspace);
	channelmappingview_init(&self->channelmappingview, &self->notebook.component,
		wire, workspace);
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	tabbar_select(&self->tabbar, 0);	
}

void wireview_inittabbar(WireView* self)
{
	tabbar_init(&self->tabbar, &self->component);
	ui_component_setalign(&self->tabbar.component, UI_ALIGN_TOP);
	tabbar_append(&self->tabbar, "Vu");
	tabbar_append(&self->tabbar, "Channel Mapping");
}

void wireview_initvolumeslider(WireView* self)
{
	ui_component_init(&self->slidergroup, &self->component);
	ui_component_setalign(&self->slidergroup, UI_ALIGN_RIGHT);
	ui_component_enablealign(&self->slidergroup);
	ui_component_resize(&self->slidergroup, 20, 0);		
	ui_button_init(&self->percvol, &self->slidergroup);
	ui_button_settext(&self->percvol, "100%");
	ui_button_setcharnumber(&self->percvol, 10);
	self->percvol.component.doublebuffered = 1;
	ui_component_setalign(&self->percvol.component, UI_ALIGN_BOTTOM);
	ui_button_init(&self->dbvol, &self->slidergroup);
	ui_button_settext(&self->dbvol, "db 100");
	ui_button_setcharnumber(&self->dbvol, 10);
	self->dbvol.component.doublebuffered = 1;
	ui_component_setalign(&self->dbvol.component, UI_ALIGN_BOTTOM);
	ui_slider_init(&self->volslider, &self->slidergroup);	
	ui_slider_setcharnumber(&self->volslider, 4);
	ui_slider_showvertical(&self->volslider);
	ui_component_resize(&self->volslider.component, 20, 0);
	ui_component_setalign(&self->volslider.component, UI_ALIGN_CLIENT);
	ui_slider_connect(&self->volslider, self, wireview_ondescribevolume,
		wireview_ontweakvolume, wireview_onvaluevolume);
}

void wireview_initbottomgroup(WireView* self)
{
	ui_component_init(&self->bottomgroup, &self->component);
	ui_component_setalign(&self->bottomgroup, UI_ALIGN_BOTTOM);
	ui_component_enablealign(&self->bottomgroup);
	ui_button_init(&self->deletewire, &self->bottomgroup);
	ui_button_settext(&self->deletewire, "Delete Connection");
	self->deletewire.component.doublebuffered = 1;
	signal_connect(&self->deletewire.signal_clicked, self,
		wireview_ondeleteconnection);
	ui_component_setalign(&self->deletewire.component, UI_ALIGN_LEFT);
}

void wireview_ondestroy(WireView* self)
{
	if (self->workspace && self->workspace->song) {
		signal_disconnect(&self->workspace->song->machines.connections.signal_disconnected, self,
			wireview_ondisconnected);		
	}
}

void wireview_onsongchanged(WireView* self, Workspace* workspace)
{		
	wireview_connectmachinessignals(self, workspace);
}

void wireview_ondescribevolume(WireView* self, ui_slider* slider, char* txt)
{
	Connections* connections;
	WireSocketEntry* input;	

	connections = &self->workspace->song->machines.connections;
	input = connection_input(connections, self->wire.src, self->wire.dst);
	if (input) {
		char text[128];
		psy_snprintf(text, 128, "%.1f dB",20.0f * log10(input->volume));
		ui_button_settext(&self->dbvol, text);
		psy_snprintf(text, 128, "%.2f %%", input->volume * 100.f);
		ui_button_settext(&self->percvol, text);
	}
}

void wireview_ontweakvolume(WireView* self, ui_slider* slider, float value)
{
	Connections* connections;
	WireSocketEntry* input;	

	connections = &self->workspace->song->machines.connections;
	input = connection_input(connections, self->wire.src, self->wire.dst);
	if (input) {		
		input->volume = (amp_t)(value * value * 4);			
	}
}

void wireview_onvaluevolume(WireView* self, ui_slider* slider, float* value)
{
	Connections* connections;
	WireSocketEntry* input;	

	connections = &self->workspace->song->machines.connections;
	input = connection_input(connections, self->wire.src, self->wire.dst);
	if (input) {		
		*value = (float)(sqrt(input->volume) * 0.5);
	}
}

void wireview_ondeleteconnection(WireView* self, ui_component* sender)
{
	if (self->workspace && self->workspace->song) {		
		machines_disconnect(&self->workspace->song->machines, self->wire.src,
			self->wire.dst);		
	}
}

void wireview_ondisconnected(WireView* self, Connections* connections,
		uintptr_t outputslot, uintptr_t inputslot)
{
	vuscope_stop(&self->vuscope);
}

int wireview_wireexists(WireView* self)
{
	return self->workspace && self->workspace->song 
		   ? machines_connected(&self->workspace->song->machines,
				self->wire.src, self->wire.dst)
		   : 0;
}

void wireview_connectmachinessignals(WireView* self, Workspace* workspace)
{	
	if (workspace->song) {
		signal_connect(
			&self->workspace->song->machines.connections.signal_disconnected,
			self, wireview_ondisconnected);
	}
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
