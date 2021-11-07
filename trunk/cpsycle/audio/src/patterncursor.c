/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patterncursor.h"
/* local */
#include "patternevent.h"
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"

/* psy_audio_PatternCursor */
/* implementation */
void psy_audio_patterncursor_init(psy_audio_PatternCursor* self)
{
	assert(self);
	self->key = psy_audio_NOTECOMMANDS_MIDDLEC;
	self->track = 0;
	self->offset = 0.0;
	self->absolute = FALSE;	
	self->lpb = 4;
	self->column = 0;
	self->digit = 0;
	self->patternid = 0;
}

psy_audio_PatternCursor psy_audio_patterncursor_make(
	uintptr_t track, psy_dsp_big_beat_t offset)
{
	psy_audio_PatternCursor rv;

	psy_audio_patterncursor_init(&rv);
	rv.track = track;
	rv.offset = offset;	
	return rv;
}

psy_audio_PatternCursor psy_audio_patterncursor_make_all(
	uintptr_t track, psy_dsp_big_beat_t offset, uint8_t key)
{
	psy_audio_PatternCursor rv;

	psy_audio_patterncursor_init(&rv);
	rv.track = track;
	rv.offset = offset;
	rv.key = key;
	return rv;
}

int psy_audio_patterncursor_equal(psy_audio_PatternCursor* lhs,
	psy_audio_PatternCursor* rhs)
{
	assert(lhs && rhs);
	return 
		rhs->column == lhs->column &&
		rhs->digit == lhs->digit &&
		rhs->track == lhs->track &&
		rhs->offset == lhs->offset &&
		rhs->patternid == lhs->patternid;
}
