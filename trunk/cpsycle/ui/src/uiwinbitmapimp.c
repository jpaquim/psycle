// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwinbitmapimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

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
	self->bitmap = 0;
}

void dispose(psy_ui_win_BitmapImp* self)
{
	if (self->bitmap) {
		DeleteObject(self->bitmap);
		self->bitmap = 0;
	}
}

int load(psy_ui_win_BitmapImp* self, const char* path)
{
	HBITMAP bitmap;

	bitmap = (HBITMAP)LoadImage(NULL,
		(LPCTSTR)path,
		IMAGE_BITMAP,
		0, 0,
		LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (bitmap != NULL) {
		dispose(self);
		self->bitmap = bitmap;
	}
	return bitmap == 0;
}

int loadresource(psy_ui_win_BitmapImp* self, int resourceid)
{
	HBITMAP bitmap;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	bitmap = LoadBitmap(winapp->instance, MAKEINTRESOURCE(resourceid));
	if (bitmap != NULL) {
		dispose(self);
		self->bitmap = bitmap;
	}
	return bitmap == 0;
}

psy_ui_Size size(psy_ui_win_BitmapImp* self)
{
	psy_ui_Size size;
	BITMAP bitmap;

	if (self->bitmap) {
		GetObject(self->bitmap, sizeof(BITMAP), &bitmap);
		size.width = psy_ui_value_makepx(bitmap.bmWidth);
		size.height = psy_ui_value_makepx(bitmap.bmHeight);
	} else {
		size.width = psy_ui_value_makepx(0);
		size.height = psy_ui_value_makepx(0);
	}
	return size;
}

int empty(psy_ui_win_BitmapImp* self)
{
	return self->bitmap == 0;
}

#endif
