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
#include <string.h>

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32 || PSYCLE_USE_TK == PSYCLE_TK_XT

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#endif

#ifdef DIVERSALIS__OS__MICROSOFT
// The WinMain function is called by the system as the initial entry point for
// a Win32-based application
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objbase.h>

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
	MainFrame mainframe;
		
	// adds the app path to the environment path find some
	// modules (scilexer, for plugins: universalis, vcredist dlls, ...)
	env = pathenv();	
	if (env) {			
		insertpathenv(workdir(workpath));
	}	
#ifdef DIVERSALIS__OS__MICROSOFT
	// win32 needs an application handle (hInstance)
	psy_ui_app_init(&app, (uintptr_t) hInstance);
	CoInitialize(NULL);
#else
	psy_ui_app_init(&app, 0);
#endif
	mainframe_init(&mainframe);	
#if defined DIVERSALIS__OS__MICROSOFT	
	if (mainframe_showmaximizedatstart(&mainframe)) {
		psy_ui_component_showstate(&mainframe.component, SW_MAXIMIZE);
	} else {
		psy_ui_component_showstate(&mainframe.component, iCmdShow);
	}	
#else
    psy_ui_component_show(&mainframe.component);
#endif    
	err = psy_ui_app_run(&app);
	psy_ui_app_dispose(&app);
	// restores the environment path
	if (env) {
		setpathenv(env);
	}	
#ifdef DIVERSALIS__OS__MICROSOFT
	CoUninitialize();
#endif 
//	_CrtDumpMemoryLeaks();	
	return err;
}
#endif

#if PSYCLE_USE_TK == PSYCLE_TK_CURSES
#include <curses.h>

typedef struct {
	psy_ui_Component component;
} MainFrm;

static void ondraw(MainFrm*, psy_ui_Component* sender, psy_ui_Graphics*);

static void mainfrm_init(MainFrm* self)
{
	psy_ui_frame_init(&self->component, 0);
	psy_signal_connect(&self->component.signal_draw, self, ondraw);
}

void ondraw(MainFrm* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{
	const char* text = "text";

	psy_ui_textout(g, 0, 0, text, strlen(text));
}

#ifdef DIVERSALIS__OS__MICROSOFT
// The WinMain function is called by the system as the initial entry point for
// a Win32-based application
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
#else
int main(int argc, char** argv)
#endif
{
	int err = 0;
	char workpath[_MAX_PATH];
	const char* env = 0;
	extern psy_ui_App app;
	static MainFrm mainframe;

	// adds the app path to the environment path find some
	// modules (scilexer, for plugins: universalis, vcredist dlls, ...)
	env = pathenv();
	if (env) {
		insertpathenv(workdir(workpath));
	}
#ifdef DIVERSALIS__OS__MICROSOFT
	// win32 needs an application handle (hInstance)
	psy_ui_app_init(&app, (uintptr_t)hInstance);
#else
	psy_ui_app_init(&app, 0);
#endif			

	//psy_ui_frame_init(&frame, 0);
	mainfrm_init(&mainframe);
	psy_ui_component_invalidate(&mainframe.component);
	getch();
	psy_ui_component_move(&mainframe.component, 10, 100);
	getch();
	psy_ui_app_dispose(&app);
	// restores the environment path
	if (env) {
		setpathenv(env);
	}
	//	_CrtDumpMemoryLeaks();	
	return err;
}
#endif
