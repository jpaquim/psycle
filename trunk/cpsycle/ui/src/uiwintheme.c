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
	self->cl_yellow = psy_ui_colour_make(0x00FFFF);
	self->cl_red = psy_ui_colour_make(0x0000FF);
	/* borders */
	psy_ui_border_setcolours(&self->raised,
		self->cl_white, self->cl_gray, self->cl_gray, self->cl_white);
	psy_ui_border_setcolours(&self->lowered,
		self->cl_gray, self->cl_white, self->cl_white, self->cl_gray);
}

void psy_ui_lighttheme_init(psy_ui_LightTheme* self)
{
	/* colours */
	self->cl_white = psy_ui_colour_make(0xFFFFFF);
	self->cl_white_1 = psy_ui_colour_make(0xF3F3F3);
	self->cl_white_2 = psy_ui_colour_make(0xE6E6E6);
	self->cl_white_3 = psy_ui_colour_make(0xD6D6D6);
	self->cl_white_4 = psy_ui_colour_make(0xC6C6C6);
	self->cl_black_1 = psy_ui_colour_make(0x262626);
	self->cl_blue_1 = psy_ui_colour_make(0xD47800);
	self->cl_blue_2 = psy_ui_colour_make(0xC97200);	
	self->cl_blue_3 = psy_ui_colour_make(0xBE6C00);
	self->cl_green_1 = psy_ui_colour_make(0x217346);
	self->cl_font_1 = psy_ui_colour_make(0x262626);
}