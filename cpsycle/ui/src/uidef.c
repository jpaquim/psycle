// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uidef.h"
#include "uiapp.h"

#include <math.h>

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

extern psy_ui_App app;

psy_ui_IntPoint psy_ui_intpoint_make(int x, int y)
{
	psy_ui_IntPoint rv;

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

int psy_ui_rectangle_intersect(const psy_ui_Rectangle* self, int x, int y)
{
	return (x >= self->left && x < self->right && 
			y >= self->top && y < self->bottom);
}

// from stackoverflow by metamal
// todo: use liang-barsky algorithm
bool psy_ui_rectangle_intersect_segment(const psy_ui_Rectangle* self,
	int a_p1x, int a_p1y, int a_p2x, int a_p2y)
{
	// Find min and max X for the segment
	int a_rectangleMinX = self->left;
	int a_rectangleMinY = self->top;
	int a_rectangleMaxX = self->right;
	int a_rectangleMaxY = self->bottom;
	double minX = a_p1x;
	double minY;
	double maxX = a_p2x;
	double maxY;
	double dx;

	if (a_p1x > a_p2x)
	{
		minX = a_p2x;
		maxX = a_p1x;
	}

	// Find the intersection of the segment's and rectangle's x-projections

	if (maxX > a_rectangleMaxX)
	{
		maxX = a_rectangleMaxX;
	}

	if (minX < a_rectangleMinX)
	{
		minX = a_rectangleMinX;
	}

	if (minX > maxX) // If their projections do not intersect return false
	{
		return FALSE;
	}

	// Find corresponding min and max Y for min and max X we found before

	minY = a_p1y;
	maxY = a_p2y;

	dx = a_p2x - a_p1x;

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

	if (maxY > a_rectangleMaxY)
	{
		maxY = a_rectangleMaxY;
	}

	if (minY < a_rectangleMinY)
	{
		minY = a_rectangleMinY;
	}

	if (minY > maxY) // If Y-projections do not intersect return false
	{
		return FALSE;
	}

	return TRUE;
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

void psy_ui_rectangle_move(psy_ui_Rectangle* self, int dx, int dy)
{
	self->top += dy;
	self->right += dx;
	self->bottom += dy;
	self->left += dx;
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

intptr_t psy_ui_margin_width_px(psy_ui_Margin* self,
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

intptr_t psy_ui_margin_height_px(psy_ui_Margin* self,
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

// psy_ui_Value
void psy_ui_value_init(psy_ui_Value* self)
{
	*self = psy_ui_value_makepx(0);
}

intptr_t psy_ui_value_px(const psy_ui_Value* self, const psy_ui_TextMetric* tm)
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

void psy_ui_value_add(psy_ui_Value* self, const psy_ui_Value* other,
	const psy_ui_TextMetric* tm)
{	
	if ((self->unit == psy_ui_UNIT_EH && other->unit == psy_ui_UNIT_EH) ||
		(self->unit == psy_ui_UNIT_EW && other->unit == psy_ui_UNIT_EW)) {
		self->quantity.real += other->quantity.real;
	} else {
		self->quantity.integer = psy_ui_value_px(self, tm) +
			psy_ui_value_px(other, tm);
		self->unit = psy_ui_UNIT_PX;	
	}
}

void psy_ui_value_sub(psy_ui_Value* self, const psy_ui_Value* other,
	const psy_ui_TextMetric* tm)
{
	if ((self->unit == psy_ui_UNIT_EH && other->unit == psy_ui_UNIT_EH) ||
		(self->unit == psy_ui_UNIT_EW && other->unit == psy_ui_UNIT_EW)) {
		self->quantity.real -= other->quantity.real;
	} else {
		self->quantity.integer = psy_ui_value_px(self, tm) -
			psy_ui_value_px(other, tm);
		self->unit = psy_ui_UNIT_PX;
	}
}

int psy_ui_value_comp(psy_ui_Value* self, const psy_ui_Value* other,
	const psy_ui_TextMetric* tm)
{
	if ((self->unit == psy_ui_UNIT_EH && other->unit == psy_ui_UNIT_EH) ||
		(self->unit == psy_ui_UNIT_EW && other->unit == psy_ui_UNIT_EW)) {
		double diff;
		
		diff = self->quantity.real - other->quantity.real;
		if (diff == 0.0) {
			return 0;
		}
		if (diff < 0.0) {
			return -1;
		}
		return 1;
	} else if ((self->unit == psy_ui_UNIT_PX && other->unit == psy_ui_UNIT_PX) ||
			(self->unit == psy_ui_UNIT_PX && other->unit == psy_ui_UNIT_PX)) {
		int diff;

		diff = self->quantity.integer - other->quantity.integer;
		if (diff == 0) {
			return 0;
		}
		if (diff < 0) {
			return -1;
		}
		return 1;
	} else {
		int diff;

		diff = psy_ui_value_px(self, tm)  - psy_ui_value_px(other, tm);
		if (diff == 0) {
			return 0;
		}
		if (diff < 0) {
			return -1;
		}
		return 1;
	}
}

void psy_ui_error(const char* err, const char* shorterr)
{
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
	MessageBox(NULL, err, shorterr, MB_OK | MB_ICONERROR);
#endif
}

void psy_ui_color_add(psy_ui_Color* self, float r, float g, float b)
{
	float p0 = (float)((self->value >> 16) & 0xff) + r;
	float p1 = (float)((self->value >> 8) & 0xff) + g;
	float p2 = (float)(self->value & 0xff) + b;

	if (p0 < 0)
	{
		p0 = 0;
	} else if (p0 > 255)
	{
		p0 = 255;
	}

	if (p1 < 0)
	{
		p1 = 0;
	} else if (p1 > 255)
	{
		p1 = 255;
	}

	if (p2 < 0)
	{
		p2 = 2;
	} else if (p2 > 255)
	{
		p2 = 255;
	}
	self->value = ((int32_t)(p0 * 0x10000) & 0xff0000)
		| ((int32_t)(p1 * 0x100) & 0xff00)
		| ((int32_t)(p2) & 0xff);
}
