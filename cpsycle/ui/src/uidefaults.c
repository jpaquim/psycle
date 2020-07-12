// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uidefaults.h"
#include "uifont.h"

static void psy_ui_defaults_initcolors(psy_ui_Defaults*);

void psy_ui_defaults_init(psy_ui_Defaults* self)
{	
	psy_ui_defaults_initcolors(self);
	psy_ui_color_set(&self->style_common.color, psy_ui_color_make(0x00D1C5B6));
	psy_ui_color_set(&self->style_common.backgroundcolor,
		psy_ui_color_make(0x00232323));
	psy_ui_border_init(&self->style_common.border);
	psy_ui_color_set(&self->style_common.border.color_top,
		psy_ui_color_make(0x00333333));
	psy_ui_color_set(&self->style_common.border.color_right,
		self->style_common.border.color_top);
	psy_ui_color_set(&self->style_common.border.color_bottom,
		self->style_common.border.color_top);
	psy_ui_color_set(&self->style_common.border.color_left,
		self->style_common.border.color_top);
	self->style_common.use_font = 1;
	{
		psy_ui_FontInfo fontinfo;
	
		psy_ui_fontinfo_init(&fontinfo, "Tahoma", 80);	
		psy_ui_font_init(&self->style_common.font, &fontinfo);
	}	
}

void psy_ui_defaults_initcolors(psy_ui_Defaults* self)
{
	self->errorcolor = 0xCF6679;
}

void psy_ui_defaults_dispose(psy_ui_Defaults* self)
{	
	psy_ui_style_dispose(&self->style_common);
}

