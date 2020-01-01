// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(HELPVIEW_H)
#define HELPVIEW_H

#include "about.h"
#include "greet.h"
#include "help.h"
#include "tabbar.h"
#include "workspace.h"

#include <uinotebook.h>

typedef struct {
	psy_ui_Component component;
	ui_notebook notebook;
	TabBar tabbar;	
	About about;
	Greet greet;
	Help help;
} HelpView;

void helpview_init(HelpView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);

#endif
