// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uilabel.h"
#include "hashtbl.h"

extern Table selfmap;

static void onpreferredsize(ui_label*, ui_component* sender, ui_size* limit, int* width, int* height);
static TEXTMETRIC textmetric(ui_component*);

void ui_label_init(ui_label* label, ui_component* parent)
{  
	HINSTANCE hInstance;
    memset(&label->component.events, 0, sizeof(ui_events));
	ui_component_init_signals(&label->component);	
	label->component.doublebuffered = 0;
#if defined(_WIN64)		
		hInstance = (HINSTANCE) GetWindowLongPtr (parent->hwnd, GWLP_HINSTANCE);
#else
		hInstance = (HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE);
#endif
	label->component.hwnd = CreateWindow (TEXT("STATIC"), NULL,
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		0, 0, 90, 90,
		parent->hwnd, NULL,
		hInstance,
		NULL);		
	table_insert(&selfmap, (int)label->component.hwnd, &label->component);	
	label->component.events.target = label;	
	ui_component_init_base(&label->component);
	ui_component_setbackgroundmode(&label->component, BACKGROUND_SET);
	signal_disconnectall(&label->component.signal_preferredsize);
	signal_connect(&label->component.signal_preferredsize, label, onpreferredsize);
	label->charnumber = 0;
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

