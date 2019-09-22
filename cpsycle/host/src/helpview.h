// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(HELPVIEW_H)
#define HELPVIEW_H

#include <uinotebook.h>
#include "tabbar.h"
#include "workspace.h"
#include "greet.h"
#include "about.h"

typedef struct {
	ui_component component;
	ui_notebook notebook;
	TabBar tabbar;
	About about;
	Greet greet;
} HelpView;

void InitHelpView(HelpView*, ui_component* parent,
	ui_component* tabbarparent, Workspace*);

#endif