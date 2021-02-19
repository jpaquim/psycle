// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternevent.h"

void psy_audio_parameterrange_init(psy_audio_ParameterRange* self, 
	uintptr_t low, uintptr_t high, uintptr_t min, uintptr_t max)
{
	self->low = low;
	self->high = high;
	self->min = min;
	self->max = high;
}

int psy_audio_parameterrange_intersect(psy_audio_ParameterRange* self, uintptr_t value)
{
	return value >= self->low && value <= self->high;
}

void psy_audio_patternevent_init_all(psy_audio_PatternEvent* self,
	uint8_t note,
	uint16_t inst,
	uint8_t mach,
	uint8_t vol,
	uint8_t cmd,
	uint8_t parameter)
{
	self->note = note;
	self->inst = inst;
	self->mach = mach;
	self->vol = vol;
	self->cmd = cmd;
	self->parameter = parameter;		
}

void psy_audio_patternevent_clear(psy_audio_PatternEvent* self)
{
	self->note = psy_audio_NOTECOMMANDS_EMPTY;
	self->inst = psy_audio_NOTECOMMANDS_INST_EMPTY;
	self->mach = psy_audio_NOTECOMMANDS_psy_audio_EMPTY;
	self->vol = psy_audio_NOTECOMMANDS_VOL_EMPTY;
	self->cmd = 0;
	self->parameter = 0;
}

int psy_audio_patternevent_empty(const psy_audio_PatternEvent* self)
{	
	return
		self->note == psy_audio_NOTECOMMANDS_EMPTY &&
		self->inst == psy_audio_NOTECOMMANDS_INST_EMPTY &&
		self->mach == psy_audio_NOTECOMMANDS_psy_audio_EMPTY &&
		self->vol == psy_audio_NOTECOMMANDS_VOL_EMPTY &&
		self->cmd == 0 &&		
		self->parameter == 0;		
}

int psy_audio_patternevent_empty_except_volume(const psy_audio_PatternEvent* self)
{
	return
		self->note == psy_audio_NOTECOMMANDS_EMPTY &&
		self->inst == psy_audio_NOTECOMMANDS_INST_EMPTY &&
		self->mach == psy_audio_NOTECOMMANDS_psy_audio_EMPTY &&		
		self->cmd == 0 &&		
		self->parameter == 0;
}

int psy_audio_patternevent_has_volume(const psy_audio_PatternEvent* self)
{
	return self->vol != psy_audio_NOTECOMMANDS_VOL_EMPTY;
}
