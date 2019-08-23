// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(UIBUTTON_H)
#define UIBUTTON_H

#include "uicomponent.h"

typedef struct {
	void (*clicked)(void* self);
} ui_buttonevents;

typedef struct {
   ui_component component;
   Signal signal_clicked;
} ui_button;

void ui_button_init(ui_button* button, ui_component* parent);
void ui_button_connect(ui_button* edit, void* target);
void ui_button_settext(ui_button* button, const char* text);

#endif