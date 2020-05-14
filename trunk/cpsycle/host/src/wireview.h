// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(WIREVIEW_H)
#define WIREVIEW_H

#include "channelmappingview.h"
#include <uinotebook.h>
#include <uislider.h>
#include "tabbar.h"
#include "vuscope.h"
#include "oscilloscope.h"
#include "spectrumanalyzer.h"
#include "stereophase.h"
#include "workspace.h"

typedef struct {
	psy_ui_Component component;
	TabBar tabbar;
	psy_ui_Notebook notebook;
	psy_ui_Component slidergroup;
	psy_ui_Slider volslider;
	psy_ui_Button dbvol;
	psy_ui_Button percvol;
	VuScope vuscope;
	OscilloscopeView oscilloscopeview;
	SpectrumAnalyzer spectrumanalyzer;
	StereoPhase stereophase;
	ChannelMappingView channelmappingview;
	psy_ui_Component rategroup;
	psy_ui_Slider modeslider;
	psy_ui_Slider rateslider;
	psy_ui_Button hold;
	psy_ui_Component bottomgroup;
	psy_ui_Button addeffect;
	psy_ui_Button deletewire;
	psy_audio_Wire wire;
	Workspace* workspace;
	float scope_spec_rate;
	float scope_spec_mode;
} WireView;

void wireview_init(WireView*, psy_ui_Component* parent, psy_audio_Wire, Workspace*);
int wireview_wireexists(WireView*);

INLINE psy_ui_Component* wireview_base(WireView* self)
{
	return &self->component;
}

typedef struct {
	psy_ui_Component component;
	WireView* wireview;
} WireFrame;

void wireframe_init(WireFrame*, psy_ui_Component* parent, WireView* view);

INLINE psy_ui_Component* wireframe_base(WireFrame* self)
{
	return &self->component;
}



#endif
