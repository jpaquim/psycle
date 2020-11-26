// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLAYPOSBAR_H)
#define PLAYPOSBAR_H

#include "uilabel.h"
#include "uibutton.h"
#include "player.h"
#include "workspace.h"

// aim: displays a playback counter in beats unit

typedef struct {
	psy_ui_Component component;
	psy_ui_Label header;
	psy_ui_Label position;
	Workspace* workspace;
} PlayPosBar;

void playposbar_init(PlayPosBar*, psy_ui_Component* parent,
	Workspace* workspace);

INLINE psy_ui_Component* playposbar_base(PlayPosBar* self)
{
	return &self->component;
}

#endif
