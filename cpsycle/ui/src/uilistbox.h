// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_LISTBOX_H
#define psy_ui_LISTBOX_H

#include "uicomponent.h"

typedef struct {
   psy_ui_Component component;
   psy_Signal signal_selchanged;
} ui_listbox;

void ui_listbox_init(ui_listbox*, psy_ui_Component* parent);
void ui_listbox_init_multiselect(ui_listbox*, psy_ui_Component* parent);
intptr_t ui_listbox_addstring(ui_listbox*, const char* text);
void ui_listbox_setstring(ui_listbox*, const char* text, intptr_t index);
void ui_listbox_clear(ui_listbox*);
void ui_listbox_setcursel(ui_listbox*, intptr_t index);
intptr_t ui_listbox_cursel(ui_listbox*);
void ui_listbox_selitems(ui_listbox*, int* items, int maxitems);
intptr_t ui_listbox_selcount(ui_listbox*);

#endif /* psy_ui_LISTBOX_H */
