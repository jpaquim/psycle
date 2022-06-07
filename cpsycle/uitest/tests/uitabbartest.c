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

int psycle_run(uintptr_t instance, int options)
{
	int err = 0;
	char workpath[_MAX_PATH];
	const char* env = 0;
	psy_ui_App app;
	psy_ui_Frame mainframe;
	psy_ui_TabBar tabbar;
		
	psy_ui_app_init(&app, psy_ui_DARKTHEME, instance);
	// Creates the mainframe
	psy_ui_frame_init(&mainframe, NULL);
	app.main = &mainframe;
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
	return err;
}
