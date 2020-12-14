// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "wireview.h"
#include "resources/resource.h"
#include <uiframe.h>

#include <math.h>

#include "../../detail/portable.h"

static void wireview_ondestroy(WireView*);
static void wireview_initvolumeslider(WireView*);
static void wireview_inittabbar(WireView*);
static void wireview_initrategroup(WireView*);
static void wireview_initbottomgroup(WireView*);
static void wireview_updatetext(WireView*, psy_Translator*);
static void wireview_onsongchanged(WireView*, Workspace*);
static void wireview_connectmachinessignals(WireView*, Workspace*);
static void wireview_ondescribevolume(WireView*, psy_ui_Slider*, char* txt);
static void wireview_ontweakvolume(WireView*, psy_ui_Slider*, float value);
static void wireview_onvaluevolume(WireView*, psy_ui_Slider*, float* value);
static void wireview_ontweakmode(WireView*, psy_ui_Slider*, float value);
static void wireview_onvaluemode(WireView*, psy_ui_Slider*, float* value);
static void wireview_ontweakrate(WireView*, psy_ui_Slider*, float value);
static void wireview_onvaluerate(WireView*, psy_ui_Slider*, float* value);
static void wireview_onhold(WireView*, psy_ui_Component* sender);
static void wireview_ondeleteconnection(WireView*, psy_ui_Component* sender);
static void wireview_onaddeffect(WireView*, psy_ui_Component* sender);
static void wireview_ondisconnected(WireView*, psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void wireview_ontabbarchanged(WireView*, psy_ui_Component* sender,
	uintptr_t tabindex);
static void wireview_dockscope(WireView*, int index);
static void wireview_movescope(WireView* self, int index,
	psy_ui_AlignType align, int width);
static psy_ui_Component* wireview_scope(WireView*, int index);
static bool wireview_scopedocked(WireView* self, int index);
static void wireview_ontogglevu(WireView*, psy_ui_Component* sender);
static int wireview_currscope(WireView*);

static void wireframe_ondestroy(WireFrame*, psy_ui_Component* frame);
static void wireframe_onsize(WireFrame*, psy_ui_Component* sender, psy_ui_Size*);

enum {
	TABPAGE_VUMETER = 0,
	TABPAGE_OSCILLOSCOPE = 1,
	TABPAGE_SPECTRUM = 2,
	TABPAGE_PHASE = 3,	
};

void wireview_init(WireView* self, psy_ui_Component* parent, psy_audio_Wire wire,
	Workspace* workspace)
{					
	self->wire = wire;	
	self->workspace = workspace;
	self->scope_spec_mode = 0.2f;
	self->scope_spec_rate = 0.f;
	psy_ui_component_init(wireview_base(self), parent);
	psy_ui_component_doublebuffer(wireview_base(self));
	wireview_initvolumeslider(self);
	wireview_initbottomgroup(self);
	wireview_initrategroup(self);
	wireview_inittabbar(self);	
	psy_ui_notebook_init(&self->notebook, wireview_base(self));	
	psy_ui_component_setalign(&self->notebook.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&wireview_base(self)->signal_destroy, self,
		wireview_ondestroy);
	wireview_connectmachinessignals(self, workspace);	
	vuscope_init(&self->vuscope, psy_ui_notebook_base(&self->notebook), wire,
		workspace);
	psy_ui_component_setalign(&self->vuscope.component, psy_ui_ALIGN_CLIENT);
	oscilloscopeview_init(&self->oscilloscopeview,
		psy_ui_notebook_base(&self->notebook), wire, workspace);
	oscilloscope_setzoom(&self->oscilloscopeview.oscilloscope, 0.2f);
	psy_ui_component_setalign(&self->oscilloscopeview.component, psy_ui_ALIGN_CLIENT);
	spectrumanalyzer_init(&self->spectrumanalyzer,
		psy_ui_notebook_base(&self->notebook), wire,
		workspace);
	psy_ui_component_setalign(&self->spectrumanalyzer.component, psy_ui_ALIGN_CLIENT);
	stereophase_init(&self->stereophase, psy_ui_notebook_base(&self->notebook), wire,
		workspace);
	psy_ui_component_setalign(&self->stereophase.component, psy_ui_ALIGN_CLIENT);
	channelmappingview_init(&self->channelmappingview,
		psy_ui_notebook_base(&self->notebook), wire, workspace);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	psy_signal_connect(&self->tabbar.signal_change, self,
		wireview_ontabbarchanged);
	tabbar_select(&self->tabbar, TABPAGE_VUMETER);	
}

void wireview_ondestroy(WireView* self)
{
	if (self->workspace && workspace_song(self->workspace)) {
		psy_signal_disconnect(
			&workspace_song(self->workspace)->machines.connections.signal_disconnected,
			self, wireview_ondisconnected);		
	}
}

void wireview_inittabbar(WireView* self)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->top, &self->component);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	psy_ui_component_setalignexpand(&self->top, psy_ui_HORIZONTALEXPAND);
	psy_ui_button_init_connect(&self->togglevu, &self->top,
		self, wireview_ontogglevu);
	psy_ui_button_seticon(&self->togglevu, psy_ui_ICON_MORE);
	psy_ui_component_setalign(&self->togglevu.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->vulabel, &self->top);
	psy_ui_label_settext(&self->vulabel, "Vu");
	psy_ui_label_settextalignment(&self->vulabel, psy_ui_ALIGNMENT_LEFT);
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(1), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->vulabel.component, &margin);
	psy_ui_component_setalign(&self->vulabel.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(&self->vulabel.component);
	tabbar_init(&self->tabbar, &self->top);	
	tabbar_append_tabs(&self->tabbar, "Vu", "Osc", "Spectrum", "Stereo Phase",
		"Channel Mapping", NULL);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);	
}

void wireview_initvolumeslider(WireView* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makeew(2));
	psy_ui_component_init(&self->slidergroup, wireview_base(self));
	psy_ui_component_doublebuffer(wireview_base(self));
	psy_ui_component_setalign(&self->slidergroup, psy_ui_ALIGN_RIGHT);
	psy_ui_component_setmargin(&self->slidergroup, &margin);
	//psy_ui_component_resize(&self->slidergroup, psy_ui_value_makeew(2),
		//psy_ui_value_makepx(0));
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
	psy_ui_component_resize(&self->volslider.component,
		psy_ui_size_make(psy_ui_value_makeew(2),
		psy_ui_value_makepx(0)));
	psy_ui_component_setalign(&self->volslider.component, psy_ui_ALIGN_CLIENT);	
	psy_ui_slider_connect(&self->volslider, self,
		(ui_slider_fpdescribe)wireview_ondescribevolume,
		(ui_slider_fptweak)wireview_ontweakvolume,
		(ui_slider_fpvalue)wireview_onvaluevolume);
}

void wireview_initrategroup(WireView* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->rategroup, wireview_base(self));
	psy_ui_component_setalign(&self->rategroup, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setmargin(&self->rategroup, &margin);
	psy_ui_button_init_connect(&self->hold, &self->rategroup,
		self, wireview_onhold);
	psy_ui_button_settext(&self->hold, "Hold");
	psy_ui_component_setalign(&self->hold.component, psy_ui_ALIGN_RIGHT);
	psy_ui_slider_init(&self->modeslider, &self->rategroup);
	psy_ui_slider_showhorizontal(&self->modeslider);
	psy_ui_slider_hidevaluelabel(&self->modeslider);
	psy_ui_component_setmargin(&self->modeslider.component, &margin);
	psy_ui_component_setalign(&self->modeslider.component, psy_ui_ALIGN_TOP);
	psy_ui_slider_connect(&self->modeslider, self, NULL,
		wireview_ontweakmode, wireview_onvaluemode);
	psy_ui_slider_init(&self->rateslider, &self->rategroup);
	psy_ui_slider_showhorizontal(&self->rateslider);	
	psy_ui_slider_showhorizontal(&self->rateslider);
	psy_ui_slider_hidevaluelabel(&self->rateslider);
	psy_ui_component_setalign(&self->rateslider.component, psy_ui_ALIGN_TOP);
	psy_ui_slider_connect(&self->rateslider, self, NULL,
		wireview_ontweakrate, wireview_onvaluerate);
}

void wireview_initbottomgroup(WireView* self)
{
	psy_ui_component_init(&self->bottomgroup, wireview_base(self));
	psy_ui_component_setalign(&self->bottomgroup, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setdefaultalign(&self->bottomgroup, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init_connect(&self->deletewire, &self->bottomgroup,
		self, wireview_ondeleteconnection);
	psy_ui_button_settext(&self->deletewire, "Delete Connection");
	psy_ui_button_init_connect(&self->addeffect, &self->bottomgroup,
		self, wireview_onaddeffect);	
	psy_ui_button_settext(&self->addeffect, "Add Effect");
}

void wireview_onsongchanged(WireView* self, Workspace* workspace)
{		
	wireview_connectmachinessignals(self, workspace);
}

void wireview_ondescribevolume(WireView* self, psy_ui_Slider* slider, char* txt)
{
	psy_audio_Connections* connections;
	psy_audio_WireSocket* input;	

	connections = &workspace_song(self->workspace)->machines.connections;
	input = psy_audio_connections_input(connections, self->wire);
	if (input) {
		char text[128];

		psy_snprintf(text, 128, "%.1f dB",20.0f * log10(input->volume));
		psy_ui_button_settext(&self->dbvol, text);
		psy_snprintf(text, 128, "%.2f %%", (float)(input->volume * 100.0));
		psy_ui_button_settext(&self->percvol, text);
	}
}

void wireview_ontweakvolume(WireView* self, psy_ui_Slider* slider, float value)
{
	psy_audio_Connections* connections;
	psy_audio_WireSocket* input;	

	connections = &workspace_song(self->workspace)->machines.connections;
	input = psy_audio_connections_input(connections, self->wire);
	if (input) {		
		input->volume = (psy_dsp_amp_t)(value * value * 4);			
	}
}

void wireview_onvaluevolume(WireView* self, psy_ui_Slider* slider, float* value)
{
	psy_audio_Connections* connections;
	psy_audio_WireSocket* input;	

	connections = &workspace_song(self->workspace)->machines.connections;
	input = psy_audio_connections_input(connections, self->wire);
	if (input) {		
		*value = (float)(sqrt(input->volume) * 0.5);
	}
}

void wireview_ontweakmode(WireView* self, psy_ui_Slider* slider, float value)
{
	self->scope_spec_mode = value;
	oscilloscope_setzoom(&self->oscilloscopeview.oscilloscope, value);
}

void wireview_onvaluemode(WireView* self, psy_ui_Slider* slider, float* value)
{
	*value = self->scope_spec_mode;
}

void wireview_ontweakrate(WireView* self, psy_ui_Slider* slider, float value)
{
	self->scope_spec_rate = value;
	oscilloscope_setbegin(&self->oscilloscopeview.oscilloscope, value);
}

void wireview_onvaluerate(WireView* self, psy_ui_Slider* slider, float* value)
{
	*value = self->scope_spec_rate;
}

void wireview_onhold(WireView* self, psy_ui_Component* sender)
{	
	switch (tabbar_selected(&self->tabbar)) {
		case TABPAGE_VUMETER:
		break;
		case TABPAGE_OSCILLOSCOPE:
			if (oscilloscope_stopped(&self->oscilloscopeview.oscilloscope)) {
				oscilloscope_continue(&self->oscilloscopeview.oscilloscope);
			} else {
				self->scope_spec_rate = 0.0;
				oscilloscope_setbegin(&self->oscilloscopeview.oscilloscope,
					self->scope_spec_rate);
				oscilloscope_hold(&self->oscilloscopeview.oscilloscope);
			}
		break;
		case TABPAGE_SPECTRUM:
			if (spectrumanalyzer_stopped(&self->spectrumanalyzer)) {
				spectrumanalyzer_continue(&self->spectrumanalyzer);
			} else {								
				spectrumanalyzer_hold(&self->spectrumanalyzer);
			}
		break;
		case TABPAGE_PHASE:
		break;
		default:
		break;
	}
	if (psy_ui_button_highlighted(&self->hold)) {
		psy_ui_button_disablehighlight(&self->hold);
	} else {
		psy_ui_button_highlight(&self->hold);
	}
}

void wireview_ondeleteconnection(WireView* self, psy_ui_Component* sender)
{
	if (self->workspace && workspace_song(self->workspace)) {		
		psy_audio_machines_disconnect(&workspace_song(self->workspace)->machines,
			self->wire);
	}
}

void wireview_onaddeffect(WireView* self, psy_ui_Component* sender)
{
	if (self->workspace && workspace_song(self->workspace)) {
		workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW, 1, 20);
	}
}

void wireview_ondisconnected(WireView* self, psy_audio_Connections* connections,
		uintptr_t outputslot, uintptr_t inputslot)
{
	vuscope_disconnect(&self->vuscope);
}

int wireview_wireexists(WireView* self)
{
	return (self->workspace && workspace_song(self->workspace))
		   ? psy_audio_machines_connected(&workspace_song(self->workspace)->machines,
				self->wire)
		   : 0;
}

void wireview_connectmachinessignals(WireView* self, Workspace* workspace)
{	
	if (workspace->song) {
		psy_signal_connect(
			&workspace_song(self->workspace)->machines.connections.signal_disconnected,
			self, wireview_ondisconnected);
	}
}

void wireview_ontabbarchanged(WireView* self, psy_ui_Component* sender,
	uintptr_t tabindex)
{		
	/*if (wireview_currscope(self) == TABPAGE_VUMETER) {
		vuscope_start(&self->vuscope);
		psy_ui_component_hide(&self->modeslider.component);
	} else {
		vuscope_stop(&self->vuscope);
		psy_ui_component_show(&self->modeslider.component);
	}*/
	//psy_ui_component_align(&self->component);

}

void wireview_dockscope(WireView* self, int index)
{
	psy_ui_Component* scope;

	scope = wireview_scope(self, index);
	if (scope == NULL) {
		return;
	}	
	if (psy_ui_component_parent(scope) != psy_ui_notebook_base(&self->notebook)) {
		psy_ui_component_setparent(scope, psy_ui_notebook_base(&self->notebook));
		psy_ui_component_setalign(scope, psy_ui_ALIGN_CLIENT);
	}
}

bool wireview_scopedocked(WireView* self, int index)
{
	psy_ui_Component* scope;

	scope = wireview_scope(self, index);
	if (scope == NULL) {
		return FALSE;
	}	
	return (psy_ui_component_parent(scope) == psy_ui_notebook_base(&self->notebook));
}

void wireview_movescope(WireView* self, int index, psy_ui_AlignType align, int width)
{
	psy_ui_Component* scope;

	scope = wireview_scope(self, index);
	if (scope == NULL) {
		return;
	}	
	if (psy_ui_component_parent(scope) == psy_ui_notebook_base(&self->notebook)) {
		psy_ui_component_setparent(scope, &self->component);		
		psy_ui_component_resize(scope,
			psy_ui_size_make(psy_ui_value_makepx(width),
			psy_ui_value_makepx(0)));
		psy_ui_component_show(scope);
	}
	psy_ui_component_setalign(scope, align);
}

psy_ui_Component* wireview_scope(WireView* self, int index)
{
	psy_ui_Component* rv;

	switch (index) {
		case TABPAGE_VUMETER:
			rv = &self->vuscope.component;
		break;
		case TABPAGE_OSCILLOSCOPE:
			rv = &self->oscilloscopeview.component;
		break;
		case TABPAGE_SPECTRUM:
			rv = &self->spectrumanalyzer.component;
		break;
		case TABPAGE_PHASE:
			rv = &self->stereophase.component;
		break;
		default:
			rv = 0;
		break;
	}
	return rv;
}

void wireview_ontogglevu(WireView* self, psy_ui_Component* sender)
{
	int oldtabindex;

	oldtabindex = tabbar_selected(&self->tabbar);
	tabbar_clear(&self->tabbar);
	if (wireview_scopedocked(self, TABPAGE_VUMETER)) {
		wireview_movescope(self, TABPAGE_VUMETER, psy_ui_ALIGN_RIGHT, 150);
		psy_ui_component_show(&self->vulabel.component);
		tabbar_append_tabs(&self->tabbar, "Osc", "Spectrum", "Stereo Phase",
			"Channel Mapping", NULL);
		psy_ui_button_seticon(&self->togglevu, psy_ui_ICON_LESS);
		tabbar_select(&self->tabbar, (oldtabindex - 1 > 0) ? oldtabindex - 1 : 0);
	} else {
		wireview_dockscope(self, TABPAGE_VUMETER);
		tabbar_append_tabs(&self->tabbar, "Vu", "Osc", "Spectrum", "Stereo Phase",
			"Channel Mapping", NULL);
		psy_ui_component_hide(&self->vulabel.component);
		psy_ui_button_seticon(&self->togglevu, psy_ui_ICON_MORE);
		tabbar_select(&self->tabbar, oldtabindex + 1);
	}
	psy_ui_component_align(wireview_base(self));
}

int wireview_currscope(WireView* self)
{	
	return tabbar_selected(&self->tabbar) +
		(wireview_scopedocked(self, 0))
		? 1
		: 0;
}

void wireframe_init(WireFrame* self, psy_ui_Component* parent, WireView* view)
{	
	psy_ui_frame_init(wireframe_base(self), parent);
	self->wireview = view;
	psy_ui_component_seticonressource(wireframe_base(self), IDI_MACPARAM);
	psy_ui_component_move(wireframe_base(self),
		psy_ui_point_make(
			psy_ui_value_makepx(200),
			psy_ui_value_makepx(150)));
	psy_ui_component_resize(wireframe_base(self),
		psy_ui_size_make(
		psy_ui_value_makeew(80), psy_ui_value_makeeh(25)));
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
		psy_ui_component_resize(wireview_base(self->wireview), *size);
	}
}
