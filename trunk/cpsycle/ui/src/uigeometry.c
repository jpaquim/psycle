// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uigeometry.h"

#include <math.h>

// extern
psy_ui_Point psy_ui_internal_point_zero;
psy_ui_RealPoint psy_ui_internal_realpoint_zero;

void psy_ui_geometry_init(void)
{
	psy_ui_point_init(&psy_ui_internal_point_zero);
	psy_ui_realpoint_init(&psy_ui_internal_realpoint_zero);
}

void psy_ui_setrectangle(psy_ui_RealRectangle* self, double left, double top, double width,
	double height)
{
   self->left = left;
   self->top = top;
   self->right = left + width;
   self->bottom = top + height;   
}

int psy_ui_realrectangle_intersect_rectangle(const psy_ui_RealRectangle* self,
	const psy_ui_RealRectangle* other)
{
	return !(other->left > self->right ||
		other->right < self->left ||
		other->top > self->bottom ||
		other->bottom < self->top);
}

// from stackoverflow by metamal
// todo: use liang-barsky algorithm
bool psy_ui_realrectangle_intersect_segment(const psy_ui_RealRectangle* self,
	double a_p1x, double a_p1y, double a_p2x, double a_p2y)
{
	// Find min and max X for the segment
	double a_rectangleMinX = self->left;
	double a_rectangleMinY = self->top;
	double a_rectangleMaxX = self->right;
	double a_rectangleMaxY = self->bottom;
	double minX = (double)a_p1x;
	double minY;
	double maxX = (double)a_p2x;
	double maxY;
	double dx;

	if (a_p1x > a_p2x)
	{
		minX = (double)a_p2x;
		maxX = (double)a_p1x;
	}

	// Find the intersection of the segment's and rectangle's x-projections

	if (maxX > (double)a_rectangleMaxX)
	{
		maxX = (double)a_rectangleMaxX;
	}

	if (minX < (double)a_rectangleMinX)
	{
		minX = (double)a_rectangleMinX;
	}

	if (minX > maxX) // If their projections do not intersect return false
	{
		return FALSE;
	}

	// Find corresponding min and max Y for min and max X we found before

	minY = (double)a_p1y;
	maxY = (double)a_p2y;

	dx = (double)a_p2x - (double)a_p1x;

	if (fabs(dx) > 0.0000001)
	{
		double a = (a_p2y - a_p1y) / dx;
		double b = a_p1y - a * a_p1x;
		minY = a * minX + b;
		maxY = a * maxX + b;
	}

	if (minY > maxY)
	{
		double tmp = maxY;
		maxY = minY;
		minY = tmp;
	}

	// Find the intersection of the segment's and rectangle's y-projections

	if (maxY > (double)a_rectangleMaxY)
	{
		maxY = (double)a_rectangleMaxY;
	}

	if (minY < (double)a_rectangleMinY)
	{
		minY = (double)a_rectangleMinY;
	}

	if (minY > maxY) // If Y-projections do not intersect return false
	{
		return FALSE;
	}

	return TRUE;
}

void psy_ui_realrectangle_union(psy_ui_RealRectangle* self,
	const psy_ui_RealRectangle* other)
{
	self->left = (self->left < other->left) ? self->left : other->left;
	self->right = (self->right > other->right) ? self->right : other->right;
	self->top = (self->top < other->top) ? self->top : other->top;
	self->bottom = (self->bottom > other->bottom) ? self->bottom : other->bottom;
}

void psy_ui_realrectangle_expand(psy_ui_RealRectangle* self, double top, double right, double bottom, double left)
{
	self->top -= top;
	self->right += right;
	self->bottom += bottom;
	self->left -= left;	
}

void psy_ui_realrectangle_move(psy_ui_RealRectangle* self, double dx, double dy)
{
	self->top += dy;
	self->right += dx;
	self->bottom += dy;
	self->left += dx;
}

void psy_ui_realrectangle_settopleft(psy_ui_RealRectangle* self, psy_ui_RealPoint topleft)
{
	psy_ui_RealSize size;

	size = psy_ui_realrectangle_size(self);
	self->left = topleft.x;
	self->top = topleft.y;
	self->right = self->left + size.width;
	self->bottom = self->top + size.height;	
}

void psy_ui_margin_init(psy_ui_Margin* self)
{
	self->top = psy_ui_value_makepx(0);
	self->right = psy_ui_value_makepx(0);
	self->bottom = psy_ui_value_makepx(0);
	self->left = psy_ui_value_makepx(0);
}

void psy_ui_margin_init_all(psy_ui_Margin* self, psy_ui_Value top,
	psy_ui_Value right, psy_ui_Value bottom, psy_ui_Value left)
{   
   self->top = top;
   self->right = right;
   self->bottom = bottom;
   self->left = left;
}

void psy_ui_margin_init_all_em(psy_ui_Margin* self, double top,
	double right, double bottom, double left)
{
	psy_ui_value_seteh(&self->top, top);
	psy_ui_value_setew(&self->right, right);
	psy_ui_value_seteh(&self->bottom, bottom);
	psy_ui_value_setew(&self->left, left);
}

void psy_ui_margin_settop(psy_ui_Margin* self, psy_ui_Value value)
{
	self->top = value;
}

void psy_ui_margin_setright(psy_ui_Margin* self, psy_ui_Value value)
{
	self->right = value;
}

void psy_ui_margin_setbottom(psy_ui_Margin* self, psy_ui_Value value)
{
	self->bottom = value;
}

void psy_ui_margin_setleft(psy_ui_Margin* self, psy_ui_Value value)
{
	self->left = value;
}

double psy_ui_margin_width_px(psy_ui_Margin* self,
	const psy_ui_TextMetric* tm)
{
	return psy_ui_value_px(&self->left, tm) +
		psy_ui_value_px(&self->right, tm);
}

psy_ui_Value psy_ui_margin_width(psy_ui_Margin* self,
	const psy_ui_TextMetric* tm)
{
	return psy_ui_add_values(self->left, self->right, tm);
}

double psy_ui_margin_height_px(psy_ui_Margin* self,
	const psy_ui_TextMetric* tm)
{
	return psy_ui_value_px(&self->top, tm) +
		psy_ui_value_px(&self->bottom, tm);
}

psy_ui_Value psy_ui_margin_height(psy_ui_Margin* self,
	const psy_ui_TextMetric* tm)
{
	return psy_ui_add_values(self->top, self->bottom, tm);
}
