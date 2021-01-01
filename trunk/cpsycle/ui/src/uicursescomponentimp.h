// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_CURSES

#ifndef psy_ui_curses_COMPONENTIMP_H
#define psy_ui_curses_COMPONENTIMP_H

#include "uicomponent.h"
#include <curses.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_curses_ComponentImp {
	psy_ui_ComponentImp imp;
	struct psy_ui_Component* component;
	WINDOW* hwnd;	
} psy_ui_curses_ComponentImp;

void psy_ui_curses_componentimp_init(psy_ui_curses_ComponentImp* self,
	struct psy_ui_Component* component,	
	psy_ui_ComponentImp* parent,
	int x, int y, int width, int height);

psy_ui_curses_ComponentImp* psy_ui_curses_componentimp_alloc(void);
psy_ui_curses_ComponentImp* psy_ui_curses_componentimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent,	
	int x, int y, int width, int height);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_win_ComponentImp_H */

#endif
