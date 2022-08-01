/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uix11bitmapimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11

/* local */
#include "uiapp.h"
#include "uigraphics.h"
#include "uibmpreader.h"
#include "uix11app.h"
/* x11 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xdbe.h>
/* std */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../../detail/portable.h"


/* prototypes */
static void dispose(psy_ui_x11_BitmapImp*);
static int load(psy_ui_x11_BitmapImp*, struct psy_ui_Bitmap* bitmap,
	const char* path);
static int loadresource(psy_ui_x11_BitmapImp*, int resourceid);
static psy_ui_RealSize dev_size(psy_ui_x11_BitmapImp*);
static int empty(psy_ui_x11_BitmapImp*);
static void dev_settransparency(psy_ui_x11_BitmapImp*, psy_ui_Colour colour);
static void dev_preparemask(psy_ui_x11_BitmapImp*, psy_ui_Colour clrtrans);
static uintptr_t dev_native(psy_ui_x11_BitmapImp*);
static uintptr_t dev_native_mask(psy_ui_x11_BitmapImp*);

/* vtable */
static psy_ui_BitmapImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_x11_BitmapImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose =
			(psy_ui_bitmap_imp_fp_dispose)
			dispose;
		imp_vtable.dev_load =
			(psy_ui_bitmap_imp_fp_load)
			load;
		imp_vtable.dev_loadresource =
			(psy_ui_bitmap_imp_fp_loadresource)
			loadresource;
		imp_vtable.dev_size =
			(psy_ui_bitmap_imp_fp_size)
			dev_size;
		imp_vtable.dev_empty =
			(psy_ui_bitmap_imp_fp_empty)
			empty;
		imp_vtable.dev_settransparency =
			(psy_ui_bitmap_imp_fp_settransparency)
			dev_settransparency;
		imp_vtable.dev_native =
			(psy_ui_bitmap_imp_fp_native)
			dev_native;
		imp_vtable.dev_native_mask =
			(psy_ui_bitmap_imp_fp_native)
			dev_native_mask;
		imp_vtable_initialized = TRUE;
	}
}

/* implementation */
void psy_ui_x11_bitmapimp_init(psy_ui_x11_BitmapImp* self,
	psy_ui_RealSize size)
{
	psy_ui_bitmap_imp_init(&self->imp);
	imp_vtable_init(self);
	self->imp.vtable = &imp_vtable;
	self->mask = 0;	
	if (size.width == 0 && size.height == 0) {
		self->pixmap = 0;
	} else {
		psy_ui_X11App* x11app;
		int screen;
				
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		screen = DefaultScreen(x11app->dpy);		
		self->pixmap = XCreatePixmap(x11app->dpy,
			DefaultRootWindow(x11app->dpy), (int)size.width, (int)size.height,
			DefaultDepth(x11app->dpy, screen));		
	}
}

void dispose(psy_ui_x11_BitmapImp* self)
{
	if (self->pixmap) {
		psy_ui_X11App* x11app;		
				
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		XFreePixmap(x11app->dpy, self->pixmap);
		self->pixmap = 0;
	}
	if (self->mask) {
		psy_ui_X11App* x11app;		
				
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		XFreePixmap(x11app->dpy, self->mask);
		self->mask = 0;
	}
}

int load(psy_ui_x11_BitmapImp* self, struct psy_ui_Bitmap* bitmap,
	const char* path)
{	
	int rv;
	
	assert(path);

	psy_ui_BmpReader bmpreader;

	psy_ui_bmpreader_init(&bmpreader, bitmap);
	rv = psy_ui_bmpreader_load(&bmpreader, path);
	psy_ui_bmpreader_dispose(&bmpreader);
	return rv;	
}

int loadresource(psy_ui_x11_BitmapImp* self, int resourceid)
{
	/*HBITMAP bitmap;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	bitmap = LoadBitmap(winapp->instance, MAKEINTRESOURCE(resourceid));
	if (bitmap != NULL) {
		dispose(self);
		self->bitmap = bitmap;
	}
	return bitmap == 0;*/
	return 0;
}

psy_ui_RealSize dev_size(psy_ui_x11_BitmapImp* self)
{
	psy_ui_RealSize size;    

	if (self->pixmap) {
        Window root;
        unsigned int temp;
        unsigned int width = 0;
        unsigned int height = 0;
        psy_ui_X11App* xtapp;		

        xtapp = (psy_ui_X11App*)psy_ui_app()->imp;
        XGetGeometry(xtapp->dpy, self->pixmap, &root, &temp, &temp,
            &width, &height, &temp, &temp);
		size.width = width;
		size.height = height;
	} else {
		size.width = 0.0;
		size.height = 0.0;
	}
	return size;
}

int empty(psy_ui_x11_BitmapImp* self)
{
	return self->pixmap == 0;
}

void dev_settransparency(psy_ui_x11_BitmapImp* self, psy_ui_Colour colour)
{
	dev_preparemask(self, colour);
}

void dev_preparemask(psy_ui_x11_BitmapImp* self, psy_ui_Colour clrtrans)
{	
	psy_ui_RealSize size;
	psy_ui_X11App* x11app;
	int screen;
	GC gc_src;
	GC gc_dst;
	XImage* xi;
	int x, y;
	XGCValues gcv;
	
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	screen = DefaultScreen(x11app->dpy);
	/* Get the dimensions of the source bitmap */	
	size = dev_size(self);
	if (size.width == 0 || size.height == 0) {
		return;
	}
	/* Create the mask bitmap */
	if (self->mask) {		
		XFreePixmap(x11app->dpy, self->mask);
		self->mask = 0;
	}
	self->mask = XCreatePixmap(x11app->dpy,
		RootWindow(x11app->dpy, screen),
		(int)size.width, (int)size.height,
		1);
	gc_src = XCreateGC(x11app->dpy, self->pixmap, 0, NULL);
	gc_dst = XCreateGC(x11app->dpy, self->mask, 0, NULL);
		
	/* Change the background to trans color */
	XSetBackground(x11app->dpy, gc_dst, psy_ui_x11app_colourindex(
		x11app, clrtrans));
	/* This call sets up the mask bitmap. */	
	XCopyPlane(x11app->dpy, self->pixmap, self->mask, gc_dst,
		0, 0, (int)size.width, (int)size.height, 0, 0, 1);
	
	/*
	** Now, we need to paint onto the original image, making
	** sure that the "transparent" area is set to black.
	*/
	
	/* todo: this doesnt work
	XSetForeground(x11app->dpy, gc_src,
		WhitePixel(x11app->dpy, DefaultScreen(x11app->dpy)));
	XSetBackground(x11app->dpy, gc_src,
		BlackPixel(x11app->dpy, DefaultScreen(x11app->dpy)));
	gcv.function = GXand;
	XChangeGC(x11app->dpy, gc_src, GCFunction, &gcv);
	XCopyPlane(x11app->dpy, self->mask, self->pixmap, gc_src,
		0, 0, (int)size.width, (int)size.height, 0, 0, 1); */
		
	/* instead changing manually */
	xi = XGetImage(x11app->dpy, self->pixmap, 0, 0, (int)size.width,
		(int)size.height, AllPlanes, ZPixmap);
	for (int x = 0; x < (int)size.width; x++) {
		for (int y = 0; y < (int)size.height; y++) {
			uint8_t r; uint8_t g; uint8_t b;
			unsigned long pixel;
			
			pixel = XGetPixel(xi, x, y);
			r = (uint8_t)(pixel>>16);
			g = (uint8_t)((pixel&0x00ff00)>>8);
			b = (uint8_t)(pixel&0x0000ff);
			if (r == clrtrans.r && g == clrtrans.g && b == clrtrans.b) {				
				XPutPixel(xi, x, y, BlackPixel(x11app->dpy, screen));				
			}
		}
	}
	XPutImage(x11app->dpy, self->pixmap, gc_src, xi, 0, 0, 0, 0,
		(int)size.width, (int)size.height);
	XDestroyImage(xi);	
	XFreeGC(x11app->dpy, gc_src);
	XFreeGC(x11app->dpy, gc_dst);		
}

uintptr_t dev_native(psy_ui_x11_BitmapImp* self)
{
	return (uintptr_t)self->pixmap;
}

uintptr_t dev_native_mask(psy_ui_x11_BitmapImp* self)
{
	return (uintptr_t)self->mask;
}

#endif
