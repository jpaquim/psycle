// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(WIREVIEW_H)
#define WIREVIEW_H

#include "channelmappingview.h"
#include <uinotebook.h>
#include <uislider.h>
#include "tabbar.h"
#include "vuscope.h"
#include "workspace.h"

typedef struct {
	ui_component component;
	TabBar tabbar;
	ui_notebook notebook;
	ui_component slidergroup;
	ui_slider volslider;
	ui_button dbvol;
	ui_button percvol;
	VuScope vuscope;
	ChannelMappingView channelmappingview;
	ui_component bottomgroup;
	ui_button deletewire;
	psy_audio_Wire wire;
	Workspace* workspace;
} WireView;

void wireview_init(WireView*, ui_component* parent, psy_audio_Wire, Workspace*);
int wireview_wireexists(WireView*);

typedef struct {
	ui_component component;
	WireView* wireview;
} WireFrame;

void wireframe_init(WireFrame*, ui_component* parent, WireView* view);

#endif
