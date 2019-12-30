// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIBITMAP_H)
#define UIBITMAP_H

#include "uidef.h"

typedef struct {
   HBITMAP hBitmap;
} psy_ui_Bitmap;

void psy_ui_bitmap_init(psy_ui_Bitmap*);
int psy_ui_bitmap_load(psy_ui_Bitmap*, const char* path);
void psy_ui_bitmap_dispose(psy_ui_Bitmap*);
ui_size psy_ui_bitmap_size(psy_ui_Bitmap*);
int psy_ui_bitmap_loadresource(psy_ui_Bitmap*, int resourceid);

#endif
