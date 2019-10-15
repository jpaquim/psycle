// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uislider.h"
#include <commctrl.h>
#include <stdio.h>

static void ondestroy(ui_slider*, ui_component* sender);
static void onwindowproc(ui_slider*, ui_component* sender, int message,
	WPARAM wParam, LPARAM lParam);

void ui_slider_init(ui_slider* self, ui_component* parent)
{	
	ui_win32_component_init(&self->component, parent, TRACKBAR_CLASS, 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE,
		0);
	signal_init(&self->signal_clicked);
	signal_init(&self->signal_changed);	
	signal_connect(&self->component.signal_destroy, self, ondestroy);
	signal_connect(&self->component.signal_windowproc, self, onwindowproc);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
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
	SendMessage(self->component.hwnd, TBM_SETRANGE, TRUE,
		MAKELONG(minrange, maxrange));
}

void ui_slider_setvalue(ui_slider* self, int value)
{
	SendMessage(self->component.hwnd, TBM_SETPOS, TRUE, value);
}

int ui_slider_value(ui_slider* self)
{
	return SendMessage(self->component.hwnd, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
}

void onwindowproc(ui_slider* self, ui_component* sender, int message,
	WPARAM wParam, LPARAM lParam)
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
