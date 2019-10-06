// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uimenu.h"
#include "hashtbl.h"

Table menumap;
static int id = 40000;

void ui_menu_setup(void)
{
	table_init(&menumap);
}

void ui_menu_init(ui_menu* menu, char* label, void (*execute)(ui_menu*))
{  
  menu->hmenu = CreateMenu();
  menu->label = label;
  menu->id = 0;
  menu->execute = execute;
}

void ui_menu_append(ui_menu* self, ui_menu* child, int popup)
{
	if (popup != 0)
	{		
		AppendMenu(self->hmenu, MF_POPUP, (unsigned int)child->hmenu, child->label);		
	} else {
		AppendMenu(self->hmenu, MF_STRING, id, child->label);
		table_insert(&menumap, id, child);
		++id;
	}
}
