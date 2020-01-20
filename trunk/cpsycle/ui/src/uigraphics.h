// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_GRAPHICS_H
#define psy_ui_GRAPHICS_H

#include <windows.h>
#include "uidef.h"
#include "uibitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  HDC hdc;  
  HFONT hfont;
  HFONT hFontPrev;
  psy_ui_Rectangle clip;
  HPEN pen;  
  HPEN penprev;
  HBRUSH brush;
  HBRUSH hBrushPrev;
} psy_ui_Graphics;

void psy_ui_graphics_init(psy_ui_Graphics*, HDC);
void psy_ui_graphics_dispose(psy_ui_Graphics*);
void psy_ui_textout(psy_ui_Graphics*, int x, int y,  const char*, size_t len);
void psy_ui_textoutrectangle(psy_ui_Graphics*, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len);
void psy_ui_drawrectangle(psy_ui_Graphics*, const psy_ui_Rectangle);
void psy_ui_drawroundrectangle(psy_ui_Graphics*, const psy_ui_Rectangle,
	psy_ui_Size cornersize);
psy_ui_Size psy_ui_textsize(psy_ui_Graphics*, const char*);
void psy_ui_drawsolidrectangle(psy_ui_Graphics*, const psy_ui_Rectangle r,
	unsigned int color);
void psy_ui_drawsolidroundrectangle(psy_ui_Graphics*, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color);
void psy_ui_drawsolidpolygon(psy_ui_Graphics*, psy_ui_Point*,
	unsigned int numpoints,  unsigned int inner, unsigned int outter);
void psy_ui_drawline(psy_ui_Graphics*, int x1, int y1, int x2, int y2);
void psy_ui_drawarc(psy_ui_Graphics*, int x1, int y1, int x2, int y2);
void psy_ui_drawfullbitmap(psy_ui_Graphics*, psy_ui_Bitmap*, int x, int y);
void psy_ui_drawbitmap(psy_ui_Graphics*, psy_ui_Bitmap*, int x, int y, int width,
	int height, int xsrc, int ysrc);
void psy_ui_setbackgroundcolor(psy_ui_Graphics*, unsigned int color);
void psy_ui_setbackgroundmode(psy_ui_Graphics*, unsigned int mode);
void psy_ui_settextcolor(psy_ui_Graphics*, unsigned int color);
void psy_ui_setcolor(psy_ui_Graphics*, unsigned int color);
void psy_ui_setfont(psy_ui_Graphics*, psy_ui_Font* font);
void psy_ui_moveto(psy_ui_Graphics*, psy_ui_Point pt);
void psy_ui_devcurveto(psy_ui_Graphics*, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_GRAPHICS_H */
