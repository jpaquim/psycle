// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(WIREVIEW_H)
#define WIREVIEW_H

// host
#include "channelmappingview.h"
#include "oscilloscope.h"
#include "spectrumanalyzer.h"
#include "stereophase.h"
#include <uitabbar.h>
#include "vuscope.h"
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uilabel.h>
#include <uinotebook.h>
#include <uislider.h>

// WireView
//
// Displays the connection volume, signal analyser and channel mappings

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WireView {
	// inherits
	psy_ui_Component component;
	// ui elements
	// Scope select
	psy_ui_Component top;	
	psy_ui_TabBar tabbar;
	psy_ui_Notebook notebook;
	// Volume
	psy_ui_Component slidergroup;
	psy_ui_Slider volslider;
	psy_ui_Button dbvol;
	psy_ui_Button percvol;
	// Scopes
	VuScope vuscope;
	OscilloscopeView oscilloscopeview;
	SpectrumAnalyzer spectrumanalyzer;
	StereoPhase stereophase;
	ChannelMappingView channelmappingview;
	// Scope Control
	psy_ui_Component rategroup;
	psy_ui_Slider modeslider;
	psy_ui_Slider rateslider;
	psy_ui_Button hold;
	// Toolbar
	psy_ui_Component bottomgroup;
	psy_ui_Button addeffect;
	psy_ui_Button deletewire;
	psy_audio_Wire wire;	
	// data members
	float scope_spec_rate;
	float scope_spec_mode;	
	Workspace* workspace;
} WireView;

void wireview_init(WireView*, psy_ui_Component* parent, psy_audio_Wire, Workspace*);

INLINE WireView* wireview_alloc(void)
{
	return (WireView*)malloc(sizeof(WireView));
}

INLINE WireView* wireview_allocinit(psy_ui_Component* parent, psy_audio_Wire wire,
	Workspace* workspace)
{
	WireView* rv;	
	
	rv = wireview_alloc();
	if (rv) {
		wireview_init(rv, parent, wire, workspace);
	}
	return rv;
}

bool wireview_wireexists(const WireView*);

INLINE psy_ui_Component* wireview_base(WireView* self)
{
	assert(self);

	return &self->component;
}

// WireFrame
//
// Wraps a WireView into frame

typedef struct WireFrame{
	// inherits
	psy_ui_Component component;
	WireView wireview;
} WireFrame;

void wireframe_init(WireFrame*, psy_ui_Component* parent, psy_audio_Wire,
	Workspace*);

INLINE WireFrame* wireframe_alloc(void)
{
	return (WireFrame*)malloc(sizeof(WireFrame));
}

INLINE WireFrame* wireframe_allocinit(psy_ui_Component* parent,
	psy_audio_Wire wire, Workspace* workspace)
{
	WireFrame* rv;

	rv = wireframe_alloc();
	if (rv) {
		wireframe_init(rv, parent, wire, workspace);
	}
	return rv;
}

INLINE const psy_audio_Wire* wireframe_wire(const WireFrame* self)
{
	return &self->wireview.wire;
}

INLINE psy_ui_Component* wireframe_base(WireFrame* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* WIREVIEW_H */
