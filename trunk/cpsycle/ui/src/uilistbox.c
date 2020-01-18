// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uilistbox.h"

static void oncommand(psy_ui_Listbox*, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam);
static void ondestroy(psy_ui_Listbox*, psy_ui_Component* sender);
static void ui_listbox_init_style(psy_ui_Listbox* listbox, psy_ui_Component* parent,
	int style);

void ui_listbox_init(psy_ui_Listbox* listbox, psy_ui_Component* parent)
{  
	ui_listbox_init_style(listbox, parent, 
		(WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_NOTIFY) &~WS_BORDER);	
}

void ui_listbox_init_multiselect(psy_ui_Listbox* listbox, psy_ui_Component* parent)
{  
	ui_listbox_init_style(listbox, parent, 
		(WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_EXTENDEDSEL | LBS_NOTIFY)
		&~WS_BORDER);    
}

void ui_listbox_init_style(psy_ui_Listbox* self, psy_ui_Component* parent, int style)
{  		
	ui_win32_component_init(&self->component, parent, TEXT("LISTBOX"), 
		0, 0, 100, 200,
		style,
		1);	
	psy_signal_connect(&self->component.signal_command, self, oncommand);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);	
	psy_signal_init(&self->signal_selchanged);	
}

void ondestroy(psy_ui_Listbox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_selchanged);
}

intptr_t ui_listbox_addstring(psy_ui_Listbox* listbox, const char* text)
{
	return SendMessage((HWND)listbox->component.platform->hwnd, LB_ADDSTRING, 0, (LPARAM)text);
}

void ui_listbox_setstring(psy_ui_Listbox* self, const char* text, intptr_t index)
{
	intptr_t sel;

	sel = ui_listbox_cursel(self);
	SendMessage((HWND)self->component.platform->hwnd, LB_DELETESTRING, (WPARAM)index, (LPARAM)text);
	SendMessage((HWND)self->component.platform->hwnd, LB_INSERTSTRING, (WPARAM)index, (LPARAM)text);
	ui_listbox_setcursel(self, sel);
}

void ui_listbox_clear(psy_ui_Listbox* listbox)
{
	SendMessage((HWND)listbox->component.platform->hwnd, LB_RESETCONTENT, 0, (LPARAM)0);
}

void ui_listbox_setcursel(psy_ui_Listbox* listbox, intptr_t index)
{
	SendMessage((HWND)listbox->component.platform->hwnd, LB_SETCURSEL, (WPARAM)index, (LPARAM)0);	
}

intptr_t ui_listbox_cursel(psy_ui_Listbox* listbox)
{
	return SendMessage((HWND)listbox->component.platform->hwnd, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
}

void ui_listbox_selitems(psy_ui_Listbox* listbox, int* items, int maxitems)
{	
	SendMessage((HWND)listbox->component.platform->hwnd, LB_GETSELITEMS, (WPARAM)maxitems,
		(LPARAM)items); 
}

intptr_t ui_listbox_selcount(psy_ui_Listbox* listbox)
{
	return SendMessage((HWND)listbox->component.platform->hwnd, LB_GETSELCOUNT, 
		(WPARAM)0, (LPARAM)0); 
}

void oncommand(psy_ui_Listbox* self, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam) {
	switch(HIWORD(wParam))
    {
        case LBN_SELCHANGE :
        {
            if (self->signal_selchanged.slots) {
				intptr_t sel = SendMessage((HWND)self->component.platform->hwnd,
					LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				psy_signal_emit(&self->signal_selchanged, self, 1, sel);
			}
        }
		break;
		default:
		break;
    }
}
