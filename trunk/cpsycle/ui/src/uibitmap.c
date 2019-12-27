// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uibitmap.h"
#include "uiapp.h"
#include "uiwinapp.h"

extern psy_ui_App app;

void ui_bitmap_init(psy_ui_Bitmap* self)
{
	self->hBitmap = 0;
}

void ui_bitmap_dispose(psy_ui_Bitmap* self)
{
	if (self->hBitmap) {
		DeleteObject(self->hBitmap);
		self->hBitmap = 0;
	}
}

int ui_bitmap_load(psy_ui_Bitmap* self, const char* path)
{	
	HBITMAP bmp;
	
	bmp = (HBITMAP) LoadImage(NULL,
		(LPCTSTR) path,
		IMAGE_BITMAP,
		0, 0,
		LR_DEFAULTSIZE | LR_LOADFROMFILE);	
	if (bmp != 0) {
		ui_bitmap_dispose(self);
		self->hBitmap = bmp;
	}
	return bmp == 0;
}

ui_size ui_bitmap_size(psy_ui_Bitmap* self)
{
	ui_size size;
	BITMAP bitmap ;
	
	if (self->hBitmap) {
		GetObject(self->hBitmap, sizeof (BITMAP), &bitmap);
		size.width = bitmap.bmWidth;
		size.height = bitmap.bmHeight;
	} else {
		size.width = 0;
		size.height = 0;
	}
	return size;
}

int ui_bitmap_loadresource(psy_ui_Bitmap* self, int resourceid)
{
	HBITMAP bmp;	
	psy_ui_WinApp* winapp;
	
	winapp = (psy_ui_WinApp*) app.platform;
	bmp = LoadBitmap (winapp->instance, MAKEINTRESOURCE(resourceid));	
	if (bmp != 0) {
		ui_bitmap_dispose(self);
		self->hBitmap = bmp;
	}	
	return bmp == 0;
}
