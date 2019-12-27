// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(HELP_H)
#define HELP_H

#include "tabbar.h"
#include "workspace.h"
#include "greet.h"
#include "about.h"

#include <uieditor.h>
#include <uinotebook.h>

typedef struct {
	psy_ui_Component component;
	ui_notebook notebook;
	psy_ui_Editor editor;
	TabBar tabbar;
	Workspace* workspace;
} Help;

void help_init(Help*, psy_ui_Component* parent, Workspace*);

#endif
