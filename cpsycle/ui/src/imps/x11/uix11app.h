// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11
#ifndef psy_ui_X11APP_H
#define psy_ui_X11APP_H

/* local */
#include "../../uiapp.h"
#include "../../detail/os.h"
#include "../../timers.h"
#include "uix11colours.h"
/* container */
#include <list.h>
/* X11 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef __cplusplus
extern "C" {
#endif

//typedef LRESULT (CALLBACK *psy_ui_fp_winproc)(HWND hwnd, UINT message,
	//WPARAM wParam, LPARAM lParam);

struct psy_ui_Component;

typedef struct psy_ui_X11App {
	// implements
	psy_ui_AppImp imp;
	// internal
	int reserved;	
	Display * dpy;	
	const char* appclass;
	const char* componentclass;
	// winproc
	// comwinproc
	psy_Table selfmap;
	psy_Table winidmap;
	uintptr_t winid;
	// defaultbackgroundbrush
	struct psy_ui_Component* eventretarget;
	// mousehook
	Atom wmDeleteMessage;
	bool running;	
	psy_ui_X11Colours colourmap;
	XVisualInfo* vinfo;
	Visual* visual;	
	bool dbe;
	bool dograb;
	Window grabwin;
	psy_List* targetids;
	psy_Timers wintimers;
	bool shiftstate;
	bool controlstate;
	bool altstate;
	// references
	psy_ui_App* app;
} psy_ui_X11App;

void psy_ui_x11app_init(psy_ui_X11App*, psy_ui_App*, void* instance);

void psy_ui_x11app_onappdefaultschange(psy_ui_X11App*);

void psy_ui_x11app_starttimer(psy_ui_X11App*, uintptr_t hwnd, uintptr_t id,
	uintptr_t interval);
void psy_ui_x11app_stoptimer(psy_ui_X11App*, uintptr_t hwnd, uintptr_t id);

INLINE int psy_ui_x11app_colourindex(psy_ui_X11App* self, psy_ui_Colour colour)
{
	return psy_ui_x11colours_index(&self->colourmap, colour);
}

void psy_ui_x11app_destroy_window(psy_ui_X11App*, Window);

INLINE void psy_ui_x11app_startgrab(psy_ui_X11App* self, Window w)
{
	self->dograb = TRUE;
	self->grabwin = w;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_X11APP_H */
#endif /* PSYCLE_TK_X11 */
