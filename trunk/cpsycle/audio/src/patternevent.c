/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

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

uintptr_t psy_audio_patternevent_value(const psy_audio_PatternEvent* self,
	psy_audio_PatternEventColumn column)
{	
	switch (column) {
	case PATTERNEVENT_COLUMN_NOTE:
		return self->note;
	case PATTERNEVENT_COLUMN_INST:
		return self->inst;
	case PATTERNEVENT_COLUMN_MACH:
		return self->mach;
	case PATTERNEVENT_COLUMN_VOL:
		return self->vol;
	case PATTERNEVENT_COLUMN_CMD:
		return self->cmd;
	case PATTERNEVENT_COLUMN_PARAM:
		return self->parameter;
	default:
		return 0;
	}		
}

void psy_audio_patternevent_setvalue(psy_audio_PatternEvent* self,
	psy_audio_PatternEventColumn column, uintptr_t value)
{
	switch (column) {
	case PATTERNEVENT_COLUMN_NOTE:
		self->note = value;
		break;
	case PATTERNEVENT_COLUMN_INST:
		self->inst = value;
		break;
	case PATTERNEVENT_COLUMN_MACH:
		self->mach = value;
		break;
	case PATTERNEVENT_COLUMN_VOL:
		self->vol = value;
		break;
	case PATTERNEVENT_COLUMN_CMD:
		self->cmd = value;
		break;
	case PATTERNEVENT_COLUMN_PARAM:
		self->parameter = value;
		break;
	default:
		break;
	}
}