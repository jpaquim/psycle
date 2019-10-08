// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uiframe.h"
#include "hashtbl.h"

extern TCHAR szAppClass[];

void ui_frame_init(ui_frame* self, ui_component* parent)
{			
	ui_win32_component_init(self, parent, szAppClass, 
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WS_OVERLAPPEDWINDOW,
		0);
	ui_component_setbackgroundmode(self, BACKGROUND_NONE);
}
