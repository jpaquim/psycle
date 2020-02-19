// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT
#ifndef psy_ui_xt_COLORDIALOGIMP_H
#define psy_ui_xt_COLORDIALOGIMP_H

#include "uicolordialog.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_xt_ColorDialogImp {
	psy_ui_ColorDialogImp imp;
	struct psy_ui_Component* parent;		
	psy_ui_Color color;
} psy_ui_xt_ColorDialogImp;

void psy_ui_xt_colordialogimp_init(psy_ui_xt_ColorDialogImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_xt_COLORDIALOGIMP_H */
#endif /* PSYCLE_TK_XT */
