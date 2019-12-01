// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uilabel.h"

static void onpreferredsize(ui_label*, ui_component* sender, ui_size* limit, ui_size* rv);
static ui_textmetric textmetric(ui_component*);

void ui_label_init(ui_label* self, ui_component* parent)
{  		
	ui_win32_component_init(&self->component, parent, TEXT("STATIC"), 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
		0);	
	signal_disconnectall(&self->component.signal_preferredsize);
	signal_connect(&self->component.signal_preferredsize, self, onpreferredsize);
	self->charnumber = 0;	
}

void ui_label_settext(ui_label* label, const char* text)
{
	SetWindowText((HWND)label->component.hwnd, text);	
}

void ui_label_setcharnumber(ui_label* self, int number)
{
	self->charnumber = number;
}

void onpreferredsize(ui_label* self, ui_component* sender, ui_size* limit, ui_size* rv)
{	
	if (rv) {
		ui_textmetric tm;	
		char text[256];
		
		tm = ui_component_textmetric(&self->component);	
		if (self->charnumber == 0) {
			ui_size size;
			GetWindowText((HWND)self->component.hwnd, text, 256);
			size = ui_component_textsize(&self->component, text);
			rv->width = size.width + 2;
		} else {		
			rv->width = tm.tmAveCharWidth * self->charnumber;
		}
		rv->height = tm.tmHeight;
	}
}

void ui_label_setstyle(ui_label* self, int style)
{
	#if defined(_WIN64)
	SetWindowLongPtr((HWND)self->component.hwnd, GWL_STYLE, style);		
#else
	SetWindowLong((HWND)self->component.hwnd, GWL_STYLE, style);
#endif
}

