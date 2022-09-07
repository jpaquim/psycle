/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* local */
#include "uivalue.h"
#include "uigeometry.h"
/* platform */
#include "../../detail/portable.h"
/* std */
#include <ctype.h>
#include <math.h>


/* psy_ui_TextMetric */

/* implementation */
void psy_ui_textmetric_init(psy_ui_TextMetric* self)
{
	assert(self);
	
	self->tmHeight = 0;
	self->tmAscent = 0;
	self->tmDescent = 0;
	self->tmInternalLeading = 0;
	self->tmExternalLeading = 0;
	self->tmAveCharWidth = 0;
	self->tmMaxCharWidth = 0;
	self->tmWeight = 0;
	self->tmOverhang = 0;
	self->tmDigitizedAspectX = 0;
	self->tmDigitizedAspectY = 0;
	self->tmFirstChar = 0;
	self->tmLastChar = 0;
	self->tmDefaultChar = 0;
	self->tmBreakChar = 0;
	self->tmItalic = 0;
	self->tmUnderlined = 0;
	self->tmStruckOut = 0;
	self->tmPitchAndFamily = 0;
	self->tmCharSet = 0;
}


/* psy_ui_Value */

enum {	
	STATE_NUMBER = 1,
	STATE_DECIMAL = 2,
	STATE_SPACE = 3
};

/* implementation */

void psy_ui_value_init(psy_ui_Value* self)
{
	assert(self);

	*self = psy_ui_value_make_px(0.0);
	self->set = TRUE;
}

psy_ui_Value psy_ui_value_make_string(const char* str)
{
	psy_ui_Value rv;
	char* s;
	char* p;
	char* q;
	char* unit;	
	int state;
	
	psy_ui_value_init(&rv);
	if (!str) {
		return rv;
	}
	s = psy_strdup(str);
	p = s;
	state = STATE_NUMBER;	
	
	while ((*p) != '\0' && (*p) == ' ') ++p;
	q = p;	
	while ((*q) != '\0') {
		if (state == STATE_NUMBER) {
			if (isdigit(*q)) {	
				state == STATE_NUMBER;
			} else if ((*q) == '.') {
				state == STATE_DECIMAL;
			} else if((*q) == ' ') {
				state = STATE_SPACE;
			} else {
				break;
			}
		} else if (state == STATE_DECIMAL) {
			if (isdigit(*q)) {				
				state == STATE_DECIMAL;
			} else if((*q) == ' ') {
				state = STATE_SPACE;
			} else {
				break;
			}
		} else if (state == STATE_SPACE) {		
			if ((*q) == ' ') {
				state = STATE_SPACE;
			} else {
				break;
			}
		}
		++q;
	}
	unit = psy_strdup(q);
	*q = '\0';
	rv.quantity = atof(s);
	if (strcmp(unit, "eh") == 0) {
		rv.unit = psy_ui_UNIT_EH;
	} else if (strcmp(unit, "ew") == 0) {
		rv.unit = psy_ui_UNIT_EW;
	} else if (strcmp(unit, "px") == 0) {
		rv.unit = psy_ui_UNIT_PX;
	} else if (strcmp(unit, "pw") == 0) {
		rv.unit = psy_ui_UNIT_PW;
	} else if (strcmp(unit, "pw") == 0) {
		rv.unit = psy_ui_UNIT_PH;
	}
	free(unit);
	free(s);
	return rv;	
}



void psy_ui_value_add(psy_ui_Value* self, const psy_ui_Value* other,
	const psy_ui_TextMetric* tm, const psy_ui_Size* pesize)
{	
	if (psy_ui_value_iszero(other)) {
		return;
	}
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
	if (psy_ui_value_iszero(other)) {
		return;
	}
	if ((self->unit == psy_ui_UNIT_EH && other->unit == psy_ui_UNIT_EH) ||
		(self->unit == psy_ui_UNIT_EW && other->unit == psy_ui_UNIT_EW)) {
		self->quantity -= other->quantity;
	} else {
		self->quantity = psy_ui_value_px(self, tm, pesize) -
			psy_ui_value_px(other, tm, pesize);
		self->unit = psy_ui_UNIT_PX;
	}
}

void psy_ui_value_mul(psy_ui_Value* self, const psy_ui_Value* other,
	const psy_ui_TextMetric* tm, const psy_ui_Size* pesize)
{
	if ((self->unit == psy_ui_UNIT_EH && other->unit == psy_ui_UNIT_EH) ||
		(self->unit == psy_ui_UNIT_EW && other->unit == psy_ui_UNIT_EW)) {
		self->quantity *= other->quantity;
	} else {
		self->quantity = psy_ui_value_px(self, tm, pesize) *
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

double psy_ui_value_ew(const psy_ui_Value* self,
	const psy_ui_TextMetric* tm, const struct psy_ui_Size* size)
{
	double rv;

	if (tm) {
		switch (self->unit) {
		case psy_ui_UNIT_EW:
			rv = self->quantity;
			break;
		case psy_ui_UNIT_EH:
			rv = (self->quantity * tm->tmHeight) / tm->tmAveCharWidth;
			break;
		case psy_ui_UNIT_PW:
			if (size) {
				rv = (self->quantity * psy_ui_value_px(&size->width, tm, NULL)) / tm->tmAveCharWidth;
			} else {
				rv = self->quantity / tm->tmAveCharWidth;
			}
			break;
		case psy_ui_UNIT_PH:
			if (size) {
				rv = self->quantity * psy_ui_value_px(&size->height, tm, NULL) / tm->tmAveCharWidth;
			} else {
				rv = self->quantity / tm->tmAveCharWidth;
			}
			break;
		default:
			rv = self->quantity / tm->tmAveCharWidth;
			break;
		}
	} else {
		rv = self->quantity;
	}	
	return rv;
}

char* psy_ui_value_to_string(psy_ui_Value* self, char* rv)
{
	assert(self);
	
	if (!rv) {
		return rv;
	}
	switch (self->unit) {
	case psy_ui_UNIT_EH:
		psy_snprintf(rv, 64, "%f eh", self->quantity);
		break;
	case psy_ui_UNIT_EW:
		psy_snprintf(rv, 64, "%f ew", self->quantity);
		break;
	case psy_ui_UNIT_PX:
		psy_snprintf(rv, 64, "%f px", self->quantity);
		break;
	case psy_ui_UNIT_PW:
		psy_snprintf(rv, 64, "%f pw", self->quantity);
		break;
	case psy_ui_UNIT_PH:
		psy_snprintf(rv, 64, "%f ph", self->quantity);
		break;
	default:
		psy_snprintf(rv, 64, "%f", self->quantity);
		break;
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

psy_ui_Value psy_ui_mul_values(psy_ui_Value lhs, psy_ui_Value rhs,
	const psy_ui_TextMetric* tm, const psy_ui_Size* pesize)
{
	psy_ui_Value rv;

	rv = lhs;
	psy_ui_value_mul(&rv, &rhs, tm, pesize);
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

psy_ui_Value psy_ui_min_values(psy_ui_Value lhs, psy_ui_Value rhs,
	const psy_ui_TextMetric* tm, const psy_ui_Size* pesize)
{
	if (psy_ui_value_comp(&lhs, &rhs, tm, pesize) < 0) {
		return lhs;
	}
	return rhs;
}
