// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_EVENTS_H
#define psy_ui_EVENTS_H

#include "uigraphics.h"
#include <signal.h>
#include "../../detail/psydef.h"

typedef struct {
	int keycode;
	int keydata;
	int shift;
	int ctrl;
	int repeat;
	BOOL bubble;
} psy_ui_KeyEvent;

void psy_ui_keyevent_init(psy_ui_KeyEvent*, int keycode, int keydata,
	int shift, int ctrl, int repeat);
void psy_ui_keyevent_stoppropagation(psy_ui_KeyEvent*);

typedef struct {	
	int x;
	int y;
	int button;
	int delta;
	BOOL bubble;
} psy_ui_MouseEvent;

void psy_ui_mouseevent_init(psy_ui_MouseEvent*, int x, int y, int button,
	int delta);
void psy_ui_mouseevent_stoppropagation(psy_ui_MouseEvent*);

#endif /* psy_ui_EVENTS_H */
