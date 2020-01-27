// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_win_FOLDERDIALOGIMP_H
#define psy_ui_win_FOLDERDIALOGIMP_H

#include "uifolderdialog.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_win_FolderDialogImp {
	psy_ui_FolderDialogImp imp;
	struct psy_ui_Component* parent;
	char* title;
	char* initialdir;
	char* path;
} psy_ui_win_FolderDialogImp;

void psy_ui_win_folderdialogimp_init(psy_ui_win_FolderDialogImp*);
void psy_ui_win_folderdialogimp_init_all(psy_ui_win_FolderDialogImp* self,
	psy_ui_Component* parent,
	const char* title,
	const char* initialdir);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_win_FOLDERDIALOGIMP_H */

