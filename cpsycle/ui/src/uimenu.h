// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_MENU_H
#define psy_ui_MENU_H

#include "uidef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_Menu {
  HMENU hmenu;
  int id;
  char* label;
  void (*execute)(struct psy_ui_Menu*);  
} psy_ui_Menu;

void ui_menu_setup(void);
void ui_menu_init(psy_ui_Menu*, char* label, void (*execute)(psy_ui_Menu*));
void ui_menu_append(psy_ui_Menu*, psy_ui_Menu* child, int popup);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_MENU_H */
