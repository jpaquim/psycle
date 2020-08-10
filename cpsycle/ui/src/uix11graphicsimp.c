// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uix11graphicsimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT

//#include "uiwinfontimp.h"
#include "uix11bitmapimp.h"
#include "uiapp.h"
#include <stdlib.h>
#include	<X11/Xmu/Drawing.h>

extern psy_ui_App app;

// VTable Prototypes
static void psy_ui_x11_g_imp_dispose(psy_ui_x11_GraphicsImp*);
static void psy_ui_x11_g_imp_textout(psy_ui_x11_GraphicsImp*, int x, int y,
	const char*, size_t len);
static void psy_ui_x11_g_imp_textoutrectangle(psy_ui_x11_GraphicsImp*, int x,
	int y, unsigned int options, psy_ui_Rectangle, const char* text,
	size_t len);
static void psy_ui_x11_g_imp_drawrectangle(psy_ui_x11_GraphicsImp*,
	const psy_ui_Rectangle);
static void psy_ui_x11_g_imp_drawroundrectangle(psy_ui_x11_GraphicsImp*,
	const psy_ui_Rectangle, psy_ui_Size cornersize);
static psy_ui_Size psy_ui_x11_g_imp_textsize(psy_ui_x11_GraphicsImp*,
	const char*);
static void psy_ui_x11_g_imp_drawsolidrectangle(psy_ui_x11_GraphicsImp*,
	const psy_ui_Rectangle, psy_ui_Color color);
static void psy_ui_x11_g_imp_drawsolidroundrectangle(psy_ui_x11_GraphicsImp*,
	const psy_ui_Rectangle r, psy_ui_Size cornersize, psy_ui_Color);
static void psy_ui_x11_g_imp_drawsolidpolygon(psy_ui_x11_GraphicsImp*,
	psy_ui_IntPoint*,
	unsigned int numpoints, unsigned int inner, unsigned int outter);
static void psy_ui_x11_g_imp_drawline(psy_ui_x11_GraphicsImp*, int x1, int y1,
	int x2, int y2);
static void psy_ui_x11_g_imp_drawfullbitmap(psy_ui_x11_GraphicsImp*,
	psy_ui_Bitmap*, int x, int y);
static void psy_ui_x11_g_imp_drawbitmap(psy_ui_x11_GraphicsImp*, psy_ui_Bitmap*,
	int x, int y, int width,
	int height, int xsrc, int ysrc);
static void psy_ui_x11_g_imp_setbackgroundcolor(psy_ui_x11_GraphicsImp*,
	psy_ui_Color);
static void psy_ui_x11_g_imp_setbackgroundmode(psy_ui_x11_GraphicsImp*,
	unsigned int mode);
static void psy_ui_x11_g_imp_settextcolor(psy_ui_x11_GraphicsImp*,
	psy_ui_Color);
static void psy_ui_x11_g_imp_setcolor(psy_ui_x11_GraphicsImp*, psy_ui_Color);
static void psy_ui_x11_g_imp_setfont(psy_ui_x11_GraphicsImp*, psy_ui_Font*);
static void psy_ui_x11_g_imp_moveto(psy_ui_x11_GraphicsImp*, psy_ui_IntPoint);
static void psy_ui_x11_g_imp_devcurveto(psy_ui_x11_GraphicsImp*,
	psy_ui_IntPoint control_p1, psy_ui_IntPoint control_p2, psy_ui_IntPoint p);

// VTable init
static psy_ui_GraphicsImpVTable xt_imp_vtable;
static int xt_imp_vtable_initialized = 0;

static void xt_imp_vtable_init(psy_ui_x11_GraphicsImp* self)
{
	if (!xt_imp_vtable_initialized) {
		xt_imp_vtable = *self->imp.vtable;
		xt_imp_vtable.dev_dispose = (psy_ui_fp_graphicsimp_dev_dispose)
			psy_ui_x11_g_imp_dispose;
		xt_imp_vtable.dev_textout = (psy_ui_fp_graphicsimp_dev_textout)
			psy_ui_x11_g_imp_textout;
		xt_imp_vtable.dev_textoutrectangle =
			(psy_ui_fp_graphicsimp_dev_textoutrectangle)
			psy_ui_x11_g_imp_textoutrectangle;
		xt_imp_vtable.dev_drawrectangle =
			(psy_ui_fp_graphicsimp_dev_drawrectangle)
			psy_ui_x11_g_imp_drawrectangle;
		xt_imp_vtable.dev_drawroundrectangle =
			(psy_ui_fp_graphicsimp_dev_drawroundrectangle)
			psy_ui_x11_g_imp_drawroundrectangle;
		xt_imp_vtable.dev_textsize = (psy_ui_fp_graphicsimp_dev_textsize)
			psy_ui_x11_g_imp_textsize;
		xt_imp_vtable.dev_drawsolidrectangle =
			(psy_ui_fp_graphicsimp_dev_drawsolidrectangle)
			psy_ui_x11_g_imp_drawsolidrectangle;
		xt_imp_vtable.dev_drawsolidroundrectangle =
			(psy_ui_fp_graphicsimp_dev_drawsolidroundrectangle)
			psy_ui_x11_g_imp_drawsolidroundrectangle;
		xt_imp_vtable.dev_drawsolidpolygon =
			(psy_ui_fp_graphicsimp_dev_drawsolidpolygon)
			psy_ui_x11_g_imp_drawsolidpolygon;
		xt_imp_vtable.dev_drawline = (psy_ui_fp_graphicsimp_dev_drawline)
			psy_ui_x11_g_imp_drawline;
		xt_imp_vtable.dev_drawfullbitmap =
			(psy_ui_fp_graphicsimp_dev_drawfullbitmap)
			psy_ui_x11_g_imp_drawfullbitmap;
		xt_imp_vtable.dev_drawbitmap = (psy_ui_fp_graphicsimp_dev_drawbitmap)
			psy_ui_x11_g_imp_drawbitmap;
		xt_imp_vtable.dev_setbackgroundcolor =
			(psy_ui_fp_graphicsimp_dev_setbackgroundcolor)
			psy_ui_x11_g_imp_setbackgroundcolor;
		xt_imp_vtable.dev_setbackgroundmode =
			(psy_ui_fp_graphicsimp_dev_setbackgroundmode)
			psy_ui_x11_g_imp_setbackgroundmode;
		xt_imp_vtable.dev_settextcolor =
			(psy_ui_fp_graphicsimp_dev_settextcolor)
			psy_ui_x11_g_imp_settextcolor;
		xt_imp_vtable.dev_setcolor =
			(psy_ui_fp_graphicsimp_dev_setcolor)psy_ui_x11_g_imp_setcolor;
		xt_imp_vtable.dev_setfont =
			(psy_ui_fp_graphicsimp_dev_setfont)psy_ui_x11_g_imp_setfont;
		xt_imp_vtable.dev_moveto =
			(psy_ui_fp_graphicsimp_dev_moveto)psy_ui_x11_g_imp_moveto;
		xt_imp_vtable.dev_curveto =
			(psy_ui_fp_graphicsimp_dev_curveto)psy_ui_x11_g_imp_devcurveto;
		xt_imp_vtable_initialized = 1;
	}
}

void psy_ui_x11_graphicsimp_init(psy_ui_x11_GraphicsImp* self, 
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
		s, "arial");
	if (!self->xftfont) {
		self->xftfont = XftFontOpenName(self->display,
			s, "arial");
	}		
	XftColorAllocName(self->display,
		DefaultVisual(self->display, s),
		DefaultColormap(self->display, s),
		"black", &self->black);
	self->textcolor.color.red   = 0xFFFF;
	self->textcolor.color.green = 0xFFFF;
	self->textcolor.color.blue  = 0xFFFF;
	self->textcolor.color.alpha = 0xFFFF;
	self->textbackgroundcolor = psy_ui_color_make(0x00232323);
	self->backgroundmode = psy_ui_OPAQUE;
}

// xt implementation method for psy_ui_Graphics
void psy_ui_x11_g_imp_dispose(psy_ui_x11_GraphicsImp* self)
{	
	XFreeGC(self->display, self->gc);
	XftColorFree(self->display,
	   DefaultVisual(self->display, DefaultScreen(self->display)),
	   DefaultColormap(self->display, DefaultScreen(self->display)),
	   &self->black);
	XftFontClose(self->display, self->xftfont);
	XftDrawDestroy(self->xfd);
}

void psy_ui_x11_g_imp_textout(psy_ui_x11_GraphicsImp* self, int x, int y,
	const char* str, size_t len)
{	
	if (!str) {
		return;
	}
	if (self->backgroundmode  == psy_ui_OPAQUE) {
		XGlyphInfo extents;
		psy_ui_Rectangle r;
	
		XftTextExtentsUtf8(self->display, self->xftfont,
			(const FcChar8*)str,
			strlen(str),
			&extents);
		r.left = x;
		r.top = y;
		r.right = extents.width;
		r.bottom = extents.height;
		psy_ui_x11_g_imp_drawsolidrectangle(self, r,
			self->textbackgroundcolor);		
	}
	XftDrawStringUtf8(self->xfd, &self->textcolor, 
		self->xftfont, x, y + self->xftfont->ascent, (const FcChar8*)str,
		(int) len);
}

void psy_ui_x11_g_imp_textoutrectangle(psy_ui_x11_GraphicsImp* self, int x, int y,
	unsigned int options, psy_ui_Rectangle r, const char* str, size_t len)
{
	if (self->backgroundmode  == psy_ui_OPAQUE ||
			((options & psy_ui_ETO_OPAQUE) == psy_ui_ETO_OPAQUE)) {
		psy_ui_Rectangle temp;
		
		temp.left = x;
		temp.top = y;
		temp.right = r.right;
		temp.bottom = r.bottom;
		psy_ui_x11_g_imp_drawsolidrectangle(self, r,
			self->textbackgroundcolor);	
	}
	XftDrawStringUtf8(self->xfd, &self->textcolor, 
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

psy_ui_Size psy_ui_x11_g_imp_textsize(psy_ui_x11_GraphicsImp* self,
    const char* str)
{
	psy_ui_Size	rv;

	if (str) {
		XGlyphInfo extents;
	
		XftTextExtentsUtf8(self->display, self->xftfont,
		(const FcChar8*)str,
		strlen(str),
		&extents);
						
		rv.width = psy_ui_value_makepx(extents.width); 
		rv.height = psy_ui_value_makepx(extents.height);
	} else {
		rv.width = psy_ui_value_makepx(0); 
		rv.height = psy_ui_value_makepx(0);
	}
	return rv;
}

void psy_ui_x11_g_imp_drawrectangle(psy_ui_x11_GraphicsImp* self,
    const psy_ui_Rectangle r)
{
	XDrawRectangle(self->display, self->window, self->gc, r.left, r.top,
	   r.right - r.left - 1, r.bottom - r.top - 1);
}

void psy_ui_x11_g_imp_drawroundrectangle(psy_ui_x11_GraphicsImp* self,
    const psy_ui_Rectangle r, psy_ui_Size cornersize)
{	
	XmuDrawRoundedRectangle(self->display,
		self->window, self->gc,
		r.left, r.top, r.right - r.left, r.bottom - r.top,
		psy_ui_value_px(&cornersize.width, NULL),
		psy_ui_value_px(&cornersize.height, NULL));	
}

void psy_ui_x11_g_imp_drawsolidrectangle(psy_ui_x11_GraphicsImp* self,
	const psy_ui_Rectangle r, psy_ui_Color color)
{
	XGCValues xgcv;
	
	XGetGCValues(self->display, self->gc, GCForeground | GCBackground, &xgcv);
	XSetForeground(self->display, self->gc, color.value);
	XFillRectangle(self->display, self->window, self->gc,
		r.left, r.top,
		r.right- r.left, r.bottom - r.top);
	XChangeGC(self->display, self->gc, GCForeground | GCBackground,
			&xgcv);
}

void psy_ui_x11_g_imp_drawsolidroundrectangle(psy_ui_x11_GraphicsImp* self,
    const psy_ui_Rectangle r, psy_ui_Size cornersize, psy_ui_Color color)
{
	XmuFillRoundedRectangle(self->display,
		self->window, self->gc,
		r.left, r.top, r.right - r.left, r.bottom - r.top,
		psy_ui_value_px(&cornersize.width, NULL),
		psy_ui_value_px(&cornersize.height, NULL));
}

void psy_ui_x11_g_imp_drawsolidpolygon(psy_ui_x11_GraphicsImp* self,
    psy_ui_IntPoint* pts, unsigned int numpoints, unsigned int inner,
    unsigned int outter)
{
	XPoint* xpts;
	unsigned int i;
	XGCValues xgcv;
	
	xpts = (XPoint*)malloc(sizeof(XPoint) * numpoints);
	for (i = 0; i < numpoints; ++i) {
		xpts[i].x = pts[i].x;
		xpts[i].y = pts[i].y;
	} 	
	XGetGCValues(self->display, self->gc, GCForeground | GCBackground, &xgcv);
	XSetForeground(self->display, self->gc, inner);	
	XFillPolygon(
      self->display,
      self->window,
      self->gc,
      xpts,
      numpoints,
      Complex,
      CoordModeOrigin);
	XChangeGC(self->display, self->gc, GCForeground | GCBackground,
			&xgcv);  
	free(xpts); 
}

void psy_ui_x11_g_imp_drawfullbitmap(psy_ui_x11_GraphicsImp* self,
    psy_ui_Bitmap* bitmap, int x, int y)
{
    Pixmap xtbitmap;    
    
    xtbitmap = ((psy_ui_x11_BitmapImp*)bitmap->imp)->pixmap;
    if (xtbitmap) {
        psy_ui_Size size;       
        
        size = psy_ui_bitmap_size(bitmap);
        XCopyArea(self->display, xtbitmap, self->window, self->gc,
            0, 0, psy_ui_value_px(&size.width, 0),
            psy_ui_value_px(&size.height, 0),
            x, y);
    }
}

void psy_ui_x11_g_imp_drawbitmap(psy_ui_x11_GraphicsImp* self,
	psy_ui_Bitmap* bitmap, int x, int y, int width,
	int height, int xsrc, int ysrc)
{
    Pixmap xtbitmap; 
    
    xtbitmap = ((psy_ui_x11_BitmapImp*)bitmap->imp)->pixmap;
    if (xtbitmap) {        
        XCopyArea(self->display, xtbitmap, self->window, self->gc,
            xsrc, ysrc, width, height,
            x, y);
    }
}

void psy_ui_x11_g_imp_setcolor(psy_ui_x11_GraphicsImp* self, psy_ui_Color color)
{
	XSetForeground(self->display, self->gc, color.value);
}

void psy_ui_x11_g_imp_setbackgroundmode(psy_ui_x11_GraphicsImp* self,
    unsigned int mode)
{
	self->backgroundmode = mode;	
}

void psy_ui_x11_g_imp_setbackgroundcolor(psy_ui_x11_GraphicsImp* self,
    psy_ui_Color color)
{
	self->textbackgroundcolor = color;
	XSetBackground(self->display, self->gc, color.value);
}

void psy_ui_x11_g_imp_settextcolor(psy_ui_x11_GraphicsImp* self,
    psy_ui_Color color)
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	
	psy_ui_color_rgb(&color, &r, &g, &b);
	self->textcolor.color.red   = r * 256;
	self->textcolor.color.green = g * 256;
	self->textcolor.color.blue  = b * 256;
	self->textcolor.color.alpha = 0xFFFF;
}

void psy_ui_x11_g_imp_setfont(psy_ui_x11_GraphicsImp* self, psy_ui_Font* font)
{	
//	if (font && ((psy_ui_x11_FontImp*)font->imp)->hfont) {		
//		SelectObject(self->hdc, ((psy_ui_x11_FontImp*)font->imp)->hfont);
//	}
}

void psy_ui_x11_g_imp_drawline(psy_ui_x11_GraphicsImp* self, int x1, int y1,
    int x2, int y2)
{
	XDrawLine(self->display, self->window, self->gc, x1, y1, x2, y2);
}

void psy_ui_x11_g_imp_moveto(psy_ui_x11_GraphicsImp* self, psy_ui_IntPoint pt)
{	
//	MoveToEx(self->hdc, pt.x, pt.y, NULL);
}

void psy_ui_x11_g_imp_devcurveto(psy_ui_x11_GraphicsImp* self,
    psy_ui_IntPoint control_p1, psy_ui_IntPoint control_p2, psy_ui_IntPoint p)
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
