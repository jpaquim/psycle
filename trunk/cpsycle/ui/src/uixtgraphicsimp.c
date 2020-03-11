// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uixtgraphicsimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT

//#include "uiwinfontimp.h"
#include "uixtbitmapimp.h"
#include "uiapp.h"
#include <stdlib.h>

extern psy_ui_App app;

// VTable Prototypes
static void psy_ui_xt_g_imp_dispose(psy_ui_xt_GraphicsImp*);
static void psy_ui_xt_g_imp_textout(psy_ui_xt_GraphicsImp*, int x, int y, const char*, size_t len);
static void psy_ui_xt_g_imp_textoutrectangle(psy_ui_xt_GraphicsImp*, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len);
static void psy_ui_xt_g_imp_drawrectangle(psy_ui_xt_GraphicsImp*, const psy_ui_Rectangle);
static void psy_ui_xt_g_imp_drawroundrectangle(psy_ui_xt_GraphicsImp*, const psy_ui_Rectangle,
	psy_ui_Size cornersize);
static psy_ui_Size psy_ui_xt_g_imp_textsize(psy_ui_xt_GraphicsImp*, const char*);
static void psy_ui_xt_g_imp_drawsolidrectangle(psy_ui_xt_GraphicsImp*, const psy_ui_Rectangle r,
	unsigned int color);
static void psy_ui_xt_g_imp_drawsolidroundrectangle(psy_ui_xt_GraphicsImp*, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color);
static void psy_ui_xt_g_imp_drawsolidpolygon(psy_ui_xt_GraphicsImp*, psy_ui_Point*,
	unsigned int numpoints, unsigned int inner, unsigned int outter);
static void psy_ui_xt_g_imp_drawline(psy_ui_xt_GraphicsImp*, int x1, int y1, int x2, int y2);
static void psy_ui_xt_g_imp_drawfullbitmap(psy_ui_xt_GraphicsImp*, psy_ui_Bitmap*, int x, int y);
static void psy_ui_xt_g_imp_drawbitmap(psy_ui_xt_GraphicsImp*, psy_ui_Bitmap*, int x, int y, int width,
	int height, int xsrc, int ysrc);
static void psy_ui_xt_g_imp_setbackgroundcolor(psy_ui_xt_GraphicsImp*, unsigned int color);
static void psy_ui_xt_g_imp_setbackgroundmode(psy_ui_xt_GraphicsImp*, unsigned int mode);
static void psy_ui_xt_g_imp_settextcolor(psy_ui_xt_GraphicsImp*, unsigned int color);
static void psy_ui_xt_g_imp_setcolor(psy_ui_xt_GraphicsImp*, unsigned int color);
static void psy_ui_xt_g_imp_setfont(psy_ui_xt_GraphicsImp*, psy_ui_Font* font);
static void psy_ui_xt_g_imp_moveto(psy_ui_xt_GraphicsImp*, psy_ui_Point pt);
static void psy_ui_xt_g_imp_devcurveto(psy_ui_xt_GraphicsImp*, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p);

// VTable init
static psy_ui_GraphicsImpVTable xt_imp_vtable;
static int xt_imp_vtable_initialized = 0;

static void xt_imp_vtable_init(psy_ui_xt_GraphicsImp* self)
{
	if (!xt_imp_vtable_initialized) {
		xt_imp_vtable = *self->imp.vtable;
		xt_imp_vtable.dev_dispose = (psy_ui_fp_graphicsimp_dev_dispose)psy_ui_xt_g_imp_dispose;
		xt_imp_vtable.dev_textout = (psy_ui_fp_graphicsimp_dev_textout)psy_ui_xt_g_imp_textout;
		xt_imp_vtable.dev_textoutrectangle = (psy_ui_fp_graphicsimp_dev_textoutrectangle)psy_ui_xt_g_imp_textoutrectangle;
		xt_imp_vtable.dev_drawrectangle = (psy_ui_fp_graphicsimp_dev_drawrectangle)psy_ui_xt_g_imp_drawrectangle;
		xt_imp_vtable.dev_drawroundrectangle = (psy_ui_fp_graphicsimp_dev_drawroundrectangle)psy_ui_xt_g_imp_drawroundrectangle;
		xt_imp_vtable.dev_textsize = (psy_ui_fp_graphicsimp_dev_textsize)psy_ui_xt_g_imp_textsize;
		xt_imp_vtable.dev_drawsolidrectangle = (psy_ui_fp_graphicsimp_dev_drawsolidrectangle)psy_ui_xt_g_imp_drawsolidrectangle;
		xt_imp_vtable.dev_drawsolidroundrectangle = (psy_ui_fp_graphicsimp_dev_drawsolidroundrectangle)psy_ui_xt_g_imp_drawsolidroundrectangle;
		xt_imp_vtable.dev_drawsolidpolygon = (psy_ui_fp_graphicsimp_dev_drawsolidpolygon)psy_ui_xt_g_imp_drawsolidpolygon;
		xt_imp_vtable.dev_drawline = (psy_ui_fp_graphicsimp_dev_drawline)psy_ui_xt_g_imp_drawline;
		xt_imp_vtable.dev_drawfullbitmap = (psy_ui_fp_graphicsimp_dev_drawfullbitmap)psy_ui_xt_g_imp_drawfullbitmap;
		xt_imp_vtable.dev_drawbitmap = (psy_ui_fp_graphicsimp_dev_drawbitmap)psy_ui_xt_g_imp_drawbitmap;
		xt_imp_vtable.dev_setbackgroundcolor = (psy_ui_fp_graphicsimp_dev_setbackgroundcolor)psy_ui_xt_g_imp_setbackgroundcolor;
		xt_imp_vtable.dev_setbackgroundmode = (psy_ui_fp_graphicsimp_dev_setbackgroundmode)psy_ui_xt_g_imp_setbackgroundmode;
		xt_imp_vtable.dev_settextcolor = (psy_ui_fp_graphicsimp_dev_settextcolor)psy_ui_xt_g_imp_settextcolor;
		xt_imp_vtable.dev_setcolor = (psy_ui_fp_graphicsimp_dev_setcolor)psy_ui_xt_g_imp_setcolor;
		xt_imp_vtable.dev_setfont = (psy_ui_fp_graphicsimp_dev_setfont)psy_ui_xt_g_imp_setfont;
		xt_imp_vtable.dev_moveto = (psy_ui_fp_graphicsimp_dev_moveto)psy_ui_xt_g_imp_moveto;
		xt_imp_vtable.dev_devcurveto = (psy_ui_fp_graphicsimp_dev_curveto)psy_ui_xt_g_imp_devcurveto;
		xt_imp_vtable_initialized = 1;
	}
}

void psy_ui_xt_graphicsimp_init(psy_ui_xt_GraphicsImp* self, 
	PlatformXtGC* platformgc)
{
	int s;
	
	psy_ui_graphics_imp_init(&self->imp);
	xt_imp_vtable_init(self);
	self->imp.vtable = &xt_imp_vtable;
	self->display = platformgc->display;
	self->window = platformgc->window;
	self->gc = platformgc->gc;
	s = DefaultScreen(self->display);
	self->xfd = XftDrawCreate(
		self->display,
		self->window,
		DefaultVisual(self->display, s),
	    DefaultColormap(self->display, s));
	self->xftfont = XftFontOpenXlfd(self->display,
		s,
		"arial");
	if (!self->xftfont) {
		self->xftfont = XftFontOpenName(self->display,
			s, "arial");
	}		
	XftColorAllocName(self->display,
		DefaultVisual(self->display, s),
		DefaultColormap(self->display, s),
		"black", &self->black);
}

// xt implementation method for psy_ui_Graphics
void psy_ui_xt_g_imp_dispose(psy_ui_xt_GraphicsImp* self)
{	
	XFreeGC(self->display, self->gc);
	XftColorFree(self->display,
	   DefaultVisual(self->display, DefaultScreen(self->display)),
	   DefaultColormap(self->display, DefaultScreen(self->display)),
	   &self->black);
	XftFontClose(self->display, self->xftfont);
	XftDrawDestroy(self->xfd);
}

void psy_ui_xt_g_imp_textout(psy_ui_xt_GraphicsImp* self, int x, int y,
	const char* str, size_t len)
{	
	XftDrawStringUtf8(self->xfd, &self->black, 
		self->xftfont, x, y + self->xftfont->ascent, (const FcChar8*)str,
		(int) len);
}

void psy_ui_xt_g_imp_textoutrectangle(psy_ui_xt_GraphicsImp* self, int x, int y,
	unsigned int options, psy_ui_Rectangle r, const char* str, size_t len)
{
	XftDrawStringUtf8(self->xfd, &self->black, 
		self->xftfont, x, y + self->xftfont->ascent, (const FcChar8*)str,
		(int) len);
//	RECT rect;
//	int woptions = 0;

//	if ((options & psy_ui_ETO_OPAQUE) == psy_ui_ETO_OPAQUE) {
//		woptions |= ETO_OPAQUE;
//	}
//	if ((options & psy_ui_ETO_CLIPPED) == psy_ui_ETO_CLIPPED) {
//		woptions |= ETO_CLIPPED;
//	}
//    SetRect (&rect, r.left, r.top, r.right, r.bottom) ;     	
//	ExtTextOut(self->hdc, x, y, woptions, &rect, text, (int)len, NULL);
}

psy_ui_Size psy_ui_xt_g_imp_textsize(psy_ui_xt_GraphicsImp* self,
    const char* str)
{
	psy_ui_Size	rv;

	if (str) {
		XGlyphInfo extents;
	
		XftTextExtentsUtf8(self->display, self->xftfont,
		(const FcChar8*)str,
		strlen(str),
		&extents);
						
		rv.width = extents.width; 
		rv.height = extents.height;
	} else {
		rv.width = 0; 
		rv.height = 0;
	}
	return rv;
}

void psy_ui_xt_g_imp_drawrectangle(psy_ui_xt_GraphicsImp* self,
    const psy_ui_Rectangle r)
{
	XDrawRectangle(self->display, self->window, self->gc, r.left, r.top,
	   r.right- r.left, r.bottom - r.top);
}

void psy_ui_xt_g_imp_drawroundrectangle(psy_ui_xt_GraphicsImp* self,
    const psy_ui_Rectangle r, psy_ui_Size cornersize)
{
//	HBRUSH hBrush;
//	HBRUSH hOldBrush;

//	hBrush = GetStockObject (NULL_BRUSH);
//	hOldBrush = SelectObject(self->hdc, hBrush);
//	RoundRect(self->hdc, r.left, r.top, r.right, r.bottom, cornersize.width,
//		cornersize.height);
//	SelectObject(self->hdc, hOldBrush);
}

void psy_ui_xt_g_imp_drawsolidrectangle(psy_ui_xt_GraphicsImp* self,
	const psy_ui_Rectangle r, unsigned int color)
{
	XGCValues xgcv;
	
	XGetGCValues(self->display, self->gc, GCForeground | GCBackground, &xgcv);
	XSetForeground(self->display, self->gc, color);
	XFillRectangle(self->display, self->window, self->gc,
		r.left, r.top,
		r.right- r.left, r.bottom - r.top);
	XChangeGC(self->display, self->gc, GCForeground | GCBackground,
			&xgcv);
}

void psy_ui_xt_g_imp_drawsolidroundrectangle(psy_ui_xt_GraphicsImp* self,
    const psy_ui_Rectangle r, psy_ui_Size cornersize, unsigned int color)
{
//	HBRUSH hBrush;
//	HBRUSH hOldBrush;
//	HPEN hPen;
//	HPEN hOldPen;

//	hBrush = CreateSolidBrush(color);
//	hOldBrush = SelectObject (self->hdc, hBrush);
//	hPen = CreatePen(PS_SOLID, 1, color);
//	hOldPen = SelectObject(self->hdc, hPen);
//	RoundRect(self->hdc, r.left, r.top, r.right, r.bottom, cornersize.width,
//		cornersize.height);
//	SelectObject(self->hdc, hOldBrush);
//	SelectObject(self->hdc, hOldPen);
//	DeleteObject(hBrush) ;
//	DeleteObject(hPen) ;
}

void psy_ui_xt_g_imp_drawsolidpolygon(psy_ui_xt_GraphicsImp* self,
    psy_ui_Point* pts, unsigned int numpoints, unsigned int inner,
    unsigned int outter)
{
//	HBRUSH hBrush;     
//	HBRUSH hBrushPrev;
//	HPEN hPen;     
//	HPEN hPenPrev;
//	POINT* wpts;
//	unsigned int i;

//	wpts = (POINT*)malloc(sizeof(POINT) * numpoints);
//	for (i = 0; i < numpoints; ++i) {
//		wpts[i].x = pts[i].x;
//		wpts[i].y = pts[i].y;
//	}    
//    hBrush = CreateSolidBrush(inner);
//	hBrushPrev = SelectObject(self->hdc, hBrush);
//	hPen = CreatePen(PS_SOLID, 1, outter);
//	hPenPrev = SelectObject(self->hdc, hPen);	
//	Polygon(self->hdc, wpts, numpoints);
//	SelectObject(self->hdc, hBrushPrev);
//	SelectObject(self->hdc, hPenPrev);
//  DeleteObject (hBrush);
//	DeleteObject (hPen);
//	free(wpts);
}

void psy_ui_xt_g_imp_drawfullbitmap(psy_ui_xt_GraphicsImp* self,
    psy_ui_Bitmap* bitmap, int x, int y)
{
    Pixmap xtbitmap;    
    
    xtbitmap = ((psy_ui_xt_BitmapImp*)bitmap->imp)->pixmap;
    if (xtbitmap) {
        psy_ui_Size size;       
        
        size = psy_ui_bitmap_size(bitmap);
        XCopyArea(self->display, xtbitmap, self->window, self->gc,
            0, 0, size.width, size.height,
            x, y);
    }
}

void psy_ui_xt_g_imp_drawbitmap(psy_ui_xt_GraphicsImp* self,
	psy_ui_Bitmap* bitmap, int x, int y, int width,
	int height, int xsrc, int ysrc)
{
    Pixmap xtbitmap; 
    
    xtbitmap = ((psy_ui_xt_BitmapImp*)bitmap->imp)->pixmap;
    if (xtbitmap) {        
        XCopyArea(self->display, xtbitmap, self->window, self->gc,
            xsrc, ysrc, width, height,
            x, y);
    }
}

void psy_ui_xt_g_imp_setcolor(psy_ui_xt_GraphicsImp* self, unsigned int color)
{
	XSetForeground(self->display, self->gc, color);
}

void psy_ui_xt_g_imp_setbackgroundmode(psy_ui_xt_GraphicsImp* self,
    unsigned int mode)
{
	if (mode == psy_ui_TRANSPARENT) {
//		SetBkMode(self->hdc, TRANSPARENT);
	} else
	if (mode == psy_ui_OPAQUE) {
//		SetBkMode(self->hdc, OPAQUE);
	}
}

void psy_ui_xt_g_imp_setbackgroundcolor(psy_ui_xt_GraphicsImp* self,
    unsigned int color)
{
	XSetBackground(self->display, self->gc, color);
}

void psy_ui_xt_g_imp_settextcolor(psy_ui_xt_GraphicsImp* self,
    unsigned int color)
{
//	SetTextColor(self->hdc, color);
}

void psy_ui_xt_g_imp_setfont(psy_ui_xt_GraphicsImp* self, psy_ui_Font* font)
{	
//	if (font && ((psy_ui_xt_FontImp*)font->imp)->hfont) {		
//		SelectObject(self->hdc, ((psy_ui_xt_FontImp*)font->imp)->hfont);
//	}
}

void psy_ui_xt_g_imp_drawline(psy_ui_xt_GraphicsImp* self, int x1, int y1,
    int x2, int y2)
{
	XDrawLine(self->display, self->window, self->gc, x1, y1, x2, y2);
}

void psy_ui_xt_g_imp_moveto(psy_ui_xt_GraphicsImp* self, psy_ui_Point pt)
{	
//	MoveToEx(self->hdc, pt.x, pt.y, NULL);
}

void psy_ui_xt_g_imp_devcurveto(psy_ui_xt_GraphicsImp* self,
    psy_ui_Point control_p1, psy_ui_Point control_p2, psy_ui_Point p)
{
	/* POINT pts[3];
   
	pts[0].x = control_p1.x;
	pts[0].y = control_p1.y;
	pts[1].x = control_p2.x;
	pts[1].y = control_p2.y;
	pts[2].x = p.x;
	pts[2].y = p.y;
	PolyBezierTo(self->hdc, pts, 3); */
}

#endif
