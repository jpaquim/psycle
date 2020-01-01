// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiframe.h"
#include "hashtbl.h"
#include "uiapp.h"
#include "uiwinapp.h"

extern psy_ui_App app;

void ui_frame_init(ui_frame* self, psy_ui_Component* parent)
{			
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*) app.platform;
	ui_win32_component_init(self, parent, winapp->appclass, 
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WS_OVERLAPPEDWINDOW,
		0);
	ui_component_setbackgroundmode(self, BACKGROUND_NONE);
}
