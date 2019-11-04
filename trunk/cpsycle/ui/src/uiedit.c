// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiedit.h"

static void oncommand(ui_edit*, ui_component* sender, WPARAM wParam, LPARAM lParam);
static void ondestroy(ui_edit*, ui_component* sender);
static void onpreferredsize(ui_edit*, ui_component* sender, ui_size* limit,
	ui_size* rv);

void ui_edit_init(ui_edit* self, ui_component* parent, int styles)
{  		
	ui_win32_component_init(&self->component, parent, TEXT("EDIT"), 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | ES_LEFT | styles,
		1);
	signal_connect(&self->component.signal_command, self, oncommand);
	signal_connect(&self->component.signal_destroy, self, ondestroy);	
	signal_disconnectall(&self->component.signal_preferredsize);
	signal_connect(&self->component.signal_preferredsize, self, onpreferredsize);
	signal_init(&self->signal_change);
	self->charnumber = 0;
	self->linenumber = 1;
}

void ondestroy(ui_edit* self, ui_component* sender)
{
	signal_dispose(&self->signal_change);
}

void ui_edit_settext(ui_edit* edit, const char* text)
{
	SetWindowText((HWND)edit->component.hwnd, text);
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
	GetWindowText((HWND)edit->component.hwnd, buf, 255);
	return buf;
}

void oncommand(ui_edit* self, ui_component* sender, WPARAM wParam,
	LPARAM lParam) {
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

void onpreferredsize(ui_edit* self, ui_component* sender, ui_size* limit,
	ui_size* rv)
{			
	if (rv) {
		char text[256];
		TEXTMETRIC tm;
		
		tm = ui_component_textmetric(&self->component);	
		if (self->charnumber == 0) {
			ui_size size;
			GetWindowText((HWND)self->component.hwnd, text, 256);
			size = ui_component_textsize(&self->component, text);	
			rv->width = size.width + 2;		
			rv->height = (int)(tm.tmHeight * self->linenumber * 1.5);
		} else {				
			rv->width = tm.tmAveCharWidth * self->charnumber + 2;
			rv->height = (int)(tm.tmHeight * self->linenumber * 1.5);
		}
	}
}

void ui_edit_enableedit(ui_edit* self)
{
	SendMessage((HWND)self->component.hwnd, EM_SETREADONLY, (WPARAM)0, (LPARAM)0);
}

void ui_edit_preventedit(ui_edit* self)
{
	SendMessage((HWND)self->component.hwnd, EM_SETREADONLY, (WPARAM)1, (LPARAM)0);
}
