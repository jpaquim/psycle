// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwingraphicsimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "uiwinfontimp.h"
#include "uiwinbitmapimp.h"
#include "uiapp.h"
#include <stdlib.h>

extern psy_ui_App app;

// VTable Prototypes
static void psy_ui_win_g_imp_dispose(psy_ui_win_GraphicsImp*);
static void psy_ui_win_g_imp_textout(psy_ui_win_GraphicsImp*, int x, int y, const char*, size_t len);
static void psy_ui_win_g_imp_textoutrectangle(psy_ui_win_GraphicsImp*, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len);
static void psy_ui_win_g_imp_drawrectangle(psy_ui_win_GraphicsImp*, const psy_ui_Rectangle);
static void psy_ui_win_g_imp_drawroundrectangle(psy_ui_win_GraphicsImp*, const psy_ui_Rectangle,
	psy_ui_Size cornersize);
static psy_ui_Size psy_ui_win_g_imp_textsize(psy_ui_win_GraphicsImp*, const char*);
static void psy_ui_win_g_imp_drawsolidrectangle(psy_ui_win_GraphicsImp*, const psy_ui_Rectangle r,
	unsigned int color);
static void psy_ui_win_g_imp_drawsolidroundrectangle(psy_ui_win_GraphicsImp*, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color);
static void psy_ui_win_g_imp_drawsolidpolygon(psy_ui_win_GraphicsImp*, psy_ui_Point*,
	unsigned int numpoints, unsigned int inner, unsigned int outter);
static void psy_ui_win_g_imp_drawline(psy_ui_win_GraphicsImp*, int x1, int y1, int x2, int y2);
static void psy_ui_win_g_imp_drawfullbitmap(psy_ui_win_GraphicsImp*, psy_ui_Bitmap*, int x, int y);
static void psy_ui_win_g_imp_drawbitmap(psy_ui_win_GraphicsImp*, psy_ui_Bitmap*, int x, int y, int width,
	int height, int xsrc, int ysrc);
static void psy_ui_win_g_imp_setbackgroundcolor(psy_ui_win_GraphicsImp*, unsigned int color);
static void psy_ui_win_g_imp_setbackgroundmode(psy_ui_win_GraphicsImp*, unsigned int mode);
static void psy_ui_win_g_imp_settextcolor(psy_ui_win_GraphicsImp*, unsigned int color);
static void psy_ui_win_g_imp_setcolor(psy_ui_win_GraphicsImp*, unsigned int color);
static void psy_ui_win_g_imp_setfont(psy_ui_win_GraphicsImp*, psy_ui_Font* font);
static void psy_ui_win_g_imp_moveto(psy_ui_win_GraphicsImp*, psy_ui_Point pt);
static void psy_ui_win_g_imp_devcurveto(psy_ui_win_GraphicsImp*, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p);

// VTable init
static psy_ui_GraphicsImpVTable win_imp_vtable;
static int win_imp_vtable_initialized = 0;

static void win_imp_vtable_init(psy_ui_win_GraphicsImp* self)
{
	if (!win_imp_vtable_initialized) {
		win_imp_vtable = *self->imp.vtable;
		win_imp_vtable.dev_dispose = (psy_ui_g_imp_fp_dispose)psy_ui_win_g_imp_dispose;
		win_imp_vtable.dev_textout = (psy_ui_g_imp_fp_textout)psy_ui_win_g_imp_textout;
		win_imp_vtable.dev_textoutrectangle = (psy_ui_g_imp_fp_textoutrectangle)psy_ui_win_g_imp_textoutrectangle;
		win_imp_vtable.dev_drawrectangle = (psy_ui_g_imp_fp_drawrectangle)psy_ui_win_g_imp_drawrectangle;
		win_imp_vtable.dev_drawroundrectangle = (psy_ui_g_imp_fp_drawroundrectangle)psy_ui_win_g_imp_drawroundrectangle;
		win_imp_vtable.dev_textsize = (psy_ui_g_imp_fp_textsize)psy_ui_win_g_imp_textsize;
		win_imp_vtable.dev_drawsolidrectangle = (psy_ui_g_imp_fp_drawsolidrectangle)psy_ui_win_g_imp_drawsolidrectangle;
		win_imp_vtable.dev_drawsolidroundrectangle = (psy_ui_g_imp_fp_drawsolidroundrectangle)psy_ui_win_g_imp_drawsolidroundrectangle;
		win_imp_vtable.dev_drawsolidpolygon = (psy_ui_g_imp_fp_drawsolidpolygon)psy_ui_win_g_imp_drawsolidpolygon;
		win_imp_vtable.dev_drawline = (psy_ui_g_imp_fp_drawline)psy_ui_win_g_imp_drawline;
		win_imp_vtable.dev_drawfullbitmap = (psy_ui_g_imp_fp_drawfullbitmap)psy_ui_win_g_imp_drawfullbitmap;
		win_imp_vtable.dev_drawbitmap = (psy_ui_g_imp_fp_drawbitmap)psy_ui_win_g_imp_drawbitmap;
		win_imp_vtable.dev_setbackgroundcolor = (psy_ui_g_imp_fp_setbackgroundcolor)psy_ui_win_g_imp_setbackgroundcolor;
		win_imp_vtable.dev_setbackgroundmode = (psy_ui_g_imp_fp_setbackgroundmode)psy_ui_win_g_imp_setbackgroundmode;
		win_imp_vtable.dev_settextcolor = (psy_ui_g_imp_fp_settextcolor)psy_ui_win_g_imp_settextcolor;
		win_imp_vtable.dev_setcolor = (psy_ui_g_imp_fp_setcolor)psy_ui_win_g_imp_setcolor;
		win_imp_vtable.dev_setfont = (psy_ui_g_imp_fp_setfont)psy_ui_win_g_imp_setfont;
		win_imp_vtable.dev_moveto = (psy_ui_g_imp_fp_moveto)psy_ui_win_g_imp_moveto;
		win_imp_vtable.dev_devcurveto = (psy_ui_g_imp_fp_devcurveto)psy_ui_win_g_imp_devcurveto;
		win_imp_vtable_initialized = 1;
	}
}

void psy_ui_win_graphicsimp_init(psy_ui_win_GraphicsImp* self, HDC hdc)
{
	psy_ui_graphics_imp_init(&self->imp);
	win_imp_vtable_init(self);
	self->imp.vtable = &win_imp_vtable;
	self->hdc = hdc;
	self->pen = CreatePen(PS_SOLID, 1, app.defaults.defaultcolor);
	self->brush = 0;
	self->hBrushPrev = 0;
	self->penprev = SelectObject(self->hdc, self->pen);
	self->hFontPrev = SelectObject(self->hdc, ((psy_ui_win_FontImp*)app.defaults.style_common.font.imp)->hfont);
}

// win32 implementation method for psy_ui_Graphics
void psy_ui_win_g_imp_dispose(psy_ui_win_GraphicsImp* self)
{
	SelectObject(self->hdc, self->penprev);
	SelectObject(self->hdc, self->hFontPrev);
	if (self->pen) {
		DeleteObject(self->pen);
	}
}

void psy_ui_win_g_imp_textout(psy_ui_win_GraphicsImp* self, int x, int y, const char* str, size_t len)
{
	TextOut(self->hdc, x, y, str, (int)len);
}

void psy_ui_win_g_imp_textoutrectangle(psy_ui_win_GraphicsImp* self, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len)
{
	RECT rect;
	int woptions = 0;

	if ((options & psy_ui_ETO_OPAQUE) == psy_ui_ETO_OPAQUE) {
		woptions |= ETO_OPAQUE;
	}
	if ((options & psy_ui_ETO_CLIPPED) == psy_ui_ETO_CLIPPED) {
		woptions |= ETO_CLIPPED;
	}
    SetRect (&rect, r.left, r.top, r.right, r.bottom) ;     	
	ExtTextOut(self->hdc, x, y, woptions, &rect, text, (int)len, NULL);
}

psy_ui_Size psy_ui_win_g_imp_textsize(psy_ui_win_GraphicsImp* self, const char* text)
{
	psy_ui_Size	rv;

	if (text) {
		SIZE size;
				
		GetTextExtentPoint(self->hdc, text, (int)strlen(text), &size) ;	
		rv.width = size.cx; 
		rv.height = size.cy;
	} else {
		rv.width = 0; 
		rv.height = 0;
	}
	return rv;
}

void psy_ui_win_g_imp_drawrectangle(psy_ui_win_GraphicsImp* self, const psy_ui_Rectangle r)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;

	hBrush = GetStockObject (NULL_BRUSH);
	hOldBrush = SelectObject (self->hdc, hBrush);
	Rectangle(self->hdc, r.left, r.top, r.right, r.bottom);
	SelectObject(self->hdc, hOldBrush);
}

void psy_ui_win_g_imp_drawroundrectangle(psy_ui_win_GraphicsImp* self, const psy_ui_Rectangle r, psy_ui_Size cornersize)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;

	hBrush = GetStockObject (NULL_BRUSH);
	hOldBrush = SelectObject(self->hdc, hBrush);
	RoundRect(self->hdc, r.left, r.top, r.right, r.bottom, cornersize.width,
		cornersize.height);
	SelectObject(self->hdc, hOldBrush);
}

void psy_ui_win_g_imp_drawsolidrectangle(psy_ui_win_GraphicsImp* self, const psy_ui_Rectangle r, unsigned int color)
{
     HBRUSH hBrush;     
     RECT   rect;	 
	                
     SetRect (&rect, r.left, r.top, r.right, r.bottom) ;     
     hBrush = CreateSolidBrush(color);
     FillRect (self->hdc, &rect, hBrush);     
     DeleteObject (hBrush) ;
}

void psy_ui_win_g_imp_drawsolidroundrectangle(psy_ui_win_GraphicsImp* self, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;
	HPEN hPen;
	HPEN hOldPen;

	hBrush = CreateSolidBrush(color);
	hOldBrush = SelectObject (self->hdc, hBrush);
	hPen = CreatePen(PS_SOLID, 1, color);
	hOldPen = SelectObject(self->hdc, hPen);
	RoundRect(self->hdc, r.left, r.top, r.right, r.bottom, cornersize.width,
		cornersize.height);
	SelectObject(self->hdc, hOldBrush);
	SelectObject(self->hdc, hOldPen);
	DeleteObject(hBrush) ;
	DeleteObject(hPen) ;
}

void psy_ui_win_g_imp_drawsolidpolygon(psy_ui_win_GraphicsImp* self, psy_ui_Point* pts,
	unsigned int numpoints, unsigned int inner, unsigned int outter)
{
	HBRUSH hBrush;     
	HBRUSH hBrushPrev;
	HPEN hPen;     
	HPEN hPenPrev;
	POINT* wpts;
	unsigned int i;

	wpts = (POINT*)malloc(sizeof(POINT) * numpoints);
	for (i = 0; i < numpoints; ++i) {
		wpts[i].x = pts[i].x;
		wpts[i].y = pts[i].y;
	}    
    hBrush = CreateSolidBrush(inner);
	hBrushPrev = SelectObject(self->hdc, hBrush);
	hPen = CreatePen(PS_SOLID, 1, outter);
	hPenPrev = SelectObject(self->hdc, hPen);	
	Polygon(self->hdc, wpts, numpoints);
	SelectObject(self->hdc, hBrushPrev);
	SelectObject(self->hdc, hPenPrev);
    DeleteObject (hBrush);
	DeleteObject (hPen);
	free(wpts);
}

void psy_ui_win_g_imp_drawfullbitmap(psy_ui_win_GraphicsImp* self, psy_ui_Bitmap* bitmap, int x, int y)
{
	HDC hdcmem;
	psy_ui_Size size;
	HBITMAP wbitmap;

	hdcmem = CreateCompatibleDC(self->hdc);
	wbitmap = ((psy_ui_win_BitmapImp*)bitmap->imp)->bitmap;
	SelectObject (hdcmem, wbitmap);
	size = psy_ui_bitmap_size(bitmap);
	BitBlt(self->hdc, x, y, size.width, size.height, hdcmem, 0, 0, SRCCOPY);
	DeleteDC(hdcmem);
}

void psy_ui_win_g_imp_drawbitmap(psy_ui_win_GraphicsImp* self,
	psy_ui_Bitmap* bitmap, int x, int y, int width,
	int height, int xsrc, int ysrc)
{
	HDC hdcmem;
	HBITMAP wbitmap;

	wbitmap = ((psy_ui_win_BitmapImp*)bitmap->imp)->bitmap;
	hdcmem = CreateCompatibleDC (self->hdc) ;
	SelectObject (hdcmem, wbitmap) ;
	BitBlt(self->hdc, x, y, width, height, hdcmem, xsrc, ysrc, SRCCOPY);
	DeleteDC (hdcmem);
}

void psy_ui_win_g_imp_setcolor(psy_ui_win_GraphicsImp* self, unsigned int color)
{
	HPEN pen;

	pen = CreatePen(PS_SOLID, 1, color);
	SelectObject(self->hdc, pen);
	if (self->pen) {
		DeleteObject(self->pen);
	}
	self->pen = pen;
}

void psy_ui_win_g_imp_setbackgroundmode(psy_ui_win_GraphicsImp* self, unsigned int mode)
{
	if (mode == psy_ui_TRANSPARENT) {
		SetBkMode(self->hdc, TRANSPARENT);
	} else
	if (mode == psy_ui_OPAQUE) {
		SetBkMode(self->hdc, OPAQUE);
	}
}

void psy_ui_win_g_imp_setbackgroundcolor(psy_ui_win_GraphicsImp* self, unsigned int color)
{
	SetBkColor(self->hdc, color);
}

void psy_ui_win_g_imp_settextcolor(psy_ui_win_GraphicsImp* self, unsigned int color)
{
	SetTextColor(self->hdc, color);
}

void psy_ui_win_g_imp_setfont(psy_ui_win_GraphicsImp* self, psy_ui_Font* font)
{	
	if (font && ((psy_ui_win_FontImp*)font->imp)->hfont) {		
		SelectObject(self->hdc, ((psy_ui_win_FontImp*)font->imp)->hfont);
	}
}

void psy_ui_win_g_imp_drawline(psy_ui_win_GraphicsImp* self, int x1, int y1, int x2, int y2)
{
	MoveToEx(self->hdc, x1, y1, NULL) ;
	LineTo(self->hdc, x2, y2);
}

void psy_ui_win_g_imp_moveto(psy_ui_win_GraphicsImp* self, psy_ui_Point pt)
{	
	MoveToEx(self->hdc, pt.x, pt.y, NULL);
}

void psy_ui_win_g_imp_devcurveto(psy_ui_win_GraphicsImp* self, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p)
{
	POINT pts[3];
   
	pts[0].x = control_p1.x;
	pts[0].y = control_p1.y;
	pts[1].x = control_p2.x;
	pts[1].y = control_p2.y;
	pts[2].x = p.x;
	pts[2].y = p.y;
	PolyBezierTo(self->hdc, pts, 3);
}

#endif
