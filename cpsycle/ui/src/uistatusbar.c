// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uistatusbar.h"
#include <commctrl.h>

void ui_statusbar_init(ui_statusbar* self, ui_component* parent)
{	
   	ui_win32_component_init(&self->component, parent, STATUSCLASSNAME, 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		0);
}

void ui_statusbar_setfields(ui_statusbar* self, int parts, int iStatusWidths[])
{	
	SendMessage(self->component.hwnd, SB_SETPARTS, parts, 
		(LPARAM)iStatusWidths);           
}

void ui_statusbar_settext(ui_statusbar* self, int field, const char* text)
{
	SendMessage(self->component.hwnd, SB_SETTEXT, field,(LPARAM)text);
}

