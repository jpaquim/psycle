// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
#ifndef psy_ui_WINAPP_H
#define psy_ui_WINAPP_H

#include <list.h>
#include "../../uiapp.h"

// target win98 or nt 4 or later systems
#if defined _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x400

#include <windows.h>

#include "../../detail/stdint.h"
#include "../../detail/os.h"

#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef LRESULT (CALLBACK *psy_ui_fp_winproc)(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);

struct psy_ui_Component;

typedef struct psy_ui_WinApp {
	// implements
	psy_ui_AppImp imp;
	// internal
	int reserved;
	HINSTANCE instance;
	TCHAR* appclass;
	TCHAR* componentclass;	
	psy_ui_fp_winproc winproc;
	psy_ui_fp_winproc comwinproc;
	psy_Table selfmap;
	psy_Table winidmap;
	uintptr_t winid;
	HBRUSH defaultbackgroundbrush;
	struct psy_ui_Component* eventretarget;
	psy_List* targetids;	
	// references
	psy_ui_App* app;
} psy_ui_WinApp;

void psy_ui_winapp_init(psy_ui_WinApp*, psy_ui_App* app, HINSTANCE instance);

void psy_ui_winapp_onappdefaultschange(psy_ui_WinApp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_WINAPP_H */
#endif /* PSYCLE_TK_WIN32 */
