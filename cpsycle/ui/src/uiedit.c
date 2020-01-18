// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiedit.h"

static void oncommand(psy_ui_Edit*, psy_ui_Component* sender, WPARAM wParam, LPARAM lParam);
static void ondestroy(psy_ui_Edit*, psy_ui_Component* sender);
static void onpreferredsize(psy_ui_Edit*, psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_Edit* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize) onpreferredsize;
	}	vtable_initialized = 1;
}

void psy_ui_edit_init(psy_ui_Edit* self, psy_ui_Component* parent, int styles)
{  		
	ui_win32_component_init(&self->component, parent, TEXT("EDIT"), 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | ES_LEFT | styles,
		1);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_signal_connect(&self->component.signal_command, self, oncommand);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);	
	psy_signal_init(&self->signal_change);
	self->charnumber = 0;
	self->linenumber = 1;
}

void ondestroy(psy_ui_Edit* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_change);
}

void psy_ui_edit_settext(psy_ui_Edit* edit, const char* text)
{
	SetWindowText((HWND)edit->component.platform->hwnd, text);
}

void psy_ui_edit_setcharnumber(psy_ui_Edit* self, int number)
{
	self->charnumber = number;
}

void psy_ui_edit_setlinenumber(psy_ui_Edit* self, int number)
{
	self->linenumber = number;
}

const char* psy_ui_edit_text(psy_ui_Edit* edit)
{
	static char buf[256];
	GetWindowText((HWND)edit->component.platform->hwnd, buf, 255);
	return buf;
}

void oncommand(psy_ui_Edit* self, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam) {
	switch(HIWORD(wParam))
    {
        case EN_CHANGE:
        {
            if (self->signal_change.slots) {
				psy_signal_emit(&self->signal_change, self, 0);
			}
        }
		break;
		default:
		break;
    }
}

void onpreferredsize(psy_ui_Edit* self, psy_ui_Size* limit, psy_ui_Size* rv)
{			
	if (rv) {
		char text[256];
		psy_ui_TextMetric tm;
		
		tm = ui_component_textmetric(&self->component);	
		if (self->charnumber == 0) {
			psy_ui_Size size;
			GetWindowText((HWND)self->component.platform->hwnd, text, 256);
			size = ui_component_textsize(&self->component, text);	
			rv->width = size.width + 2;		
			rv->height = (int)(tm.tmHeight * self->linenumber);
		} else {				
			rv->width = tm.tmAveCharWidth * self->charnumber + 2;
			rv->height = (int)(tm.tmHeight * self->linenumber);
		}
	}
}

void psy_ui_edit_enableedit(psy_ui_Edit* self)
{
	SendMessage((HWND)self->component.platform->hwnd, EM_SETREADONLY, (WPARAM) 0,
		(LPARAM) 0);
}

void psy_ui_edit_preventedit(psy_ui_Edit* self)
{
	SendMessage((HWND)self->component.platform->hwnd, EM_SETREADONLY, (WPARAM) 1,
		(LPARAM) 0);
}
