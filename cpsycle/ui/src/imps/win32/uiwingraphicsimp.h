/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
#ifndef psy_ui_win_GRAPHICSIMP_H
#define psy_ui_win_GRAPHICSIMP_H

#include "../../uigraphics.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_win_GraphicsImp {
	psy_ui_GraphicsImp imp;
	HDC hdc;
	HFONT hfont;
	HFONT hFontPrev;
	HPEN pen;
	HPEN penprev;
	HBRUSH brush;
	HBRUSH hBrushPrev;
	HBITMAP oldbmp;
	bool shareddc;
	psy_ui_RealPoint org;
	psy_ui_RealRectangle clip;
	/* reference */
	const psy_ui_Font* font;
} psy_ui_win_GraphicsImp;

void psy_ui_win_graphicsimp_init(psy_ui_win_GraphicsImp* self, HDC hdc);
void psy_ui_win_graphicsimp_init_bitmap(psy_ui_win_GraphicsImp*, psy_ui_Bitmap*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_win_GRAPHICSIMP_H */
#endif /* PSYCLE_TK_WIN32 */
