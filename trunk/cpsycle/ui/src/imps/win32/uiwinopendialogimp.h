// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
#ifndef psy_ui_win_OPENDIALOGIMP_H
#define psy_ui_win_OPENDIALOGIMP_H

// local
#include "../../uiopendialog.h"
// file
#include <dir.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_win_OpenDialogImp {
	psy_ui_OpenDialogImp imp;
	psy_ui_Component* parent;
	char* title;
	char* filter;
	char* defaultextension;
	char* initialdir;	
	psy_Path path;
} psy_ui_win_OpenDialogImp;

void psy_ui_win_opendialogimp_init(psy_ui_win_OpenDialogImp*, struct psy_ui_Component* parent);
void psy_ui_win_opendialogimp_init_all(psy_ui_win_OpenDialogImp*,
	psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_win_OPENDIALOGIMP_H */
#endif /* PSYCLE_TK_WIN32 */
