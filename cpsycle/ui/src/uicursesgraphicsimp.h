// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_CURSES

#ifndef psy_ui_curses_GRAPHICSIMP_H
#define psy_ui_curses_GRAPHICSIMP_H

#include "uigraphics.h"

#include <curses.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_curses_GraphicsImp {
	psy_ui_GraphicsImp imp;
	WINDOW* w;
} psy_ui_curses_GraphicsImp;

void psy_ui_curses_graphicsimp_init(psy_ui_curses_GraphicsImp* self, uintptr_t* hdc);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_curses_GRAPHICSIMP_H */

#endif