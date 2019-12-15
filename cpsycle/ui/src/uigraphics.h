// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIGRAPHICS_H)
#define UIGRAPHICS_H

#include <windows.h>
#include "uidef.h"
#include "uibitmap.h"

typedef struct {
  HDC hdc;  
  HFONT hfont;
  HFONT hFontPrev;
  ui_rectangle clip;
  HPEN pen;  
  HPEN penprev;
  HBRUSH brush;
  HBRUSH hBrushPrev;
} ui_graphics;

void ui_graphics_init(ui_graphics* g, HDC hdc);
void ui_graphics_dispose(ui_graphics* g);
void ui_textout(ui_graphics*, int x, int y,  const char*, size_t len);
void ui_textoutrectangle(ui_graphics* g, int x, int y, unsigned int options,
	ui_rectangle r, const char* text, size_t len);
void ui_drawrectangle(ui_graphics*, const ui_rectangle);
void ui_drawroundrectangle(ui_graphics*, const ui_rectangle,
	ui_size cornersize);
ui_size ui_textsize(ui_graphics*, const char*);
void ui_drawsolidrectangle(ui_graphics* g, const ui_rectangle r,
	unsigned int color);
void ui_drawsolidroundrectangle(ui_graphics* self, const ui_rectangle r,
	ui_size cornersize, unsigned int color);
void ui_drawsolidpolygon(ui_graphics* g, ui_point* , unsigned int numpoints, 
	unsigned int inner, unsigned int outter);
void ui_drawline(ui_graphics* g, int x1, int y1, int x2, int y2);			
void ui_drawfullbitmap(ui_graphics* g, ui_bitmap* bitmap, int x, int y);
void ui_drawbitmap(ui_graphics* g, ui_bitmap* bitmap, int x, int y, int width,
	int height, int xsrc, int ysrc);
void ui_setbackgroundcolor(ui_graphics* g, unsigned int color);
void ui_setbackgroundmode(ui_graphics* g, unsigned int mode);
void ui_settextcolor(ui_graphics* g, unsigned int color);
void ui_setcolor(ui_graphics* g, unsigned int color);
void ui_setfont(ui_graphics* g, ui_font* font);
ui_font ui_createfont(const char* name, int size);

#endif
