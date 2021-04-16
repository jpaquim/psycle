// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_BORDER_H
#define psy_ui_BORDER_H

// local
#include "uicolour.h"
#include "uivalue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	psy_ui_BORDER_NONE,
	psy_ui_BORDER_SOLID
} psy_ui_BorderStyle;

typedef struct {
	psy_ui_PropertyMode mode;
	psy_ui_BorderStyle top;
	psy_ui_BorderStyle right;
	psy_ui_BorderStyle bottom;
	psy_ui_BorderStyle left;
	psy_ui_Colour colour_top;
	psy_ui_Colour colour_right;
	psy_ui_Colour colour_bottom;
	psy_ui_Colour colour_left;
	psy_ui_Value border_top_left_radius;
	psy_ui_Value border_top_right_radius;
	psy_ui_Value border_bottom_right_radius;
	psy_ui_Value border_bottom_left_radius;
} psy_ui_Border;

INLINE void psy_ui_border_init_all(psy_ui_Border* self, psy_ui_BorderStyle top,
	psy_ui_BorderStyle right, psy_ui_BorderStyle bottom, psy_ui_BorderStyle left)
{	
	self->mode.set =
		top != psy_ui_BORDER_NONE ||
		right != psy_ui_BORDER_NONE ||
		bottom != psy_ui_BORDER_NONE ||
		left != psy_ui_BORDER_NONE;
	self->top = top;
	self->right = right;
	self->bottom = bottom;
	self->left = left;
	self->border_top_left_radius = psy_ui_value_zero();
	self->border_top_right_radius = psy_ui_value_zero();
	self->border_bottom_right_radius = psy_ui_value_zero();
	self->border_bottom_left_radius = psy_ui_value_zero();
	psy_ui_colour_init(&self->colour_top);
	psy_ui_colour_init(&self->colour_right);
	psy_ui_colour_init(&self->colour_bottom);
	psy_ui_colour_init(&self->colour_left);
}

INLINE void psy_ui_border_init_style(psy_ui_Border* self, psy_ui_BorderStyle style)
{
	psy_ui_border_init_all(self, style, style, style, style);
}

INLINE void psy_ui_border_init_top(psy_ui_Border* self, psy_ui_BorderStyle style,
	psy_ui_Colour  colour)
{
	psy_ui_border_init_all(self, style, psy_ui_BORDER_NONE, psy_ui_BORDER_NONE,
		psy_ui_BORDER_NONE);
	self->colour_top = colour;
}

INLINE void psy_ui_border_setcolour(psy_ui_Border* self, psy_ui_Colour colour)
{
	self->colour_top = colour;
	self->colour_right = colour;
	self->colour_bottom = colour;
	self->colour_left = colour;
}

INLINE void psy_ui_border_setradius_px(psy_ui_Border* self, double radius)
{
	self->border_top_left_radius = psy_ui_value_makepx(radius);
	self->border_top_right_radius = self->border_top_left_radius;
	self->border_bottom_right_radius = self->border_top_left_radius;
	self->border_bottom_left_radius = self->border_top_left_radius;
}

INLINE void psy_ui_border_init(psy_ui_Border* self)
{
	psy_ui_border_init_style(self, psy_ui_BORDER_NONE);	
}

INLINE bool psy_ui_border_isrect(const psy_ui_Border* self)
{
	return self->left == psy_ui_BORDER_SOLID &&
		self->top == psy_ui_BORDER_SOLID &&
		self->right == psy_ui_BORDER_SOLID &&
		self->bottom == psy_ui_BORDER_SOLID;		
}

INLINE bool psy_ui_border_isset(const psy_ui_Border* self)
{
	return self->left == psy_ui_BORDER_SOLID ||
		self->top == psy_ui_BORDER_SOLID ||
		self->right == psy_ui_BORDER_SOLID ||
		self->bottom == psy_ui_BORDER_SOLID;
}

INLINE bool psy_ui_border_monochrome(const psy_ui_Border* self)
{
	return psy_ui_equal_colours(&self->colour_top, &self->colour_right) &&
		psy_ui_equal_colours(&self->colour_right, &self->colour_bottom) &&
		psy_ui_equal_colours(&self->colour_bottom, &self->colour_left);
}

INLINE bool psy_ui_border_isround(const psy_ui_Border* self)
{
	return self->border_top_left_radius.quantity != 0 &&
		(psy_ui_value_px(&self->border_top_left_radius, 0) ==
			psy_ui_value_px(&self->border_top_right_radius, 0)) &&
		(psy_ui_value_px(&self->border_top_right_radius, 0) ==
			psy_ui_value_px(&self->border_bottom_right_radius, 0)) &&
		(psy_ui_value_px(&self->border_bottom_right_radius, 0) ==
			psy_ui_value_px(&self->border_bottom_left_radius, 0));
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_BORDER_H */
