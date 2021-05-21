/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11
#ifndef psy_ui_X11COLOURMAP_H
#define psy_ui_X11COLOURMAP_H

/* local */
#include "../../uicolour.h"
/* container */
#include <hashtbl.h>
/* X11 */
#include <X11/Xlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_X11Colours {	
	/* internal	*/
	psy_Table colourmap;
	/* references */
	Display * dpy;	
} psy_ui_X11Colours;

void psy_ui_x11colours_init(psy_ui_X11Colours*, Display*);
void psy_ui_x11colours_dispose(psy_ui_X11Colours*);

int psy_ui_x11colours_index(psy_ui_X11Colours*, psy_ui_Colour);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_X11COLOURMAP_H */
#endif /* PSYCLE_TK_X11 */
