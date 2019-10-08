// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uicheckbox.h"
#include <string.h>

static void oncommand(ui_checkbox*, ui_component*, WPARAM wParam,
	LPARAM lParam);
static void ondestroy(ui_checkbox*, ui_component*);
static void onpreferredsize(ui_checkbox*, ui_component* sender, ui_size* limit,
	int* width, int* height);

void ui_checkbox_init(ui_checkbox* self, ui_component* parent)
{  	
	signal_init(&self->signal_clicked);	
	ui_win32_component_init(&self->component, parent, TEXT("BUTTON"), 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		1);
	signal_connect(&self->component.signal_destroy, self,  ondestroy);	
	signal_connect(&self->component.signal_preferredsize, self,
		onpreferredsize);
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

void oncommand(ui_checkbox* self, ui_component* sender, WPARAM wParam,
	LPARAM lParam)
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

void onpreferredsize(ui_checkbox* self, ui_component* sender, ui_size* limit,
	int* width, int* height)
{			
	ui_size size;	
	char text[256];

	GetWindowText(self->component.hwnd, text, 256);
	size = ui_component_textsize(&self->component, text);	
	*width = size.width + 20;
	*height = size.height + 4;	
}
