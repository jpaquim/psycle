// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uix11bitmapimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT

#include "uiapp.h"
#include "uix11app.h"
#include <X11/xpm.h>
#include <stdio.h>
#include <stdlib.h>

static Bool
bigendian (void)
{
  union { int i; char c[sizeof(int)]; } u;
  u.i = 1;
  return !u.c[0];
}

// VTable Prototypes
static void dispose(psy_ui_x11_BitmapImp*);
static int load(psy_ui_x11_BitmapImp*, const char* path);
static int loadresource(psy_ui_x11_BitmapImp*, int resourceid);
static psy_ui_RealSize size(psy_ui_x11_BitmapImp*);
static int empty(psy_ui_x11_BitmapImp*);

// VTable init
static psy_ui_BitmapImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(psy_ui_x11_BitmapImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose = (psy_ui_bitmap_imp_fp_dispose) dispose;
		imp_vtable.dev_load = (psy_ui_bitmap_imp_fp_load) load;
		imp_vtable.dev_loadresource = (psy_ui_bitmap_imp_fp_loadresource) loadresource;
		imp_vtable.dev_size = (psy_ui_bitmap_imp_fp_size) size;
		imp_vtable.dev_empty = (psy_ui_bitmap_imp_fp_empty) empty;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_x11_bitmapimp_init(psy_ui_x11_BitmapImp* self,
	psy_ui_RealSize size)
{
	psy_ui_bitmap_imp_init(&self->imp);
	imp_vtable_init(self);
	self->imp.vtable = &imp_vtable;
	self->pixmap = 0;
}

void dispose(psy_ui_x11_BitmapImp* self)
{
	//if (self->bitmap) {
	//	DeleteObject(self->bitmap);
		self->pixmap = 0;
	//}
}

int load(psy_ui_x11_BitmapImp* self, const char* path)
{	
	return 0;
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

psy_ui_RealSize size(psy_ui_x11_BitmapImp* self)
{
	psy_ui_RealSize size;    

	if (self->pixmap) {
        Window root;
        unsigned int temp;
        unsigned int width = 0;
        unsigned int height = 0;
        psy_ui_X11App* xtapp;		

        xtapp = (psy_ui_X11App*)psy_ui_app()->imp;
        // XGetGeometry(xtapp->dpy, self->pixmap, &root, &temp, &temp,
            //&width, &height, &temp, &temp);
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


#endif
