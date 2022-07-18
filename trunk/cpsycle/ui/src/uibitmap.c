/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uibitmap.h"
/* local */
#include "uiapp.h"
#include "uiimpfactory.h"
/* std */
#include <stdlib.h>
/* platform */
#include "../../detail/os.h"

/* psy_ui_BitmapAnimate */
void psy_ui_bitmapanimate_init(psy_ui_BitmapAnimate* self)
{
	self->enabled = FALSE;
	self->interval = 0;
	psy_ui_realsize_init(&self->framesize);
	self->horizontal = FALSE;
}

/* prototypes */
static void dispose(psy_ui_Bitmap*);
static int load(psy_ui_Bitmap*, const char* path);
static int loadresource(psy_ui_Bitmap*, uintptr_t resourceid);
static psy_ui_RealSize size(const psy_ui_Bitmap*);
static bool empty(const psy_ui_Bitmap*);
static void settransparency(psy_ui_Bitmap*, psy_ui_Colour);
static void createimp(const psy_ui_Bitmap*);
/* vtable */
static psy_ui_BitmapVTable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.dispose = dispose;		
		vtable.load = load;
		vtable.loadresource = loadresource;
		vtable.size = size;
		vtable.empty = empty;
		vtable.settransparency = settransparency;
		vtable_initialized = TRUE;
	}
}

/* implementation */
void psy_ui_bitmap_init(psy_ui_Bitmap* self)
{
	vtable_init();
	self->vtable = &vtable;
	self->imp = NULL;
}

void psy_ui_bitmap_init_size(psy_ui_Bitmap* self, psy_ui_RealSize size)
{
	vtable_init();
	self->vtable = &vtable;
	self->imp = psy_ui_impfactory_allocinit_bitmapimp(
		psy_ui_app_impfactory(psy_ui_app()), size);
}

void dispose(psy_ui_Bitmap* self)
{
	if (self->imp) {
		self->imp->vtable->dev_dispose(self->imp);
		free(self->imp);
		self->imp = NULL;
	}
}

int load(psy_ui_Bitmap* self, const char* path)
{
	createimp(self);
	return self->imp->vtable->dev_load(self->imp, self, path);
}

int loadresource(psy_ui_Bitmap* self, uintptr_t id)
{	
	createimp(self);
	return psy_ui_bitmaps_load(&psy_ui_app()->bitmaps, id, self);	
}

psy_ui_RealSize size(const psy_ui_Bitmap* self)
{
	if (self->imp) {
		return self->imp->vtable->dev_size(self->imp);
	}
	return psy_ui_realsize_zero();
}

bool empty(const psy_ui_Bitmap* self)
{
	if (!self->imp) {
		return TRUE;
	}
	createimp(self);
	return self->imp->vtable->dev_empty(self->imp);
}

void settransparency(psy_ui_Bitmap* self, psy_ui_Colour colour)
{
	createimp(self);
	self->imp->vtable->dev_settransparency(self->imp, colour);
}

void psy_ui_bitmap_copy(psy_ui_Bitmap* self, const psy_ui_Bitmap* other)
{	
	if (!other->imp) {
		if (self->imp) {
			psy_ui_bitmap_dispose(self);
			self->imp = NULL;
		}
	} else {
		createimp(self);
		self->imp->vtable->dev_copy(self->imp, other->imp);
	}
}

uintptr_t psy_ui_bitmap_native(psy_ui_Bitmap* self)
{	
	if (self->imp) {
		return self->imp->vtable->dev_native(self->imp);
	}
	return 0;
}

uintptr_t psy_ui_bitmap_native_mask(psy_ui_Bitmap* self)
{
	if (self->imp) {
		return self->imp->vtable->dev_native_mask(self->imp);
	}
	return 0;
}

void createimp(const psy_ui_Bitmap* self)
{
	if (!self->imp) {
		((psy_ui_Bitmap*)self)->imp =
			psy_ui_impfactory_allocinit_bitmapimp(
			psy_ui_app_impfactory(psy_ui_app()),
			psy_ui_realsize_zero());
	}
}

/* psy_ui_BitmapImp */

static void psy_ui_bitmap_imp_dispose(psy_ui_BitmapImp* self) { }
static int psy_ui_bitmap_imp_load(psy_ui_BitmapImp* self,
	struct psy_ui_Bitmap* bitmap, const char* path)
{
	return 0;
}

static int psy_ui_bitmap_imp_loadresource(psy_ui_BitmapImp* self,
	uintptr_t resourceid)
{
	return 0;
}

static psy_ui_RealSize psy_ui_bitmap_imp_size(const psy_ui_BitmapImp* self)
{
	return psy_ui_realsize_zero();	
}

static bool psy_ui_bitmap_imp_empty(const psy_ui_BitmapImp* self)
{
	return TRUE;
}

static void psy_ui_bitmap_imp_settransparency(psy_ui_BitmapImp* self,
	psy_ui_Colour colour)
{
}

static void dev_copy(psy_ui_BitmapImp* self, const psy_ui_BitmapImp* other)
{
}

uintptr_t dev_native(psy_ui_BitmapImp* self) { return 0; }



/* psy_ui_BitmapImp */
/* vtable */
static psy_ui_BitmapImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = psy_ui_bitmap_imp_dispose;
		imp_vtable.dev_load = psy_ui_bitmap_imp_load;
		imp_vtable.dev_loadresource = psy_ui_bitmap_imp_loadresource;
		imp_vtable.dev_size = psy_ui_bitmap_imp_size;
		imp_vtable.dev_empty = psy_ui_bitmap_imp_empty;
		imp_vtable.dev_settransparency = psy_ui_bitmap_imp_settransparency;
		imp_vtable.dev_copy = dev_copy;
		imp_vtable.dev_native = dev_native;
		imp_vtable.dev_native_mask = dev_native;
		imp_vtable_initialized = TRUE;
	}
}
/* implementation */
void psy_ui_bitmap_imp_init(psy_ui_BitmapImp* self)
{
	imp_vtable_init();
	self->vtable = &imp_vtable;
}
