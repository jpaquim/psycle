// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(HELPVIEW_H)
#define HELPVIEW_H

// host
#include "about.h"
#include "greet.h"
#include "help.h"
#include "tabbar.h"
// ui
#include <uibutton.h>
#include <uilabel.h>
#include <uinotebook.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	HELPVIEWSECTION_ABOUT = 0,
	HELPVIEWSECTION_GREET,
	HELPVIEWSECTION_HELP
} HelpViewSection;

// Helpview
//
// Groups About, Greet and Help (textfiles).

typedef struct HelpView {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Notebook notebook;
	psy_ui_Component bar;
	TabBar tabbar;
	psy_ui_Button floatsection;
	// sections
	Help help;
	About about;
	Greet greet;
	psy_ui_Component sectionfloated;
	psy_ui_Label floatdesc;
	// references
	Workspace* workspace;
} HelpView;

void helpview_init(HelpView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);

// float to side bar (see propertiesview, too)
// todo make it more general
void helpview_float(HelpView*, HelpViewSection section, psy_ui_Component* dest);
void helpview_dock(HelpView*, HelpViewSection section, psy_ui_Component* src);

INLINE psy_ui_Component* helpview_base(HelpView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* HELPVIEW_H */
