// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net


#include "../../detail/prefix.h"
#include "../../detail/os.h"
#include "../../detail/psyconf.h"

#include "mainframe.h"
#include <uiapp.h>
#include <uiframe.h>
#include <dir.h>
#include <stdlib.h>
#if PSYCLE_USE_TK == PSYCLE_TK_CURSES
#include <curses.h>
#endif

#ifdef DIVERSALIS__OS__MICROSOFT
// The WinMain function is called by the system as the initial entry point for
// a Win32-based application
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
#else
int main(int argc, char **argv)
#endif
{	
	int err = 0;
	char workpath[_MAX_PATH];
	const char* env = 0;
	extern psy_ui_App app;
#if PSYCLE_USE_TK == PSYCLE_TK_CURSES
	static psy_ui_Frame frame;
#else
	MainFrame mainframe;
#endif
		
	// adds the app path to the environment path find some
	// modules (scilexer, for plugins: universalis, vcredist dlls, ...)
	env = pathenv();	
	if (env) {			
		insertpathenv(workdir(workpath));
	}	
#ifdef DIVERSALIS__OS__MICROSOFT
	// win32 needs an application handle (hInstance)
	psy_ui_app_init(&app, (uintptr_t) hInstance);
#else
	psy_ui_app_init(&app, 0);
#endif			

#if PSYCLE_USE_TK == PSYCLE_TK_CURSES
	psy_ui_frame_init(&frame, 0);
	getch();
#else
	mainframe_init(&mainframe);		
	if (mainframe_showmaximizedatstart(&mainframe)) {
		psy_ui_component_showstate(&mainframe.component, SW_MAXIMIZE);
	} else {
		psy_ui_component_showstate(&mainframe.component, iCmdShow);
	}	
	err = psy_ui_app_run(&app);
#endif
	psy_ui_app_dispose(&app);
	// restores the environment path
	if (env) {
		setpathenv(env);
	}	
//	_CrtDumpMemoryLeaks();	
	return err;
}
