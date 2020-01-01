// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(CHANNELMAPPINGVIEW_H)
#define CHANNELMAPPINGVIEW_H

#include <uibutton.h>
#include "workspace.h"

typedef struct {	
	psy_ui_Component component;	
	Workspace* workspace;
	psy_audio_Wire wire;
	int lineheight;
} PinEdit;

void pinedit_init(PinEdit*, psy_ui_Component* parent, psy_audio_Wire, Workspace*);

typedef struct {
	psy_ui_Component component;
	PinEdit pinedit;
	psy_ui_Component buttongroup;
	psy_ui_Button autowire;
	psy_ui_Button unselectall;
} ChannelMappingView;


void channelmappingview_init(ChannelMappingView*, psy_ui_Component* parent, psy_audio_Wire, Workspace*);

#endif
