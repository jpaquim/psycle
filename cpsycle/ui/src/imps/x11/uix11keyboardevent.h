/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11
#ifndef psy_ui_X11KEYBOARDEVENT_H
#define psy_ui_X11KEYBOARDEVENT_H

#include "../../uievents.h"
/* X11 */
#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xdbe.h>
#include <X11/keysym.h>

#ifdef __cplusplus
extern "C" {
#endif

psy_ui_KeyboardEvent psy_ui_x11_keyboardevent_make(XKeyEvent*);
XKeyEvent psy_ui_x11_xkeyevent_make(psy_ui_KeyboardEvent* e,
	Display* dpy, Window win, Window root);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_X11KEYBOARDEVENT_H */
#endif /* PSYCLE_TK_X11 */
