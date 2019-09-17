// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uistatusbar.h"
#include "hashtbl.h"
#include <commctrl.h>

extern IntHashTable selfmap;

void ui_statusbar_init(ui_statusbar* statusbar, ui_component* parent)
{  
    memset(&statusbar->component.events, 0, sizeof(ui_events));
	ui_component_init_signals(&statusbar->component);	
	statusbar->component.doublebuffered = 0;
	statusbar->component.hwnd = CreateWindowEx(
        0,
        STATUSCLASSNAME,
        NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0,
        0,
        0,
        0,
        parent->hwnd,
        (HMENU)50000,
        (HINSTANCE)GetWindowLong(parent->hwnd, GWL_HINSTANCE),
        NULL);

    if (! statusbar->component.hwnd)
    {
        MessageBox(NULL, "Failed To Create The Status Bar", "Error", MB_OK | MB_ICONERROR);
    }
	InsertIntHashTable(&selfmap, (int)statusbar->component.hwnd, &statusbar->component);	
	statusbar->component.events.target = statusbar;
	ui_component_init_base(&statusbar->component);
}

void ui_statusbar_setfields(ui_statusbar* self, int parts, int iStatusWidths[])
{	
	SendMessage(self->component.hwnd, SB_SETPARTS, parts, (LPARAM)iStatusWidths);           
}

void ui_statusbar_settext(ui_statusbar* self, int field, const char* text)
{
	SendMessage(self->component.hwnd, SB_SETTEXT, field,(LPARAM)text);
}
