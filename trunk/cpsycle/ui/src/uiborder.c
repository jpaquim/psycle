/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiborder.h"

/* psy_ui_BorderSide */
void psy_ui_borderside_init(psy_ui_BorderSide* self)
{	
	self->style = psy_ui_BORDER_NONE;	
	psy_ui_colour_init(&self->colour);
	self->radius = psy_ui_value_zero();
}

void psy_ui_borderside_init_style(psy_ui_BorderSide* self,
	psy_ui_BorderStyle style)
{
	self->style = style;	
	psy_ui_colour_init(&self->colour);
	self->radius = psy_ui_value_zero();
}

void psy_ui_borderside_init_all(psy_ui_BorderSide* self,
	psy_ui_BorderStyle style, psy_ui_Colour colour)
{
	self->style = style;	
	self->colour = colour;
	self->radius = psy_ui_value_zero();
}

/* psy_ui_Border */
psy_ui_Size psy_ui_border_size(const psy_ui_Border* self)
{	
	psy_ui_RealSize maxsize;

	psy_ui_realsize_init(&maxsize);
	if (self->left.style == psy_ui_BORDER_SOLID) {
		++(maxsize.width);
	}
	if (self->top.style == psy_ui_BORDER_SOLID) {
		++(maxsize.height);
	}
	if (self->right.style == psy_ui_BORDER_SOLID) {
		++(maxsize.width);
	}
	if (self->bottom.style == psy_ui_BORDER_SOLID) {
		++(maxsize.height);
	}
	return psy_ui_size_makereal(maxsize);	
}
