// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#include "uilistbox.h"
#include "hashtbl.h"

extern IntHashTable selfmap;
extern IntHashTable winidmap;
extern int winid;

static void OnCommand(ui_listbox* self, WPARAM wParam, LPARAM lParam);
static void OnDestroy(ui_listbox* self, ui_component* sender);

void ui_listbox_init(ui_listbox* listbox, ui_component* parent)
{  
    memset(&listbox->component.events, 0, sizeof(ui_events));
	ui_component_init_signals(&listbox->component);	
	signal_init(&listbox->signal_selchanged);
	signal_connect(&listbox->component.signal_destroy, listbox, OnDestroy);
	listbox->component.doublebuffered = 0;
	listbox->component.hwnd = CreateWindow (TEXT("LISTBOX"), NULL,
		WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_NOTIFY,
		0, 0, 90, 90,
		parent->hwnd, (HMENU)winid,
		(HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE),
		NULL);		
	InsertIntHashTable(&selfmap, (int)listbox->component.hwnd, &listbox->component);	
	InsertIntHashTable(&winidmap, (int)winid, &listbox->component);
	listbox->component.winid = (HMENU)winid;
	winid++;	
	listbox->component.events.target = listbox;
	listbox->component.events.cmdtarget = listbox;
	listbox->component.events.command = OnCommand;
}


void OnDestroy(ui_listbox* self, ui_component* sender)
{
	signal_dispose(&self->signal_selchanged);
}


void ui_listbox_addstring(ui_listbox* listbox, const char* text)
{
	SendMessage(listbox->component.hwnd, LB_ADDSTRING, 0, (LPARAM)text);
}

void ui_listbox_clear(ui_listbox* listbox)
{
	SendMessage(listbox->component.hwnd, LB_RESETCONTENT, 0, (LPARAM)0);
}

void ui_listbox_setcursel(ui_listbox* listbox, int index)
{
	SendMessage(listbox->component.hwnd, LB_SETCURSEL, (WPARAM)index, (LPARAM)0);
	
}

int ui_listbox_cursel(ui_listbox* listbox)
{
	return SendMessage(listbox->component.hwnd, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
}

void OnCommand(ui_listbox* self, WPARAM wParam, LPARAM lParam) {
	switch(HIWORD(wParam))
    {
        case LBN_SELCHANGE :
        {
            if (self->signal_selchanged.slots) {
				int sel = SendMessage(self->component.hwnd, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				signal_emit(&self->signal_selchanged, self, 1, sel);
			}
        }
		break;
		default:
		break;
    }
}




