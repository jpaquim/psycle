// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(CHANNELMAPPINGVIEW_H)
#define CHANNELMAPPINGVIEW_H

#include <uibutton.h>
#include <uilabel.h>
#include "workspace.h"

// aim: The channel mapping view allows accessing to the audio mappings of
//      a wire. Usually, machines have just one stereo output (and one
//      stereo input, if they are effects). But several types of machines
//      have multiple inputs and/or multiple outputs. In this case, a side
//      chain compressor is showing one stereo input, and one stereo side
//      chain. With the help of this screen, audio can be routed exactly to
//      the pins that are intended to be used.

typedef enum {
	PINEDIT_DRAG_NONE,
	PINEDIT_DRAG_DST,
	PINEDIT_DRAG_SRC,
	PINEDIT_DRAG_NEW_DST,
	PINEDIT_DRAG_NEW_SRC
} PinEditDragMode;

typedef struct {	
	psy_ui_Component component;		
	psy_audio_Wire wire;
	PinEditDragMode dragmode;
	int drag_src;
	int drag_dst;
	int mx;
	int my;
	psy_ui_Color pincolor;
	psy_ui_Color wirecolor;
	Workspace* workspace;
} PinEdit;

void pinedit_init(PinEdit*, psy_ui_Component* parent, psy_audio_Wire,
	Workspace*);
void pinedit_autowire(PinEdit*);
void pinedit_unselectall(PinEdit*);

typedef struct {
	psy_ui_Component component;
	PinEdit pinedit;
	psy_ui_Component buttongroup;
	psy_ui_Button autowire;
	psy_ui_Button unselectall;
	psy_ui_Label help;
	Workspace* workspace;
} ChannelMappingView;

void channelmappingview_init(ChannelMappingView*, psy_ui_Component* parent,
	psy_audio_Wire, Workspace*);

#endif /* CHANNELMAPPINGVIEW_H */
