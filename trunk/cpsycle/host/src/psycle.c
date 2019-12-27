// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "mainframe.h"
#include <uiapp.h>
#include <dir.h>

#ifdef DIVERSALIS__OS__MICROSOFT
// The WinMain function is called by the system as the initial entry point for
// a Win32-based application
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
#else
	psy_ui_app_init(&app, 0);
#endif	
	mainframe_init(&mainframe);		
	if (mainframe_showmaximizedatstart(&mainframe)) {
		ui_component_show_state(&mainframe.component, SW_MAXIMIZE);
	} else {
		ui_component_show_state(&mainframe.component, iCmdShow);
	}	
	err = psy_ui_app_run(&app);	
	psy_ui_app_dispose(&app);
	// restores the environment path
	if (env) {
		setpathenv(env);
	}
//	_CrtDumpMemoryLeaks();	
	return err;
}
