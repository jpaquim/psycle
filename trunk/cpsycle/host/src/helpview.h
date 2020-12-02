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

#ifdef __cplusplus
extern "C" {
#endif

// aim: Helpview combines the About, Greet and textfile help in
//      a tabbed notebook. 

typedef struct {
	psy_ui_Component component;
	psy_ui_Notebook notebook;
	TabBar tabbar;	
	About about;
	Greet greet;
	Help help;
	Workspace* workspace;
} HelpView;

TabBar* helpview_init(HelpView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);

INLINE psy_ui_Component* helpview_base(HelpView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif
