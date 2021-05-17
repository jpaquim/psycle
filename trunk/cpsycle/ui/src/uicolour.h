/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_COLOUR_H
#define psy_ui_COLOUR_H

/* platform */
#include "../../detail/psydef.h"
/* std */
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_ColourMode {
	bool set;	
} psy_ui_ColourMode;

typedef struct psy_ui_Colour
{
	psy_ui_ColourMode mode;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;	
	uint8_t overlay;
} psy_ui_Colour;

INLINE void psy_ui_colour_init(psy_ui_Colour* self)
{	
	self->mode.set = FALSE;
	self->overlay = 0;
	self->r = self->g = self->b = self->a = 0;
}

INLINE void psy_ui_colour_init_rgb(psy_ui_Colour* self, uint8_t r, uint8_t g, uint8_t b)
{	
	self->mode.set = TRUE;
	self->overlay = 0;
	self->a = 0;
	self->r = r;
	self->g = g;
	self->b = b;	
}

void psy_ui_colour_init_str(psy_ui_Colour* self, const char* str);

INLINE psy_ui_Colour psy_ui_colour_make(uint32_t colorref)
{
	psy_ui_Colour rv;	

	psy_ui_colour_init_rgb(&rv,
		(uint8_t)((colorref) & 0xFF),
		(uint8_t)((colorref >> 8) & 0xFF),
		(uint8_t)((colorref >> 16) & 0xFF));	
	return rv;
}

INLINE uint32_t psy_ui_colour_colorref(const psy_ui_Colour* self)
{
	uint32_t rv;

	rv = (self->r) | (self->g << 8) | (self->b << 16);
	return rv;
}

INLINE psy_ui_Colour psy_ui_colour_make_overlay(uint8_t value)
{
	psy_ui_Colour rv;

	rv.mode.set = TRUE;
	rv.overlay = value;
	rv.r = rv.b = rv.g = 0;	
	return rv;
}

INLINE psy_ui_Colour psy_ui_colour_make_argb(uint32_t argb)
{
	psy_ui_Colour rv;
	
	psy_ui_colour_init_rgb(&rv,		
		(uint8_t)(argb & 0xFF),
		(uint8_t)((argb >> 8) & 0xFF),		
		(uint8_t)((argb >> 16) & 0xFF));
	rv.a = (uint8_t)((argb >> 24) & 0xFF);
	return rv;
}

INLINE psy_ui_Colour psy_ui_colour_make_notset(void)
{
	psy_ui_Colour rv;

	psy_ui_colour_init(&rv);
	return rv;
}

INLINE void psy_ui_colour_set(psy_ui_Colour* self, psy_ui_Colour colour)
{	
	assert(self);

	*self = colour;	
}

INLINE psy_ui_Colour psy_ui_colour_make_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	psy_ui_Colour rv;
	
	psy_ui_colour_init_rgb(&rv, r, g, b);	
	return rv;
}

INLINE void psy_ui_colour_rgb(const psy_ui_Colour* self,
	uint8_t* r, uint8_t* g, uint8_t* b)
{
	*r = self->r;
	*g = self->g;
	*b = self->b;
}

psy_ui_Colour* psy_ui_colour_add_rgb(psy_ui_Colour* self, float r, float g, float b);
psy_ui_Colour* psy_ui_colour_mul_rgb(psy_ui_Colour* self, float r, float g, float b);
psy_ui_Colour psy_ui_diffadd_colours(psy_ui_Colour base, psy_ui_Colour adjust,
	psy_ui_Colour add);

INLINE bool psy_ui_equal_colours(const psy_ui_Colour* lhs, const psy_ui_Colour* rhs)
{
	return lhs->mode.set && rhs->mode.set &&
		(lhs->r == rhs->r) &&
		(lhs->g == rhs->g) &&
		(lhs->b == rhs->b) &&		
		(lhs->a == rhs->a);
}
INLINE psy_ui_Colour psy_ui_colour_overlayed(psy_ui_Colour* self,
	const psy_ui_Colour* colour, double p)
{
	uint8_t r1, g1, b1;
	uint8_t r2, g2, b2;

	psy_ui_colour_rgb(self, &r1, &g1, &b1);
	psy_ui_colour_rgb(colour, &r2, &g2, &b2);
	return psy_ui_colour_make_rgb(
		(uint8_t)((1 - p) * r1 + p * r2),
		(uint8_t)((1 - p) * g1 + p * g2),
		(uint8_t)((1 - p) * b1 + p * b2));
}

INLINE int psy_ui_equal_colour_weight(const psy_ui_Colour* self)
{
	uint8_t r;
	double r_p;
	uint8_t g;
	double g_p;
	uint8_t b;
	double b_p;
	double perc;
	int rv;

	psy_ui_colour_rgb(self, &r, &g, &b);
	r_p = r / 255.f;
	g_p = g / 255.f;
	b_p = b / 255.f;
	perc = (r_p * 0.40) + (g_p * 0.20) + (b_p * 0.40);
	if (perc >= 0.9) {
		rv = 50;
	} else if (perc >= 0.86) {
		rv = 100;
	} else if (perc >= 0.77) {
		rv = 200;
	} else if (perc >= 0.68) {
		rv = 300;
	} else if (perc >= 0.61) {
		rv = 400;
	} else if (perc >= 0.54) {
		rv = 500;
	} else if (perc >= 0.51) {
		rv = 600;
	} else if (perc >= 0.46) {
		rv = 700;
	} else if (perc >= 0.42) {
		rv = 800;
	} else {
		rv = 900;
	}
	return rv;
}

INLINE psy_ui_Colour psy_ui_colour_weighted(const psy_ui_Colour* self, int weight)
{	
	if (psy_ui_equal_colour_weight(self) != weight) {
		double gain;
		
		uint8_t r;
		uint16_t rd;
		double r_p;
		uint8_t g;
		uint16_t gd;
		double g_p;
		uint8_t b;
		uint16_t bd;
		double b_p;
		double s;
		double perc;

		psy_ui_colour_rgb(self, &r, &g, &b);
		r_p = r / 255.f;
		g_p = g / 255.f;
		b_p = b / 255.f;
		if (weight < 100) {
			perc = 0.9;				/* 50 */
		} else if (weight < 200) {
			perc = 0.86;			/* 100 */
		} else if (weight < 300) {
			perc = 0.77;			/* 200 */
		} else if (weight < 400) {
			perc = 0.68;			/* 300 */
		} else if (weight < 500) {
			perc = 0.61;			/* 400 */
		} else if (weight < 600) {
			perc = 0.54;			/* 500 */
		} else if (weight < 700) {
			perc = 0.51;			/* 600 */
		} else if (weight < 800) {
			perc = 0.46;			/* 700 */
		} else if (weight < 900) {
			perc = 0.42;			/* 800 */
		} else {
			perc = 0.34;			/* 900 */
		}
		s = (r_p * 0.40) + (g_p * 0.20) + (b_p * 0.40);
		gain = perc / s;		
		rd = (uint16_t)(r * gain);
		gd = (uint16_t)(g * gain);
		bd = (uint16_t)(b * gain);
		return psy_ui_colour_make_rgb(
			(uint8_t)(rd & 0xFF),
			(uint8_t)(gd & 0xFF),
			(uint8_t)(bd & 0xFF));
	}
	return *self;
}

/* standard colours */

INLINE psy_ui_Colour psy_ui_colour_white(void)
{
	return psy_ui_colour_make_argb(0x00FFFFFF);
}

INLINE psy_ui_Colour psy_ui_colour_black(void)
{
	return psy_ui_colour_make_argb(0x00000000);
}

INLINE psy_ui_Colour psy_ui_colour_red(void)
{
	return psy_ui_colour_make_argb(0x00FF0000);
}

INLINE psy_ui_Colour psy_ui_colour_green(void)
{
	return psy_ui_colour_make_argb(0x0000FF00);
}

INLINE psy_ui_Colour psy_ui_colour_blue(void)
{
	return psy_ui_colour_make_argb(0x000000FF);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COLOUR_H */
