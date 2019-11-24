// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uievents.h"

void keyevent_init(KeyEvent* self, int keycode, int keydata, int shift,
	int ctrl)
{
	self->keycode = keycode;
	self->keydata = keydata;
	self->shift = shift;
	self->ctrl = ctrl;
}

void mouseevent_init(MouseEvent* self, int x, int y, int button, int delta)
{
	self->x = x;
	self->y = y;
	self->button = button;
	self->delta = 0;
}