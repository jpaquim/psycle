// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(CHANNELMAPPINGVIEW_H)
#define CHANNELMAPPINGVIEW_H

#include <uibutton.h>
#include "workspace.h"

typedef struct {	
	ui_component component;	
	Workspace* workspace;
	Wire wire;
	int lineheight;
} PinEdit;

void pinedit_init(PinEdit*, ui_component* parent, Wire, Workspace*);

typedef struct {
	ui_component component;
	PinEdit pinedit;
	ui_component buttongroup;
	ui_button autowire;
	ui_button unselectall;
} ChannelMappingView;


void channelmappingview_init(ChannelMappingView*, ui_component* parent, Wire, Workspace*);

#endif
