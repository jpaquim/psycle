// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_COMBOBOX_H
#define psy_ui_COMBOBOX_H

#include "uicomponent.h"

typedef struct {
   psy_ui_Component component;
   psy_ui_Component combo;
   psy_ui_Component* currcombo;
   psy_Signal signal_selchanged;
   int ownerdrawn;   
   int hover;
   int charnumber;
} psy_ui_ComboBox;

void ui_combobox_init(psy_ui_ComboBox*, psy_ui_Component* parent);
int ui_combobox_addstring(psy_ui_ComboBox*, const char* text);
void ui_combobox_clear(psy_ui_ComboBox*);
void ui_combobox_setcursel(psy_ui_ComboBox*, intptr_t index);
intptr_t ui_combobox_cursel(psy_ui_ComboBox*);
void ui_combobox_setcharnumber(psy_ui_ComboBox*, int num);

#endif /* psy_ui_COMBOBOX_H */
