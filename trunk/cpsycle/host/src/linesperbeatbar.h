// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(LINESPERBEATBAR_H)
#define LINESPERBEATBAR_H

// host
#include "workspace.h"
// ui
#include "uilabel.h"
#include "uibutton.h"

#ifdef __cplusplus
extern "C" {
#endif

// LinesPerBeatBar
//
// Sets the Lines per beat ranging from 1 to 31.

typedef struct LinesPerBeatBar {
	//inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Label lpbdesclabel;
	psy_ui_Label lpblabel;	
	psy_ui_Button lessbutton;
	psy_ui_Button morebutton;
	uintptr_t lpb;
	// references
	Workspace* workspace;
} LinesPerBeatBar;

void linesperbeatbar_init(LinesPerBeatBar*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* linesperbeatbar_base(LinesPerBeatBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* LINESPERBEATBAR_H */
