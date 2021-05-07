// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

// platform
#include "../../detail/prefix.h"
#include "../../detail/os.h"
// host
#include "uiframe.h"
#include "uitabbar.h"
// ui
#include <uiapp.h>
// file
#include <dir.h>
#include <signal.h>
// std
#include <stdio.h>

// Initial entry point for the startup of psycleo
//
// Normally, main is the c entry point, but win32 uses WinMain, so we have
// to define for win32 a different entry point as the usual main method.
// We split the startup call with os defines of diversalis and call then
// psycle_run as main method

// General app run method
static int psycle_run(uintptr_t instance, int options);

// Platform run methods, that are called first at program start
#ifdef DIVERSALIS__OS__MICROSOFT

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objbase.h>

// The WinMain function is called by the system as the initial entry point for
// a Win32-based application
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	int err;
	
	// call the platform independend startup code with
	// instance and startup show options
	err = psycle_run((uintptr_t)hInstance, iCmdShow);
#if defined _CRTDBG_MAP_ALLOC
	_CrtDumpMemoryLeaks();
#endif
	return err;
}
#else
#define _MAX_PATH 4096

int main(int argc, char** argv)
{	
	return psycle_run(0, 0);
	return 0;
}
#endif

int psycle_run(uintptr_t instance, int options)
{
	int err = 0;
	char workpath[_MAX_PATH];
	const char* env = 0;
	psy_ui_App app;
	psy_ui_Frame mainframe;
	psy_ui_TabBar tabbar;
	
	// Adds the app path to the environment path to find some
	// modules (scilexer ...)
	// native-plugin env's are not set here but in plugin.c for each plugin
	// (library_setenv)
	env = pathenv();
	if (env) {
		insertpathenv(workdir(workpath));
	}
	// Initialize the ui
	psy_ui_app_init(&app, psy_ui_DARKTHEME, instance);
	// Creates the mainframe
	psy_ui_frame_init(&mainframe, NULL);
	psy_ui_tabbar_init(&tabbar, &mainframe);
	psy_ui_tabbar_append_tabs(&tabbar, "tab1", 
	"tab2", "tab3", NULL);
	psy_ui_component_setalign(psy_ui_tabbar_base(&tabbar), psy_ui_ALIGN_TOP);
	psy_ui_tabbar_select(&tabbar, 0);
	psy_ui_component_showstate(&mainframe, options);	
	// Starts the app event loop	
	err = psy_ui_app_run(&app);
	printf("Loop finished\n");
	// The event loop has finished, dispose any global ui resources
	psy_ui_app_dispose(&app);
	// Restores the environment path
	if (env) {
		setpathenv(env);
	}
	return err;
}
