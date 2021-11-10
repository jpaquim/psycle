/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternviewstate.h"
/* audio */
#include <pattern.h>
/* std */
#include <math.h>
#include <assert.h>
/* platform */
#include "../../detail/trace.h"
#include "../../detail/portable.h"

/* implementation */
void patternviewstate_init(PatternViewState* self, psy_audio_Song* song, PatternCmds* cmds)
{
	psy_audio_sequencecursor_init(&self->cursor);
	psy_audio_blockselection_init(&self->selection);	
	self->pgupdownstepmode = PATTERNCURSOR_STEP_BEAT;
	self->pgupdownstep = 4;	
	self->singlemode = TRUE;
	self->song = song;
	self->pattern = NULL;
	self->skin = NULL;		
	self->display = PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER;
	self->cmds = cmds;
	self->movecursorwhenpaste = TRUE;
	psy_audio_pattern_init(&self->patternpaste);
}

void patternviewstate_dispose(PatternViewState* self)
{	
	psy_audio_pattern_dispose(&self->patternpaste);
}

void patternviewstate_selectcol(PatternViewState* self)
{
	assert(self);

	if (patternviewstate_pattern(self)) {
		psy_audio_blockselection_select(&self->selection,
			self->cursor.track, 1, 0.0,
			psy_audio_pattern_length(patternviewstate_pattern(self)));
	}
}

void patternviewstate_selectbar(PatternViewState* self)
{
	assert(self);

	if (patternviewstate_pattern(self)) {
		psy_audio_blockselection_select(&self->selection,
			self->cursor.track, 1,
			self->cursor.offset,
			4.0);
		if (self->cursor.offset > patternviewstate_pattern(self)->length) {
			self->cursor.offset = patternviewstate_pattern(self)->length;
		}
	}
}

void patternviewstate_selectall(PatternViewState* self)
{
	assert(self);

	if (patternviewstate_pattern(self)) {
		psy_audio_blockselection_init(&self->selection);
		self->selection.topleft.key = psy_audio_NOTECOMMANDS_B9;
		psy_audio_blockselection_select(&self->selection,
			0, patternviewstate_numsongtracks(self),
			0.0, psy_audio_pattern_length(
				patternviewstate_pattern(self)));
	}
}

