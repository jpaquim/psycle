// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicheckbox.h"
#include <string.h>

static void oncommand(ui_checkbox*, psy_ui_Component*, WPARAM wParam,
	LPARAM lParam);
static void ondestroy(ui_checkbox*, psy_ui_Component*);
static void onpreferredsize(ui_checkbox*, psy_ui_Component* sender, ui_size* limit,
	ui_size* rv);

void ui_checkbox_init(ui_checkbox* self, psy_ui_Component* parent)
{  	
	psy_signal_init(&self->signal_clicked);
	ui_win32_component_init(&self->component, parent, TEXT("BUTTON"), 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		1);
	psy_signal_connect(&self->component.signal_command, self, oncommand);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	psy_signal_disconnectall(&self->component.signal_preferredsize);	
	psy_signal_connect(&self->component.signal_preferredsize, self,
		onpreferredsize);
}

void ondestroy(ui_checkbox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_clicked);
}

void ui_checkbox_settext(ui_checkbox* self, const char* text)
{
	SetWindowText((HWND)self->component.hwnd, text);	
}

void ui_checkbox_check(ui_checkbox* self)
{
	SendMessage((HWND)self->component.hwnd, BM_SETSTATE, (WPARAM)1, (LPARAM)0);
}

int ui_checkbox_checked(ui_checkbox* self)
{
	int rv;
	
	rv = SendMessage((HWND)self->component.hwnd, BM_GETSTATE, (WPARAM)0, (LPARAM)0);
	return rv;
}

void ui_checkbox_disablecheck(ui_checkbox* self)
{
	SendMessage((HWND)self->component.hwnd, BM_SETSTATE, (WPARAM)0, (LPARAM)0);
}

void oncommand(ui_checkbox* self, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam)
{
	switch(HIWORD(wParam))
    {
        case BN_CLICKED:
        {            
			if (self->signal_clicked.slots) {
				psy_signal_emit(&self->signal_clicked, self, 0);
			}
        }
		break;
		default:
		break;
    }
}

void onpreferredsize(ui_checkbox* self, psy_ui_Component* sender, ui_size* limit,
	ui_size* rv)
{	
	if (rv) {
		ui_size size;	
		char text[256];

		GetWindowText((HWND)self->component.hwnd, text, 256);
		size = ui_component_textsize(&self->component, text);	
		rv->width = size.width + 20;
		rv->height = size.height + 4;
	}
}
