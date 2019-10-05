// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uislider.h"
#include "hashtbl.h"
#include <commctrl.h>
#include <stdio.h>

extern IntHashTable selfmap;
extern IntHashTable winidmap;
extern winid_t winid;

static void oncommand(ui_slider*, ui_component* sender, WPARAM wParam, LPARAM lParam);
static void ondestroy(ui_slider*, ui_component* sender);
static void onwindowproc(ui_slider*, ui_component* sender, int message,
	WPARAM wParam, LPARAM lParam);

void ui_slider_init(ui_slider* slider, ui_component* parent)
{
	HINSTANCE hInstance;
    
#if defined(_WIN64)
		hInstance = (HINSTANCE) GetWindowLongPtr (parent->hwnd, GWLP_HINSTANCE);
#else
		hInstance = (HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE);
#endif
    memset(&slider->component.events, 0, sizeof(ui_events));	
	ui_component_init_signals(&slider->component);
	signal_init(&slider->signal_clicked);
	signal_init(&slider->signal_changed);
	slider->component.doublebuffered = 0;
	slider->component.hwnd = CreateWindow (TRACKBAR_CLASS, NULL,
		WS_CHILD | WS_VISIBLE,
		0, 0, 90, 90,
		parent->hwnd, (HMENU)winid,
		hInstance,
		NULL);		
	InsertIntHashTable(&selfmap, (int)slider->component.hwnd, &slider->component);	
	slider->component.events.target = slider;
	slider->component.events.cmdtarget = slider;
	InsertIntHashTable(&winidmap, (int)winid, &slider->component);
	winid++;		
	ui_component_init_base(&slider->component);
	signal_connect(&slider->component.signal_destroy, slider,  ondestroy);
	signal_connect(&slider->component.signal_windowproc, slider,  onwindowproc);
	signal_connect(&slider->component.signal_command, slider, oncommand);
	ui_component_setbackgroundmode(&slider->component, BACKGROUND_SET);
}

void ondestroy(ui_slider* self, ui_component* sender)
{
	signal_dispose(&self->signal_clicked);
	signal_dispose(&self->signal_changed);
}

void ui_slider_settext(ui_slider* slider, const char* text)
{
	SetWindowText(slider->component.hwnd, text);
}

void ui_slider_setrange(ui_slider* self, int minrange, int maxrange)
{	
	SendMessage(self->component.hwnd, TBM_SETRANGE, TRUE, MAKELONG(minrange, maxrange));	
}

void ui_slider_setvalue(ui_slider* self, int value)
{
	SendMessage(self->component.hwnd, TBM_SETPOS, TRUE, value);
}

int ui_slider_value(ui_slider* self)
{
	return SendMessage(self->component.hwnd, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
}

void onwindowproc(ui_slider* self, ui_component* sender, int message, WPARAM wParam, LPARAM lParam)
{	
	switch (message) {
		case WM_VSCROLL:
		case WM_HSCROLL:            
			switch(LOWORD(wParam))
			{
				case TB_BOTTOM:					
					break;
	
				case TB_ENDTRACK:					
					break;

				case TB_LINEDOWN:					
					break;

				case TB_LINEUP:					
					break;

				case TB_PAGEDOWN:					
					break;

				case TB_PAGEUP:					
					break;

				case TB_THUMBPOSITION:
					break;

				case TB_THUMBTRACK:					
					signal_emit(&self->signal_changed, sender, 0);
					break;					
				default:
					break;
			}			
			break;
			default:
			break;
	}
}

void oncommand(ui_slider* self, ui_component* sender, WPARAM wParam, LPARAM lParam)
{
	
}

