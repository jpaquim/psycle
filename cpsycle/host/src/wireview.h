// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(WIREVIEW_H)
#define WIREVIEW_H

#include "channelmappingview.h"
#include <uinotebook.h>
#include <uislider.h>
#include "tabbar.h"
#include "workspace.h"

typedef struct {
	ui_component component;
	TabBar tabbar;
	ui_notebook notebook;
	ui_slider volslider;
	ChannelMappingView channelmappingview;
	uintptr_t src;
	uintptr_t dst;
	Workspace* workspace;
} WireView;

void wireview_init(WireView*, ui_component* parent, uintptr_t src, uintptr_t dst, Workspace*);

typedef struct {
	ui_component component;
	WireView* wireview;
} WireFrame;

void wireframe_init(WireFrame*, ui_component* parent, WireView* view);

#endif
