// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#include "uibutton.h"
#include "hashtbl.h"

extern IntHashTable selfmap;
extern IntHashTable winidmap;
extern int winid;

static void OnCommand(ui_button* self, WPARAM wParam, LPARAM lParam);

void ui_button_init(ui_button* button, ui_component* parent)
{  
    memset(&button->component.events, 0, sizeof(ui_events));
	button->component.doublebuffered = 0;
	button->component.hwnd = CreateWindow (TEXT("BUTTON"), NULL,
		WS_CHILD | WS_VISIBLE,
		0, 0, 90, 90,
		parent->hwnd, (HMENU)winid,
		(HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE),
		NULL);		
	InsertIntHashTable(&selfmap, (int)button->component.hwnd, &button->component);	
	button->component.events.target = button;
	button->component.events.cmdtarget = button;
	InsertIntHashTable(&winidmap, (int)winid, &button->component);
	winid++;
	button->component.align = 0;
	button->component.events.command = OnCommand;
}

void ui_button_settext(ui_button* button, const char* text)
{
	SetWindowText(button->component.hwnd, text);
}

void ui_button_connect(ui_button* button, void* target)
{
	button->component.events.target = target;
}

static void OnCommand(ui_button* self, WPARAM wParam, LPARAM lParam)
{
	switch(HIWORD(wParam))
    {
        case BN_CLICKED:
        {
            if (self->events.clicked) {
				self->events.clicked(self->component.events.target);
			}
        }
		break;
		default:
		break;
    }
}

