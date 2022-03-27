/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_VALUE_H
#define psy_ui_VALUE_H

#include "../../detail/psydef.h"

/* std */
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_TextMetric
{
	int32_t tmHeight;
	int32_t tmAscent;
	int32_t tmDescent;
	int32_t tmInternalLeading;
	int32_t tmExternalLeading;
	int32_t tmAveCharWidth;
	int32_t tmMaxCharWidth;
	int32_t tmWeight;
	int32_t tmOverhang;
	int32_t tmDigitizedAspectX;
	int32_t tmDigitizedAspectY;
	uint8_t tmFirstChar;
	uint8_t tmLastChar;
	uint8_t tmDefaultChar;
	uint8_t tmBreakChar;
	uint8_t tmItalic;
	uint8_t tmUnderlined;
	uint8_t tmStruckOut;
	uint8_t tmPitchAndFamily;
	uint8_t tmCharSet;
} psy_ui_TextMetric;

typedef enum {
	psy_ui_ROUND_NONE,
	psy_ui_ROUND,
	psy_ui_ROUND_FLOOR,
	psy_ui_ROUND_CEIL,	
} psy_ui_Round;

typedef enum {
	psy_ui_UNIT_EH,
	psy_ui_UNIT_EW,
	psy_ui_UNIT_PX,
	psy_ui_UNIT_PW,
	psy_ui_UNIT_PH
} psy_ui_Unit;

struct psy_ui_Size;

typedef struct psy_ui_Value {
	double quantity;
	psy_ui_Unit unit;
	psy_ui_Round round;
	bool set;
} psy_ui_Value;

void psy_ui_value_init(psy_ui_Value*);

INLINE void psy_ui_value_setroundmode(psy_ui_Value* self, psy_ui_Round round)
{
	self->round = round;
}

INLINE psy_ui_Round psy_ui_value_roundmode(const psy_ui_Value* self)
{
	return self->round;
}

INLINE void psy_ui_value_setpx(psy_ui_Value* self, double value)
{
	self->unit = psy_ui_UNIT_PX;
	self->quantity = value;
}

INLINE void psy_ui_value_setew(psy_ui_Value* self, double value)
{
	self->unit = psy_ui_UNIT_EW;
	self->quantity = value;	
}

INLINE void psy_ui_value_seteh(psy_ui_Value* self, double value)
{
	self->unit = psy_ui_UNIT_EH;
	self->quantity = value;	
}

INLINE void psy_ui_value_setpw(psy_ui_Value* self, double value)
{
	self->unit = psy_ui_UNIT_PW;
	self->quantity = value;
}

INLINE void psy_ui_value_setph(psy_ui_Value* self, double value)
{
	self->unit = psy_ui_UNIT_PH;
	self->quantity = value;
}

INLINE psy_ui_Value psy_ui_value_make_px(double px)
{
	psy_ui_Value rv;

	rv.quantity = px;
	rv.unit = psy_ui_UNIT_PX;
	rv.round = psy_ui_ROUND_FLOOR;
	rv.set = TRUE;
	return rv;
}

INLINE psy_ui_Value psy_ui_value_make_ew(double em)
{	
	psy_ui_Value rv;

	rv.quantity = em;
	rv.unit = psy_ui_UNIT_EW;
	rv.round = psy_ui_ROUND_FLOOR;
	rv.set = TRUE;
	return rv;
}

INLINE psy_ui_Value psy_ui_value_make_eh(double em)
{
	psy_ui_Value rv;

	rv.quantity = em;
	rv.unit = psy_ui_UNIT_EH;
	rv.round = psy_ui_ROUND_FLOOR;
	rv.set = TRUE;
	return rv;
}

INLINE psy_ui_Value psy_ui_value_make_pw(double pw)
{
	psy_ui_Value rv;

	rv.quantity = pw;
	rv.unit = psy_ui_UNIT_PW;
	rv.round = psy_ui_ROUND_FLOOR;
	rv.set = TRUE;
	return rv;
}

INLINE psy_ui_Value psy_ui_value_make_ph(double ph)
{
	psy_ui_Value rv;

	rv.quantity = ph;
	rv.unit = psy_ui_UNIT_PH;
	rv.round = psy_ui_ROUND_FLOOR;
	rv.set = TRUE;
	return rv;
}

double psy_ui_value_px(const psy_ui_Value*, const psy_ui_TextMetric*,
	const struct psy_ui_Size*);
double psy_ui_value_ew(const psy_ui_Value*,
	const psy_ui_TextMetric*, const struct psy_ui_Size*);

void psy_ui_value_add(psy_ui_Value*, const psy_ui_Value* other,
	const psy_ui_TextMetric*, const struct psy_ui_Size* pesize);
void psy_ui_value_sub(psy_ui_Value*, const psy_ui_Value* other,
	const psy_ui_TextMetric*, const struct psy_ui_Size* pesize);
void psy_ui_value_mul_real(psy_ui_Value*, double factor);
int psy_ui_value_comp(psy_ui_Value* self, const psy_ui_Value* other,
	const psy_ui_TextMetric* tm, const struct psy_ui_Size* pesize);
psy_ui_Value psy_ui_add_values(psy_ui_Value lhs, psy_ui_Value rhs,
	const psy_ui_TextMetric* tm, const struct psy_ui_Size* pesize);
psy_ui_Value psy_ui_sub_values(psy_ui_Value lhs, psy_ui_Value rhs,
	const psy_ui_TextMetric* tm, const struct psy_ui_Size* pesize);

INLINE psy_ui_Value psy_ui_mul_value_real(psy_ui_Value lhs, double factor)
{
	psy_ui_Value rv;

	rv = lhs;
	psy_ui_value_mul_real(&rv, factor);
	return rv;
}

psy_ui_Value psy_ui_max_values(psy_ui_Value lhs, psy_ui_Value rhs,
	const psy_ui_TextMetric* tm, const struct psy_ui_Size* pesize);

INLINE psy_ui_Value psy_ui_value_zero(void)
{
	return psy_ui_value_make_px(0);
}

INLINE bool psy_ui_value_iszero(const psy_ui_Value* self)
{
	return self->quantity == 0.0;
}

INLINE bool psy_ui_isvaluezero(psy_ui_Value value)
{
	return psy_ui_value_iszero(&value);
}


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_VALUE_H */
