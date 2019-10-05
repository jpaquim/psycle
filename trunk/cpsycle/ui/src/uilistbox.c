// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#include "uilistbox.h"
#include "hashtbl.h"

extern IntHashTable selfmap;
extern IntHashTable winidmap;
extern winid_t winid;

static void oncommand(ui_listbox*, ui_component* sender, WPARAM wParam, LPARAM lParam);
static void ondestroy(ui_listbox*, ui_component* sender);
static void ui_listbox_init_style(ui_listbox* listbox, ui_component* parent, int style);

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

void ui_listbox_init_style(ui_listbox* listbox, ui_component* parent, int style)
{  
	HINSTANCE hInstance;
    
#if defined(_WIN64)
		hInstance = (HINSTANCE) GetWindowLongPtr (parent->hwnd, GWLP_HINSTANCE);
#else
		hInstance = (HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE);
#endif
	memset(&listbox->component.events, 0, sizeof(ui_events));
	ui_component_init_signals(&listbox->component);		
	signal_init(&listbox->signal_selchanged);
	signal_connect(&listbox->component.signal_destroy, listbox, ondestroy);
	listbox->component.doublebuffered = 0;
	listbox->component.hwnd = CreateWindow (TEXT("LISTBOX"), NULL,
		style,
		0, 0, 90, 90,
		parent->hwnd, (HMENU)winid,
		hInstance,
		NULL);		
	InsertIntHashTable(&selfmap, (int)listbox->component.hwnd, &listbox->component);	
	InsertIntHashTable(&winidmap, (int)winid, &listbox->component);
	listbox->component.winid = (HMENU)winid;
	winid++;		
	ui_component_init_base(&listbox->component);
	signal_connect(&listbox->component.signal_command, listbox, oncommand);
	ui_component_setbackgroundmode(&listbox->component, BACKGROUND_SET);
}

void ondestroy(ui_listbox* self, ui_component* sender)
{
	signal_dispose(&self->signal_selchanged);
}

int ui_listbox_addstring(ui_listbox* listbox, const char* text)
{
	return SendMessage(listbox->component.hwnd, LB_ADDSTRING, 0, (LPARAM)text);
}

void ui_listbox_setstring(ui_listbox* self, const char* text, int index)
{
	int sel;

	sel = ui_listbox_cursel(self);
	SendMessage(self->component.hwnd, LB_DELETESTRING, (WPARAM)index, (LPARAM)text);
	SendMessage(self->component.hwnd, LB_INSERTSTRING, (WPARAM)index, (LPARAM)text);
	ui_listbox_setcursel(self, sel);
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

void ui_listbox_selitems(ui_listbox* listbox, int* items, int maxitems)
{	
	SendMessage(listbox->component.hwnd, LB_GETSELITEMS, (WPARAM)maxitems,
		(LPARAM)items); 
}

int ui_listbox_selcount(ui_listbox* listbox)
{
	return SendMessage(listbox->component.hwnd, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0); 
}

void oncommand(ui_listbox* self, ui_component* sender, WPARAM wParam, LPARAM lParam) {
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




