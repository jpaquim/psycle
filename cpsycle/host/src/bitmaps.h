/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef BITMAPS_H
#define BITMAPS_H

#include <uiapp.h>

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_App;

// #define IDB_GUI 40000
#define IDB_SAMPULSE 40001

void register_bitmaps(struct psy_ui_App*, const char* app_bmp_path);

#ifdef __cplusplus
}
#endif

#endif /* BITMAPS_H */
