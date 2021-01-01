// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

// prefix file for each .c file
#include "../../detail/prefix.h"

#include "trackerlinestate.h"

// audio
#include <pattern.h>
#include <patternio.h>
#include "../../driver/eventdriver.h"
// std
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
// platform
#include "../../detail/trace.h"
#include "../../detail/portable.h"

static int testcursor(psy_audio_PatternCursor cursor, uintptr_t track,
	psy_dsp_big_beat_t offset, uintptr_t lpb)
{
	return cursor.track == track && psy_dsp_testrange(cursor.offset, offset, 1.0 / lpb);
}

// TrackerLineState
// implementation
void trackerlinestate_init(TrackerLineState* self)
{
	assert(self);

	self->defaultlineheight = psy_ui_value_makeeh(1.0);
	self->lineheight = self->defaultlineheight;
	self->lineheightpx = 13;
	self->flatsize = 8;
	self->lpb = 4;
	self->bpl = 1.0 / self->lpb;
	self->skin = NULL;
	self->pattern = NULL;	
	self->lastplayposition = -1.f;
	self->sequenceentryoffset = 0.f;
	self->drawcursor = TRUE;
	self->visilines = 25;
	self->cursorchanging = FALSE;
	self->gridfont = NULL;
}

void trackerlinestate_dispose(TrackerLineState* self)
{
	assert(self);
}

int trackerlinestate_beattoline(const TrackerLineState* self,
	psy_dsp_big_beat_t offset)
{
	assert(self);

	return cast_decimal(offset * self->lpb);	
}

bool trackerlinestate_testplaybar(TrackerLineState* self, psy_dsp_big_beat_t offset)
{
	assert(self);

	return psy_dsp_testrange(self->lastplayposition - self->sequenceentryoffset,
		offset, self->bpl);
}

uintptr_t trackerlinestate_numlines(const TrackerLineState* self)
{
	assert(self);

	if (self->pattern) {
		return trackerlinestate_beattoline(self, psy_audio_pattern_length(
			self->pattern));
	}
	return 0;
}
