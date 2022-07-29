/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uix11keyboardevent.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11

/* X11 */
#include <X11/keysym.h>

static int psy_ui_x11_keyboardevent_keycode_from_sym(KeySym);

psy_ui_KeyboardEvent psy_ui_x11_keyboardevent_make(const XKeyEvent* event)
{
	psy_ui_KeyboardEvent rv;	
	XKeyEvent xkey;
	assert(event);
		
	if (event) {
		KeySym sym;	
		
		xkey = *event;
		XLookupString(&xkey, NULL, 0, &sym, NULL);	
		psy_ui_keyboardevent_init_all(&rv,
			psy_ui_x11_keyboardevent_keycode_from_sym(sym),
			0,
			(event->state & ShiftMask) == ShiftMask,
			(event->state & ControlMask) == ControlMask,
			0,
			0);	
	} else {
		psy_ui_keyboardevent_init(&rv);
	}
	return rv;
}
	
int psy_ui_x11_keyboardevent_keycode_from_sym(KeySym sym)
{
	switch (sym) {
	case XK_Home:
		return psy_ui_KEY_HOME;
	case XK_End:
		return psy_ui_KEY_END;
	case XK_Shift_L:
	case XK_Shift_R:
		return psy_ui_KEY_SHIFT;
	case XK_Control_L:
	case XK_Control_R:
		return psy_ui_KEY_CONTROL;
	case XK_Escape:
		return psy_ui_KEY_ESCAPE;
	case XK_Return:
		return psy_ui_KEY_RETURN;
	case XK_Tab:
		return psy_ui_KEY_TAB;
	case XK_Prior:
		return psy_ui_KEY_PRIOR;
	case XK_Next:
		return psy_ui_KEY_NEXT;		
	case XK_Left:
		return psy_ui_KEY_LEFT;		
	case XK_Up:
		return psy_ui_KEY_UP;		
	case XK_Right:
		return psy_ui_KEY_RIGHT;		
	case XK_Down:
		return psy_ui_KEY_DOWN;		
	case XK_Delete:
		return psy_ui_KEY_DELETE;		
	case XK_BackSpace:
		return psy_ui_KEY_BACK;			
	case XK_F1:
		return psy_ui_KEY_F1;		
	case XK_F2:
		return psy_ui_KEY_F2;		
	case XK_F3:
		return psy_ui_KEY_F3;		
	case XK_F4:
		return psy_ui_KEY_F4;		
	case XK_F5:
		return psy_ui_KEY_F5;
	case XK_F6:
		return psy_ui_KEY_F6;
	case XK_F7:
		return psy_ui_KEY_F7;
	case XK_F8:
		return psy_ui_KEY_F8;
	case XK_F9:
		return psy_ui_KEY_F9;
	case XK_F10:
		return psy_ui_KEY_F10;		
	case XK_F11:
		return psy_ui_KEY_F11;		
	case XK_F12:
		return psy_ui_KEY_F12;
	case XK_space:
		return psy_ui_KEY_SPACE;
	case XK_0:
		return psy_ui_KEY_DIGIT0;
	case XK_1:
		return psy_ui_KEY_DIGIT1;
	case XK_2:
		return psy_ui_KEY_DIGIT2;
	case XK_3:
		return psy_ui_KEY_DIGIT3;
	case XK_4:
		return psy_ui_KEY_DIGIT4;
	case XK_5:
		return psy_ui_KEY_DIGIT5;
	case XK_6:
		return psy_ui_KEY_DIGIT6;
	case XK_7:
		return psy_ui_KEY_DIGIT7;
	case XK_8:
		return psy_ui_KEY_DIGIT8;
	case XK_9:
		return psy_ui_KEY_DIGIT9;
	case XK_A:
	case XK_a:
		return psy_ui_KEY_A;
	case XK_B:
	case XK_b:
		return psy_ui_KEY_B;
	case XK_C:
	case XK_c:
		return psy_ui_KEY_C;
	case XK_D:
	case XK_d:
		return psy_ui_KEY_D;
	case XK_E:
	case XK_e:
		return psy_ui_KEY_E;
	case XK_F:
	case XK_f:
		return psy_ui_KEY_F;
	case XK_G:
	case XK_g:
		return psy_ui_KEY_G;
	case XK_H:
	case XK_h:
		return psy_ui_KEY_H;
	case XK_I:
	case XK_i:
		return psy_ui_KEY_I;
	case XK_J:
	case XK_j:
		return psy_ui_KEY_J;
	case XK_K:
	case XK_k:
		return psy_ui_KEY_K;
	case XK_L:
	case XK_l:
		return psy_ui_KEY_L;
	case XK_M:		
	case XK_m:
		return psy_ui_KEY_M;
	case XK_N:
	case XK_n:
		return psy_ui_KEY_N;
	case XK_O:
	case XK_o:
		return psy_ui_KEY_O;
	case XK_P:
	case XK_p:
		return psy_ui_KEY_P;
	case XK_Q:
	case XK_q:
		return psy_ui_KEY_Q;
	case XK_R:
	case XK_r:
		return psy_ui_KEY_R;
	case XK_S:
	case XK_s:
		return psy_ui_KEY_S;
	case XK_T:
	case XK_t:
		return psy_ui_KEY_T;
	case XK_U:
	case XK_u:
		return psy_ui_KEY_U;
	case XK_V:
	case XK_v:
		return psy_ui_KEY_V;
	case XK_W:
	case XK_w:
		return psy_ui_KEY_W;
	case XK_X:
	case XK_x:
		return psy_ui_KEY_X;
	case XK_Y:
	case XK_y:
		return psy_ui_KEY_Y;
	case XK_Z:
	case XK_z:
		return psy_ui_KEY_Z;
	case XK_ISO_Left_Tab:
		return psy_ui_KEY_TAB;
	case XK_semicolon:
		return psy_ui_KEY_SEMICOLON;
	case XK_equal:
		return psy_ui_KEY_EQUAL;
	case XK_comma:
		return psy_ui_KEY_COMMA;
	case XK_plus:
	case XK_KP_Add:
		return psy_ui_KEY_ADD;		
	case XK_minus:
		return psy_ui_KEY_MINUS;
	case XK_KP_Multiply:
		return psy_ui_KEY_MULTIPLY;
	case XK_KP_Divide:
		return psy_ui_KEY_DIVIDE;
	case XK_period:
		return psy_ui_KEY_PERIOD;
	case XK_slash:	
		return psy_ui_KEY_SLASH;
	case XK_apostrophe:	
		return psy_ui_KEY_BACKQUOTE;
	case XK_bracketleft:	
		return psy_ui_KEY_BRACKETLEFT;
	case XK_backslash:	
		return psy_ui_KEY_BACKSLASH;
	case XK_grave:	
		return psy_ui_KEY_QUOTE;
	case XK_bracketright:	
		return psy_ui_KEY_BRACKETRIGHT;
	default:		
		return 0;		
	}	
}

XKeyEvent psy_ui_x11_xkeyevent_make(psy_ui_KeyboardEvent* e,
	Display* dpy, Window win, Window root)
{
	XKeyEvent rv;
	KeyCode keycode;
	uint32_t keysym;
	
	
	switch (psy_ui_keyboardevent_keycode(e)) {
	case psy_ui_KEY_CONTROL:
		keysym = XK_Control_L;
	break;
	case psy_ui_KEY_SHIFT:	
		keysym = XK_Shift_L;
		break;		
	case psy_ui_KEY_HOME:
		keysym = XK_Home;
		break;
	case psy_ui_KEY_END:
		keysym = XK_End;
		break;
	case psy_ui_KEY_UP:
		keysym = XK_Up;
		break;
	case psy_ui_KEY_DOWN:
		keysym = XK_Down;
		break;
	case psy_ui_KEY_LEFT:
		keysym = XK_Left;
		break;
	case psy_ui_KEY_RIGHT:
		keysym = XK_Right;
		break;
	case psy_ui_KEY_TAB:
		keysym = XK_Tab;
		break;
	case psy_ui_KEY_PRIOR:
		keysym = XK_Prior;
		break;
	case psy_ui_KEY_NEXT:
		keysym = XK_Next;
		break;
	case psy_ui_KEY_DELETE:
		keysym = XK_Delete;
		break;
	case psy_ui_KEY_BACK:
		keysym = XK_BackSpace;
		break;	
	case psy_ui_KEY_F1:
		keysym = XK_F1;
		break;
	case psy_ui_KEY_F2:
		keysym = XK_F2;
		break;
	case psy_ui_KEY_F3:
		keysym = XK_F3;
		break;
	case psy_ui_KEY_F4:
		keysym = XK_F4;
		break;
	case psy_ui_KEY_F5:
		keysym = XK_F5;
		break;
	case psy_ui_KEY_F6:
		keysym = XK_F6;
		break;
	case psy_ui_KEY_F7:
		keysym = XK_F7;
		break;
	case psy_ui_KEY_F8:
		keysym = XK_F8;
		break;
	case psy_ui_KEY_F9:
		keysym = XK_F9;
		break;
	case psy_ui_KEY_F10:
		keysym = XK_F10;
		break;
	case psy_ui_KEY_F11:
		keysym = XK_F11;
		break;
	case psy_ui_KEY_F12:
		keysym = XK_F12;
		break;
	default:
		keysym = psy_ui_keyboardevent_keycode(e);
		break;	
	}	
	keycode = XKeysymToKeycode(dpy, keysym); 	
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
    rv.state = 0; /* modifiers */
    if (psy_ui_keyboardevent_shiftkey(e) != FALSE) {
		rv.state |= ShiftMask;
	}
	if (psy_ui_keyboardevent_ctrlkey(e) != 0) {
		rv.state |= ControlMask;
	}    
	return rv;
}

#endif /* PSYCLE_TK_X11 */
