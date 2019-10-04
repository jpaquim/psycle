// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "mainframe.h"
#include <ui_app.h>
#include <dir.h>
#include <stdio.h>


// #define _CRTDBG_MAP_ALLOC
// #include <crtdbg.h>

void precision(void)
{
	int line;
	double bpl;
	double offset;
	double ctrl;

	bpl = 1.0 / 9;
	offset = 0;
	for (line = 0; line < 10; ++line) {
		ctrl = line * bpl;
		offset += bpl;		
	}

}

UIMAIN
{    	
	MainFrame main;			
	int err;
	char workpath[_MAX_PATH];
	const char* env = 0;	

	env = pathenv();	
	if (env) {			
		insertpathenv(workdir(workpath));
	}
	UIINIT;		
	precision();
	InitMainFrame(&main);	
	ui_component_show_state(&main.component, iCmdShow);		 	 
	err = ui_run();	
	UIDISPOSE;
	if (env) {
		setpathenv(env);
	}
	//  _CrtDumpMemoryLeaks( );
	return err;
}
