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
	psy_ui_Color color);
static void psy_ui_win_g_imp_drawsolidroundrectangle(psy_ui_win_GraphicsImp*, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, psy_ui_Color color);
static void psy_ui_win_g_imp_drawsolidpolygon(psy_ui_win_GraphicsImp*, psy_ui_IntPoint*,
	unsigned int numpoints, unsigned int inner, unsigned int outter);
static void psy_ui_win_g_imp_drawline(psy_ui_win_GraphicsImp*, int x1, int y1, int x2, int y2);
static void psy_ui_win_g_imp_drawfullbitmap(psy_ui_win_GraphicsImp*, psy_ui_Bitmap*, int x, int y);
static void psy_ui_win_g_imp_drawbitmap(psy_ui_win_GraphicsImp*, psy_ui_Bitmap*, int x, int y, int width,
	int height, int xsrc, int ysrc);
static void psy_ui_win_g_imp_drawstretchedbitmap(psy_ui_win_GraphicsImp*, psy_ui_Bitmap*, int x, int y, int width,
	int height, int xsrc, int ysrc, int wsrc, int hsrc);
static void psy_ui_win_g_imp_setbackgroundcolor(psy_ui_win_GraphicsImp*, psy_ui_Color color);
static void psy_ui_win_g_imp_setbackgroundmode(psy_ui_win_GraphicsImp*, unsigned int mode);
static void psy_ui_win_g_imp_settextcolor(psy_ui_win_GraphicsImp*, psy_ui_Color color);
static void psy_ui_win_g_imp_settextalign(psy_ui_win_GraphicsImp*, unsigned int align);
static void psy_ui_win_g_imp_setcolor(psy_ui_win_GraphicsImp*, psy_ui_Color color);
static void psy_ui_win_g_imp_setfont(psy_ui_win_GraphicsImp*, psy_ui_Font* font);
static void psy_ui_win_g_imp_moveto(psy_ui_win_GraphicsImp*, psy_ui_IntPoint pt);
static void psy_ui_win_g_imp_devcurveto(psy_ui_win_GraphicsImp*, psy_ui_IntPoint control_p1,
	psy_ui_IntPoint control_p2, psy_ui_IntPoint p);
static void psy_ui_win_g_imp_devdrawarc(psy_ui_win_GraphicsImp*,
	int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
static void psy_ui_win_g_devsetlinewidth(psy_ui_win_GraphicsImp* self, unsigned int width);
static unsigned int psy_ui_win_g_devlinewidth(psy_ui_win_GraphicsImp* self);

static psy_ui_TextMetric converttextmetric(const TEXTMETRIC*);

// VTable init
static psy_ui_GraphicsImpVTable win_imp_vtable;
static int win_imp_vtable_initialized = 0;

static void win_imp_vtable_init(psy_ui_win_GraphicsImp* self)
{
	if (!win_imp_vtable_initialized) {
		win_imp_vtable = *self->imp.vtable;
		win_imp_vtable.dev_dispose = (psy_ui_fp_graphicsimp_dev_dispose) psy_ui_win_g_imp_dispose;
		win_imp_vtable.dev_textout = (psy_ui_fp_graphicsimp_dev_textout) psy_ui_win_g_imp_textout;
		win_imp_vtable.dev_textoutrectangle = (psy_ui_fp_graphicsimp_dev_textoutrectangle) psy_ui_win_g_imp_textoutrectangle;
		win_imp_vtable.dev_drawrectangle = (psy_ui_fp_graphicsimp_dev_drawrectangle) psy_ui_win_g_imp_drawrectangle;
		win_imp_vtable.dev_drawroundrectangle = (psy_ui_fp_graphicsimp_dev_drawroundrectangle) psy_ui_win_g_imp_drawroundrectangle;
		win_imp_vtable.dev_textsize = (psy_ui_fp_graphicsimp_dev_textsize) psy_ui_win_g_imp_textsize;
		win_imp_vtable.dev_drawsolidrectangle = (psy_ui_fp_graphicsimp_dev_drawsolidrectangle) psy_ui_win_g_imp_drawsolidrectangle;
		win_imp_vtable.dev_drawsolidroundrectangle = (psy_ui_fp_graphicsimp_dev_drawsolidroundrectangle) psy_ui_win_g_imp_drawsolidroundrectangle;
		win_imp_vtable.dev_drawsolidpolygon = (psy_ui_fp_graphicsimp_dev_drawsolidpolygon) psy_ui_win_g_imp_drawsolidpolygon;
		win_imp_vtable.dev_drawline = (psy_ui_fp_graphicsimp_dev_drawline) psy_ui_win_g_imp_drawline;
		win_imp_vtable.dev_drawfullbitmap = (psy_ui_fp_graphicsimp_dev_drawfullbitmap) psy_ui_win_g_imp_drawfullbitmap;
		win_imp_vtable.dev_drawbitmap = (psy_ui_fp_graphicsimp_dev_drawbitmap) psy_ui_win_g_imp_drawbitmap;
		win_imp_vtable.dev_drawstretchedbitmap = (psy_ui_fp_graphicsimp_dev_drawstretchedbitmap)psy_ui_win_g_imp_drawstretchedbitmap;
		win_imp_vtable.dev_setbackgroundcolor = (psy_ui_fp_graphicsimp_dev_setbackgroundcolor) psy_ui_win_g_imp_setbackgroundcolor;
		win_imp_vtable.dev_setbackgroundmode = (psy_ui_fp_graphicsimp_dev_setbackgroundmode) psy_ui_win_g_imp_setbackgroundmode;
		win_imp_vtable.dev_settextcolor = (psy_ui_fp_graphicsimp_dev_settextcolor) psy_ui_win_g_imp_settextcolor;
		win_imp_vtable.dev_settextalign = (psy_ui_fp_graphicsimp_dev_settextalign)psy_ui_win_g_imp_settextalign;
		win_imp_vtable.dev_setcolor = (psy_ui_fp_graphicsimp_dev_setcolor) psy_ui_win_g_imp_setcolor;
		win_imp_vtable.dev_setfont = (psy_ui_fp_graphicsimp_dev_setfont) psy_ui_win_g_imp_setfont;
		win_imp_vtable.dev_moveto = (psy_ui_fp_graphicsimp_dev_moveto) psy_ui_win_g_imp_moveto;
		win_imp_vtable.dev_curveto = (psy_ui_fp_graphicsimp_dev_curveto) psy_ui_win_g_imp_devcurveto;
		win_imp_vtable.dev_drawarc = (psy_ui_fp_graphicsimp_dev_drawarc) psy_ui_win_g_imp_devdrawarc;
		win_imp_vtable.dev_setlinewidth = (psy_ui_fp_graphicsimp_dev_setlinewidth) psy_ui_win_g_devsetlinewidth;
		win_imp_vtable.dev_linewidth = (psy_ui_fp_graphicsimp_dev_linewidth) psy_ui_win_g_devlinewidth;
		win_imp_vtable_initialized = 1;
	}
}

void psy_ui_win_graphicsimp_init(psy_ui_win_GraphicsImp* self, HDC hdc)
{
	psy_ui_graphics_imp_init(&self->imp);
	win_imp_vtable_init(self);
	self->imp.vtable = &win_imp_vtable;
	self->hdc = hdc;
	self->pen = CreatePen(PS_SOLID, 1, app.defaults.style_common.color.value);
	self->brush = 0;
	self->hBrushPrev = 0;
	self->penprev = SelectObject(self->hdc, self->pen);
	self->hFontPrev = SelectObject(self->hdc,
		((psy_ui_win_FontImp*) app.defaults.style_common.font.imp)->hfont);
	SetStretchBltMode(self->hdc, STRETCH_HALFTONE);
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

void psy_ui_win_g_imp_textout(psy_ui_win_GraphicsImp* self, int x, int y,
	const char* str, size_t len)
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
		rv.width = psy_ui_value_makepx(size.cx);
		rv.height = psy_ui_value_makepx(size.cy);
	} else {
		rv.width = psy_ui_value_makepx(0);
		rv.height = psy_ui_value_makepx(0);
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
	psy_ui_TextMetric tm;
	TEXTMETRIC win_tm;
	
	hBrush = GetStockObject (NULL_BRUSH);
	hOldBrush = SelectObject(self->hdc, hBrush);
	GetTextMetrics(self->hdc, &win_tm);
	tm = converttextmetric(&win_tm);
	RoundRect(self->hdc, r.left, r.top, r.right, r.bottom,
		psy_ui_value_px(&cornersize.width, &tm),
		psy_ui_value_px(&cornersize.height, &tm));
	SelectObject(self->hdc, hOldBrush);
}

void psy_ui_win_g_imp_drawsolidrectangle(psy_ui_win_GraphicsImp* self, const psy_ui_Rectangle r, psy_ui_Color color)
{
     HBRUSH hBrush;     
     RECT   rect;	 
	                
     SetRect (&rect, r.left, r.top, r.right, r.bottom) ;     
     hBrush = CreateSolidBrush(color.value);
     FillRect (self->hdc, &rect, hBrush);     
     DeleteObject (hBrush) ;
}

void psy_ui_win_g_imp_drawsolidroundrectangle(psy_ui_win_GraphicsImp* self, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, psy_ui_Color color)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;
	HPEN hPen;
	HPEN hOldPen;
	psy_ui_TextMetric tm;
	TEXTMETRIC win_tm;

	hBrush = CreateSolidBrush(color.value);
	hOldBrush = SelectObject (self->hdc, hBrush);
	hPen = CreatePen(PS_SOLID, 1, color.value);
	hOldPen = SelectObject(self->hdc, hPen);
	GetTextMetrics(self->hdc, &win_tm);
	tm = converttextmetric(&win_tm);
	RoundRect(self->hdc, r.left, r.top, r.right, r.bottom,
		psy_ui_value_px(&cornersize.width, &tm),
		psy_ui_value_px(&cornersize.height, &tm));
	SelectObject(self->hdc, hOldBrush);
	SelectObject(self->hdc, hOldPen);
	DeleteObject(hBrush) ;
	DeleteObject(hPen) ;
}

void psy_ui_win_g_imp_drawsolidpolygon(psy_ui_win_GraphicsImp* self, psy_ui_IntPoint* pts,
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
	psy_ui_TextMetric tm;
	TEXTMETRIC win_tm;

	hdcmem = CreateCompatibleDC(self->hdc);
	wbitmap = ((psy_ui_win_BitmapImp*)bitmap->imp)->bitmap;
	SelectObject (hdcmem, wbitmap);
	size = psy_ui_bitmap_size(bitmap);
	GetTextMetrics(self->hdc, &win_tm);
	tm = converttextmetric(&win_tm);
	BitBlt(self->hdc, x, y, psy_ui_value_px(&size.width, &tm),
		psy_ui_value_px(&size.height, &tm), hdcmem, 0, 0, SRCCOPY);
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

void psy_ui_win_g_imp_drawstretchedbitmap(psy_ui_win_GraphicsImp* self,
	psy_ui_Bitmap* bitmap, int x, int y, int width,
	int height, int xsrc, int ysrc, int wsrc, int hsrc)
{
	HDC hdcmem;
	HBITMAP wbitmap;

	wbitmap = ((psy_ui_win_BitmapImp*)bitmap->imp)->bitmap;
	hdcmem = CreateCompatibleDC(self->hdc);
	SelectObject(hdcmem, wbitmap);
	StretchBlt(self->hdc, x, y, width, height, hdcmem, xsrc, ysrc, wsrc, hsrc,
		SRCCOPY);
	DeleteDC(hdcmem);
}

void psy_ui_win_g_imp_setcolor(psy_ui_win_GraphicsImp* self, psy_ui_Color color)
{
	LOGPEN currpen;
	HPEN pen;

	GetObject(self->pen, sizeof(LOGPEN), &currpen);
	currpen.lopnColor = color.value;
	pen = CreatePenIndirect(&currpen);
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

void psy_ui_win_g_imp_setbackgroundcolor(psy_ui_win_GraphicsImp* self, psy_ui_Color color)
{
	SetBkColor(self->hdc, color.value);
}

void psy_ui_win_g_imp_settextcolor(psy_ui_win_GraphicsImp* self, psy_ui_Color color)
{
	SetTextColor(self->hdc, color.value);
}

void psy_ui_win_g_imp_settextalign(psy_ui_win_GraphicsImp* self, unsigned int align)
{	
	SetTextAlign(self->hdc, align);
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

void psy_ui_win_g_imp_moveto(psy_ui_win_GraphicsImp* self, psy_ui_IntPoint pt)
{	
	MoveToEx(self->hdc, pt.x, pt.y, NULL);
}

void psy_ui_win_g_imp_devcurveto(psy_ui_win_GraphicsImp* self, psy_ui_IntPoint control_p1,
	psy_ui_IntPoint control_p2, psy_ui_IntPoint p)
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

void psy_ui_win_g_imp_devdrawarc(psy_ui_win_GraphicsImp* self,
	int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
	Arc(self->hdc, x1, y1, x2, y2, x3, y3, x4, y4);
}

void psy_ui_win_g_devsetlinewidth(psy_ui_win_GraphicsImp* self, unsigned int width)
{
	LOGPEN currpen;
	HPEN pen;

	GetObject(self->pen, sizeof(LOGPEN), &currpen);
	currpen.lopnWidth.x = width;	
	pen = CreatePenIndirect(&currpen);
	SelectObject(self->hdc, pen);
	if (self->pen) {
		DeleteObject(self->pen);
	}
	self->pen = pen;
}

unsigned int psy_ui_win_g_devlinewidth(psy_ui_win_GraphicsImp* self)
{
	LOGPEN currpen;

	GetObject(self->pen, sizeof(LOGPEN), &currpen);
	return currpen.lopnWidth.x;
}

psy_ui_TextMetric converttextmetric(const TEXTMETRIC* tm)
{
	psy_ui_TextMetric rv;

	rv.tmHeight = tm->tmHeight;
	rv.tmAscent = tm->tmAscent;
	rv.tmDescent = tm->tmDescent;
	rv.tmInternalLeading = tm->tmInternalLeading;
	rv.tmExternalLeading = tm->tmExternalLeading;
	rv.tmAveCharWidth = tm->tmAveCharWidth;
	rv.tmMaxCharWidth = tm->tmMaxCharWidth;
	rv.tmWeight = tm->tmWeight;
	rv.tmOverhang = tm->tmOverhang;
	rv.tmDigitizedAspectX = tm->tmDigitizedAspectX;
	rv.tmDigitizedAspectY = tm->tmDigitizedAspectY;
	rv.tmFirstChar = tm->tmFirstChar;
	rv.tmLastChar = tm->tmLastChar;
	rv.tmDefaultChar = tm->tmDefaultChar;
	rv.tmBreakChar = tm->tmBreakChar;
	rv.tmItalic = tm->tmItalic;
	rv.tmUnderlined = tm->tmUnderlined;
	rv.tmStruckOut = tm->tmStruckOut;
	rv.tmPitchAndFamily = tm->tmPitchAndFamily;
	rv.tmCharSet = tm->tmCharSet;
	return rv;
}

#endif
