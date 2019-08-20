// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uiedit.h"
#include "hashtbl.h"

extern IntHashTable selfmap;
extern IntHashTable winidmap;
extern int winid;

static void OnCommand(ui_edit* self, WPARAM wParam, LPARAM lParam);
static void OnDestroy(ui_edit* self, ui_component* sender);

void ui_edit_init(ui_edit* edit, ui_component* parent, int styles)
{  
    memset(&edit->component.events, 0, sizeof(ui_events));	
	ui_component_init_signals(&edit->component);	
	signal_init(&edit->signal_change);
	edit->component.doublebuffered = 0;
	edit->component.hwnd = CreateWindow (TEXT("EDIT"), NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | styles,
		0, 0, 90, 90,
		parent->hwnd, 
		(HMENU)winid,
		(HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE),
		NULL);		
	InsertIntHashTable(&selfmap, (int)edit->component.hwnd, &edit->component);	
	InsertIntHashTable(&winidmap, (int)winid, &edit->component);
	winid++;
	edit->component.align = 0;
	edit->component.events.target = edit;
	edit->component.events.cmdtarget = edit;
	edit->component.events.command = OnCommand;
	signal_connect(&edit->component.signal_destroy, edit,  OnDestroy);
}

void OnDestroy(ui_edit* self, ui_component* sender)
{
	signal_dispose(&self->signal_change);
}

void ui_edit_connect(ui_edit* edit, void* target)
{
	edit->component.events.target = target;
}

void ui_edit_settext(ui_edit* edit, const char* text)
{
	SetWindowText(edit->component.hwnd, text);
}

const char* ui_edit_text(ui_edit* edit)
{
	static char buf[256];
	GetWindowText(edit->component.hwnd, buf, 255);
	return buf;
}

void OnCommand(ui_edit* self, WPARAM wParam, LPARAM lParam) {
	switch(HIWORD(wParam))
    {
        case EN_CHANGE:
        {
            if (self->signal_change.slots) {
				signal_emit(&self->signal_change, self, 0);				
			}
        }
		break;
		default:
		break;
    }
}