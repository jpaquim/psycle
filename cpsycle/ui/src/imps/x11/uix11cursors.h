/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11
#ifndef psy_ui_x11_CURSORS_H
#define psy_ui_x11_CURSORS_H

/* local */
#include "uidef.h"
/* container */
#include <hashtbl.h>
/* X11 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_x11_Cursors {
	Display* dpy;
	psy_Table cursors;
} psy_ui_x11_Cursors;

void psy_ui_x11_cursors_init(psy_ui_x11_Cursors*, Display*);
void psy_ui_x11_cursors_dispose(psy_ui_x11_Cursors*);

Cursor psy_ui_x11_cursors_cursor(psy_ui_x11_Cursors*,
	unsigned int shape);
unsigned int psy_ui_x11_cursors_shape(psy_ui_x11_Cursors*,
	psy_ui_CursorStyle style);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_x11_CURSORS_H */
#endif /* PSYCLE_TK_X11 */
