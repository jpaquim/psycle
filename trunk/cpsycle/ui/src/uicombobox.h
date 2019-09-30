// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(UICOMBOBOX)
#define UICOMBOBOX

#include "uicomponent.h"

typedef struct {
   ui_component component;
   ui_component combo;
   ui_component* currcombo;
   Signal signal_selchanged;
   int ownerdrawn;   
   int hover;
   int charnumber;
} ui_combobox;

void ui_combobox_init(ui_combobox* combobox, ui_component* parent);
int ui_combobox_addstring(ui_combobox* combobox, const char* text);
void ui_combobox_clear(ui_combobox* combobox);
void ui_combobox_setcursel(ui_combobox* combobox, int index);
int ui_combobox_cursel(ui_combobox* self);
void ui_combobox_setcharnumber(ui_combobox*, int num);

#endif