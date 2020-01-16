// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uievents.h"

void psy_ui_keyevent_init(psy_ui_KeyEvent* self, int keycode, int keydata,
	int shift, int ctrl, int repeat)
{
	self->keycode = keycode;
	self->keydata = keydata;
	self->shift = shift;
	self->ctrl = ctrl;
	self->repeat = repeat;
	self->bubble = 1;
}

void psy_ui_keyevent_stoppropagation(psy_ui_KeyEvent* self)
{
	self->bubble = 0;
}

void psy_ui_mouseevent_init(psy_ui_MouseEvent* self, int x, int y, int button,
	int delta)
{
	self->x = x;
	self->y = y;
	self->button = button;
	self->delta = 0;
	self->bubble = 1;
}

void psy_ui_mouseevent_stoppropagation(psy_ui_MouseEvent* self)
{
	self->bubble = 0;
}
