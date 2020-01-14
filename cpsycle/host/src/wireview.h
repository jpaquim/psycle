// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(WIREVIEW_H)
#define WIREVIEW_H

#include "channelmappingview.h"
#include <uinotebook.h>
#include <uislider.h>
#include "tabbar.h"
#include "vuscope.h"
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
	ChannelMappingView channelmappingview;
	psy_ui_Component bottomgroup;
	psy_ui_Button deletewire;
	psy_audio_Wire wire;
	Workspace* workspace;
} WireView;

void wireview_init(WireView*, psy_ui_Component* parent, psy_audio_Wire, Workspace*);
int wireview_wireexists(WireView*);
psy_ui_Component* wireview_base(WireView*);

typedef struct {
	psy_ui_Component component;
	WireView* wireview;
} WireFrame;

void wireframe_init(WireFrame*, psy_ui_Component* parent, WireView* view);
psy_ui_Component* wireframe_base(WireFrame*);


#endif
