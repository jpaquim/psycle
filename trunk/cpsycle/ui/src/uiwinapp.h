// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_WINAPP_H
#define psy_ui_WINAPP_H

#include <windows.h>

#include "../../detail/stdint.h"
#include "../../detail/os.h"

#include <hashtbl.h>

typedef LRESULT (CALLBACK *psy_ui_fp_winproc)(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);

typedef struct {
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
} psy_ui_WinApp;

void psy_ui_winapp_init(psy_ui_WinApp*, HINSTANCE instance);
void psy_ui_winapp_dispose(psy_ui_WinApp*);
int psy_ui_winapp_run(psy_ui_WinApp*);
void psy_ui_winapp_stop(psy_ui_WinApp*);

#endif /* psy_ui_WINAPP_H */
