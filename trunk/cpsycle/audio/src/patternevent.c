// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternevent.h"

void parameterrange_init(ParameterRange* self, 
	uintptr_t low, uintptr_t high, uintptr_t min, uintptr_t max)
{
	self->low = low;
	self->high = high;
	self->min = min;
	self->max = high;
}

int parameterrange_intersect(ParameterRange* self, uintptr_t value)
{
	return value >= self->low && value <= self->high;
}

void patternevent_init(PatternEvent* self,
	unsigned char note,
	unsigned char inst,
	unsigned char mach,
	unsigned char cmd,
	unsigned char parameter)
{
	self->note = note;
	self->inst = inst;
	self->mach = mach;
	self->cmd = cmd;
	self->parameter = parameter;
}

void patternevent_clear(PatternEvent* self)
{
	self->note = 255;
	self->inst = 255;
	self->mach = 255;
	self->cmd = 0;
	self->parameter = 0;
}

int patternevent_empty(PatternEvent* self)
{
	return 
		self->note == 255 &&
		self->inst == 255 &&
		self->mach == 255 &&
		self->cmd == 0 &&
		self->parameter == 0;
}
