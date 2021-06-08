/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uix11keyboardevent.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11

/* X11 */
#include <X11/keysym.h>
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>

psy_ui_KeyboardEvent psy_ui_x11_keyboardevent_make(XKeyEvent* event)
{
	psy_ui_KeyboardEvent rv;
	KeySym keysym = NoSymbol;
	int repeat = 0;
	static unsigned char buf[64];
	static unsigned char bufnomod[2];
	int ret;
	XKeyEvent xkevent;
	bool shift;
	bool ctrl;

	xkevent = *event;
	shift = (xkevent.state & ShiftMask) == ShiftMask;
	ctrl = (xkevent.state & ControlMask) == ControlMask;
	ret = XLookupString(&xkevent, buf, sizeof buf, &keysym, 0);
	switch (keysym) {
	case XK_Home:
		keysym = psy_ui_KEY_HOME;
		break;
	case XK_End:
		keysym = psy_ui_KEY_END;
		break;
	case XK_Shift_L:
	case XK_Shift_R:
		keysym = psy_ui_KEY_SHIFT;
		break;
	case XK_Control_L:
	case XK_Control_R:
		keysym = psy_ui_KEY_CONTROL;
		break;
	case XK_Escape:
		keysym = psy_ui_KEY_ESCAPE;
		break;
	case XK_Return:
		keysym = psy_ui_KEY_RETURN;
		break;
	case XK_Tab:
		keysym = psy_ui_KEY_TAB;
		break;
	case XK_Prior:
		keysym = psy_ui_KEY_PRIOR;
		break;
	case XK_Next:
		keysym = psy_ui_KEY_NEXT;
		break;
	case XK_Left:
		keysym = psy_ui_KEY_LEFT;
		break;
	case XK_Up:
		keysym = psy_ui_KEY_UP;
		break;
	case XK_Right:
		keysym = psy_ui_KEY_RIGHT;
		break;
	case XK_Down:
		keysym = psy_ui_KEY_DOWN;
		break;
	case XK_Delete:
		keysym = psy_ui_KEY_DELETE;
		break;
	case XK_BackSpace:
		keysym = psy_ui_KEY_BACK;
		break;	
	case XK_F1:
		keysym = psy_ui_KEY_F1;
		break;
	case XK_F2:
		keysym = psy_ui_KEY_F2;
		break;
	case XK_F3:
		keysym = psy_ui_KEY_F3;
		break;
	case XK_F4:
		keysym = psy_ui_KEY_F4;
		break;
	case XK_F5:
		keysym = psy_ui_KEY_F5;
		break;
	case XK_F6:
		keysym = psy_ui_KEY_F6;
		break;
	case XK_F7:
		keysym = psy_ui_KEY_F7;
		break;
	case XK_F8:
		keysym = psy_ui_KEY_F8;
		break;
	case XK_F9:
		keysym = psy_ui_KEY_F9;
		break;
	case XK_F10:
		keysym = psy_ui_KEY_F10;
		break;
	case XK_F11:
		keysym = psy_ui_KEY_F11;
		break;
	case XK_F12:
		keysym = psy_ui_KEY_F12;
		break;
	case XK_space:
		keysym = psy_ui_KEY_SPACE;
		break;
	case XK_0:
		keysym = psy_ui_KEY_DIGIT0;
		break;
	case XK_1:
		keysym = psy_ui_KEY_DIGIT1;
		break;
	case XK_2:
		keysym = psy_ui_KEY_DIGIT2;
		break;
	case XK_3:
		keysym = psy_ui_KEY_DIGIT3;
		break;
	case XK_4:
		keysym = psy_ui_KEY_DIGIT4;
		break;
	case XK_5:
		keysym = psy_ui_KEY_DIGIT5;
		break;
	case XK_6:
		keysym = psy_ui_KEY_DIGIT6;
		break;
	case XK_7:
		keysym = psy_ui_KEY_DIGIT7;
		break;
	case XK_8:
		keysym = psy_ui_KEY_DIGIT8;
		break;	
	case XK_9:
		keysym = psy_ui_KEY_DIGIT9;
		break;
	case XK_ISO_Left_Tab:
		keysym = psy_ui_KEY_TAB;
		break;
	default:
		if (ret && buf[0] != '\0') {
			if (buf[0] >= 'A' && buf[0] <= 'Z') {
				keysym = psy_ui_KEY_A +
					buf[0] - 'A';
			} else if (buf[0] >= 'a' && buf[0] <= 'z') {
				keysym = psy_ui_KEY_A +
					buf[0] - 'a';
			} else if (buf[0] >= '0' && buf[0] <= '9') {
				keysym = psy_ui_KEY_DIGIT0 +
					buf[0] - '0';
			} else {
				keysym = psy_ui_KEY_A; //buf[0];
			}
		}
		break;
	}
	// if (ret && buf[0] != '\0') {
	// 	keysym = buf[0];
	// 	printf("%d,%d\n", ret, (int)buf[0]);
	// } else {
	// 	printf("no lookup %d\n", keysym);
	// }
	psy_ui_keyboardevent_init_all(&rv,
		keysym,
		0,
		shift,
		ctrl,
		0,
		repeat);
	return rv;
}

XKeyEvent psy_ui_x11_xkeyevent_make(psy_ui_KeyboardEvent* e,
	Display* dpy, Window win, Window root)
{
	XKeyEvent rv;
	KeyCode keycode;
	
	keycode = XKeysymToKeycode(dpy, e->keycode); 	
	rv.type = KeyPress;
    rv.display = dpy;
    rv.window = win;
    rv.root = root;
    rv.subwindow = None;
    rv.time = CurrentTime;
    rv.x = 0;
    rv.y = 0;
    rv.x_root = 0;
    rv.y_root = 0;
    rv.same_screen = True;
    rv.keycode = keycode;
    rv.state = 0; //modifiers;	
	return rv;
}

#endif /* PSYCLE_TK_X11 */
