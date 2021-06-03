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
	self->width = psy_ui_value_make_px(1.0);
}

void psy_ui_borderside_init_style(psy_ui_BorderSide* self,
	psy_ui_BorderStyle style)
{
	self->style = style;	
	psy_ui_colour_init(&self->colour);
	self->radius = psy_ui_value_zero();
	self->width = psy_ui_value_make_px(1.0);
}

void psy_ui_borderside_init_all(psy_ui_BorderSide* self,
	psy_ui_BorderStyle style, psy_ui_Colour colour)
{
	self->style = style;	
	self->colour = colour;
	self->radius = psy_ui_value_zero();
	self->width = psy_ui_value_make_px(1.0);
}

/* psy_ui_Border */
psy_ui_Size psy_ui_border_size(const psy_ui_Border* self)
{	
	psy_ui_RealSize maxsize;

	psy_ui_realsize_init(&maxsize);
	if (self->left.style == psy_ui_BORDER_SOLID) {
		maxsize.width += psy_ui_value_px(&self->left.width, NULL, NULL);
	}
	if (self->top.style == psy_ui_BORDER_SOLID) {
		maxsize.height += psy_ui_value_px(&self->top.width, NULL, NULL);
	}
	if (self->right.style == psy_ui_BORDER_SOLID) {
		maxsize.width += psy_ui_value_px(&self->right.width, NULL, NULL);
	}
	if (self->bottom.style == psy_ui_BORDER_SOLID) {
		maxsize.height += psy_ui_value_px(&self->bottom.width, NULL, NULL);
	}
	return psy_ui_size_makereal(maxsize);	
}

psy_ui_Margin psy_ui_border_margin(const psy_ui_Border* self)
{
	psy_ui_Margin rv;	

	psy_ui_margin_init(&rv);	
	if (self->top.style == psy_ui_BORDER_SOLID) {
		rv.top = self->top.width;
	}
	if (self->right.style == psy_ui_BORDER_SOLID) {
		rv.right = self->right.width;
	}
	if (self->bottom.style == psy_ui_BORDER_SOLID) {
		rv.bottom = self->bottom.width;
	}
	if (self->left.style == psy_ui_BORDER_SOLID) {
		rv.left = self->left.width;
	}
	return rv;
}
