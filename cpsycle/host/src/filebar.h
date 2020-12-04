// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(FILEBAR_H)
#define FILEBAR_H

// host
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

// aim: button panel to create, load, save and render a song

typedef struct FileBar {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Label header;
	psy_ui_Button recentbutton;
	psy_ui_Button newbutton;
	psy_ui_Button loadbutton;
	psy_ui_Button savebutton;
	psy_ui_Button renderbutton;
	// references
	Workspace* workspace;	
} FileBar;

void filebar_init(FileBar*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* filebar_base(FileBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* FILEBAR_H */
