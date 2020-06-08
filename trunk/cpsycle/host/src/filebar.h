// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(FILEBAR_H)
#define FILEBAR_H

#include "workspace.h"

#include <uibutton.h>
#include <uilabel.h>

// aim: button panel to create, load, save and render a song

typedef struct {
	psy_ui_Component component;
	psy_ui_Label header;
	psy_ui_Button recentbutton;
	psy_ui_Button newbutton;
	psy_ui_Button loadbutton;
	psy_ui_Button savebutton;
	psy_ui_Button renderbutton;
	Workspace* workspace;	
} FileBar;

void filebar_init(FileBar*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* filebar_base(FileBar* self)
{
	return &self->component;
}

#endif /* FILEBAR_H */
