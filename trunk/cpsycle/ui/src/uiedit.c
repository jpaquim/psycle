// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uiedit.h"
#include "hashtbl.h"

extern IntHashTable selfmap;
extern IntHashTable winidmap;
extern int winid;

static void oncommand(ui_edit*, ui_component* sender, WPARAM wParam, LPARAM lParam);
static void ondestroy(ui_edit*, ui_component* sender);
static void onpreferredsize(ui_edit*, ui_component* sender, ui_size* limit, int* width, int* height);

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
	signal_connect(&edit->component.signal_command, edit, oncommand);
	signal_connect(&edit->component.signal_destroy, edit, ondestroy);
	ui_component_init_base(&edit->component);
	signal_disconnectall(&edit->component.signal_preferredsize);
	signal_connect(&edit->component.signal_preferredsize, edit, onpreferredsize);
	edit->charnumber = 0;
	edit->linenumber = 1;
}

void ondestroy(ui_edit* self, ui_component* sender)
{
	signal_dispose(&self->signal_change);
}

void ui_edit_settext(ui_edit* edit, const char* text)
{
	SetWindowText(edit->component.hwnd, text);
}

void ui_edit_setcharnumber(ui_edit* self, int number)
{
	self->charnumber = number;
}

void ui_edit_setlinenumber(ui_edit* self, int number)
{
	self->linenumber = number;
}

const char* ui_edit_text(ui_edit* edit)
{
	static char buf[256];
	GetWindowText(edit->component.hwnd, buf, 255);
	return buf;
}

void oncommand(ui_edit* self, ui_component* sender, WPARAM wParam, LPARAM lParam) {
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

void onpreferredsize(ui_edit* self, ui_component* sender, ui_size* limit, int* width, int* height)
{			
	ui_size size;
	char text[256];
	TEXTMETRIC tm;
	
	tm = ui_component_textmetric(&self->component);
	GetWindowText(self->component.hwnd, text, 256);
	size = ui_component_textsize(&self->component, text);	
	
	if (self->charnumber == 0) {
		*width = size.width + 2;		
		*height = (int)(tm.tmHeight * self->linenumber * 1.5);
	} else {				
		*width = tm.tmAveCharWidth * self->charnumber + 2;
		*height = (int)(tm.tmHeight * self->linenumber * 1.5);
	}	
}
