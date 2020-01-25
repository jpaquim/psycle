// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_FOLDERNDIALOG_H
#define psy_ui_FOLDERDIALOG_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_FolderDialog {
	psy_ui_Component* parent;
	char* title;		
	char* initialdir;
	char* path;
} psy_ui_FolderDialog;

void psy_ui_folderdialog_init(psy_ui_FolderDialog*, psy_ui_Component* parent);
void psy_ui_folderdialog_init_all(psy_ui_FolderDialog*,
	psy_ui_Component* parent,
	const char* title,
	const char* initialdir);
void psy_ui_folderdialog_dispose(psy_ui_FolderDialog*);

int psy_ui_folderdialog_execute(psy_ui_FolderDialog*);
const char* psy_ui_folderdialog_path(psy_ui_FolderDialog*);

#ifdef __cplusplus
}
#endif

#endif
