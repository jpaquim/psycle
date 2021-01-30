// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uistyle.h"
// std
#include <stdlib.h>
#include <string.h>

void psy_ui_style_init(psy_ui_Style* self)
{
	psy_ui_colour_init(&self->colour);
	psy_ui_colour_init(&self->backgroundcolour);
	psy_ui_border_init(&self->border);
	self->use_font = 0;
}

void psy_ui_style_init_colours(psy_ui_Style* self, psy_ui_Colour colour,
	psy_ui_Colour background)
{
	self->colour = colour;
	self->backgroundcolour = background;
	psy_ui_border_init(&self->border);
	self->use_font = 0;
}

void psy_ui_style_dispose(psy_ui_Style* self)
{
	if (self->use_font) {
		psy_ui_font_dispose(&self->font);
	}
}

void psy_ui_style_copy(psy_ui_Style* self, psy_ui_Style* other)
{
	self->colour = other->colour;
	self->backgroundcolour = other->backgroundcolour;
}

psy_ui_Style* psy_ui_style_alloc(void)
{
	return (psy_ui_Style*)malloc(sizeof(psy_ui_Style));
}

psy_ui_Style* psy_ui_style_allocinit(void)
{
	psy_ui_Style* rv;

	rv = psy_ui_style_alloc();
	if (rv) {
		psy_ui_style_init(rv);
	}
	return rv;
}

psy_ui_Style* psy_ui_style_allocinit_colours(psy_ui_Colour colour,
	psy_ui_Colour background)
{
	psy_ui_Style* rv;

	rv = psy_ui_style_alloc();
	if (rv) {
		psy_ui_style_init_colours(rv, colour, background);
	}
	return rv;
}

void psy_ui_style_deallocate(psy_ui_Style* self)
{
	psy_ui_style_dispose(self);
	free(self);
}
