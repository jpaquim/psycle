/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(FILEVIEW_H)
#define FILEVIEW_H

/* ui */
#include "uilistbox.h"
#include <uitabbar.h>
/* file */
#include <dir.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FileView {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_selected;
	/* internal */
	psy_ui_ListBox filebox;	
	psy_ui_TabBar drives;	
	psy_Table filenames;
	psy_Path curr;	
	psy_List* files;
	intptr_t numdirectories;	
} FileView;

void fileview_init(FileView*, psy_ui_Component* parent);

INLINE psy_ui_Component* fileview_base(FileView* self)
{
	return &self->component;
}

const char* fileview_path(FileView* self);
void fileview_filename(FileView* self, char* filename, uintptr_t maxsize);

#ifdef __cplusplus
}
#endif

#endif /* FILEVIEW_H */
