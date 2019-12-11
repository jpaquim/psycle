// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UICHECKBOX_H)
#define UICHECKBOX_H

#include "uicomponent.h"

typedef struct {
   ui_component component;
   psy_Signal signal_clicked;   
} ui_checkbox;

void ui_checkbox_init(ui_checkbox*, ui_component* parent);
void ui_checkbox_settext(ui_checkbox*, const char* text);
void ui_checkbox_check(ui_checkbox*);
void ui_checkbox_disablecheck(ui_checkbox*);
int ui_checkbox_checked(ui_checkbox*);

#endif
