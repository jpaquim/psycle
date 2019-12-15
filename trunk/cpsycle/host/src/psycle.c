// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mainframe.h"
#include <uiapp.h>
#include <dir.h>
#include <stdio.h>
#include <presetio.h>
#include <presets.h>

UIMAIN
{
	MainFrame mainframe;
	int err = 0;	
	char workpath[_MAX_PATH];
	const char* env = 0;	
	
	env = pathenv();	
	if (env) {			
		insertpathenv(workdir(workpath));
	}	
	UIINIT;	
	mainframe_init(&mainframe);	
	if (mainframe_showmaximizedatstart(&mainframe)) {
		ui_component_show_state(&mainframe.component, SW_MAXIMIZE);
	} else {
		ui_component_show_state(&mainframe.component, iCmdShow);
	}
	err = ui_run();	
	ui_dispose();
	if (env) {
		setpathenv(env);
	}	
//	_CrtDumpMemoryLeaks();
	return err;
}
