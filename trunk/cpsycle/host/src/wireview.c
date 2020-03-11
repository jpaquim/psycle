// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "wireview.h"
#include "math.h"
#include "resources/resource.h"
#include "../../detail/portable.h"
#include <uiframe.h>

static void wireview_ondestroy(WireView*);
static void wireview_initvolumeslider(WireView*);
static void wireview_inittabbar(WireView* self);
static void wireview_initbottomgroup(WireView*);
static void wireview_onsongchanged(WireView*, Workspace*);
static void wireview_connectmachinessignals(WireView*, Workspace*);
static void wireview_ondescribevolume(WireView*, psy_ui_Slider*, char* txt);
static void wireview_ontweakvolume(WireView*, psy_ui_Slider*, float value);
static void wireview_onvaluevolume(WireView*, psy_ui_Slider*, float* value);
static void wireview_ondeleteconnection(WireView*, psy_ui_Component* sender);
static void wireview_onaddeffect(WireView*, psy_ui_Component* sender);
static void wireview_ondisconnected(WireView*, psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);

static void wireframe_ondestroy(WireFrame*, psy_ui_Component* frame);
static void wireframe_onsize(WireFrame*, psy_ui_Component* sender, psy_ui_Size*);


void wireview_init(WireView* self, psy_ui_Component* parent, psy_audio_Wire wire,
	Workspace* workspace)
{					
	self->wire = wire;	
	self->workspace = workspace;
	psy_ui_component_init(wireview_base(self), parent);
	psy_ui_component_doublebuffer(wireview_base(self));
	psy_ui_component_enablealign(wireview_base(self));
	wireview_initvolumeslider(self);
	wireview_inittabbar(self);
	wireview_initbottomgroup(self);	
	psy_ui_notebook_init(&self->notebook, wireview_base(self));	
	psy_ui_component_setalign(&self->notebook.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&wireview_base(self)->signal_destroy, self,
		wireview_ondestroy);
	wireview_connectmachinessignals(self, workspace);
	vuscope_init(&self->vuscope, psy_ui_notebook_base(&self->notebook), wire,
		workspace);
	oscilloscope_init(&self->oscilloscope, psy_ui_notebook_base(&self->notebook), wire,
		workspace);
	spectrumanalyzer_init(&self->spectrumanalyzer, psy_ui_notebook_base(&self->notebook), wire,
		workspace);
	stereophase_init(&self->stereophase, psy_ui_notebook_base(&self->notebook), wire,
		workspace);
	channelmappingview_init(&self->channelmappingview,
		psy_ui_notebook_base(&self->notebook), wire, workspace);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	tabbar_select(&self->tabbar, 0);	
}

void wireview_ondestroy(WireView* self)
{
	if (self->workspace && self->workspace->song) {
		psy_signal_disconnect(
			&self->workspace->song->machines.connections.signal_disconnected,
			self, wireview_ondisconnected);		
	}
}

void wireview_inittabbar(WireView* self)
{
	tabbar_init(&self->tabbar, wireview_base(self));
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_TOP);
	tabbar_append_tabs(&self->tabbar, "Vu", "Oscilloscope", "Spectrum Analyzer", "Stereo Phase",
		"Channel Mapping", NULL);	
}

void wireview_initvolumeslider(WireView* self)
{
	psy_ui_component_init(&self->slidergroup, wireview_base(self));
	psy_ui_component_doublebuffer(wireview_base(self));
	psy_ui_component_setalign(&self->slidergroup, psy_ui_ALIGN_RIGHT);
	psy_ui_component_enablealign(&self->slidergroup);
	psy_ui_component_resize(&self->slidergroup, 20, 0);
	psy_ui_button_init(&self->percvol, &self->slidergroup);
	psy_ui_button_settext(&self->percvol, "100%");
	psy_ui_button_setcharnumber(&self->percvol, 10);	
	psy_ui_component_setalign(&self->percvol.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_button_init(&self->dbvol, &self->slidergroup);
	psy_ui_component_doublebuffer(&self->dbvol.component);
	psy_ui_button_settext(&self->dbvol, "db 100");
	psy_ui_button_setcharnumber(&self->dbvol, 10);	
	psy_ui_component_setalign(&self->dbvol.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_slider_init(&self->volslider, &self->slidergroup);
	psy_ui_slider_setcharnumber(&self->volslider, 4);
	psy_ui_slider_showvertical(&self->volslider);
	psy_ui_component_resize(&self->volslider.component, 20, 0);
	psy_ui_component_setalign(&self->volslider.component, psy_ui_ALIGN_CLIENT);
	psy_ui_slider_connect(&self->volslider, self, wireview_ondescribevolume,
		wireview_ontweakvolume, wireview_onvaluevolume);
}

void wireview_initbottomgroup(WireView* self)
{
	psy_ui_component_init(&self->bottomgroup, wireview_base(self));
	psy_ui_component_setalign(&self->bottomgroup, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_enablealign(&self->bottomgroup);
	psy_ui_button_init(&self->deletewire, &self->bottomgroup);
	psy_ui_button_settext(&self->deletewire, "Delete Connection");
	psy_ui_component_setalign(&self->deletewire.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->deletewire.signal_clicked, self,
		wireview_ondeleteconnection);
	psy_ui_button_init(&self->addeffect, &self->bottomgroup);
	psy_ui_button_settext(&self->addeffect, "Add Effect");
	psy_ui_component_setalign(&self->addeffect.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->addeffect.signal_clicked, self,
		wireview_onaddeffect);
}

void wireview_onsongchanged(WireView* self, Workspace* workspace)
{		
	wireview_connectmachinessignals(self, workspace);
}

void wireview_ondescribevolume(WireView* self, psy_ui_Slider* slider, char* txt)
{
	psy_audio_Connections* connections;
	psy_audio_WireSocketEntry* input;	

	connections = &self->workspace->song->machines.connections;
	input = connection_input(connections, self->wire.src, self->wire.dst);
	if (input) {
		char text[128];
		psy_snprintf(text, 128, "%.1f dB",20.0f * log10(input->volume));
		psy_ui_button_settext(&self->dbvol, text);
		psy_snprintf(text, 128, "%.2f %%", input->volume * 100.f);
		psy_ui_button_settext(&self->percvol, text);
	}
}

void wireview_ontweakvolume(WireView* self, psy_ui_Slider* slider, float value)
{
	psy_audio_Connections* connections;
	psy_audio_WireSocketEntry* input;	

	connections = &self->workspace->song->machines.connections;
	input = connection_input(connections, self->wire.src, self->wire.dst);
	if (input) {		
		input->volume = (psy_dsp_amp_t)(value * value * 4);			
	}
}

void wireview_onvaluevolume(WireView* self, psy_ui_Slider* slider, float* value)
{
	psy_audio_Connections* connections;
	psy_audio_WireSocketEntry* input;	

	connections = &self->workspace->song->machines.connections;
	input = connection_input(connections, self->wire.src, self->wire.dst);
	if (input) {		
		*value = (float)(sqrt(input->volume) * 0.5);
	}
}

void wireview_ondeleteconnection(WireView* self, psy_ui_Component* sender)
{
	if (self->workspace && self->workspace->song) {		
		machines_disconnect(&self->workspace->song->machines, self->wire.src,
			self->wire.dst);		
	}
}

void wireview_onaddeffect(WireView* self, psy_ui_Component* sender)
{
	if (self->workspace && self->workspace->song) {
		workspace_selectview(self->workspace, TABPAGE_MACHINEVIEW, 1, 20);
	}
}

void wireview_ondisconnected(WireView* self, psy_audio_Connections* connections,
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
		psy_signal_connect(
			&self->workspace->song->machines.connections.signal_disconnected,
			self, wireview_ondisconnected);
	}
}

psy_ui_Component* wireview_base(WireView* self)
{
	return &self->component;
}

void wireframe_init(WireFrame* self, psy_ui_Component* parent, WireView* view)
{	
	psy_ui_frame_init(wireframe_base(self), parent);
	self->wireview = view;
	psy_ui_component_seticonressource(wireframe_base(self), IDI_MACPARAM);
	psy_ui_component_move(wireframe_base(self), 200, 150);
	psy_ui_component_resize(wireframe_base(self), 400, 400);
	psy_signal_connect(&wireframe_base(self)->signal_destroy, self,
		wireframe_ondestroy);
	psy_signal_connect(&wireframe_base(self)->signal_size, self,
		wireframe_onsize);
}

void wireframe_ondestroy(WireFrame* self, psy_ui_Component* frame)
{		
}

void wireframe_onsize(WireFrame* self, psy_ui_Component* sender, psy_ui_Size* size)
{
	if (self->wireview) {
		psy_ui_component_resize(wireview_base(self->wireview), size->width,
			size->height);
	}
}

psy_ui_Component* wireframe_base(WireFrame* self)
{
	return &self->component;
}
