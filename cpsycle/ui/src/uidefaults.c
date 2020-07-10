// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uidefaults.h"
#include "uifont.h"

static void psy_ui_defaults_initcolors(psy_ui_Defaults*);

void psy_ui_defaults_init(psy_ui_Defaults* self)
{	
	psy_ui_defaults_initcolors(self);
	self->style_common.use_color = 1;
	self->style_common.color = 0x00D1C5B6;
	self->style_common.use_backgroundcolor = 1;
	self->style_common.backgroundcolor = self->defaultbackgroundcolor;
	self->style_common.use_font = 1;
	{
		psy_ui_FontInfo fontinfo;
	
		psy_ui_fontinfo_init(&fontinfo, "Tahoma", 80);	
		psy_ui_font_init(&self->style_common.font, &fontinfo);
	}	
}

void psy_ui_defaults_initcolors(psy_ui_Defaults* self)
{
	self->defaultbackgroundcolor = 0x00232323;
	self->defaultcolor = 0x00D1C5B6;
	self->default_color_border = 0x00333333;
	self->errorcolor = 0xCF6679;
}

void psy_ui_defaults_dispose(psy_ui_Defaults* self)
{	
	psy_ui_style_dispose(&self->style_common);
}

uint32_t psy_ui_defaults_color(psy_ui_Defaults* self)
{
	return self->defaultcolor;
}

uint32_t psy_ui_defaults_backgroundcolor(psy_ui_Defaults* self)
{	
	return self->defaultbackgroundcolor;
}

uint32_t psy_ui_defaults_bordercolor(psy_ui_Defaults* self)
{
	return self->default_color_border;
}
