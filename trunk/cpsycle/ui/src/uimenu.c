// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

/*
#include "uimenu.h"
#include "hashtbl.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

psy_Table menumap;
static int id = 40000;

void psy_ui_menu_setup(void)
{
	psy_table_init(&menumap);
}

void psy_ui_menu_init(psy_ui_Menu* menu, char* label, void (*execute)(psy_ui_Menu*))
{  
  menu->hmenu = CreateMenu();
  menu->label = label;
  menu->id = 0;
  menu->execute = execute;
}

void psy_ui_menu_append(psy_ui_Menu* self, psy_ui_Menu* child, int popup)
{
	if (popup != NULL)
	{		
		AppendMenu(self->hmenu, MF_POPUP, (UINT_PTR)child->hmenu, child->label);		
	} else {
		AppendMenu(self->hmenu, MF_STRING, id, child->label);
		psy_table_insert(&menumap, id, child);
		++id;
	}
}
*/