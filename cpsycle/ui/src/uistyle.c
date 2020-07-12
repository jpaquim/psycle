// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uistyle.h"


void psy_ui_style_init(psy_ui_Style* self)
{
	psy_ui_color_init(&self->color);
	psy_ui_color_init(&self->backgroundcolor);
	psy_ui_border_init(&self->border);
	self->use_font = 0;
}

void psy_ui_style_dispose(psy_ui_Style* self)
{
	if (self->use_font) {
		psy_ui_font_dispose(&self->font);
	}
}
