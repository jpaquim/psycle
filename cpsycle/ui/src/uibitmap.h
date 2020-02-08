// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net


#ifndef psy_ui_BITMAP_H
#define psy_ui_BITMAP_H

#include "../../detail/psydef.h"
#include "uidef.h"

// Graphics Bitmap Interface
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_Bitmap
// Implementor			psy_ui_BitmapImp
// Concrete Implementor	psy_ui_win_BitmapImp
//
// psy_ui_Bitmap <>-------- psy_ui_BitmapImp
//                                ^
//      ...                       |
//                          psy_ui_win_Bitmap

#ifdef __cplusplus
extern "C" {
#endif	

struct psy_ui_Bitmap;

typedef void (*psy_ui_bitmap_fp_dispose)(struct psy_ui_Bitmap*);
typedef int (*psy_ui_bitmap_fp_load)(struct psy_ui_Bitmap*, const char* path);
typedef int (*psy_ui_bitmap_fp_loadresource)(struct psy_ui_Bitmap*, int resourceid);
typedef psy_ui_Size (*psy_ui_bitmap_fp_size)(struct psy_ui_Bitmap*);
typedef int(*psy_ui_bitmap_fp_empty)(struct psy_ui_Bitmap*);
	
typedef struct psy_ui_BitmapVTable {
	psy_ui_bitmap_fp_dispose dispose;
	psy_ui_bitmap_fp_load load;
	psy_ui_bitmap_fp_loadresource loadresource;
	psy_ui_bitmap_fp_size size;
	psy_ui_bitmap_fp_empty empty;
} psy_ui_BitmapVTable;

struct psy_ui_BitmapImp;

typedef struct psy_ui_Bitmap {
	psy_ui_BitmapVTable* vtable;
	struct psy_ui_BitmapImp* imp;
} psy_ui_Bitmap;

void psy_ui_bitmap_init(psy_ui_Bitmap*);

// vtable calls	
INLINE void psy_ui_bitmap_dispose(psy_ui_Bitmap* self)
{
	self->vtable->dispose(self);
}

INLINE int psy_ui_bitmap_load(psy_ui_Bitmap* self, const char* path)
{
	return self->vtable->load(self, path);
}

INLINE int psy_ui_bitmap_loadresource(psy_ui_Bitmap* self, int resourceid)
{
	return self->vtable->loadresource(self, resourceid);
}

INLINE psy_ui_Size psy_ui_bitmap_size(psy_ui_Bitmap* self)
{
	return self->vtable->size(self);
}

INLINE int psy_ui_bitmap_empty(psy_ui_Bitmap* self)
{
	return self->vtable->empty(self);
}

// psy_ui_BitmapImp	
typedef void (*psy_ui_bitmap_imp_fp_dispose)(struct psy_ui_BitmapImp*);
typedef int (*psy_ui_bitmap_imp_fp_load)(struct psy_ui_BitmapImp*, const char* path);
typedef int (*psy_ui_bitmap_imp_fp_loadresource)(struct psy_ui_BitmapImp*, int resourceid);
typedef psy_ui_Size(*psy_ui_bitmap_imp_fp_size)(struct psy_ui_BitmapImp*);
typedef int (*psy_ui_bitmap_imp_fp_empty)(struct psy_ui_BitmapImp*);

typedef struct psy_ui_BitmapImpVTable {
	psy_ui_bitmap_imp_fp_dispose dev_dispose;
	psy_ui_bitmap_imp_fp_load dev_load;
	psy_ui_bitmap_imp_fp_loadresource dev_loadresource;
	psy_ui_bitmap_imp_fp_size dev_size;
	psy_ui_bitmap_imp_fp_empty dev_empty;
} psy_ui_BitmapImpVTable;

typedef struct psy_ui_BitmapImp {
	psy_ui_BitmapImpVTable* vtable;
} psy_ui_BitmapImp;

void psy_ui_bitmap_imp_init(psy_ui_BitmapImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_BITMAP_H */
