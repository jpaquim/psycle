// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "uibitmap.h"
#include "uiwinbitmapimp.h"
#include <stdlib.h>

// VTable Prototypes
static void dispose(psy_ui_Bitmap*);
static int load(psy_ui_Bitmap*, const char* path);
static int loadresource(psy_ui_Bitmap*, int resourceid);
static psy_ui_Size size(psy_ui_Bitmap*);
static int empty(psy_ui_Bitmap*);


// VTable init
static psy_ui_BitmapVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.dispose = dispose;		
		vtable.load = load;
		vtable.loadresource = loadresource;
		vtable.size = size;
		vtable.empty = empty;
		vtable_initialized = 1;
	}
}

void psy_ui_bitmap_init(psy_ui_Bitmap* self)
{
	vtable_init();
	self->vtable = &vtable;
	self->imp = (psy_ui_BitmapImp*)malloc(sizeof(psy_ui_win_BitmapImp));
	psy_ui_win_bitmapimp_init((psy_ui_win_BitmapImp*)self->imp);
}

// Delegation Methods to FontImp
void dispose(psy_ui_Bitmap* self)
{
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = 0;
}

int load(psy_ui_Bitmap* self, const char* path)
{
	return self->imp->vtable->dev_load(self->imp, path);
}

int loadresource(psy_ui_Bitmap* self, int resourceid)
{
	return self->imp->vtable->dev_loadresource(self->imp, resourceid);
}

psy_ui_Size size(psy_ui_Bitmap* self)
{
	return self->imp->vtable->dev_size(self->imp);
}

int empty(psy_ui_Bitmap* self)
{
	return self->imp->vtable->dev_empty(self->imp);
}

// psy_ui_BitmapImp
static void psy_ui_bitmap_imp_dispose(psy_ui_BitmapImp* self) { }
static int  psy_ui_bitmap_imp_load(psy_ui_BitmapImp* self, const char* path)
{
	return 0;
}

static int psy_ui_bitmap_imp_loadresource(psy_ui_BitmapImp* self, int resourceid)
{
	return 0;
}
static psy_ui_Size psy_ui_bitmap_imp_size(psy_ui_BitmapImp* self)
{
	psy_ui_Size rv = { 0, 0 };

	return rv;
}

static int psy_ui_bitmap_imp_empty(psy_ui_BitmapImp* self)
{
	return 0;
}

static psy_ui_BitmapImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = psy_ui_bitmap_imp_dispose;
		imp_vtable.dev_load = psy_ui_bitmap_imp_load;
		imp_vtable.dev_loadresource = psy_ui_bitmap_imp_loadresource;
		imp_vtable.dev_size = psy_ui_bitmap_imp_size;
		imp_vtable.dev_empty = psy_ui_bitmap_imp_empty;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_bitmap_imp_init(psy_ui_BitmapImp* self)
{
	imp_vtable_init();
	self->vtable = &imp_vtable;
}
