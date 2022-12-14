/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11
#ifndef psy_ui_x11_COLORDIALOGIMP_H
#define psy_ui_x11_COLORDIALOGIMP_H

#include "uicolordialog.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_x11_ColourDialogImp {
	psy_ui_ColourDialogImp imp;
	struct psy_ui_Component* parent;		
	psy_ui_Colour colour;
} psy_ui_x11_ColourDialogImp;

void psy_ui_x11_colourdialogimp_init(psy_ui_x11_ColourDialogImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_x11_COLORDIALOGIMP_H */
#endif /* PSYCLE_TK_X11 */
