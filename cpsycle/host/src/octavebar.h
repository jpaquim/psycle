// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(OCTAVEBAR_H)
#define OCTAVEBAR_H

#include "uicombobox.h"
#include "uilabel.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

// aim: Current octave, ranging from 0 to 8, when entering notes with the PC
//      keyboard

typedef struct {
	psy_ui_Component component;
	psy_ui_Label headerlabel;
	psy_ui_ComboBox octavebox;	
	Workspace* workspace;
} OctaveBar;

void octavebar_init(OctaveBar*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* octavebar_base(OctaveBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif
