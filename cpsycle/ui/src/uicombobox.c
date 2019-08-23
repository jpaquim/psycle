// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#include "uicombobox.h"
#include "hashtbl.h"

extern IntHashTable selfmap;
extern IntHashTable winidmap;
extern int winid;

static void OnCommand(ui_combobox* self, WPARAM wParam, LPARAM lParam);
static void OnDestroy(ui_combobox* self, ui_component* sender);

void ui_combobox_init(ui_combobox* combobox, ui_component* parent)
{  
    memset(&combobox->component.events, 0, sizeof(ui_events));
	ui_component_init_signals(&combobox->component);
	signal_init(&combobox->signal_selchanged);
	signal_connect(&combobox->component.signal_destroy, combobox, OnDestroy);
	combobox->component.doublebuffered = 0;
	combobox->component.hwnd = CreateWindow (TEXT("COMBOBOX"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_SIMPLE  | CBS_DROPDOWN,
		0, 0, 90, 90,
		parent->hwnd, (HMENU)winid,
		(HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE),
		NULL);		
	InsertIntHashTable(&selfmap, (int)combobox->component.hwnd, &combobox->component);	
	InsertIntHashTable(&winidmap, (int)winid, &combobox->component);
	combobox->component.winid = (HMENU)winid;
	winid++;	
	combobox->component.events.target = combobox;
	combobox->component.events.cmdtarget = combobox;
	combobox->component.events.command = OnCommand;
}

void OnDestroy(ui_combobox* self, ui_component* sender)
{
	signal_dispose(&self->signal_selchanged);
}

void ui_combobox_addstring(ui_combobox* combobox, const char* text)
{
	SendMessage(combobox->component.hwnd, CB_ADDSTRING, 0, (LPARAM)text);
}

void ui_combobox_clear(ui_combobox* combobox)
{
	SendMessage(combobox->component.hwnd, CB_RESETCONTENT, 0, (LPARAM)0);
}

void ui_combobox_setcursel(ui_combobox* combobox, int index)
{
	SendMessage(combobox->component.hwnd, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);
	
}

void OnCommand(ui_combobox* self, WPARAM wParam, LPARAM lParam) {
	switch(HIWORD(wParam))
    {
        case CBN_SELCHANGE :
        {
            if (self->signal_selchanged.slots) {
				int sel = SendMessage(self->component.hwnd, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				signal_emit(&self->signal_selchanged, self, 1, sel);
			}
        }
		break;
		default:
		break;
    }
}

