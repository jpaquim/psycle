// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
#ifndef psy_ui_win_FONTDIALOGIMP_H
#define psy_ui_win_FONTDIALOGIMP_H

#include "../../uifontdialog.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_win_FontDialogImp {
	psy_ui_FontDialogImp imp;
	struct psy_ui_Component* parent;		
	psy_ui_FontInfo fontinfo;
} psy_ui_win_FontDialogImp;

void psy_ui_win_fontdialogimp_init(psy_ui_win_FontDialogImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_win_FONTDIALOGIMP_H */
#endif /* PSYCLE_TK_WIN32 */
