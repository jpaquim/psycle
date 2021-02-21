// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

// platform
#include "../../detail/prefix.h"
#include "../../detail/os.h"
// host
#include "mainframe.h"
// ui
#include <uiapp.h>
// std
#include <stdlib.h>
// file
#include <dir.h>

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
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ PSTR szCmdLine, _In_ int iCmdShow)
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
}
#endif

int psycle_run(uintptr_t instance, int options)
{
	int err = 0;
	char workpath[_MAX_PATH];
	const char* env = 0;
	psy_ui_App app;
	MainFrame* mainframe;
	
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
	mainframe = (MainFrame*)malloc(sizeof(MainFrame));
	if (mainframe) {
		mainframe_init(mainframe);
		// The mainframe has been initialized, so show it.
		if (mainframe_showmaximizedatstart(mainframe)) {
#ifdef DIVERSALIS__OS__MICROSOFT		
			psy_ui_component_showstate(&mainframe->component, SW_MAXIMIZE);
#else
			psy_ui_component_resize(&mainframe->component,
				psy_ui_size_make(
					psy_ui_value_makepx(1024),
					psy_ui_value_makepx(768)));
			psy_ui_component_showstate(&mainframe->component, 0);
#endif		
		} else {
			psy_ui_component_showstate(&mainframe->component, options);
		}
		// Starts the app event loop
		err = psy_ui_app_run(&app);
		// The event loop has finished, dispose any global ui resources
	}
	psy_ui_app_dispose(&app);
	free(mainframe);
	// Restores the environment path
	if (env) {
		setpathenv(env);
	}
	return err;
}
