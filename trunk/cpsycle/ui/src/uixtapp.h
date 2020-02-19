// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT
#ifndef psy_ui_XTAPP_H
#define psy_ui_XTAPP_H

#include "../../detail/stdint.h"
#include "../../detail/os.h"

#include <Xm/Xm.h>

#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

//typedef LRESULT (CALLBACK *psy_ui_fp_winproc)(HWND hwnd, UINT message,
	//WPARAM wParam, LPARAM lParam);

typedef struct {
	int reserved;
	XtAppContext context;
	Display * dpy;	
	const char* appclass;
	const char* componentclass;	
	//psy_ui_fp_winproc winproc;
	//psy_ui_fp_winproc comwinproc;
	psy_Table selfmap;
	psy_Table winidmap;
	uintptr_t winid;
//	HBRUSH defaultbackgroundbrush;	
} psy_ui_XtApp;

void psy_ui_xtapp_init(psy_ui_XtApp*, void* instance);
void psy_ui_xtapp_dispose(psy_ui_XtApp*);
int psy_ui_xtapp_run(psy_ui_XtApp*);
void psy_ui_xtapp_stop(psy_ui_XtApp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_XTAPP_H */
#endif /* PSYCLE_TK_XT */
