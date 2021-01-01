// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(CHANNELMAPPINGVIEW_H)
#define CHANNELMAPPINGVIEW_H

// host
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

// ChannelMappingView
//
// The channel mapping view allows accessing to the audio mappings of
// a wire. Usually, machines have just one stereo output (and one
// stereo input, if they are effects). But several types of machines
// have multiple inputs and/or multiple outputs. In this case, a side
// chain compressor is showing one stereo input, and one stereo side
// chain. With the help of this screen, audio can be routed exactly to
// the pins that are intended to be used.

typedef enum {
	PINEDIT_DRAG_NONE,
	PINEDIT_DRAG_DST,
	PINEDIT_DRAG_SRC,
	PINEDIT_DRAG_NEW_DST,
	PINEDIT_DRAG_NEW_SRC
} PinEditDragMode;

typedef struct PinEdit {
	// inherits
	psy_ui_Component component;
	// internal data
	psy_audio_Wire wire;
	PinEditDragMode dragmode;
	uintptr_t drag_src;
	uintptr_t drag_dst;
	intptr_t mx;
	intptr_t my;
	psy_ui_Colour pincolour;
	psy_ui_Colour wirecolour;
	// references
	Workspace* workspace;
} PinEdit;

void pinedit_init(PinEdit*, psy_ui_Component* parent, psy_audio_Wire,
	Workspace*);
void pinedit_autowire(PinEdit*);
void pinedit_unselectall(PinEdit*);

INLINE psy_ui_Component* pinedit_base(PinEdit* self)
{
	assert(self);

	return &self->component;
}

typedef struct ChannelMappingView {
	// inherits
	psy_ui_Component component;
	// ui elements
	PinEdit pinedit;
	psy_ui_Component buttongroup;
	psy_ui_Button autowire;
	psy_ui_Button unselectall;
	psy_ui_Label help;
	// references
	Workspace* workspace;
} ChannelMappingView;

void channelmappingview_init(ChannelMappingView*, psy_ui_Component* parent,
	psy_audio_Wire, Workspace*);

INLINE psy_ui_Component* channelmappingview_base(ChannelMappingView* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* CHANNELMAPPINGVIEW_H */
