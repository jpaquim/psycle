// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT
#ifndef psy_ui_xt_BITMAPIMP_H
#define psy_ui_xt_BITMAPIMP_H

#include "uibitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <X11/Xlib.h>

typedef struct psy_ui_xt_BitmapImp {
	psy_ui_BitmapImp imp;
	Pixmap pixmap;
} psy_ui_xt_BitmapImp;

void psy_ui_xt_bitmapimp_init(psy_ui_xt_BitmapImp* self);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_xt_BITMAPIMP_H */
#endif /* PSYCLE_TK_XT */
