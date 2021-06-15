/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* local */
#include "uivalue.h"
#include "uigeometry.h"
/* std */
#include <assert.h>
#include <math.h>

void psy_ui_value_init(psy_ui_Value* self)
{
	assert(self);

	*self = psy_ui_value_make_px(0.0);
}

void psy_ui_value_add(psy_ui_Value* self, const psy_ui_Value* other,
	const psy_ui_TextMetric* tm, const psy_ui_Size* pesize)
{	
	if ((self->unit == psy_ui_UNIT_PX && other->unit == psy_ui_UNIT_PX)) {
		self->quantity += other->quantity;
	} else if ((self->unit == psy_ui_UNIT_EH && other->unit == psy_ui_UNIT_EH) ||
		(self->unit == psy_ui_UNIT_EW && other->unit == psy_ui_UNIT_EW) ||
		(self->unit == psy_ui_UNIT_EW && other->unit == psy_ui_UNIT_EH) ||
		(self->unit == psy_ui_UNIT_EH && other->unit == psy_ui_UNIT_EW)) {		
		self->quantity += other->quantity;
	} else {
		self->quantity = psy_ui_value_px(self, tm, pesize) +
			psy_ui_value_px(other, tm, pesize);
		self->unit = psy_ui_UNIT_PX;	
	}
}

void psy_ui_value_sub(psy_ui_Value* self, const psy_ui_Value* other,
	const psy_ui_TextMetric* tm, const psy_ui_Size* pesize)
{
	if ((self->unit == psy_ui_UNIT_EH && other->unit == psy_ui_UNIT_EH) ||
		(self->unit == psy_ui_UNIT_EW && other->unit == psy_ui_UNIT_EW)) {
		self->quantity -= other->quantity;
	} else {
		self->quantity = psy_ui_value_px(self, tm, pesize) -
			psy_ui_value_px(other, tm, pesize);
		self->unit = psy_ui_UNIT_PX;
	}
}

void psy_ui_value_mul_real(psy_ui_Value* self, double factor)
{
	if ((self->unit == psy_ui_UNIT_EH) ||
		(self->unit == psy_ui_UNIT_EW)) {
		self->quantity *= factor;
	} else {
		self->quantity *= factor;
		self->unit = psy_ui_UNIT_PX;
	}
}

int psy_ui_value_comp(psy_ui_Value* self, const psy_ui_Value* other,
	const psy_ui_TextMetric* tm, const psy_ui_Size* pesize)
{
	if ((self->unit == psy_ui_UNIT_EH && other->unit == psy_ui_UNIT_EH) ||
		(self->unit == psy_ui_UNIT_EW && other->unit == psy_ui_UNIT_EW)) {
		double diff;
		
		diff = self->quantity - other->quantity;
		if (diff == 0.0) {
			return 0;
		}
		if (diff < 0.0) {
			return -1;
		}
		return 1;
	} else if ((self->unit == psy_ui_UNIT_PX && other->unit == psy_ui_UNIT_PX) ||
			(self->unit == psy_ui_UNIT_PX && other->unit == psy_ui_UNIT_PX)) {
		double diff;

		diff = self->quantity - other->quantity;
		if (diff == 0) {
			return 0;
		}
		if (diff < 0) {
			return -1;
		}
		return 1;
	} else {
		double diff;

		diff = psy_ui_value_px(self, tm, pesize)  - psy_ui_value_px(other, tm, pesize);
		if (diff == 0) {
			return 0;
		}
		if (diff < 0) {
			return -1;
		}
		return 1;
	}
}

double psy_ui_value_px(const psy_ui_Value* self,
	const psy_ui_TextMetric* tm, const struct psy_ui_Size* size)
{
	double rv;
	
	if (tm) {
		switch (self->unit) {
		case psy_ui_UNIT_EW:
			rv = self->quantity * tm->tmAveCharWidth;
			break;
		case psy_ui_UNIT_EH:
			rv = self->quantity * tm->tmHeight;
			break;
		case psy_ui_UNIT_PW:
			if (size) {
				rv = self->quantity * psy_ui_value_px(&size->width, tm, NULL);
			} else {
				rv = self->quantity;
			}
			break;
		case psy_ui_UNIT_PH:
			if (size) {
				rv = self->quantity * psy_ui_value_px(&size->height, tm, NULL);
			} else {
				rv = self->quantity;
			}
			break;
		default:
			rv = self->quantity;
			break;
		}
	} else {
		rv = self->quantity;
	}
	if (self->round == psy_ui_ROUND_NONE) {
		return rv;
	} else if (self->round == psy_ui_ROUND_FLOOR) {
		return floor(rv);
	} else if (self->round == psy_ui_ROUND_CEIL) {
		return ceil(rv);
	}
	return rv;
}

psy_ui_Value psy_ui_add_values(psy_ui_Value lhs, psy_ui_Value rhs,
	const psy_ui_TextMetric* tm, const psy_ui_Size* pesize)
{
	psy_ui_Value rv;

	rv = lhs;
	psy_ui_value_add(&rv, &rhs, tm, pesize);
	return rv;
}

psy_ui_Value psy_ui_sub_values(psy_ui_Value lhs, psy_ui_Value rhs,
	const psy_ui_TextMetric* tm, const struct psy_ui_Size* pesize)
{
	psy_ui_Value rv;

	rv = lhs;
	psy_ui_value_sub(&rv, &rhs, tm, pesize);
	return rv;
}

psy_ui_Value psy_ui_max_values(psy_ui_Value lhs, psy_ui_Value rhs,
	const psy_ui_TextMetric* tm, const psy_ui_Size* pesize)
{
	if (psy_ui_value_comp(&lhs, &rhs, tm, pesize) > 0) {
		return lhs;
	}
	return rhs;
}