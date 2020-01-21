// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uistatusbar.h"
#include <commctrl.h>
#include "uiwincompdetail.h"

void psy_ui_statusbar_init(psy_ui_StatusBar* self, psy_ui_Component* parent)
{	
   	psy_ui_win32_component_init(&self->component, parent, STATUSCLASSNAME, 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		0);
}

void psy_ui_statusbar_setfields(psy_ui_StatusBar* self, int parts,
	int iStatusWidths[])
{	
	SendMessage((HWND)psy_ui_win_component_details(&self->component)->hwnd, SB_SETPARTS, parts, 
		(LPARAM)iStatusWidths);           
}

void psy_ui_statusbar_settext(psy_ui_StatusBar* self, int field,
	const char* text)
{
	SendMessage((HWND)psy_ui_win_component_details(&self->component)->hwnd, SB_SETTEXT, field,(LPARAM)text);
}
