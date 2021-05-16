/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_BORDER_H
#define psy_ui_BORDER_H

/* local */
#include "uicolour.h"
#include "uigeometry.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	psy_ui_BORDER_NONE,
	psy_ui_BORDER_SOLID
} psy_ui_BorderStyle;

typedef struct psy_ui_BorderSide {
	psy_ui_BorderStyle style;
	psy_ui_Colour colour;
	psy_ui_Value radius;
} psy_ui_BorderSide;

void psy_ui_borderside_init(psy_ui_BorderSide*);
void psy_ui_borderside_init_style(psy_ui_BorderSide*, psy_ui_BorderStyle);
void psy_ui_borderside_init_all(psy_ui_BorderSide*, psy_ui_BorderStyle,
	psy_ui_Colour);

typedef struct psy_ui_Border {
	psy_ui_PropertyMode mode;
	psy_ui_BorderSide top;
	psy_ui_BorderSide right;
	psy_ui_BorderSide bottom;
	psy_ui_BorderSide left;	
} psy_ui_Border;

INLINE void psy_ui_border_init_all(psy_ui_Border* self, psy_ui_BorderStyle top,
	psy_ui_BorderStyle right, psy_ui_BorderStyle bottom, psy_ui_BorderStyle left)
{	
	self->mode.set =
		top != psy_ui_BORDER_NONE ||
		right != psy_ui_BORDER_NONE ||
		bottom != psy_ui_BORDER_NONE ||
		left != psy_ui_BORDER_NONE;
	psy_ui_borderside_init_style(&self->top, top);
	psy_ui_borderside_init_style(&self->right, right);
	psy_ui_borderside_init_style(&self->bottom, bottom);
	psy_ui_borderside_init_style(&self->left, left);	
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
	self->top.colour = colour;
}

INLINE void psy_ui_border_init_right(psy_ui_Border* self, psy_ui_BorderStyle style,
	psy_ui_Colour  colour)
{
	psy_ui_border_init_all(self, psy_ui_BORDER_NONE, style, psy_ui_BORDER_NONE,
		psy_ui_BORDER_NONE);
	self->right.colour = colour;
}

INLINE void psy_ui_border_init_bottom(psy_ui_Border* self, psy_ui_BorderStyle style,
	psy_ui_Colour  colour)
{
	psy_ui_border_init_all(self, psy_ui_BORDER_NONE, psy_ui_BORDER_NONE, style,
		psy_ui_BORDER_NONE);
	self->bottom.colour = colour;
}

INLINE void psy_ui_border_init_left(psy_ui_Border* self, psy_ui_BorderStyle style,
	psy_ui_Colour  colour)
{
	psy_ui_border_init_all(self, psy_ui_BORDER_NONE, psy_ui_BORDER_NONE, psy_ui_BORDER_NONE,
		style);
	self->left.colour = colour;
}

INLINE void psy_ui_border_setcolour(psy_ui_Border* self, psy_ui_Colour colour)
{
	self->top.colour = colour;
	self->right.colour = colour;
	self->bottom.colour = colour;
	self->left.colour = colour;
}

INLINE void psy_ui_border_setradius_px(psy_ui_Border* self, double radius)
{
	self->top.radius = self->right.radius = self->bottom.radius =
		self->left.radius =  psy_ui_value_make_px(radius);	
}

INLINE void psy_ui_border_init(psy_ui_Border* self)
{
	psy_ui_border_init_style(self, psy_ui_BORDER_NONE);	
}

INLINE bool psy_ui_border_isrect(const psy_ui_Border* self)
{
	return self->left.style == psy_ui_BORDER_SOLID &&
		self->top.style == psy_ui_BORDER_SOLID &&
		self->right.style == psy_ui_BORDER_SOLID &&
		self->bottom.style == psy_ui_BORDER_SOLID;		
}

INLINE bool psy_ui_border_isset(const psy_ui_Border* self)
{
	return self->left.style == psy_ui_BORDER_SOLID ||
		self->top.style == psy_ui_BORDER_SOLID ||
		self->right.style == psy_ui_BORDER_SOLID ||
		self->bottom.style == psy_ui_BORDER_SOLID;
}

INLINE bool psy_ui_border_monochrome(const psy_ui_Border* self)
{
	return psy_ui_equal_colours(&self->top.colour, &self->right.colour) &&
		psy_ui_equal_colours(&self->right.colour, &self->bottom.colour) &&
		psy_ui_equal_colours(&self->bottom.colour, &self->left.colour);
}

INLINE bool psy_ui_border_isround(const psy_ui_Border* self)
{
	return self->top.radius.quantity != 0 &&
		(psy_ui_value_px(&self->top.radius, 0, NULL) ==
			psy_ui_value_px(&self->right.radius, 0, NULL)) &&
		(psy_ui_value_px(&self->right.radius, 0, NULL) ==
			psy_ui_value_px(&self->bottom.radius, 0, NULL)) &&
		(psy_ui_value_px(&self->bottom.radius, 0, NULL) ==
			psy_ui_value_px(&self->left.radius, 0, NULL));
}

INLINE void psy_ui_border_init_solid(psy_ui_Border* self, psy_ui_Colour colour)
{
	psy_ui_border_init_all(self, psy_ui_BORDER_SOLID, psy_ui_BORDER_SOLID,
		psy_ui_BORDER_SOLID, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(self, colour);
}

INLINE void psy_ui_border_init_solid_radius(psy_ui_Border* self,
	psy_ui_Colour colour, double r)
{
	psy_ui_border_init_solid(self, colour);
	psy_ui_border_setradius_px(self, r);
}

psy_ui_Size psy_ui_border_size(const psy_ui_Border* self);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_BORDER_H */
