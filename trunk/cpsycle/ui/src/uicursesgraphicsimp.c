// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "../../detail/psyconf.h"
#include "uicursesdef.h"

#if PSYCLE_USE_TK == PSYCLE_TK_CURSES

#include "uicursesgraphicsimp.h"
#include "uiapp.h"
#include <stdlib.h>

#ifndef max
#define max(a, b) (((a > b) ? a : b))
#endif

#ifndef min
#define min(a, b) (((a < b) ? a : b))
#endif

extern psy_ui_App app;

// VTable Prototypes
static void psy_ui_curses_g_imp_dispose(psy_ui_curses_GraphicsImp*);
static void psy_ui_curses_g_imp_textout(psy_ui_curses_GraphicsImp*, int x, int y,  const char*, size_t len);
static void psy_ui_curses_g_imp_textoutrectangle(psy_ui_curses_GraphicsImp*, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len);
static void psy_ui_curses_g_imp_drawrectangle(psy_ui_curses_GraphicsImp*, const psy_ui_Rectangle);
static void psy_ui_curses_g_imp_drawroundrectangle(psy_ui_curses_GraphicsImp*, const psy_ui_Rectangle,
	psy_ui_Size cornersize);
static psy_ui_Size psy_ui_curses_g_imp_textsize(psy_ui_curses_GraphicsImp*, const char*);
static void psy_ui_curses_g_imp_drawsolidrectangle(psy_ui_curses_GraphicsImp*, const psy_ui_Rectangle r,
	unsigned int color);
static void psy_ui_curses_g_imp_drawsolidroundrectangle(psy_ui_curses_GraphicsImp*, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color);
static void psy_ui_curses_g_imp_drawsolidpolygon(psy_ui_curses_GraphicsImp*, psy_ui_Point*,
	unsigned int numpoints,  unsigned int inner, unsigned int outter);
static void psy_ui_curses_g_imp_drawline(psy_ui_curses_GraphicsImp*, int x1, int y1, int x2, int y2);
static void psy_ui_curses_g_imp_drawfullbitmap(psy_ui_curses_GraphicsImp*, psy_ui_Bitmap*, int x, int y);
static void psy_ui_curses_g_imp_drawbitmap(psy_ui_curses_GraphicsImp*, psy_ui_Bitmap*, int x, int y, int width,
	int height, int xsrc, int ysrc);
static void psy_ui_curses_g_imp_setbackgroundcolor(psy_ui_curses_GraphicsImp*, unsigned int color);
static void psy_ui_curses_g_imp_setbackgroundmode(psy_ui_curses_GraphicsImp*, unsigned int mode);
static void psy_ui_curses_g_imp_settextcolor(psy_ui_curses_GraphicsImp*, unsigned int color);
static void psy_ui_curses_g_imp_setcolor(psy_ui_curses_GraphicsImp*, unsigned int color);
static void psy_ui_curses_g_imp_setfont(psy_ui_curses_GraphicsImp*, psy_ui_Font* font);
static void psy_ui_curses_g_imp_moveto(psy_ui_curses_GraphicsImp*, psy_ui_Point pt);
static void psy_ui_curses_g_imp_devcurveto(psy_ui_curses_GraphicsImp*, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p);

// VTable init
static psy_ui_GraphicsImpVTable win_imp_vtable;
static int win_imp_vtable_initialized = 0;

static void win_imp_vtable_init(psy_ui_curses_GraphicsImp* self)
{
	if (!win_imp_vtable_initialized) {
		win_imp_vtable = *self->imp.vtable;
		win_imp_vtable.dev_dispose = (psy_ui_fp_graphicsimp_dev_dispose) psy_ui_curses_g_imp_dispose;
		win_imp_vtable.dev_textout = (psy_ui_fp_graphicsimp_dev_textout) psy_ui_curses_g_imp_textout;
		win_imp_vtable.dev_textoutrectangle = (psy_ui_fp_graphicsimp_dev_textoutrectangle) psy_ui_curses_g_imp_textoutrectangle;
		win_imp_vtable.dev_drawrectangle = (psy_ui_fp_graphicsimp_dev_drawrectangle) psy_ui_curses_g_imp_drawrectangle;
		win_imp_vtable.dev_drawroundrectangle = (psy_ui_fp_graphicsimp_dev_drawroundrectangle)psy_ui_curses_g_imp_drawroundrectangle;
		win_imp_vtable.dev_textsize = (psy_ui_fp_graphicsimp_dev_textsize) psy_ui_curses_g_imp_textsize;
		win_imp_vtable.dev_drawsolidrectangle = (psy_ui_fp_graphicsimp_dev_drawsolidrectangle)psy_ui_curses_g_imp_drawsolidrectangle;
		win_imp_vtable.dev_drawsolidroundrectangle = (psy_ui_fp_graphicsimp_dev_drawsolidroundrectangle)psy_ui_curses_g_imp_drawsolidroundrectangle;
		win_imp_vtable.dev_drawsolidpolygon = (psy_ui_fp_graphicsimp_dev_drawsolidpolygon)psy_ui_curses_g_imp_drawsolidpolygon;
		win_imp_vtable.dev_drawline = (psy_ui_fp_graphicsimp_dev_drawline)psy_ui_curses_g_imp_drawline;		
		win_imp_vtable.dev_drawfullbitmap = (psy_ui_fp_graphicsimp_dev_drawfullbitmap)psy_ui_curses_g_imp_drawfullbitmap;
		win_imp_vtable.dev_drawbitmap = (psy_ui_fp_graphicsimp_dev_drawbitmap)psy_ui_curses_g_imp_drawbitmap;
		win_imp_vtable.dev_setbackgroundcolor = (psy_ui_fp_graphicsimp_dev_setbackgroundcolor)psy_ui_curses_g_imp_setbackgroundcolor;
		win_imp_vtable.dev_setbackgroundmode = (psy_ui_fp_graphicsimp_dev_setbackgroundmode)psy_ui_curses_g_imp_setbackgroundmode;
		win_imp_vtable.dev_settextcolor = (psy_ui_fp_graphicsimp_dev_settextcolor)psy_ui_curses_g_imp_settextcolor;
		win_imp_vtable.dev_setcolor = (psy_ui_fp_graphicsimp_dev_setcolor)psy_ui_curses_g_imp_setcolor;
		win_imp_vtable.dev_setfont = (psy_ui_fp_graphicsimp_dev_setfont)psy_ui_curses_g_imp_setfont;
		win_imp_vtable.dev_moveto = (psy_ui_fp_graphicsimp_dev_moveto)psy_ui_curses_g_imp_moveto;
		win_imp_vtable.dev_devcurveto = (psy_ui_fp_graphicsimp_dev_curveto)psy_ui_curses_g_imp_devcurveto;
		win_imp_vtable_initialized = 1;
	}
}

void psy_ui_curses_graphicsimp_init(psy_ui_curses_GraphicsImp* self, uintptr_t* w)
{
	psy_ui_graphics_imp_init(&self->imp);
	win_imp_vtable_init(self);	
	self->imp.vtable = &win_imp_vtable;		
	self->w = (WINDOW*) w;
}

// win32 implementation method for psy_ui_Graphics
void psy_ui_curses_g_imp_dispose(psy_ui_curses_GraphicsImp* self)
{			
}

void psy_ui_curses_g_imp_textout(psy_ui_curses_GraphicsImp* self, int x, int y, const char* str, size_t len)
{
	int maxyscr;
	int maxxscr;
	int maxy;
	int maxx;
	int maxlen;
	int cx;
	int cy;

	getmaxyx(stdscr, maxyscr, maxxscr);
	cx = (int)((float) maxxscr / psy_ui_curses_VSCREENX * x);
	cy = (int)((float) maxyscr / psy_ui_curses_VSCREENY * y);
	getmaxyx(self->w, maxy, maxx);
	maxlen = max(0, min(maxx - cx, (int) len));
	mvwaddnstr(self->w, cy, cx, str, maxlen);
	wrefresh(self->w);
}

void psy_ui_curses_g_imp_textoutrectangle(psy_ui_curses_GraphicsImp* self, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len)
{	
}

psy_ui_Size psy_ui_curses_g_imp_textsize(psy_ui_curses_GraphicsImp* self, const char* text)
{
	psy_ui_Size	rv = { 0, 0 };
	return rv;
}

void psy_ui_curses_g_imp_drawrectangle(psy_ui_curses_GraphicsImp* self, const psy_ui_Rectangle r)
{	
}

void psy_ui_curses_g_imp_drawroundrectangle(psy_ui_curses_GraphicsImp* self, const psy_ui_Rectangle r, psy_ui_Size cornersize)
{
}

void psy_ui_curses_g_imp_drawsolidrectangle(psy_ui_curses_GraphicsImp* self, const psy_ui_Rectangle r, unsigned int color)
{ 
}

void psy_ui_curses_g_imp_drawsolidroundrectangle(psy_ui_curses_GraphicsImp* self, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color)
{
}

void psy_ui_curses_g_imp_drawsolidpolygon(psy_ui_curses_GraphicsImp* self, psy_ui_Point* pts,
	unsigned int numpoints, unsigned int inner, unsigned int outter)
{
}

void psy_ui_curses_g_imp_drawfullbitmap(psy_ui_curses_GraphicsImp* self, psy_ui_Bitmap* bitmap, int x, int y)
{
}

void psy_ui_curses_g_imp_drawbitmap(psy_ui_curses_GraphicsImp* self, psy_ui_Bitmap* bitmap, int x, int y, int width,
	int height, int xsrc, int ysrc)
{
}

void psy_ui_curses_g_imp_setcolor(psy_ui_curses_GraphicsImp* self, unsigned int color)
{
}

void psy_ui_curses_g_imp_setbackgroundmode(psy_ui_curses_GraphicsImp* self, unsigned int mode)
{
}

void psy_ui_curses_g_imp_setbackgroundcolor(psy_ui_curses_GraphicsImp* self, unsigned int color)
{	
}

void psy_ui_curses_g_imp_settextcolor(psy_ui_curses_GraphicsImp* self, unsigned int color)
{
}

void psy_ui_curses_g_imp_setfont(psy_ui_curses_GraphicsImp* self, psy_ui_Font* font)
{	
}

void psy_ui_curses_g_imp_drawline(psy_ui_curses_GraphicsImp* self, int x1, int y1, int x2, int y2)
{
}

void psy_ui_curses_g_imp_moveto(psy_ui_curses_GraphicsImp* self, psy_ui_Point pt)
{	
}

void psy_ui_curses_g_imp_devcurveto(psy_ui_curses_GraphicsImp* self, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p)
{
}

#endif
