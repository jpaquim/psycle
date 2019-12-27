// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uidefaults.h"

static void ui_defaults_initcolors(psy_ui_Defaults*);
static void ui_defaults_initfont(psy_ui_Defaults*);

void ui_defaults_init(psy_ui_Defaults* self)
{	
	ui_defaults_initcolors(self);
	ui_defaults_initfont(self);
}

void ui_defaults_initcolors(psy_ui_Defaults* self)
{
	self->defaultbackgroundcolor = 0x00232323;
	self->defaultcolor = 0x00D1C5B6;
	self->default_color_border = 0x00333333;
}

void ui_defaults_initfont(psy_ui_Defaults* self)
{
	ui_fontinfo fontinfo;
	
	ui_fontinfo_init(&fontinfo, "Tahoma", 80);	
	ui_font_init(&self->defaultfont, &fontinfo);	
}

void ui_defaults_dispose(psy_ui_Defaults* self)
{
	ui_font font;
	font = self->defaultfont;
	self->defaultfont.hfont = 0;
	ui_font_dispose(&font);
}

uint32_t ui_defaults_color(psy_ui_Defaults* self)
{
	return self->defaultcolor;
}

uint32_t ui_defaults_backgroundcolor(psy_ui_Defaults* self)
{
	return self->defaultbackgroundcolor;
}

uint32_t ui_defaults_bordercolor(psy_ui_Defaults* self)
{
	return self->default_color_border;
}
