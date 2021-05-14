/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "wireview.h"
/* local */
#include "resources/resource.h"
/* audio */
#include <operations.h>
/* ui */
#include <uiframe.h>
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"

#define SCOPE_SPEC_BANDS 256

static const int SCOPE_BARS_WIDTH = 256 / SCOPE_SPEC_BANDS;
static const uint32_t CLBARDC = 0x001010DC;
static const uint32_t CLBARPEAK = 0x00C0C0C0;
static const uint32_t CLLEFT = 0x00C06060;
static const uint32_t CLRIGHT = 0x0060C060;
static const uint32_t CLBOTH = 0x00C0C060;
static const uint32_t linepenbL = 0x00705050;
static const uint32_t linepenbR = 0x00507050;
static const uint32_t linepenL = 0x00c08080;
static const uint32_t linepenR = 0x0080c080;

static psy_dsp_amp_t dB(psy_dsp_amp_t amplitude);

// WireView
enum {
	WIREVIEW_TAB_VUMETER = 0,
	WIREVIEW_TAB_OSCILLOSCOPE,
	WIREVIEW_TAB_SPECTRUM,
	WIREVIEW_TAB_PHASE
};
// prototypes
static void wireview_initvolumeslider(WireView*);
static void wireview_inittabbar(WireView*);
static void wireview_initrategroup(WireView*);
static void wireview_initbottomgroup(WireView*);
static void wireview_updatetext(WireView*, psy_Translator*);
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
static psy_ui_Component* wireview_scope(WireView*, int index);
static uintptr_t wireview_currscope(WireView*);
static void wireview_ondrawslidervu(WireView*, psy_ui_Component* sender, psy_ui_Graphics*);
static void wireview_ontimer(WireView*, uintptr_t timerid);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(WireView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			wireview_ontimer;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}
/* implementation */
void wireview_init(WireView* self, psy_ui_Component* parent, psy_audio_Wire wire,
	Workspace* workspace)
{						
	psy_ui_component_init(wireview_base(self), parent, NULL);
	vtable_init(self);
	self->wire = wire;
	self->workspace = workspace;
	self->scope_spec_mode = 0.2f;
	self->scope_spec_rate = 0.f;
	psy_ui_component_doublebuffer(wireview_base(self));
	psy_ui_component_setalign(wireview_base(self),
		psy_ui_ALIGN_CLIENT);	
	wireview_initvolumeslider(self);
	wireview_initbottomgroup(self);
	wireview_initrategroup(self);
	wireview_inittabbar(self);	
	psy_ui_notebook_init(&self->notebook, wireview_base(self));	
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	/* Vuscope */
	vuscope_init(&self->vuscope, psy_ui_notebook_base(&self->notebook), wire,
		workspace);	
	/* Oscilloscope */
	oscilloscopeview_init(&self->oscilloscopeview,
		psy_ui_notebook_base(&self->notebook), wire, workspace);
	oscilloscope_setzoom(&self->oscilloscopeview.oscilloscope, 0.2f);	
	/* Spectrum */
	psy_ui_component_init(&self->spectrumpane,
		psy_ui_notebook_base(&self->notebook), NULL);
	spectrumanalyzer_init(&self->spectrumanalyzer, &self->spectrumpane,
		wire, workspace);
	psy_ui_component_setalign(&self->spectrumanalyzer.component,
		psy_ui_ALIGN_CENTER);
	/* Stereophase */
	psy_ui_component_init(&self->stereophasepane,
		psy_ui_notebook_base(&self->notebook), NULL);	
	stereophase_init(&self->stereophase, &self->stereophasepane, wire,
		workspace);
	psy_ui_component_setalign(&self->stereophase.component,
		psy_ui_ALIGN_CENTER);
	/* Channel Mapping */
	channelmappingview_init(&self->channelmappingview,
		psy_ui_notebook_base(&self->notebook), wire, workspace);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	psy_ui_tabbar_select(&self->tabbar, WIREVIEW_TAB_VUMETER);
	psy_signal_connect(&self->component.signal_timer, self, wireview_ontimer);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void wireview_inittabbar(WireView* self)
{
	psy_ui_component_init(&self->top, &self->component, NULL);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	psy_ui_component_setalignexpand(&self->top, psy_ui_HEXPAND);	
	psy_ui_tabbar_init(&self->tabbar, &self->top);
	psy_ui_tabbar_append_tabs(&self->tabbar, "Vu", "Osc", "Spectrum", "Stereo Phase",
		"Channel Mapping", NULL);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);	
}

void wireview_initvolumeslider(WireView* self)
{
	psy_ui_component_init(&self->slidergroup, wireview_base(self), NULL);
	psy_ui_component_doublebuffer(wireview_base(self));
	psy_ui_component_setalign(&self->slidergroup, psy_ui_ALIGN_RIGHT);
	psy_ui_component_setmargin(&self->slidergroup,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 2.0));
	//psy_ui_component_resize(&self->slidergroup, psy_ui_value_make_ew(2),
		//psy_ui_value_make_px(0));
	psy_ui_button_init(&self->percvol, &self->slidergroup, NULL);
	psy_ui_button_settext(&self->percvol, "100%");
	psy_ui_button_preventtranslation(&self->percvol);
	psy_ui_button_setcharnumber(&self->percvol, 10);	
	psy_ui_component_setalign(&self->percvol.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_button_init(&self->dbvol, &self->slidergroup, NULL);
	psy_ui_component_doublebuffer(&self->dbvol.component);
	psy_ui_button_settext(&self->dbvol, "db 100");
	psy_ui_button_preventtranslation(&self->dbvol);
	psy_ui_button_setcharnumber(&self->dbvol, 10);	
	psy_ui_component_setalign(&self->dbvol.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_slider_init(&self->volslider, &self->slidergroup, NULL);
	psy_ui_slider_setcharnumber(&self->volslider, 4);
	psy_ui_slider_showvertical(&self->volslider);
	psy_ui_component_resize(&self->volslider.component,
		psy_ui_size_make_em(2.0, 0.0));
	psy_ui_component_setalign(&self->volslider.component, psy_ui_ALIGN_CLIENT);	
	psy_ui_slider_connect(&self->volslider, self,
		(ui_slider_fpdescribe)wireview_ondescribevolume,
		(ui_slider_fptweak)wireview_ontweakvolume,
		(ui_slider_fpvalue)wireview_onvaluevolume);
	psy_signal_connect(&self->volslider.pane.signal_customdraw,
		self, wireview_ondrawslidervu);
	psy_ui_slider_startpoll(&self->volslider);
}

void wireview_initrategroup(WireView* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_em(&margin, 0.5, 0.0, 0.5, 0.0);
	psy_ui_component_init(&self->rategroup, wireview_base(self), NULL);
	psy_ui_component_setalign(&self->rategroup, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setmargin(&self->rategroup, margin);
	psy_ui_button_init_connect(&self->hold, &self->rategroup, NULL,
		self, wireview_onhold);
	psy_ui_button_settext(&self->hold, "Hold");
	psy_ui_component_setalign(&self->hold.component, psy_ui_ALIGN_RIGHT);
	psy_ui_slider_init(&self->modeslider, &self->rategroup, NULL);
	psy_ui_slider_setdefaultvalue(&self->modeslider, 0.2);
	psy_ui_slider_showhorizontal(&self->modeslider);
	psy_ui_slider_hidevaluelabel(&self->modeslider);
	psy_ui_component_setmargin(&self->modeslider.component, margin);
	psy_ui_component_setalign(&self->modeslider.component, psy_ui_ALIGN_TOP);
	psy_ui_slider_connect(&self->modeslider, self,
		(ui_slider_fpdescribe)NULL,
		(ui_slider_fptweak)wireview_ontweakmode,
		(ui_slider_fpvalue)wireview_onvaluemode);
	psy_ui_slider_init(&self->rateslider, &self->rategroup, NULL);
	psy_ui_slider_setdefaultvalue(&self->rateslider, 0.0);
	psy_ui_slider_showhorizontal(&self->rateslider);	
	psy_ui_slider_showhorizontal(&self->rateslider);
	psy_ui_slider_hidevaluelabel(&self->rateslider);
	psy_ui_component_setalign(&self->rateslider.component, psy_ui_ALIGN_TOP);
	psy_ui_slider_connect(&self->rateslider, self,
		(ui_slider_fpdescribe)NULL,		
		(ui_slider_fptweak)wireview_ontweakrate,
		(ui_slider_fpvalue)wireview_onvaluerate);
	psy_ui_slider_update(&self->rateslider);
	psy_ui_slider_update(&self->modeslider);
}

void wireview_initbottomgroup(WireView* self)
{
	psy_ui_component_init(&self->bottomgroup, wireview_base(self), NULL);
	psy_ui_component_setalign(&self->bottomgroup, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setdefaultalign(&self->bottomgroup, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init_connect(&self->deletewire, &self->bottomgroup, NULL,
		self, wireview_ondeleteconnection);
	psy_ui_button_settext(&self->deletewire, "Delete Connection");
	psy_ui_button_init_connect(&self->addeffect, &self->bottomgroup, NULL,
		self, wireview_onaddeffect);	
	psy_ui_button_settext(&self->addeffect, "Add Effect");
}

void wireview_ondescribevolume(WireView* self, psy_ui_Slider* slider, char* txt)
{
	psy_audio_Connections* connections;
	char text[128];
	psy_dsp_amp_t volume;

	connections = &workspace_song(self->workspace)->machines.connections;
	volume = psy_audio_connections_wirevolume(connections, self->wire);	
	psy_snprintf(text, 128, "%.1f dB",20.0f * log10(volume));
	psy_ui_button_settext(&self->dbvol, text);
	psy_snprintf(text, 128, "%.2f %%", (float)(volume * 100.0));
	psy_ui_button_settext(&self->percvol, text);	
}

void wireview_ontweakvolume(WireView* self, psy_ui_Slider* slider, float value)
{
	psy_audio_Connections* connections;	

	connections = &workspace_song(self->workspace)->machines.connections;	
	psy_audio_connections_setwirevolume(connections, self->wire,
		(psy_dsp_amp_t)(value * value * 4));
}

void wireview_onvaluevolume(WireView* self, psy_ui_Slider* slider, float* value)
{
	psy_audio_Connections* connections;	

	connections = &workspace_song(self->workspace)->machines.connections;	
		*value = (float)(sqrt(psy_audio_connections_wirevolume(connections, self->wire)) * 0.5);	
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
	switch (psy_ui_tabbar_selected(&self->tabbar)) {
		case WIREVIEW_TAB_VUMETER:
		break;
		case WIREVIEW_TAB_OSCILLOSCOPE:
			if (oscilloscope_stopped(&self->oscilloscopeview.oscilloscope)) {
				oscilloscope_continue(&self->oscilloscopeview.oscilloscope);
			} else {
				self->scope_spec_rate = 0.0;
				oscilloscope_setbegin(&self->oscilloscopeview.oscilloscope,
					self->scope_spec_rate);
				oscilloscope_hold(&self->oscilloscopeview.oscilloscope);
			}
		break;
		case WIREVIEW_TAB_SPECTRUM:
			if (spectrumanalyzer_stopped(&self->spectrumanalyzer)) {
				spectrumanalyzer_continue(&self->spectrumanalyzer);
			} else {								
				spectrumanalyzer_hold(&self->spectrumanalyzer);
			}
		break;
		case WIREVIEW_TAB_PHASE:
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
		workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
			SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_ADDEFFECT);
	}
}

bool wireview_wireexists(const WireView* self)
{
	return workspace_song(self->workspace) &&
		psy_audio_machines_connected(
			psy_audio_song_machines(workspace_song(self->workspace)),
			self->wire);
}

psy_ui_Component* wireview_scope(WireView* self, int index)
{
	psy_ui_Component* rv;

	switch (index) {
		case WIREVIEW_TAB_VUMETER:
			rv = &self->vuscope.component;
		break;
		case WIREVIEW_TAB_OSCILLOSCOPE:
			rv = &self->oscilloscopeview.component;
		break;
		case WIREVIEW_TAB_SPECTRUM:
			rv = &self->spectrumanalyzer.component;
		break;
		case WIREVIEW_TAB_PHASE:
			rv = &self->stereophase.component;
		break;
		default:
			rv = 0;
		break;
	}
	return rv;
}

uintptr_t wireview_currscope(WireView* self)
{	
	return psy_ui_tabbar_selected(&self->tabbar);
}

// WireFrame
// prototypes
static void wireframe_updatetitle(WireFrame*, psy_audio_Machines*);
// implementation
void wireframe_init(WireFrame* self, psy_ui_Component* parent,
	psy_audio_Wire wire, Workspace* workspace)
{	
	assert(workspace);
	assert(workspace->song);
	assert(psy_audio_wire_valid(&wire));

	psy_ui_frame_init(wireframe_base(self), parent);
	psy_ui_component_seticonressource(wireframe_base(self), IDI_MACPARAM);
	wireview_init(&self->wireview, &self->component, wire, workspace);
	wireframe_updatetitle(self,
		psy_audio_song_machines(workspace_song(workspace)));
	psy_ui_component_setposition(wireframe_base(self),
		psy_ui_rectangle_make(
		psy_ui_point_make_px(200.0, 150.0),
		psy_ui_size_make_em(80.0, 25.0)));
}

void wireframe_updatetitle(WireFrame* self, psy_audio_Machines* machines)
{
	char title[128];
	psy_audio_Machine* srcmachine;
	psy_audio_Machine* dstmachine;
	
	srcmachine = psy_audio_machines_at(machines, self->wireview.wire.src);
	dstmachine = psy_audio_machines_at(machines, self->wireview.wire.dst);
	psy_snprintf(title, 128, "[%d] %s -> %s Connection Volume",
		(int)psy_audio_connections_wireindex(&machines->connections,
			self->wireview.wire),
		(srcmachine) ? psy_audio_machine_editname(srcmachine) : "ERR",
		(dstmachine) ? psy_audio_machine_editname(dstmachine) : "ERR");
	psy_ui_component_settitle(wireframe_base(self), title);
}

void wireview_ondrawslidervu(WireView* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{
	float maxL, maxR;
	int rmsL, rmsR;
	const float multleft = self->vuscope.invol * self->vuscope.mult * 1.0f; // self->srcMachine._lVol;
	const float multright = self->vuscope.invol * self->vuscope.mult * 1.0f; // srcMachine._rVol;
	uintptr_t samplerate = 44100;
	unsigned int index = 0;
	double centerx;
	double right;
	double step;
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;
	intptr_t scopesamples;
	psy_ui_RealRectangle rect;
	float* pSamplesL;
	float* pSamplesR;
	psy_audio_Machine* machine;
	psy_audio_Buffer* buffer;
	psy_ui_Component* pane;

	machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines, self->wire.src);
	if (!machine) {
		return;
	}

	buffer = psy_audio_machine_buffermemory(machine);
	if (!buffer) {
		return;
	}
	scopesamples = psy_audio_machine_buffermemorysize(machine);
	//process the buffer that corresponds to the lapsed time. Also, force 16 bytes boundaries.
	scopesamples = psy_min(scopesamples, (int)(psy_audio_machine_samplerate(machine) *
		self->vuscope.scope_peak_rate * 0.001)) & (~3);
	pSamplesL = buffer->samples[0];
	pSamplesR = buffer->samples[1];

	pane = sender;
	size = psy_ui_component_scrollsize(pane);
	tm = psy_ui_component_textmetric(pane);
	right = psy_ui_value_px(&size.width, tm, NULL);
	centerx = psy_ui_value_px(&size.width, tm, NULL) / 2;
	step = psy_ui_value_px(&size.height, tm, NULL) / 7;

	maxL = dsp.maxvol(buffer->samples[0], scopesamples) / 32768.f;
	maxR = dsp.maxvol(buffer->samples[1], scopesamples) / 32768.f;

	maxL = ((psy_dsp_amp_t)(2 * step) - dB(maxL * multleft + 0.0000001f) * (psy_dsp_amp_t)step / 6.f);
	maxR = ((psy_dsp_amp_t)(2 * step) - dB(maxR * multright + 0.0000001f) * (psy_dsp_amp_t)step / 6.f);
	rmsL = (int)((psy_dsp_amp_t)(2 * step) - dB(self->vuscope.leftavg * multleft + 0.0000001f) * (psy_dsp_amp_t)step / 6.f);
	rmsR = (int)((psy_dsp_amp_t)(2 * step) - dB(self->vuscope.rightavg * multright + 0.0000001f) * (psy_dsp_amp_t)step / 6.f);

	if (maxL < self->vuscope.peakL) //  it is a cardinal value, so smaller means higher peak.
	{
		if (maxL < 0) maxL = 0;
		self->vuscope.peakL = maxL;
		self->vuscope.peakLifeL = 2000 / self->vuscope.scope_peak_rate; //2 seconds
	}

	if (maxR < self->vuscope.peakR)//  it is a cardinal value, so smaller means higher peak.
	{
		if (maxR < 0) maxR = 0;
		self->vuscope.peakR = maxR;		self->vuscope.peakLifeR = 2000 / self->vuscope.scope_peak_rate; //2 seconds
	}
	// now draw our scope
	// LEFT CHANNEL		
	rect.left = centerx - 60;
	rect.right = rect.left + 24;
	if (self->vuscope.peakL < 2 * step) {
		psy_ui_setcolour(g, psy_ui_colour_make(linepenbL));
	} else {
		psy_ui_setcolour(g, psy_ui_colour_make(linepenL));
	}
	psy_ui_drawline(g, psy_ui_realpoint_make(rect.left - 1, (int)self->vuscope.peakL),
		psy_ui_realpoint_make(rect.right - 1, (int)self->vuscope.peakL));

	rect.top = (int)maxL;
	rect.bottom = centerx;
	rect.bottom = psy_ui_value_px(&size.height, tm, NULL);
	if (rect.top > rect.bottom) {
		rect.top = rect.bottom;
	}
	psy_ui_drawsolidrectangle(g, rect, psy_ui_colour_make(0xC08040));

	rect.left = centerx + 6;
	rect.right = rect.left + 24;
	rect.top = rmsL;
	rect.bottom = psy_ui_value_px(&size.height, tm, NULL);
	if (rect.top > rect.bottom) {
		rect.top = rect.bottom;
	}
	psy_ui_drawsolidrectangle(g, rect, psy_ui_colour_make(0xC08040));
	// RIGHT CHANNEL 
	rect.left = centerx - 30;
	rect.right = rect.left + 24;
	if (self->vuscope.peakR < 2 * step) {
		psy_ui_setcolour(g, psy_ui_colour_make(linepenbR));
	} else {
		psy_ui_setcolour(g, psy_ui_colour_make(linepenR));
	}
	psy_ui_drawline(g, psy_ui_realpoint_make(rect.left - 1, self->vuscope.peakR),
		psy_ui_realpoint_make(rect.right - 1, self->vuscope.peakR));

	rect.top = (int)maxR;
	rect.bottom = psy_ui_value_px(&size.height, tm, NULL);
	if (rect.top > rect.bottom) {
		rect.top = rect.bottom;
	}
	psy_ui_drawsolidrectangle(g, rect, psy_ui_colour_make(0x90D040));

	rect.left = centerx + 36;
	rect.right = rect.left + 24;
	rect.top = rmsR;
	rect.bottom = psy_ui_value_px(&size.height, tm, NULL);
	if (rect.top > rect.bottom) {
		rect.top = rect.bottom;
	}
	psy_ui_drawsolidrectangle(g, rect, psy_ui_colour_make(0x90D040));
	// update peak counter.
	if (!self->vuscope.hold) {
		if (self->vuscope.peakLifeL > 0 || self->vuscope.peakLifeR > 0) {
			--self->vuscope.peakLifeL;
			--self->vuscope.peakLifeR;
			if (self->vuscope.peakLifeL <= 0) {
				self->vuscope.peakL = (psy_dsp_amp_t)INT16_MAX;
			}
			if (self->vuscope.peakLifeR <= 0) {
				self->vuscope.peakR = (psy_dsp_amp_t)INT16_MAX;
			}
		}
	}	
}

/// linear -> deciBell
/// amplitude normalized to 1.0f.
psy_dsp_amp_t dB(psy_dsp_amp_t amplitude)
{
	///\todo merge with psycle::helpers::math::linear_to_deci_bell
	return (psy_dsp_amp_t)(20.0 * log10(amplitude));
}

void wireview_ontimer(WireView* self, uintptr_t timerid)
{	
	switch (psy_ui_notebook_pageindex(&self->notebook)) {
	case 0:		
		/* slider vu needs vuscope values */
		break;
	case 1:		
		oscilloscopeview_idle(&self->oscilloscopeview);		
		break;
	case 2:
		spectrumanalyzer_idle(&self->spectrumanalyzer);
		break;
	case 3:
		stereophase_idle(&self->stereophase);
		break;
	default:
		break;
	}
	vuscope_idle(&self->vuscope);
	psy_ui_component_invalidate(&self->volslider.pane.component);
}
