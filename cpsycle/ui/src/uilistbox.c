// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uilistbox.h"

static void oncommand(ui_listbox*, ui_component* sender, WPARAM wParam,
	LPARAM lParam);
static void ondestroy(ui_listbox*, ui_component* sender);
static void ui_listbox_init_style(ui_listbox* listbox, ui_component* parent,
	int style);

void ui_listbox_init(ui_listbox* listbox, ui_component* parent)
{  
	ui_listbox_init_style(listbox, parent, 
		WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_NOTIFY);    
}

void ui_listbox_init_multiselect(ui_listbox* listbox, ui_component* parent)
{  
	ui_listbox_init_style(listbox, parent, 
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_EXTENDEDSEL | LBS_NOTIFY);    
}

void ui_listbox_init_style(ui_listbox* self, ui_component* parent, int style)
{  		
	ui_win32_component_init(&self->component, parent, TEXT("LISTBOX"), 
		0, 0, 100, 200,
		style,
		1);	
	signal_connect(&self->component.signal_command, self, oncommand);
	signal_connect(&self->component.signal_destroy, self, ondestroy);	
	signal_init(&self->signal_selchanged);	
}

void ondestroy(ui_listbox* self, ui_component* sender)
{
	signal_dispose(&self->signal_selchanged);
}

intptr_t ui_listbox_addstring(ui_listbox* listbox, const char* text)
{
	return SendMessage((HWND)listbox->component.hwnd, LB_ADDSTRING, 0, (LPARAM)text);
}

void ui_listbox_setstring(ui_listbox* self, const char* text, intptr_t index)
{
	intptr_t sel;

	sel = ui_listbox_cursel(self);
	SendMessage((HWND)self->component.hwnd, LB_DELETESTRING, (WPARAM)index, (LPARAM)text);
	SendMessage((HWND)self->component.hwnd, LB_INSERTSTRING, (WPARAM)index, (LPARAM)text);
	ui_listbox_setcursel(self, sel);
}

void ui_listbox_clear(ui_listbox* listbox)
{
	SendMessage((HWND)listbox->component.hwnd, LB_RESETCONTENT, 0, (LPARAM)0);
}

void ui_listbox_setcursel(ui_listbox* listbox, intptr_t index)
{
	SendMessage((HWND)listbox->component.hwnd, LB_SETCURSEL, (WPARAM)index, (LPARAM)0);	
}

intptr_t ui_listbox_cursel(ui_listbox* listbox)
{
	return SendMessage((HWND)listbox->component.hwnd, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
}

void ui_listbox_selitems(ui_listbox* listbox, int* items, int maxitems)
{	
	SendMessage((HWND)listbox->component.hwnd, LB_GETSELITEMS, (WPARAM)maxitems,
		(LPARAM)items); 
}

intptr_t ui_listbox_selcount(ui_listbox* listbox)
{
	return SendMessage((HWND)listbox->component.hwnd, LB_GETSELCOUNT, 
		(WPARAM)0, (LPARAM)0); 
}

void oncommand(ui_listbox* self, ui_component* sender, WPARAM wParam,
	LPARAM lParam) {
	switch(HIWORD(wParam))
    {
        case LBN_SELCHANGE :
        {
            if (self->signal_selchanged.slots) {
				intptr_t sel = SendMessage((HWND)self->component.hwnd, LB_GETCURSEL,
					(WPARAM)0, (LPARAM)0);
				signal_emit(&self->signal_selchanged, self, 1, sel);
			}
        }
		break;
		default:
		break;
    }
}
