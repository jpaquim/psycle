// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwinbitmapimp.h"
#include "uiapp.h"
#include "uiwinapp.h"

extern psy_ui_App app;

// VTable Prototypes
static void dispose(psy_ui_win_BitmapImp*);
static int load(psy_ui_win_BitmapImp*, const char* path);
static int loadresource(psy_ui_win_BitmapImp*, int resourceid);
static psy_ui_Size size(psy_ui_win_BitmapImp*);
static int empty(psy_ui_win_BitmapImp*);


// VTable init
static psy_ui_BitmapImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(psy_ui_win_BitmapImp* self)
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

void psy_ui_win_bitmapimp_init(psy_ui_win_BitmapImp* self)
{
	psy_ui_bitmap_imp_init(&self->imp);
	imp_vtable_init(self);
	self->imp.vtable = &imp_vtable;
	self->hBitmap = 0;
}

void dispose(psy_ui_win_BitmapImp* self)
{
	if (self->hBitmap) {
		DeleteObject(self->hBitmap);
		self->hBitmap = 0;
	}
}

int load(psy_ui_win_BitmapImp* self, const char* path)
{
	HBITMAP bmp;

	bmp = (HBITMAP)LoadImage(NULL,
		(LPCTSTR)path,
		IMAGE_BITMAP,
		0, 0,
		LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (bmp != 0) {
		dispose(self);
		self->hBitmap = bmp;
	}
	return bmp == 0;
}

int loadresource(psy_ui_win_BitmapImp* self, int resourceid)
{
	HBITMAP bmp;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	bmp = LoadBitmap(winapp->instance, MAKEINTRESOURCE(resourceid));
	if (bmp != 0) {
		dispose(self);
		self->hBitmap = bmp;
	}
	return bmp == 0;
}

psy_ui_Size size(psy_ui_win_BitmapImp* self)
{
	psy_ui_Size size;
	BITMAP bitmap;

	if (self->hBitmap) {
		GetObject(self->hBitmap, sizeof(BITMAP), &bitmap);
		size.width = bitmap.bmWidth;
		size.height = bitmap.bmHeight;
	}
	else {
		size.width = 0;
		size.height = 0;
	}
	return size;
}

int empty(psy_ui_win_BitmapImp* self)
{
	return self->hBitmap != 0;
}