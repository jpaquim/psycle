// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(MENU)
#define MENU

#include <windows.h>

struct ui_menu_struct {
  HMENU hmenu;
  int id;
  char* label;
  void (*execute)(struct ui_menu_struct*);  
};

typedef struct ui_menu_struct ui_menu;

void ui_menu_setup(void);
void ui_menu_init(ui_menu* menu, char* label, void (*execute)(ui_menu*));
void ui_menu_append(ui_menu* self, ui_menu* child, int popup);

#endif