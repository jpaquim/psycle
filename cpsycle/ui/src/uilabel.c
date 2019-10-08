// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uilabel.h"

static void onpreferredsize(ui_label*, ui_component* sender, ui_size* limit, int* width, int* height);
static TEXTMETRIC textmetric(ui_component*);

void ui_label_init(ui_label* self, ui_component* parent)
{  		
	ui_win32_component_init(&self->component, parent, TEXT("STATIC"), 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		0);	
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	signal_disconnectall(&self->component.signal_preferredsize);
	signal_connect(&self->component.signal_preferredsize, self, onpreferredsize);
	self->charnumber = 0;	
}

void ui_label_settext(ui_label* label, const char* text)
{
	SetWindowText(label->component.hwnd, text);
}

void ui_label_setcharnumber(ui_label* self, int number)
{
	self->charnumber = number;
}

void onpreferredsize(ui_label* self, ui_component* sender, ui_size* limit, int* width, int* height)
{			
	ui_size size;
	char text[256];

	GetWindowText(self->component.hwnd, text, 256);
	size = ui_component_textsize(&self->component, text);
	if (self->charnumber == 0) {
		*width = size.width + 2;
	} else {
		TEXTMETRIC tm;
		tm = ui_component_textmetric(&self->component);
		*width = tm.tmAveCharWidth * self->charnumber;
	}
	*height = size.height;
}

