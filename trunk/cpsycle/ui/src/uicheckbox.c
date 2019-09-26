// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uicheckbox.h"
#include "hashtbl.h"
#include <string.h>

extern IntHashTable selfmap;
extern IntHashTable winidmap;
extern int winid;

static void OnCommand(ui_checkbox* self, WPARAM wParam, LPARAM lParam);
static void ondestroy(ui_checkbox* self, ui_component* sender);
static void ui_checkbox_create_system(ui_checkbox*, ui_component* parent);

void ui_checkbox_init(ui_checkbox* checkbox, ui_component* parent)
{  
	memset(&checkbox->component.events, 0, sizeof(ui_events));	
	checkbox->hover = 0;
	ui_component_init_signals(&checkbox->component);
	signal_init(&checkbox->signal_clicked);
	checkbox->component.doublebuffered = 0;
	ui_checkbox_create_system(checkbox, parent);
	signal_connect(&checkbox->component.signal_destroy, checkbox,  ondestroy);
	ui_component_init_base(&checkbox->component);
}

void ui_checkbox_create_system(ui_checkbox* checkbox, ui_component* parent)
{    	
	checkbox->ownerdrawn = 0;
	checkbox->text = 0;
	checkbox->component.hwnd = CreateWindow (TEXT("BUTTON"), NULL,
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		0, 0, 90, 90,
		parent->hwnd, (HMENU)winid,
		(HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE),
		NULL);		
	InsertIntHashTable(&selfmap, (int)checkbox->component.hwnd, &checkbox->component);	
	checkbox->component.events.target = checkbox;
	checkbox->component.events.cmdtarget = checkbox;
	InsertIntHashTable(&winidmap, (int)winid, &checkbox->component);
	winid++;	
	checkbox->component.events.command = OnCommand;
}

void ondestroy(ui_checkbox* self, ui_component* sender)
{
	signal_dispose(&self->signal_clicked);
}

void ui_checkbox_settext(ui_checkbox* self, const char* text)
{
	SetWindowText(self->component.hwnd, text);	
}

void ui_checkbox_check(ui_checkbox* self)
{
	SendMessage(self->component.hwnd, BM_SETSTATE, (WPARAM)1, (LPARAM)0);
}

void ui_checkbox_disablecheck(ui_checkbox* self)
{
	SendMessage(self->component.hwnd, BM_SETSTATE, (WPARAM)0, (LPARAM)0);
}

static void OnCommand(ui_checkbox* self, WPARAM wParam, LPARAM lParam)
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

