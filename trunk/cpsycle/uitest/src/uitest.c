// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

// platform
#include "../../detail/prefix.h"
#include "../../detail/os.h"
// host
#include "uiframe.h"
#include "uilabel.h"
// ui
#include <uiapp.h>
// file
#include <dir.h>
#include <signal.h>
// std
#include <stdio.h>

typedef struct Pane {
	psy_ui_Component component;
} Pane;

static void pane_init(Pane* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
}

static void pane_createtest(Pane* self)
{
	psy_ui_Component* component;

	component = psy_ui_component_allocinit(&self->component, NULL);
	psy_ui_component_setalign(component, psy_ui_ALIGN_TOP);
}


int psycle_run(uintptr_t instance, int options)
{
	int err = 0;
	psy_ui_App app;
	psy_ui_Frame mainframe;
	Pane pane;

	// Initialize the ui
	psy_ui_app_init(&app, psy_ui_DARKTHEME, instance);
	// Creates the mainframe
	psy_ui_frame_init(&mainframe, NULL);
	app.main = &mainframe;
	pane_init(&pane, &mainframe);
	psy_ui_component_setalign(&pane.component, psy_ui_ALIGN_CLIENT);
	pane_createtest(&pane);
	psy_ui_component_clear(&pane);
	psy_ui_component_showstate(&mainframe, options);
	// Starts the app event loop
	err = psy_ui_app_run(&app);
	printf("Loop finished\n");
	// The event loop has finished, dispose any global ui resources
	psy_ui_app_dispose(&app);
	return err;
}
