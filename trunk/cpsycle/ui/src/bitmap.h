// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(UIBITMAP_H)
#define UIBITMAP_H

#include <windows.h>
#include "uidef.h"

typedef struct {
   HBITMAP hBitmap;
} ui_bitmap;

void ui_bitmap_init(ui_bitmap*);
void ui_bitmap_load(ui_bitmap*, const char* path);
void ui_bitmap_dispose(ui_bitmap*);
ui_size ui_bitmap_size(ui_bitmap*);
void ui_bitmap_loadresource(ui_bitmap*, int resourceid);

#endif