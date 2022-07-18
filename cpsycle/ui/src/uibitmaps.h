/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_BITMAPS_H
#define psy_ui_BITMAPS_H

/* container */
#include <hashtbl.h>


#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_Bitmap;

typedef struct psy_ui_Bitmaps {	
	psy_Table bmps;
	psy_Table files;
	char* app_bmp_path;
} psy_ui_Bitmaps;

void psy_ui_bitmaps_init(psy_ui_Bitmaps*);
void psy_ui_bitmaps_dispose(psy_ui_Bitmaps*);

void psy_ui_bitmaps_set_app_bmp_path(psy_ui_Bitmaps*,
	const char* app_bmp_path);
void psy_ui_bitmaps_add(psy_ui_Bitmaps*, uintptr_t id,
	const char* filename);
int psy_ui_bitmaps_load(psy_ui_Bitmaps*, uintptr_t id,
	struct psy_ui_Bitmap* rv);


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_BITMAPS_H */
