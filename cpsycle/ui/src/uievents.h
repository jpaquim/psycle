// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSYUIEVENTS_H)
#define PSYUIEVENTS_H

#include "uigraphics.h"
#include <signal.h>

typedef struct {
	int keycode;
	int keydata;
	int shift;
	int ctrl;
	int repeat;
} psy_ui_KeyEvent;

void psy_ui_keyevent_init(psy_ui_KeyEvent*, int keycode, int keydata,
	int shift, int ctrl, int repeat);

typedef struct {	
	int x;
	int y;
	int button;
	int delta;
} psy_ui_MouseEvent;

void psy_ui_mouseevent_init(psy_ui_MouseEvent*, int x, int y, int button,
	int delta);

#endif
