// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwingraphicsimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "uiwinfontimp.h"
#include "uiwinbitmapimp.h"
#include "../../uiapp.h"
#include "../../uicomponent.h"
#include <stdlib.h>

// VTable Prototypes
static void psy_ui_win_g_imp_dispose(psy_ui_win_GraphicsImp*);
static void psy_ui_win_g_imp_textout(psy_ui_win_GraphicsImp*, double x, double y, const char*, uintptr_t len);
static void psy_ui_win_g_imp_textoutrectangle(psy_ui_win_GraphicsImp*, double x, double y, uintptr_t options,
	psy_ui_RealRectangle r, const char* text, uintptr_t len);
static void psy_ui_win_g_imp_drawrectangle(psy_ui_win_GraphicsImp*, const psy_ui_RealRectangle);
static void psy_ui_win_g_imp_drawroundrectangle(psy_ui_win_GraphicsImp*, const psy_ui_RealRectangle,
	psy_ui_Size cornersize);
static psy_ui_Size psy_ui_win_g_imp_textsize(psy_ui_win_GraphicsImp*, const char*);
static void psy_ui_win_g_imp_drawsolidrectangle(psy_ui_win_GraphicsImp*, const psy_ui_RealRectangle r,
	psy_ui_Colour colour);
static void psy_ui_win_g_imp_drawsolidroundrectangle(psy_ui_win_GraphicsImp*, const psy_ui_RealRectangle r,
	psy_ui_Size cornersize, psy_ui_Colour colour);
static void psy_ui_win_g_imp_drawsolidpolygon(psy_ui_win_GraphicsImp*, psy_ui_RealPoint*,
	uintptr_t numpoints, uint32_t inner, uint32_t outter);
static void psy_ui_win_g_imp_drawline(psy_ui_win_GraphicsImp*, double x1, double y1, double x2, double y2);
static void psy_ui_win_g_imp_drawfullbitmap(psy_ui_win_GraphicsImp*, psy_ui_Bitmap*, double x, double y);
static void psy_ui_win_g_imp_drawbitmap(psy_ui_win_GraphicsImp*, psy_ui_Bitmap*, double x, double y, double width,
	double height, double xsrc, double ysrc);
static void psy_ui_win_g_imp_drawstretchedbitmap(psy_ui_win_GraphicsImp*, psy_ui_Bitmap*, double x, double y, double width,
	double height, double xsrc, double ysrc, double wsrc, double hsrc);
static void psy_ui_win_g_imp_setbackgroundcolour(psy_ui_win_GraphicsImp*, psy_ui_Colour colour);
static void psy_ui_win_g_imp_setbackgroundmode(psy_ui_win_GraphicsImp*, uintptr_t mode);
static void psy_ui_win_g_imp_settextcolour(psy_ui_win_GraphicsImp*, psy_ui_Colour colour);
static void psy_ui_win_g_imp_settextalign(psy_ui_win_GraphicsImp*, uintptr_t align);
static void psy_ui_win_g_imp_setcolour(psy_ui_win_GraphicsImp*, psy_ui_Colour colour);
static void psy_ui_win_g_imp_setfont(psy_ui_win_GraphicsImp*, psy_ui_Font* font);
static void psy_ui_win_g_imp_moveto(psy_ui_win_GraphicsImp*, psy_ui_RealPoint pt);
static void psy_ui_win_g_imp_devcurveto(psy_ui_win_GraphicsImp*, psy_ui_RealPoint control_p1,
	psy_ui_RealPoint control_p2, psy_ui_RealPoint p);
static void psy_ui_win_g_imp_devdrawarc(psy_ui_win_GraphicsImp*,
	double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);
static void psy_ui_win_g_devsetlinewidth(psy_ui_win_GraphicsImp*, uintptr_t width);
static unsigned int psy_ui_win_g_devlinewidth(psy_ui_win_GraphicsImp*);
static void psy_ui_win_g_devsetorigin(psy_ui_win_GraphicsImp*, double x, double y);

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
		win_imp_vtable.dev_setbackgroundcolour = (psy_ui_fp_graphicsimp_dev_setbackgroundcolour) psy_ui_win_g_imp_setbackgroundcolour;
		win_imp_vtable.dev_setbackgroundmode = (psy_ui_fp_graphicsimp_dev_setbackgroundmode) psy_ui_win_g_imp_setbackgroundmode;
		win_imp_vtable.dev_settextcolour = (psy_ui_fp_graphicsimp_dev_settextcolour) psy_ui_win_g_imp_settextcolour;
		win_imp_vtable.dev_settextalign = (psy_ui_fp_graphicsimp_dev_settextalign)psy_ui_win_g_imp_settextalign;
		win_imp_vtable.dev_setcolour = (psy_ui_fp_graphicsimp_dev_setcolour) psy_ui_win_g_imp_setcolour;
		win_imp_vtable.dev_setfont = (psy_ui_fp_graphicsimp_dev_setfont) psy_ui_win_g_imp_setfont;
		win_imp_vtable.dev_moveto = (psy_ui_fp_graphicsimp_dev_moveto) psy_ui_win_g_imp_moveto;
		win_imp_vtable.dev_curveto = (psy_ui_fp_graphicsimp_dev_curveto) psy_ui_win_g_imp_devcurveto;
		win_imp_vtable.dev_drawarc = (psy_ui_fp_graphicsimp_dev_drawarc) psy_ui_win_g_imp_devdrawarc;
		win_imp_vtable.dev_setlinewidth = (psy_ui_fp_graphicsimp_dev_setlinewidth) psy_ui_win_g_devsetlinewidth;
		win_imp_vtable.dev_linewidth = (psy_ui_fp_graphicsimp_dev_linewidth) psy_ui_win_g_devlinewidth;
		win_imp_vtable.dev_setorigin = (psy_ui_fp_graphicsimp_dev_setorigin)psy_ui_win_g_devsetorigin;
		win_imp_vtable_initialized = 1;
	}
}

void psy_ui_win_graphicsimp_init(psy_ui_win_GraphicsImp* self, HDC hdc)
{
	psy_ui_graphics_imp_init(&self->imp);
	win_imp_vtable_init(self);
	self->imp.vtable = &win_imp_vtable;
	self->hdc = hdc;
	self->shareddc = TRUE;
	self->pen = CreatePen(PS_SOLID, 1, psy_ui_style(psy_ui_STYLE_COMMON)->colour.value);
	self->brush = 0;
	self->hBrushPrev = 0;
	self->oldbmp = 0;
	self->penprev = SelectObject(self->hdc, self->pen);
	self->hFontPrev = SelectObject(self->hdc,
		((psy_ui_win_FontImp*) psy_ui_style(psy_ui_STYLE_COMMON)->font.imp)->hfont);
	self->orgx = 0;
	self->orgy = 0;	
	SetStretchBltMode(self->hdc, STRETCH_HALFTONE);
}

void psy_ui_win_graphicsimp_init_bitmap(psy_ui_win_GraphicsImp* self, psy_ui_Bitmap* bitmap)
{
	HDC hdc;
	psy_ui_win_BitmapImp* imp;

	psy_ui_graphics_imp_init(&self->imp);
	win_imp_vtable_init(self);
	self->imp.vtable = &win_imp_vtable;	
	hdc = GetDC(NULL);
	SaveDC(hdc);
	self->hdc = CreateCompatibleDC(hdc);
	self->shareddc = FALSE;
	RestoreDC(hdc, -1);
	ReleaseDC(NULL, hdc);
	imp = (psy_ui_win_BitmapImp*)bitmap->imp;
	self->oldbmp = SelectObject(self->hdc, imp->bitmap);		
	self->pen = CreatePen(PS_SOLID, 1, psy_ui_style(psy_ui_STYLE_COMMON)->colour.value);
	self->brush = 0;
	self->hBrushPrev = 0;
	self->penprev = SelectObject(self->hdc, self->pen);
	self->hFontPrev = SelectObject(self->hdc,
		((psy_ui_win_FontImp*)psy_ui_style(psy_ui_STYLE_COMMON)->font.imp)->hfont);
	self->orgx = 0;
	self->orgy = 0;
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
	if (self->oldbmp) {
		SelectObject(self->hdc, self->oldbmp);		
	}
	if (!self->shareddc) {
		DeleteDC(self->hdc);
	}
}

void psy_ui_win_g_imp_textout(psy_ui_win_GraphicsImp* self, double x, double y,
	const char* str, uintptr_t len)
{
	TextOut(self->hdc, (int)x, (int)y, str, (int)len);
}

void psy_ui_win_g_imp_textoutrectangle(psy_ui_win_GraphicsImp* self, double x, double y, uintptr_t options,
	psy_ui_RealRectangle r, const char* text, uintptr_t len)
{
	RECT rect;
	uintptr_t woptions = 0;

	if ((options & psy_ui_ETO_OPAQUE) == psy_ui_ETO_OPAQUE) {
		woptions |= ETO_OPAQUE;
	}
	if ((options & psy_ui_ETO_CLIPPED) == psy_ui_ETO_CLIPPED) {
		woptions |= ETO_CLIPPED;
	}
    SetRect (&rect, (int)r.left, (int)r.top, (int)r.right, (int)r.bottom) ;
	ExtTextOut(self->hdc, (int)x, (int)y, (UINT)woptions, &rect, text, (int)len, NULL);
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

void psy_ui_win_g_imp_drawrectangle(psy_ui_win_GraphicsImp* self, const psy_ui_RealRectangle r)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;

	hBrush = GetStockObject (NULL_BRUSH);
	hOldBrush = SelectObject (self->hdc, hBrush);
	Rectangle(self->hdc, (int)r.left, (int)r.top, (int)r.right, (int)r.bottom);
	SelectObject(self->hdc, hOldBrush);
}

void psy_ui_win_g_imp_drawroundrectangle(psy_ui_win_GraphicsImp* self, const psy_ui_RealRectangle r, psy_ui_Size cornersize)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;
	psy_ui_TextMetric tm;
	TEXTMETRIC win_tm;
	
	hBrush = GetStockObject (NULL_BRUSH);
	hOldBrush = SelectObject(self->hdc, hBrush);
	GetTextMetrics(self->hdc, &win_tm);
	tm = converttextmetric(&win_tm);
	RoundRect(self->hdc, (int)r.left, (int)r.top, (int)r.right, (int)r.bottom,
		(int)psy_ui_value_px(&cornersize.width, &tm),
		(int)psy_ui_value_px(&cornersize.height, &tm));
	SelectObject(self->hdc, hOldBrush);
}

void psy_ui_win_g_imp_drawsolidrectangle(psy_ui_win_GraphicsImp* self, const psy_ui_RealRectangle r, psy_ui_Colour colour)
{
     HBRUSH hBrush;     
     RECT   rect;	 
	                
     SetRect (&rect, (int)r.left, (int)r.top, (int)r.right, (int)r.bottom) ;
     hBrush = CreateSolidBrush(colour.value);
     FillRect (self->hdc, &rect, hBrush);     
     DeleteObject (hBrush) ;
}

void psy_ui_win_g_imp_drawsolidroundrectangle(psy_ui_win_GraphicsImp* self, const psy_ui_RealRectangle r,
	psy_ui_Size cornersize, psy_ui_Colour colour)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;
	HPEN hPen;
	HPEN hOldPen;
	psy_ui_TextMetric tm;
	TEXTMETRIC win_tm;

	hBrush = CreateSolidBrush(colour.value);
	hOldBrush = SelectObject (self->hdc, hBrush);
	hPen = CreatePen(PS_SOLID, 1, colour.value);
	hOldPen = SelectObject(self->hdc, hPen);
	GetTextMetrics(self->hdc, &win_tm);
	tm = converttextmetric(&win_tm);
	RoundRect(self->hdc, (int)r.left, (int)r.top, (int)r.right, (int)r.bottom,
		(int)psy_ui_value_px(&cornersize.width, &tm),
		(int)psy_ui_value_px(&cornersize.height, &tm));
	SelectObject(self->hdc, hOldBrush);
	SelectObject(self->hdc, hOldPen);
	DeleteObject(hBrush) ;
	DeleteObject(hPen) ;
}

void psy_ui_win_g_imp_drawsolidpolygon(psy_ui_win_GraphicsImp* self, psy_ui_RealPoint* pts,
	uintptr_t numpoints, uint32_t inner, uint32_t outter)
{
	HBRUSH hBrush;     
	HBRUSH hBrushPrev;
	HPEN hPen;     
	HPEN hPenPrev;
	POINT* wpts;
	uintptr_t i;

	wpts = (POINT*)malloc(sizeof(POINT) * numpoints);
	for (i = 0; i < numpoints; ++i) {
		wpts[i].x = (int)pts[i].x;
		wpts[i].y = (int)pts[i].y;
	}    
    hBrush = CreateSolidBrush(inner);
	hBrushPrev = SelectObject(self->hdc, hBrush);
	hPen = CreatePen(PS_SOLID, 1, outter);
	hPenPrev = SelectObject(self->hdc, hPen);	
	Polygon(self->hdc, wpts, (int)numpoints);
	SelectObject(self->hdc, hBrushPrev);
	SelectObject(self->hdc, hPenPrev);
    DeleteObject (hBrush);
	DeleteObject (hPen);
	free(wpts);
}

void psy_ui_win_g_imp_drawfullbitmap(psy_ui_win_GraphicsImp* self, psy_ui_Bitmap* bitmap, double x, double y)
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
	BitBlt(self->hdc, (int)x, (int)y, (int)psy_ui_value_px(&size.width, &tm),
		(int)psy_ui_value_px(&size.height, &tm), hdcmem, 0, 0, SRCCOPY);
	DeleteDC(hdcmem);
}

void psy_ui_win_g_imp_drawbitmap(psy_ui_win_GraphicsImp* self,
	psy_ui_Bitmap* bitmap, double x, double y, double width,
	double height, double xsrc, double ysrc)
{
	HDC hdcmem;
	HBITMAP mask;
	HBITMAP wbitmap;
	psy_ui_win_BitmapImp* winimp;

	winimp = (psy_ui_win_BitmapImp*)bitmap->imp;	
	mask = winimp->mask;
	wbitmap = winimp->bitmap;
	hdcmem = CreateCompatibleDC(self->hdc);
	if (winimp->mask) {
		uint32_t restoretextcolour;

		// We are going to paint the two DDB's in sequence to the destination.
		// 1st the monochrome bitmap will be blitted using an AND operation to
		// cut a hole in the destination. The color image will then be ORed
		// with the destination, filling it into the hole, but leaving the
		// surrounding area untouched.
		SelectObject(hdcmem, winimp->mask);
		restoretextcolour = GetTextColor(self->hdc);
		SetTextColor(self->hdc, RGB(0, 0, 0));
		SetBkColor(self->hdc, RGB(255, 255, 255));
		BitBlt(self->hdc, (int)x, (int)y, (int)width, (int)height, hdcmem, (int)xsrc, (int)ysrc, SRCAND);
		SelectObject(hdcmem, wbitmap);
		// Also note the use of SRCPAINT rather than SRCCOPY.
		BitBlt(self->hdc, (int)x, (int)y, (int)width, (int)height, hdcmem, (int)xsrc, (int)ysrc, SRCPAINT);
		SetTextColor(self->hdc, restoretextcolour);
	} else {
		SelectObject(hdcmem, wbitmap);
		BitBlt(self->hdc, (int)x, (int)y, (int)width, (int)height, hdcmem, (int)xsrc, (int)ysrc, SRCCOPY);
	}
	DeleteDC(hdcmem);
}

void psy_ui_win_g_imp_drawstretchedbitmap(psy_ui_win_GraphicsImp* self,
	psy_ui_Bitmap* bitmap, double x, double y, double width,
	double height, double xsrc, double ysrc, double wsrc, double hsrc)
{
	HDC hdcmem;
	HBITMAP mask;
	HBITMAP wbitmap;
	psy_ui_win_BitmapImp* winimp;

	winimp = (psy_ui_win_BitmapImp*)bitmap->imp;
	mask = winimp->mask;
	wbitmap = winimp->bitmap;
	hdcmem = CreateCompatibleDC(self->hdc);
	if (winimp->mask) {
		uint32_t restoretextcolour;
		// We are going to paint the two DDB's in sequence to the destination.
		// 1st the monochrome bitmap will be blitted using an AND operation to
		// cut a hole in the destination. The color image will then be ORed
		// with the destination, filling it into the hole, but leaving the
		// surrounding area untouched.
		restoretextcolour = GetTextColor(self->hdc);
		SelectObject(hdcmem, winimp->mask);
		SetTextColor(self->hdc, RGB(0, 0, 0));
		SetBkColor(self->hdc, RGB(255, 255, 255));		
		StretchBlt(self->hdc, (int)x, (int)y, (int)width, (int)height, hdcmem, (int)xsrc, (int)ysrc, (int)wsrc, (int)hsrc,
			SRCAND);
		SelectObject(hdcmem, wbitmap);
		// Also note the use of SRCPAINT rather than SRCCOPY.		
		StretchBlt(self->hdc, (int)x, (int)y, (int)width, (int)height, hdcmem, (int)xsrc, (int)ysrc, (int)wsrc, (int)hsrc,
			SRCPAINT);
		SetTextColor(self->hdc, restoretextcolour);
	} else {
		SelectObject(hdcmem, wbitmap);
		StretchBlt(self->hdc, (int)x, (int)y, (int)width, (int)height, hdcmem, (int)xsrc, (int)ysrc, (int)wsrc, (int)hsrc,
			SRCCOPY);
	}
	DeleteDC(hdcmem);
}

void psy_ui_win_g_imp_setcolour(psy_ui_win_GraphicsImp* self, psy_ui_Colour colour)
{
	LOGPEN currpen;
	HPEN pen;

	GetObject(self->pen, sizeof(LOGPEN), &currpen);
	currpen.lopnColor = colour.value;
	pen = CreatePenIndirect(&currpen);
	SelectObject(self->hdc, pen);
	if (self->pen) {
		DeleteObject(self->pen);
	}
	self->pen = pen;
}

void psy_ui_win_g_imp_setbackgroundmode(psy_ui_win_GraphicsImp* self, uintptr_t mode)
{
	if (mode == psy_ui_TRANSPARENT) {
		SetBkMode(self->hdc, TRANSPARENT);
	} else
	if (mode == psy_ui_OPAQUE) {
		SetBkMode(self->hdc, OPAQUE);
	}
}

void psy_ui_win_g_imp_setbackgroundcolour(psy_ui_win_GraphicsImp* self, psy_ui_Colour colour)
{
	SetBkColor(self->hdc, colour.value);
}

void psy_ui_win_g_imp_settextcolour(psy_ui_win_GraphicsImp* self, psy_ui_Colour colour)
{
	SetTextColor(self->hdc, colour.value);
}

void psy_ui_win_g_imp_settextalign(psy_ui_win_GraphicsImp* self, uintptr_t align)
{	
	SetTextAlign(self->hdc, (UINT)align);
}

void psy_ui_win_g_imp_setfont(psy_ui_win_GraphicsImp* self, psy_ui_Font* font)
{	
	if (font && ((psy_ui_win_FontImp*)font->imp)->hfont) {		
		SelectObject(self->hdc, ((psy_ui_win_FontImp*)font->imp)->hfont);
	}
}

void psy_ui_win_g_imp_drawline(psy_ui_win_GraphicsImp* self, double x1, double y1, double x2, double y2)
{
	MoveToEx(self->hdc, (int)x1, (int)y1, NULL) ;
	LineTo(self->hdc, (int)x2, (int)y2);
}

void psy_ui_win_g_imp_moveto(psy_ui_win_GraphicsImp* self, psy_ui_RealPoint pt)
{	
	MoveToEx(self->hdc, (int)pt.x, (int)pt.y, NULL);
}

void psy_ui_win_g_imp_devcurveto(psy_ui_win_GraphicsImp* self, psy_ui_RealPoint control_p1,
	psy_ui_RealPoint control_p2, psy_ui_RealPoint p)
{
	POINT pts[3];
   
	pts[0].x = (int)control_p1.x;
	pts[0].y = (int)control_p1.y;
	pts[1].x = (int)control_p2.x;
	pts[1].y = (int)control_p2.y;
	pts[2].x = (int)p.x;
	pts[2].y = (int)p.y;
	PolyBezierTo(self->hdc, pts, 3);
}

void psy_ui_win_g_imp_devdrawarc(psy_ui_win_GraphicsImp* self,
	double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
	Arc(self->hdc, (int)x1, (int)y1, (int)x2, (int)y2, (int)x3, (int)y3, (int)x4, (int)y4);
}

void psy_ui_win_g_devsetlinewidth(psy_ui_win_GraphicsImp* self, uintptr_t width)
{
	LOGPEN currpen;
	HPEN pen;

	GetObject(self->pen, sizeof(LOGPEN), &currpen);
	currpen.lopnWidth.x = (int)width;	
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

void psy_ui_win_g_devsetorigin(psy_ui_win_GraphicsImp* self, double x, double y)
{
	SetWindowOrgEx(self->hdc,
		self->orgx + (int)x,
		self->orgy + (int)y,
		NULL);
}

#endif
