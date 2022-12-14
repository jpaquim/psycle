/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* host */
#include "bitmaps.h"
#include "mainframe.h"
/* ui */
#include <uiapp.h>
/* file */
#include <dir.h>

#ifdef DIVERSALIS__OS__MICROSOFT

#else
#define _MAX_PATH 4096
#endif

int psycle_run(uintptr_t instance, int options)
{
	int err = 0;
	char workpath[_MAX_PATH];	
	psy_ui_App app;
	MainFrame* mainframe;
		
	/* set env for scintilla module */
	psy_insertpathenv(psy_workdir(workpath));
	psy_ui_app_init(&app, psy_ui_DARKTHEME, instance);
#ifdef DIVERSALIS__OS__UNIX
	register_bitmaps(&app, PSYCLE_RES_DIR);		
#endif
	mainframe = mainframe_allocinit();	
	if (mainframe) {	
		if (mainframe_showmaximizedatstart(mainframe)) {
			psy_ui_component_showmaximized(mainframe_base(mainframe));
		} else {
			psy_ui_component_showstate(mainframe_base(mainframe), options);
		}
		mainframe->mainviews.machineview.wireview.pane.centermaster = TRUE;
		err = psy_ui_app_run(&app); /* event loop start */
	}	
	psy_ui_app_dispose(&app); /* clean up */
	return err;
}
