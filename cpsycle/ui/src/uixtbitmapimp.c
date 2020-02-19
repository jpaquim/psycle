// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uixtbitmapimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT

#include "uiapp.h"
#include "uixtapp.h"

extern psy_ui_App app;

// VTable Prototypes
static void dispose(psy_ui_xt_BitmapImp*);
static int load(psy_ui_xt_BitmapImp*, const char* path);
static int loadresource(psy_ui_xt_BitmapImp*, int resourceid);
static psy_ui_Size size(psy_ui_xt_BitmapImp*);
static int empty(psy_ui_xt_BitmapImp*);

// VTable init
static psy_ui_BitmapImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(psy_ui_xt_BitmapImp* self)
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

void psy_ui_xt_bitmapimp_init(psy_ui_xt_BitmapImp* self)
{
	psy_ui_bitmap_imp_init(&self->imp);
	imp_vtable_init(self);
	self->imp.vtable = &imp_vtable;
	self->pixmap = 0;
}

void dispose(psy_ui_xt_BitmapImp* self)
{
	//if (self->bitmap) {
	//	DeleteObject(self->bitmap);
		self->pixmap = 0;
	//}
}

int load(psy_ui_xt_BitmapImp* self, const char* path)
{
	/*HBITMAP bitmap;

	bitmap = (HBITMAP)LoadImage(NULL,
		(LPCTSTR)path,
		IMAGE_BITMAP,
		0, 0,
		LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (bitmap != 0) {
		dispose(self);
		self->bitmap = bitmap;
	}
	return bitmap == 0;*/
	return 0;
}

int loadresource(psy_ui_xt_BitmapImp* self, int resourceid)
{
	/*HBITMAP bitmap;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	bitmap = LoadBitmap(winapp->instance, MAKEINTRESOURCE(resourceid));
	if (bitmap != 0) {
		dispose(self);
		self->bitmap = bitmap;
	}
	return bitmap == 0;*/
	return 0;
}

psy_ui_Size size(psy_ui_xt_BitmapImp* self)
{
	psy_ui_Size size;    

	if (self->pixmap) {
        Window root;
        unsigned int temp;
        unsigned int width;
        unsigned int height;
        psy_ui_XtApp* xtapp;		

        xtapp = (psy_ui_XtApp*) app.platform;        
        XGetGeometry(xtapp->dpy, self->pixmap, &root, &temp, &temp,
            &width, &height, &temp, &temp);
		size.width = width;
		size.height = height;
	} else {
		size.width = 0;
		size.height = 0;
	}
	return size;
}

int empty(psy_ui_xt_BitmapImp* self)
{
	return self->pixmap == 0;
}

#endif
