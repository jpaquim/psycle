// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#include "uibutton.h"
#include "hashtbl.h"

extern IntHashTable selfmap;
extern IntHashTable winidmap;
extern int winid;

static void OnCommand(ui_button* self, WPARAM wParam, LPARAM lParam);
static void OnDestroy(ui_button* self, ui_component* sender);

void ui_button_init(ui_button* button, ui_component* parent)
{  
    memset(&button->component.events, 0, sizeof(ui_events));	
	ui_component_init_signals(&button->component);
	signal_init(&button->signal_clicked);
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
	button->component.events.command = OnCommand;
	signal_connect(&button->component.signal_destroy, button,  OnDestroy);
	ui_component_init_base(&button->component);
}

void OnDestroy(ui_button* self, ui_component* sender)
{
	signal_dispose(&self->signal_clicked);
}

void ui_button_settext(ui_button* self, const char* text)
{
	SetWindowText(self->component.hwnd, text);
}

void ui_button_highlight(ui_button* self)
{
	SendMessage(self->component.hwnd, BM_SETSTATE, (WPARAM)1, (LPARAM)0);
}

void ui_button_disablehighlight(ui_button* self)
{
	SendMessage(self->component.hwnd, BM_SETSTATE, (WPARAM)0, (LPARAM)0);
}

static void OnCommand(ui_button* self, WPARAM wParam, LPARAM lParam)
{
	switch(HIWORD(wParam))
    {
        case BN_CLICKED:
        {            
			if (self->signal_clicked.slots) {
				signal_emit(&self->signal_clicked, self, 0);				
			}
        }
		break;
		default:
		break;
    }
}

