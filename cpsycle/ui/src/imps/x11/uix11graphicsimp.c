/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uix11graphicsimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11

#include "uix11app.h"
#include "uix11fontimp.h"
#include "uix11bitmapimp.h"
#include "uiapp.h"
#include <stdlib.h>
#include <X11/Xmu/Drawing.h>

/* prototypes */
static void psy_ui_x11_g_imp_dispose(psy_ui_x11_GraphicsImp*);
static void psy_ui_x11_g_imp_textout(psy_ui_x11_GraphicsImp*, double x, double y, const char*, uintptr_t len);
static void psy_ui_x11_g_imp_textoutrectangle(psy_ui_x11_GraphicsImp*, double x, double y, uintptr_t options,
	psy_ui_RealRectangle r, const char* text, uintptr_t len);
static void psy_ui_x11_g_imp_drawrectangle(psy_ui_x11_GraphicsImp*, const psy_ui_RealRectangle);
static void psy_ui_x11_g_imp_drawroundrectangle(psy_ui_x11_GraphicsImp*, const psy_ui_RealRectangle,
	psy_ui_RealSize cornersize);
static psy_ui_Size psy_ui_x11_g_imp_textsize(psy_ui_x11_GraphicsImp*,
	const char*, uintptr_t count);
static void psy_ui_x11_g_imp_drawsolidrectangle(psy_ui_x11_GraphicsImp*, const psy_ui_RealRectangle r,
	psy_ui_Colour colour);
static void psy_ui_x11_g_imp_drawsolidroundrectangle(psy_ui_x11_GraphicsImp*, const psy_ui_RealRectangle r,
	psy_ui_RealSize cornersize, psy_ui_Colour colour);
static void psy_ui_x11_g_imp_drawsolidpolygon(psy_ui_x11_GraphicsImp*, psy_ui_RealPoint*,
	uintptr_t numpoints, uint32_t inner, uint32_t outter);
static void psy_ui_x11_g_imp_drawline(psy_ui_x11_GraphicsImp*, double x1, double y1, double x2, double y2);
static void psy_ui_x11_g_imp_drawfullbitmap(psy_ui_x11_GraphicsImp*, psy_ui_Bitmap*, double x, double y);
static void psy_ui_x11_g_imp_drawbitmap(psy_ui_x11_GraphicsImp*, psy_ui_Bitmap*, double x, double y, double width,
	double height, double xsrc, double ysrc);
static void psy_ui_x11_g_imp_drawstretchedbitmap(psy_ui_x11_GraphicsImp*, psy_ui_Bitmap*, double x, double y, double width,
	double height, double xsrc, double ysrc, double wsrc, double hsrc);
static void psy_ui_x11_g_imp_setbackgroundcolour(psy_ui_x11_GraphicsImp*, psy_ui_Colour colour);
static void psy_ui_x11_g_imp_setbackgroundmode(psy_ui_x11_GraphicsImp*, uintptr_t mode);
static void psy_ui_x11_g_imp_settextcolour(psy_ui_x11_GraphicsImp*, psy_ui_Colour colour);
static void psy_ui_x11_g_imp_settextalign(psy_ui_x11_GraphicsImp*, uintptr_t align);
static void psy_ui_x11_g_imp_setcolour(psy_ui_x11_GraphicsImp*, psy_ui_Colour colour);
static void psy_ui_x11_g_imp_setfont(psy_ui_x11_GraphicsImp*, psy_ui_Font* font);
static void psy_ui_x11_g_imp_moveto(psy_ui_x11_GraphicsImp*, psy_ui_RealPoint pt);
static void psy_ui_x11_g_imp_devcurveto(psy_ui_x11_GraphicsImp*, psy_ui_RealPoint control_p1,
	psy_ui_RealPoint control_p2, psy_ui_RealPoint p);
static void psy_ui_x11_g_imp_devdrawarc(psy_ui_x11_GraphicsImp*,
	psy_ui_RealRectangle, double angle_start, double angle_end);
static void psy_ui_x11_g_devsetlinewidth(psy_ui_x11_GraphicsImp*, uintptr_t width);
static unsigned int psy_ui_x11_g_devlinewidth(psy_ui_x11_GraphicsImp*);
static void psy_ui_x11_g_devsetorigin(psy_ui_x11_GraphicsImp*, double x, double y);
static psy_ui_RealPoint psy_ui_x11_g_devorigin(const psy_ui_x11_GraphicsImp*);
static int psy_ui_x11_g_imp_colourindex(psy_ui_x11_GraphicsImp*, psy_ui_Colour);
static uintptr_t psy_ui_x11_g_dev_gc(psy_ui_x11_GraphicsImp*);
static void psy_ui_x11_g_dev_setcliprect(psy_ui_x11_GraphicsImp*, psy_ui_RealRectangle);
static psy_ui_RealRectangle psy_ui_x11_g_dev_cliprect(const psy_ui_x11_GraphicsImp*);

/* vtable */
static psy_ui_GraphicsImpVTable x11_imp_vtable;
static bool x11_imp_vtable_initialized = FALSE;

static void x11_imp_vtable_init(psy_ui_x11_GraphicsImp* self)
{
	if (!x11_imp_vtable_initialized) {
		x11_imp_vtable = *self->imp.vtable;		
		x11_imp_vtable.dev_dispose =
			(psy_ui_fp_graphicsimp_dev_dispose)
			psy_ui_x11_g_imp_dispose;
		x11_imp_vtable.dev_textout =
			(psy_ui_fp_graphicsimp_dev_textout)
			psy_ui_x11_g_imp_textout;
		x11_imp_vtable.dev_textoutrectangle =
			(psy_ui_fp_graphicsimp_dev_textoutrectangle)
			psy_ui_x11_g_imp_textoutrectangle;
		x11_imp_vtable.dev_drawrectangle =
			(psy_ui_fp_graphicsimp_dev_drawrectangle)
			psy_ui_x11_g_imp_drawrectangle;
		x11_imp_vtable.dev_drawroundrectangle =
			(psy_ui_fp_graphicsimp_dev_drawroundrectangle)
			psy_ui_x11_g_imp_drawroundrectangle;
		x11_imp_vtable.dev_textsize =
			(psy_ui_fp_graphicsimp_dev_textsize)
			psy_ui_x11_g_imp_textsize;
		x11_imp_vtable.dev_drawsolidrectangle =
			(psy_ui_fp_graphicsimp_dev_drawsolidrectangle)
			psy_ui_x11_g_imp_drawsolidrectangle;
		x11_imp_vtable.dev_drawsolidroundrectangle =
			(psy_ui_fp_graphicsimp_dev_drawsolidroundrectangle)
			psy_ui_x11_g_imp_drawsolidroundrectangle;
		x11_imp_vtable.dev_drawsolidpolygon =
			(psy_ui_fp_graphicsimp_dev_drawsolidpolygon)
			psy_ui_x11_g_imp_drawsolidpolygon;
		x11_imp_vtable.dev_drawline =
			(psy_ui_fp_graphicsimp_dev_drawline)
			psy_ui_x11_g_imp_drawline;
		x11_imp_vtable.dev_drawfullbitmap =
			(psy_ui_fp_graphicsimp_dev_drawfullbitmap)
			psy_ui_x11_g_imp_drawfullbitmap;
		x11_imp_vtable.dev_drawbitmap =
			(psy_ui_fp_graphicsimp_dev_drawbitmap)
			psy_ui_x11_g_imp_drawbitmap;
		x11_imp_vtable.dev_drawstretchedbitmap =
			(psy_ui_fp_graphicsimp_dev_drawstretchedbitmap)
			psy_ui_x11_g_imp_drawstretchedbitmap;
		x11_imp_vtable.dev_setbackgroundcolour =
			(psy_ui_fp_graphicsimp_dev_setbackgroundcolour)
			psy_ui_x11_g_imp_setbackgroundcolour;
		x11_imp_vtable.dev_setbackgroundmode =
			(psy_ui_fp_graphicsimp_dev_setbackgroundmode)
			psy_ui_x11_g_imp_setbackgroundmode;
		x11_imp_vtable.dev_settextcolour =
			(psy_ui_fp_graphicsimp_dev_settextcolour)
			psy_ui_x11_g_imp_settextcolour;
		x11_imp_vtable.dev_settextalign =
			(psy_ui_fp_graphicsimp_dev_settextalign)
			psy_ui_x11_g_imp_settextalign;
		x11_imp_vtable.dev_setcolour =
			(psy_ui_fp_graphicsimp_dev_setcolour)
			psy_ui_x11_g_imp_setcolour;
		x11_imp_vtable.dev_setfont =
			(psy_ui_fp_graphicsimp_dev_setfont)
			psy_ui_x11_g_imp_setfont;
		x11_imp_vtable.dev_moveto =
			(psy_ui_fp_graphicsimp_dev_moveto)
			psy_ui_x11_g_imp_moveto;
		x11_imp_vtable.dev_curveto =
			(psy_ui_fp_graphicsimp_dev_curveto)
			psy_ui_x11_g_imp_devcurveto;
		x11_imp_vtable.dev_drawarc =
			(psy_ui_fp_graphicsimp_dev_drawarc)
			psy_ui_x11_g_imp_devdrawarc;
		x11_imp_vtable.dev_setlinewidth =
			(psy_ui_fp_graphicsimp_dev_setlinewidth)
			psy_ui_x11_g_devsetlinewidth;
		x11_imp_vtable.dev_linewidth =
			(psy_ui_fp_graphicsimp_dev_linewidth)
			psy_ui_x11_g_devlinewidth;
		x11_imp_vtable.dev_setorigin =
			(psy_ui_fp_graphicsimp_dev_setorigin)
			psy_ui_x11_g_devsetorigin;
		x11_imp_vtable.dev_origin =
			(psy_ui_fp_graphicsimp_dev_origin)
			psy_ui_x11_g_devorigin;
		x11_imp_vtable.dev_gc =
			(psy_ui_fp_graphicsimp_dev_gc)
			psy_ui_x11_g_dev_gc;
		x11_imp_vtable.dev_setcliprect =
			(psy_ui_fp_graphicsimp_dev_setcliprect)
			psy_ui_x11_g_dev_setcliprect;
		x11_imp_vtable.dev_cliprect =
			(psy_ui_fp_graphicsimp_dev_cliprect)
			psy_ui_x11_g_dev_cliprect;
		x11_imp_vtable_initialized = TRUE;
	}
	self->imp.vtable = &x11_imp_vtable;
}

void psy_ui_x11_graphicsimp_init(psy_ui_x11_GraphicsImp* self, 
	PlatformXtGC* platformgc)
{
	int screen;
	
	psy_ui_graphics_imp_init(&self->imp);
	x11_imp_vtable_init(self);	
	self->display = platformgc->display;
	self->window = platformgc->window;
	self->visual = platformgc->visual;
	self->gc = platformgc->gc;
	screen = DefaultScreen(self->display);
	self->xfd = XftDrawCreate(
		self->display,
		self->window,
		self->visual,
	    DefaultColormap(self->display, screen));
	self->defaultfont = XftFontOpenXlfd(self->display,
		screen, "arial");
	if (!self->defaultfont) {
		self->defaultfont = XftFontOpenName(self->display, screen, "arial");
	}
	self->xftfont = self->defaultfont;
	XftColorAllocName(self->display,
		self->visual,
		DefaultColormap(self->display, screen),
		"black", &self->black);
	self->textcolor.color.red   = 0xFFFF;
	self->textcolor.color.green = 0xFFFF;
	self->textcolor.color.blue  = 0xFFFF;
	self->textcolor.color.alpha = 0xFFFF;
	self->textbackgroundcolor = psy_ui_colour_make(0x00232323);
	self->backgroundmode = psy_ui_OPAQUE;
	psy_ui_realpoint_init(&self->org);
	self->region = XCreateRegion();
	self->bitmap = FALSE;
	psy_ui_realpoint_init(&self->cp);
	self->clip = psy_ui_realrectangle_zero();
}

void psy_ui_x11_graphicsimp_init_bitmap(psy_ui_x11_GraphicsImp* self,
	psy_ui_Bitmap* bitmap)
{
	psy_ui_x11_BitmapImp* imp;
	psy_ui_X11App* x11app;
	int screen;
		
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;	
	psy_ui_graphics_imp_init(&self->imp);
	x11_imp_vtable_init(self);
	imp = (psy_ui_x11_BitmapImp*)bitmap->imp;
	self->display = x11app->dpy;
	self->window = imp->pixmap;
	self->visual = x11app->visual;
	screen = DefaultScreen(self->display);
	self->xfd = XftDrawCreate(
		self->display,
		self->window,
		self->visual,
	    DefaultColormap(self->display, screen));
	self->defaultfont = XftFontOpenXlfd(self->display,
		screen, "arial");
	if (!self->defaultfont) {
		self->defaultfont = XftFontOpenName(self->display, screen, "arial");
	}
	self->xftfont = self->defaultfont;
	XftColorAllocName(self->display,
		self->visual,
		DefaultColormap(self->display, screen),
		"black", &self->black);
	psy_ui_realpoint_init(&self->org);
	self->region = XCreateRegion();
	self->shareddc = FALSE;
	self->bitmap = TRUE;
	self->gc = XCreateGC(x11app->dpy, self->window, 0, NULL);
	psy_ui_realpoint_init(&self->cp);
}

void psy_ui_x11_graphicsimp_updatexft(psy_ui_x11_GraphicsImp* self)
{
	XftDrawDestroy(self->xfd);
	self->xfd = XftDrawCreate(
		self->display,
		self->window,
		self->visual,
	    DefaultColormap(self->display, DefaultScreen(self->display)));	
}

// xt implementation method for psy_ui_Graphics
void psy_ui_x11_g_imp_dispose(psy_ui_x11_GraphicsImp* self)
{		
	XDestroyRegion(self->region);	
	XFreeGC(self->display, self->gc);	
	XftColorFree(self->display,
	   self->visual,	   
	   DefaultColormap(self->display, DefaultScreen(self->display)),
	   &self->black);
	XftFontClose(self->display, self->defaultfont);
	XftDrawDestroy(self->xfd);	
}

void psy_ui_x11_g_imp_textout(psy_ui_x11_GraphicsImp* self, double x, double y,
	const char* str, size_t len)
{	
	if (!str || len == 0) {
		return;
	}
	if (self->backgroundmode  == psy_ui_OPAQUE) {
		XGlyphInfo extents;
		psy_ui_RealRectangle r;
	
		XftTextExtentsUtf8(self->display, self->xftfont,
			(const FcChar8*)str,
			strlen(str),
			&extents);
		r.left = x - (int)(self->org.x),
		r.top = y - (int)(self->org.y);
		r.right = extents.width;
		r.bottom = extents.height;
		psy_ui_x11_g_imp_drawsolidrectangle(self, r,
			self->textbackgroundcolor);		
	}	
	XftDrawStringUtf8(self->xfd, &self->textcolor, 
		self->xftfont,
		x - (int)(self->org.x),
		y - (int)(self->org.y)
		+ self->xftfont->ascent,
		(const FcChar8*)str,
		(int) len);
}

void psy_ui_x11_g_imp_textoutrectangle(psy_ui_x11_GraphicsImp* self,
	double x, double y, uintptr_t options,
	psy_ui_RealRectangle r, const char* str, uintptr_t len)
{
	if (self->backgroundmode  == psy_ui_OPAQUE ||
			((options & psy_ui_ETO_OPAQUE) == psy_ui_ETO_OPAQUE)) {
		psy_ui_RealRectangle temp;
		
		temp.left = x - (int)(self->org.x);
		temp.top = y - (int)(self->org.y);
		temp.right = r.right;
		temp.bottom = r.bottom;
		psy_ui_x11_g_imp_drawsolidrectangle(self, r,
			self->textbackgroundcolor);	
	}
	XftDrawStringUtf8(self->xfd, &self->textcolor, 
		self->xftfont,
		x - (int)(self->org.x),
		y - (int)(self->org.y) +
		self->xftfont->ascent,
		(const FcChar8*)str, (int) len);
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
    const char* str, uintptr_t count)
{
	psy_ui_Size	rv;

	if (str) {		
		XGlyphInfo extents;
							
		XftTextExtentsUtf8(self->display, self->xftfont,
		(const FcChar8*)str,
		count,
		&extents);
								
		rv.width = psy_ui_value_make_px(extents.width); 
		rv.height = psy_ui_value_make_px(extents.height);
	} else {
		rv.width = psy_ui_value_make_px(0); 
		rv.height = psy_ui_value_make_px(0);
	}
	return rv;
}

void psy_ui_x11_g_imp_drawrectangle(psy_ui_x11_GraphicsImp* self,
    const psy_ui_RealRectangle r)
{
	XDrawRectangle(self->display, self->window, self->gc,
		r.left - (int)(self->org.x),
		r.top - (int)(self->org.y),
		r.right - r.left - 1, r.bottom - r.top - 1);
}

void psy_ui_x11_g_imp_drawroundrectangle(psy_ui_x11_GraphicsImp* self,
    const psy_ui_RealRectangle r, psy_ui_RealSize cornersize)
{	
	XmuDrawRoundedRectangle(self->display,
		self->window, self->gc,
		r.left - (int)(self->org.x),
		r.top - (int)(self->org.y),
		r.right - r.left - 1, r.bottom - r.top - 1,
		cornersize.width, cornersize.height);	
}

void psy_ui_x11_g_imp_drawsolidrectangle(psy_ui_x11_GraphicsImp* self,
	const psy_ui_RealRectangle r, psy_ui_Colour color)
{
	XGCValues xgcv;
	psy_ui_X11App* x11app;		

	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	XGetGCValues(self->display, self->gc, GCForeground | GCBackground, &xgcv);
	XSetForeground(self->display, self->gc,
		psy_ui_x11app_colourindex(x11app, color));
	XFillRectangle(self->display, self->window, self->gc,
		r.left - (int)(self->org.x),
		r.top - (int)(self->org.y),
		r.right- r.left, r.bottom - r.top);
	XChangeGC(self->display, self->gc, GCForeground | GCBackground,
			&xgcv);
}

void psy_ui_x11_g_imp_drawsolidroundrectangle(psy_ui_x11_GraphicsImp* self,
    const psy_ui_RealRectangle r, psy_ui_RealSize cornersize, psy_ui_Colour color)
{
	XGCValues xgcv;
	psy_ui_X11App* x11app;		

	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	XGetGCValues(self->display, self->gc, GCForeground | GCBackground, &xgcv);
	XSetForeground(self->display, self->gc,
		psy_ui_x11app_colourindex(x11app, color));		
	XmuFillRoundedRectangle(self->display,
		self->window, self->gc,
		r.left - (int)(self->org.x),
		r.top - (int)(self->org.y),
		r.right - r.left, r.bottom - r.top,
		cornersize.width, cornersize.height);
	XChangeGC(self->display, self->gc, GCForeground | GCBackground,
			&xgcv);
}

void psy_ui_x11_g_imp_drawsolidpolygon(psy_ui_x11_GraphicsImp* self,
     psy_ui_RealPoint* pts,
	uintptr_t numpoints, uint32_t inner, uint32_t outter)
{
	XPoint* xpts;
	unsigned int i;
	XGCValues xgcv;
	psy_ui_X11App* x11app;
	
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	xpts = (XPoint*)malloc(sizeof(XPoint) * numpoints);
	for (i = 0; i < numpoints; ++i) {
		xpts[i].x = pts[i].x - (int)(self->org.x);
		xpts[i].y = pts[i].y - (int)(self->org.y);
	} 	
	XGetGCValues(self->display, self->gc, GCForeground | GCBackground, &xgcv);
	XSetForeground(self->display, self->gc, psy_ui_x11app_colourindex(x11app,
		psy_ui_colour_make(inner)));	
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
    psy_ui_Bitmap* bitmap, double x, double y)
{
    Pixmap xtbitmap;    
    
    xtbitmap = ((psy_ui_x11_BitmapImp*)bitmap->imp)->pixmap;
    if (xtbitmap) {
        psy_ui_RealSize size;       
        
        size = psy_ui_bitmap_size(bitmap);
        XCopyArea(self->display, xtbitmap, self->window, self->gc,
            0, 0, size.width, size.height,
            x - (int)(self->org.x),
            y - (int)(self->org.y));
    }
}

void psy_ui_x11_g_imp_drawbitmap(psy_ui_x11_GraphicsImp* self,
	psy_ui_Bitmap* bitmap, double x, double y, double width,
	double height, double xsrc, double ysrc)
{
    Pixmap xtbitmap; 
    
    xtbitmap = ((psy_ui_x11_BitmapImp*)bitmap->imp)->pixmap;
    if (xtbitmap) {        
        XCopyArea(self->display, xtbitmap, self->window, self->gc,
            xsrc, ysrc, width, height,
            x - (int)(self->org.x),
            y - (int)(self->org.y));
    }
}

void psy_ui_x11_g_imp_drawstretchedbitmap(psy_ui_x11_GraphicsImp* self,
	psy_ui_Bitmap* bitmap, double x, double y, double width,
	double height, double xsrc, double ysrc, double wsrc, double hsrc)
{
	Pixmap xtbitmap; 
    
    xtbitmap = ((psy_ui_x11_BitmapImp*)bitmap->imp)->pixmap;
    if (xtbitmap) {        
        XCopyArea(self->display, xtbitmap, self->window, self->gc,
            xsrc, ysrc, width, height,
            x - (int)(self->org.x),
            y - (int)(self->org.y));
    }
}

void psy_ui_x11_g_imp_setcolour(psy_ui_x11_GraphicsImp* self, psy_ui_Colour color)
{
	psy_ui_X11App* x11app;		

	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	XSetForeground(self->display, self->gc,
		psy_ui_x11app_colourindex(x11app, color));
}

void psy_ui_x11_g_imp_setbackgroundmode(psy_ui_x11_GraphicsImp* self,
    uintptr_t mode)
{
	self->backgroundmode = mode;	
}

void psy_ui_x11_g_imp_setbackgroundcolour(psy_ui_x11_GraphicsImp* self,
    psy_ui_Colour color)
{
	psy_ui_X11App* x11app;		

	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	self->textbackgroundcolor = color;
	XSetBackground(self->display, self->gc,
		psy_ui_x11app_colourindex(x11app, color));
}

void psy_ui_x11_g_imp_settextcolour(psy_ui_x11_GraphicsImp* self,
    psy_ui_Colour color)
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	
	psy_ui_colour_rgb(&color, &r, &g, &b);
	self->textcolor.color.red   = r * 256;
	self->textcolor.color.green = g * 256;
	self->textcolor.color.blue  = b * 256;
	self->textcolor.color.alpha = 0xFFFF;
}

void psy_ui_x11_g_imp_settextalign(psy_ui_x11_GraphicsImp* self, uintptr_t align)
{	
	
}

void psy_ui_x11_g_imp_setfont(psy_ui_x11_GraphicsImp* self, psy_ui_Font* font)
{	
	if (font && ((psy_ui_x11_FontImp*)font->imp)->hfont) {
		self->xftfont = ((psy_ui_x11_FontImp*)(font->imp))->hfont;
	}
}

void psy_ui_x11_g_imp_drawline(psy_ui_x11_GraphicsImp* self,
	double x1, double y1, double x2, double y2)
{
	XDrawLine(self->display, self->window, self->gc,
		x1  - (int)(self->org.x),
		y1  - (int)(self->org.y),
		x2 - (int)(self->org.x),
		y2  - (int)(self->org.y));
}

void psy_ui_x11_g_imp_moveto(psy_ui_x11_GraphicsImp* self, psy_ui_RealPoint pt)
{	
	self->cp = pt;
//	MoveToEx(self->hdc, pt.x, pt.y, NULL);
}

void psy_ui_x11_g_imp_devcurveto(psy_ui_x11_GraphicsImp* self,
	psy_ui_RealPoint control_p1, psy_ui_RealPoint control_p2,
	psy_ui_RealPoint p)    
{
	/* todo */
	
	psy_ui_x11_g_imp_drawline(self,
		self->cp.x - - (int)(self->org.x), self->cp.y - - (int)(self->org.y),
		p.x - (int)(self->org.x), p.y - (int)(self->org.y));
	
	/* POINT pts[3];
   
	pts[0].x = control_p1.x;
	pts[0].y = control_p1.y;
	pts[1].x = control_p2.x;
	pts[1].y = control_p2.y;
	pts[2].x = p.x;
	pts[2].y = p.y;
	PolyBezierTo(self->hdc, pts, 3); */
}

void psy_ui_x11_g_imp_devdrawarc(psy_ui_x11_GraphicsImp* self,
	psy_ui_RealRectangle r, double angle_start, double angle_end)
{
	XDrawArc(self->display, self->window, self->gc,
	 r.left, r.top, r.right - r.left, r.bottom - r.top,
	 (int)(angle_start*64),(int)((angle_end - angle_start)*64));
}

void psy_ui_x11_g_devsetlinewidth(psy_ui_x11_GraphicsImp* self, uintptr_t width)
{
	
}

unsigned int psy_ui_x11_g_devlinewidth(psy_ui_x11_GraphicsImp* self)
{
	return 1;
}

void psy_ui_x11_g_devsetorigin(psy_ui_x11_GraphicsImp* self, double x, double y)
{	
	self->org.x = x;
	self->org.y = y;	
}

psy_ui_RealPoint psy_ui_x11_g_devorigin(const psy_ui_x11_GraphicsImp* self)
{
	return self->org;
}

uintptr_t psy_ui_x11_g_dev_gc(psy_ui_x11_GraphicsImp* self)
{
	return (uintptr_t)self->gc;
}

void psy_ui_x11_g_dev_setcliprect(psy_ui_x11_GraphicsImp* self,
	psy_ui_RealRectangle clip)
{		
	self->clip = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			clip.left - (int)(self->org.x),
			clip.top - (int)(self->org.y)),
		psy_ui_realsize_make(
			clip.right - clip.left,
			clip.bottom - clip.top));	
	if ((((int)clip.right - (int)clip.left) == 0) ||
			(((int)clip.bottom - (int)clip.top) == 0)) {				
		XSetClipRectangles(self->display, self->gc, 0, 0,
			0, 0, Unsorted);
		XftDrawSetClipRectangles(self->xfd, 0, 0, 0, 0);
	} else {		
		XRectangle r;
				
		r.x = (int)self->clip.left;
		r.y = (int)self->clip.top;
		r.width = (int)clip.right - (int)clip.left;
		r.height = (int)clip.bottom - (int)clip.top;		
		XSetClipRectangles(self->display, self->gc, 0, 0,
			&r, 1, Unsorted);		
		XftDrawSetClipRectangles(self->xfd, 0, 0, &r, 1);		
	}	
}

psy_ui_RealRectangle psy_ui_x11_g_dev_cliprect(
	const psy_ui_x11_GraphicsImp* self)
{
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			self->clip.left + (int)(self->org.x),
			self->clip.top + (int)(self->org.y)),
		psy_ui_realsize_make(
			self->clip.right - self->clip.left,
			self->clip.bottom - self->clip.top));	
}

#endif /* PSYCLE_USE_TK == PSYCLE_TK_X11 */
