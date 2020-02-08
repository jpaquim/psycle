// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
#ifndef psy_ui_win_FONTIMP_H
#define psy_ui_win_FONTIMP_H

#include "uifont.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_win_FontImp {
	psy_ui_FontImp imp;
	HFONT hfont;	
} psy_ui_win_FontImp;

void psy_ui_win_fontimp_init(psy_ui_win_FontImp* self, const psy_ui_FontInfo*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_win_FONTIMP_H */
#endif /* PSYCLE_TK_WIN32 */
