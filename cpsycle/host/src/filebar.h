// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(FILEBAR_H)
#define FILEBAR_H

#include "workspace.h"

#include <uibutton.h>
#include <uilabel.h>

typedef struct {
	ui_component component;
	ui_label header;
	ui_button newbutton;
	ui_button loadbutton;
	ui_button savebutton;
	ui_button renderbutton;
	Workspace* workspace;	
} FileBar;

void filebar_init(FileBar*, ui_component* parent, Workspace*);

#endif
