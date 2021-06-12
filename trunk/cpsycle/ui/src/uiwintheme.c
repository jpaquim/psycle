/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiwintheme.h"

void psy_ui_wintheme_init(psy_ui_WinTheme* self)
{
	/* colours */
	self->cl_black = psy_ui_colour_make(0x000000);
	self->cl_green = psy_ui_colour_make(0x008000);
	self->cl_lime = psy_ui_colour_make(0x00FF00);
	self->cl_teal = psy_ui_colour_make(0x808000);
	self->cl_gray = psy_ui_colour_make(0x808080);
	self->cl_medgray = psy_ui_colour_make(0xA4A0A0);
	self->cl_silver = psy_ui_colour_make(0xC0C0C0);
	self->cl_navy = psy_ui_colour_make(0x800000);
	self->cl_blue = psy_ui_colour_make(0xFF0000);
	self->cl_skyblue = psy_ui_colour_make(0xF0CAA6);
	self->cl_aqua = psy_ui_colour_make(0xFFFF00);
	self->cl_white = psy_ui_colour_make(0xFFFFFF);
	/* borders */
	psy_ui_border_setcolours(&self->raised,
		self->cl_white, self->cl_gray, self->cl_gray, self->cl_white);
	psy_ui_border_setcolours(&self->lowered,
		self->cl_gray, self->cl_white, self->cl_white, self->cl_gray);
}
