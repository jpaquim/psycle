// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mainframe.h"
#include <uiapp.h>
#include <dir.h>
#include <stdio.h>

UIMAIN
{    	
	MainFrame main;			
	int err = 0;	
	char workpath[_MAX_PATH];
	const char* env = 0;	
	
	env = pathenv();	
	if (env) {			
		insertpathenv(workdir(workpath));
	}
	UIINIT;
	mainframe_init(&main);	
	if (mainframe_showmaximizedatstart(&main)) {
		ui_component_show_state(&main.component, SW_MAXIMIZE);
	} else {
		ui_component_show_state(&main.component, iCmdShow);
	}
	err = ui_run();	
	UIDISPOSE;
	if (env) {
		setpathenv(env);
	}	
	// _CrtDumpMemoryLeaks();
	return err;
}
