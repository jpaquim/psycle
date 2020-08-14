// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT
#ifndef psy_ui_XTAPP_H
#define psy_ui_XTAPP_H

#include "../../detail/stdint.h"
#include "../../detail/os.h"

#include <list.h>

#include <Xm/Xm.h>

#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

//typedef LRESULT (CALLBACK *psy_ui_fp_winproc)(HWND hwnd, UINT message,
	//WPARAM wParam, LPARAM lParam);
	
typedef struct
{
	uintptr_t hwnd;
	uintptr_t id;
	uintptr_t tick;
	uintptr_t numticks;
} psy_ui_X11TickCounter;

typedef struct {
	int reserved;	
	Display * dpy;	
	const char* appclass;
	const char* componentclass;	
	psy_Table selfmap;
	psy_Table winidmap;
	uintptr_t winid;
	Atom wmDeleteMessage;
	bool running;
	psy_List* timers;
} psy_ui_X11App;

void psy_ui_x11app_init(psy_ui_X11App*, void* instance);
void psy_ui_x11app_dispose(psy_ui_X11App*);
int psy_ui_x11app_run(psy_ui_X11App*);
void psy_ui_x11app_stop(psy_ui_X11App*);
void psy_ui_x11app_starttimer(psy_ui_X11App*, uintptr_t hwnd, uintptr_t id,
	uintptr_t interval);
void psy_ui_x11app_stoptimer(psy_ui_X11App*, uintptr_t hwnd, uintptr_t id);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_XTAPP_H */
#endif /* PSYCLE_TK_XT */
