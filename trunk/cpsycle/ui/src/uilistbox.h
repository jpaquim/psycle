// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(UILISTBOX)
#define UILISTBOX

#include "uicomponent.h"

typedef struct {
   ui_component component;
} ui_listbox;


void ui_listbox_init(void* self, ui_listbox* listbox, ui_component* parent);
void ui_listbox_addstring(ui_listbox* listbox, const char* text);

#endif