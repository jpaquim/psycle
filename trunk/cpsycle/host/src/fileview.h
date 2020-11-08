// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(FILEVIEW_H)
#define FILEVIEW_H

#include "uilistbox.h"

#include "tabbar.h"
#include "workspace.h"

// FileView

typedef struct FileView {
	psy_ui_Component component;
	psy_ui_ListBox filebox;
	TabBar drives;
	Workspace* workspace;
	psy_Table filenames;
	char* curr;
	char* drive;
	psy_Signal signal_selected;
	psy_List* files;
	intptr_t numdirectories;
} FileView;

void fileview_init(FileView*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* fileview_base(FileView* self)
{
	return &self->component;
}

const char* fileview_path(FileView* self);

#endif /* FILEVIEW_H */
