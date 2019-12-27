// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UICOMBOBOX_H)
#define UICOMBOBOX_H

#include "uicomponent.h"

typedef struct {
   psy_ui_Component component;
   psy_ui_Component combo;
   psy_ui_Component* currcombo;
   psy_Signal signal_selchanged;
   int ownerdrawn;   
   int hover;
   int charnumber;
} ui_combobox;

void ui_combobox_init(ui_combobox*, psy_ui_Component* parent);
int ui_combobox_addstring(ui_combobox*, const char* text);
void ui_combobox_clear(ui_combobox*);
void ui_combobox_setcursel(ui_combobox*, intptr_t index);
intptr_t ui_combobox_cursel(ui_combobox*);
void ui_combobox_setcharnumber(ui_combobox*, int num);

#endif
