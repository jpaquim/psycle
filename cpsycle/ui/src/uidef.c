// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uidef.h"
#include "uiapp.h"
#include <math.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

extern psy_ui_App app;

psy_ui_Point psy_ui_point_make(int x, int y)
{
	psy_ui_Point rv;

	rv.x = x;
	rv.y = y;
	return rv;
}

void psy_ui_setrectangle(psy_ui_Rectangle* self, int left, int top, int width,
	int height)
{
   self->left = left;
   self->top = top;
   self->right = left + width;
   self->bottom = top + height;   
}

int psy_ui_rectangle_intersect_rectangle(const psy_ui_Rectangle* self,
	const psy_ui_Rectangle* other)
{
	return !(other->left > self->right ||
		other->right < self->left ||
		other->top > self->bottom ||
		other->bottom < self->top);
}

int psy_ui_rectangle_intersect(psy_ui_Rectangle* self, int x, int y)
{
	return (x >= self->left && x < self->right && 
			y >= self->top && y < self->bottom);
}

void psy_ui_rectangle_union(psy_ui_Rectangle* self,
	const psy_ui_Rectangle* other)
{
	self->left = self->left < other->left ? self->left : other->left;
	self->right = self->right > other->right ? self->right : other->right;
	self->top = self->top < other->top ? self->top : other->top;
	self->bottom = self->bottom > other->bottom ? self->bottom : other->bottom;
}

void psy_ui_rectangle_expand(psy_ui_Rectangle* self, int top, int right, int bottom, int left)
{
	self->top -= top;
	self->right += right;
	self->bottom += bottom;
	self->left -= left;	
}

void psy_ui_margin_init(psy_ui_Margin* self, psy_ui_Value top,
	psy_ui_Value right, psy_ui_Value bottom, psy_ui_Value left)
{   
   self->top = top;
   self->right = right;
   self->bottom = bottom;
   self->left = left;
}

intptr_t psy_ui_margin_width_px(psy_ui_Margin* self,
	const psy_ui_TextMetric* tm)
{
	return psy_ui_value_px(&self->left, tm) +
		psy_ui_value_px(&self->right, tm);
}

intptr_t psy_ui_margin_height_px(psy_ui_Margin* self,
	const psy_ui_TextMetric* tm)
{
	return psy_ui_value_px(&self->top, tm) +
		psy_ui_value_px(&self->bottom, tm);
}

psy_ui_Value psy_ui_value_makepx(intptr_t px)
{
	psy_ui_Value rv;

	rv.quantity.integer = px;
	rv.unit = psy_ui_UNIT_PX;
	return rv;
}

psy_ui_Value psy_ui_value_makeew(double em)
{
	psy_ui_Value rv;

	rv.quantity.real = em;
	rv.unit = psy_ui_UNIT_EW;
	return rv;
}

psy_ui_Value psy_ui_value_makeeh(double em)
{
	psy_ui_Value rv;

	rv.quantity.real = em;
	rv.unit = psy_ui_UNIT_EH;
	return rv;
}

intptr_t psy_ui_value_px(psy_ui_Value* self, const psy_ui_TextMetric* tm)
{
	intptr_t rv = self->quantity.integer;

	switch (self->unit) {
		case psy_ui_UNIT_PX:
			rv = self->quantity.integer;
		break;
		case psy_ui_UNIT_EW:
			if (tm) {
				rv = (intptr_t)(self->quantity.real * tm->tmAveCharWidth);
			}
		break;
		case psy_ui_UNIT_EH:
			if (tm) {
				rv = (intptr_t)(self->quantity.real * tm->tmHeight);
			}
		break;
		default:			
		break;
	}
	return rv;
}

void psy_ui_error(const char* err, const char* shorterr)
{
	MessageBox(NULL, err, shorterr, MB_OK | MB_ICONERROR);
}
