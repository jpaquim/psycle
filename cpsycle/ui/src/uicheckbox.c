// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicheckbox.h"
#include <string.h>

static void psy_ui_checkbox_oncommand(psy_ui_CheckBox*, psy_ui_Component*,
	WPARAM wParam, LPARAM lParam);
static void psy_ui_checkbox_ondestroy(psy_ui_CheckBox*, psy_ui_Component*);
static void psy_ui_checkbox_preferredsize(psy_ui_CheckBox*, ui_size* limit,
	ui_size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_CheckBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.preferredsize = (psy_ui_fp_preferredsize)
			psy_ui_checkbox_preferredsize;
	}
}

void psy_ui_checkbox_init(psy_ui_CheckBox* self, psy_ui_Component* parent)
{  	
	psy_signal_init(&self->signal_clicked);
	ui_win32_component_init(&self->component, parent, TEXT("BUTTON"), 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		1);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_signal_connect(&self->component.signal_command, self,
		psy_ui_checkbox_oncommand);
	psy_signal_connect(&self->component.signal_destroy, self,
		psy_ui_checkbox_ondestroy);	
}

void psy_ui_checkbox_ondestroy(psy_ui_CheckBox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_clicked);
}

void psy_ui_checkbox_settext(psy_ui_CheckBox* self, const char* text)
{
	SetWindowText((HWND)self->component.hwnd, text);	
}

void psy_ui_checkbox_check(psy_ui_CheckBox* self)
{
	SendMessage((HWND)self->component.hwnd, BM_SETCHECK, (WPARAM)BST_CHECKED,
		(LPARAM)0);
}

int psy_ui_checkbox_checked(psy_ui_CheckBox* self)
{
	return SendMessage((HWND)self->component.hwnd, BM_GETCHECK, (WPARAM)0,
		(LPARAM)0) != 0;	
}

void psy_ui_checkbox_disablecheck(psy_ui_CheckBox* self)
{
	SendMessage((HWND)self->component.hwnd, BM_SETCHECK, (WPARAM)0, (LPARAM)0);
}

void psy_ui_checkbox_oncommand(psy_ui_CheckBox* self, psy_ui_Component* sender,
	WPARAM wParam, LPARAM lParam)
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

void psy_ui_checkbox_preferredsize(psy_ui_CheckBox* self, ui_size* limit,
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
