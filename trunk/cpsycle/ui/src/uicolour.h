// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_COLOUR_H
#define psy_ui_COLOUR_H

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_Colour
{
	psy_ui_PropertyMode mode;
	uint32_t value;
} psy_ui_Colour;

INLINE void psy_ui_colour_init(psy_ui_Colour* self)
{
	self->mode.inherited = TRUE;
	self->mode.set = FALSE;
	self->value = 0x00000000;
}

INLINE void psy_ui_colour_init_rgb(psy_ui_Colour* self, uint8_t r, uint8_t g, uint8_t b)
{
	self->mode.inherited = TRUE;
	self->mode.set = TRUE;
	self->mode.inherited = TRUE;
	self->mode.set = TRUE;
	self->value = (uint32_t)(((uint16_t)r) | (((uint16_t)g) << 8) | (((uint16_t)b) << 16));
}

void psy_ui_colour_init_str(psy_ui_Colour* self, const char* str);

INLINE psy_ui_Colour psy_ui_colour_make(uint32_t value)
{
	psy_ui_Colour rv;

	rv.mode.inherited = TRUE;
	rv.mode.set = TRUE;
	rv.value = value;
	return rv;
}

INLINE psy_ui_Colour psy_ui_colour_make_argb(uint32_t value)
{
	psy_ui_Colour rv;

	rv.mode.inherited = TRUE;
	rv.mode.set = TRUE;	
	//0x00B6C5D1
	rv.value = ((((value) << 16) & 0xFF0000)
		| (((value) & 0xFF00)) | (((value >> 16) & 0xFF)));
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
	self->mode.inherited = TRUE;
	self->mode.set = TRUE;
	self->value = colour.value;
}

INLINE psy_ui_Colour psy_ui_colour_make_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	psy_ui_Colour rv;

	rv.mode.inherited = TRUE;
	rv.mode.set = TRUE;
	rv.value = (uint32_t)(((uint16_t)r) | (((uint16_t)g) << 8) | (((uint16_t)b) << 16));
	return rv;
}

INLINE void psy_ui_colour_rgb(const psy_ui_Colour* self,
	uint8_t* r, uint8_t* g, uint8_t* b)
{
	uint32_t temp;
		
	temp = (self->value & 0xFF);
	*r = (uint8_t) temp;	
	temp = ((self->value >> 8) & 0xFF);
	*g = (uint8_t) temp;	
	temp = ((self->value >> 16) & 0xFF);
	*b = (uint8_t) temp;
}

psy_ui_Colour* psy_ui_colour_add_rgb(psy_ui_Colour* self, float r, float g, float b);
psy_ui_Colour* psy_ui_colour_mul_rgb(psy_ui_Colour* self, float r, float g, float b);
psy_ui_Colour psy_ui_diffadd_colours(psy_ui_Colour base, psy_ui_Colour adjust,
	psy_ui_Colour add);

INLINE bool psy_ui_equal_colours(const psy_ui_Colour* lhs, const psy_ui_Colour* rhs)
{
	return lhs->mode.set && rhs->mode.set && lhs->value == rhs->value;
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


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COLOUR_H */
