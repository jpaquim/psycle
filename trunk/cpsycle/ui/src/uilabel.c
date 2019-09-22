// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#include "uilabel.h"
#include "hashtbl.h"

extern IntHashTable selfmap;

void ui_label_init(ui_label* label, ui_component* parent)
{  
    memset(&label->component.events, 0, sizeof(ui_events));
	ui_component_init_signals(&label->component);	
	label->component.doublebuffered = 0;
	label->component.hwnd = CreateWindow (TEXT("STATIC"), NULL,
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		0, 0, 90, 90,
		parent->hwnd, NULL,
		(HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE),
		NULL);		
	InsertIntHashTable(&selfmap, (int)label->component.hwnd, &label->component);	
	label->component.events.target = label;	
	ui_component_init_base(&label->component);
	ui_component_setbackgroundmode(&label->component, BACKGROUND_SET);
	ui_component_setbackgroundcolor(&label->component, 0x000000);
}


void ui_label_settext(ui_label* label, const char* text)
{
	SetWindowText(label->component.hwnd, text);
}

