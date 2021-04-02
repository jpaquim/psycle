// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(HELP_H)
#define HELP_H

// host
#include <uitabbar.h>
#include "workspace.h"
// ui
#include <uieditor.h>

#ifdef __cplusplus
extern "C" {
#endif

// Help
//
// Integrated textfile viewer to show the psycle help files inside the psycle
// doc directory. Configure the used files with PSYCLE_HELPFILES in psyconf.h.
// It uses Scintilla, an open source editor component, first used with
// psycle 1.12.

typedef struct Help {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Editor editor;
	psy_ui_TabBar tabbar;
	// internal data
	psy_Table filenames;
	psy_ui_AlignType lastalign;
	// references
	Workspace* workspace;
} Help;

void help_init(Help*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* help_base(Help* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* HELP_H */
