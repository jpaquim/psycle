// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uigroupbox.h"
#include "hashtbl.h"

extern IntHashTable selfmap;

void ui_groupbox_init(ui_groupbox* groupbox, ui_component* parent)
{  
    memset(&groupbox->component.events, 0, sizeof(ui_events));
	ui_component_init_signals(&groupbox->component);	
	groupbox->component.doublebuffered = 0;
	groupbox->component.hwnd = CreateWindow (TEXT("BUTTON"), NULL,
		WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
		0, 0, 90, 90,
		parent->hwnd, NULL,
		(HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE),
		NULL);		
	InsertIntHashTable(&selfmap, (int)groupbox->component.hwnd, &groupbox->component.events);	
	groupbox->component.events.target = groupbox;
	groupbox->component.events.cmdtarget = groupbox;
}


void ui_groupbox_settext(ui_groupbox* groupbox, const char* text)
{
	SetWindowText(groupbox->component.hwnd, text);
}