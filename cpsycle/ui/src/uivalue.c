// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uivalue.h"

#include <math.h>

void psy_ui_value_init(psy_ui_Value* self)
{
	*self = psy_ui_value_makepx(0.0);
}

void psy_ui_value_add(psy_ui_Value* self, const psy_ui_Value* other,
	const psy_ui_TextMetric* tm)
{	
	if ((self->unit == psy_ui_UNIT_PX && other->unit == psy_ui_UNIT_PX)) {
		self->quantity += other->quantity;
	} else if ((self->unit == psy_ui_UNIT_EH && other->unit == psy_ui_UNIT_EH) ||
		(self->unit == psy_ui_UNIT_EW && other->unit == psy_ui_UNIT_EW) ||
		(self->unit == psy_ui_UNIT_EW && other->unit == psy_ui_UNIT_EH) ||
		(self->unit == psy_ui_UNIT_EH && other->unit == psy_ui_UNIT_EW)) {		
		self->quantity += other->quantity;
	} else {
		self->quantity = psy_ui_value_px(self, tm) +
			psy_ui_value_px(other, tm);
		self->unit = psy_ui_UNIT_PX;	
	}
}

void psy_ui_value_sub(psy_ui_Value* self, const psy_ui_Value* other,
	const psy_ui_TextMetric* tm)
{
	if ((self->unit == psy_ui_UNIT_EH && other->unit == psy_ui_UNIT_EH) ||
		(self->unit == psy_ui_UNIT_EW && other->unit == psy_ui_UNIT_EW)) {
		self->quantity -= other->quantity;
	} else {
		self->quantity = psy_ui_value_px(self, tm) -
			psy_ui_value_px(other, tm);
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
	const psy_ui_TextMetric* tm)
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
