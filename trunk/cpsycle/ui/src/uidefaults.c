// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uidefaults.h"
#include "uifont.h"

static void psy_ui_defaults_initcolours(psy_ui_Defaults*);

void psy_ui_defaults_init(psy_ui_Defaults* self)
{	
	psy_ui_defaults_initcolours(self);
	psy_ui_colour_set(&self->style_common.colour, psy_ui_colour_make(0x00D1C5B6));
	psy_ui_colour_set(&self->style_common.backgroundcolour,
		psy_ui_colour_make(0x00232323));
	psy_ui_border_init(&self->style_common.border);
	psy_ui_colour_set(&self->style_common.border.colour_top,
		psy_ui_colour_make(0x00333333));
	psy_ui_colour_set(&self->style_common.border.colour_right,
		self->style_common.border.colour_top);
	psy_ui_colour_set(&self->style_common.border.colour_bottom,
		self->style_common.border.colour_top);
	psy_ui_colour_set(&self->style_common.border.colour_left,
		self->style_common.border.colour_top);
	self->style_common.use_font = 1;
	{
		psy_ui_FontInfo fontinfo;
	
		psy_ui_fontinfo_init(&fontinfo, "Tahoma", 80);	
		psy_ui_font_init(&self->style_common.font, &fontinfo);
	}	
	psy_ui_margin_init_all(&self->hmargin,
		psy_ui_value_makepx(0), psy_ui_value_makeew(1.0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));
	psy_ui_margin_init_all(&self->vmargin,
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makeeh(0.5), psy_ui_value_makepx(0));
	psy_ui_margin_init_all(&self->cmargin,
		psy_ui_value_makeew(0.5), psy_ui_value_makeew(0.5),
		psy_ui_value_makeeh(0.5), psy_ui_value_makeeh(0.5));
	// orange 0x002279F1
	// green 0x00B1C8B0
}

void psy_ui_defaults_initcolours(psy_ui_Defaults* self)
{
	self->errorcolour = 0xCF6679;
}

void psy_ui_defaults_dispose(psy_ui_Defaults* self)
{	
	psy_ui_style_dispose(&self->style_common);
}

