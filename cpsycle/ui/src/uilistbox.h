// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_LISTBOX_H
#define psy_ui_LISTBOX_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
   psy_ui_Component component;
   psy_Signal signal_selchanged;
} psy_ui_Listbox;

void ui_listbox_init(psy_ui_Listbox*, psy_ui_Component* parent);
void ui_listbox_init_multiselect(psy_ui_Listbox*, psy_ui_Component* parent);
intptr_t ui_listbox_addstring(psy_ui_Listbox*, const char* text);
void ui_listbox_setstring(psy_ui_Listbox*, const char* text, intptr_t index);
void ui_listbox_clear(psy_ui_Listbox*);
void ui_listbox_setcursel(psy_ui_Listbox*, intptr_t index);
intptr_t ui_listbox_cursel(psy_ui_Listbox*);
void ui_listbox_selitems(psy_ui_Listbox*, int* items, int maxitems);
intptr_t ui_listbox_selcount(psy_ui_Listbox*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_LISTBOX_H */
