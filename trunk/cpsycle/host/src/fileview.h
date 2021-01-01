// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(FILEVIEW_H)
#define FILEVIEW_H

// host
#include "tabbar.h"
#include "workspace.h"
// ui
#include "uilistbox.h"

#ifdef __cplusplus
extern "C" {
#endif

// FileView
//
// Displays a file chooser added for xlib missing a native one. The win32
// build is configured to use the native instead this one.
// To activate undefine PSYCLE_USE_PLATFORM_FILEOPEN in psyconf.h.

typedef struct FileView {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_selected;
	// ui elements
	psy_ui_ListBox filebox;
	// internal data
	TabBar drives;	
	psy_Table filenames;
	char* curr;
	char* drive;	
	psy_List* files;
	intptr_t numdirectories;
	// references
	Workspace* workspace;
} FileView;

void fileview_init(FileView*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* fileview_base(FileView* self)
{
	return &self->component;
}

const char* fileview_path(FileView* self);

#ifdef __cplusplus
}
#endif

#endif /* FILEVIEW_H */
