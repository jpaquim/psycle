/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternviewstate.h"
/* host */
#include "styles.h"
/* audio */
#include <pattern.h>
/* std */
#include <math.h>
#include <assert.h>
/* platform */
#include "../../detail/trace.h"
#include "../../detail/portable.h"


/* BeatLine */

/* implementation */
void beatline_init(BeatLine* self, uintptr_t lpb)
{	
	assert(self);
	
	self->lpb = 0;
	beatline_set_lpb(self, lpb);	
}

/* BeatConvert */

/* implementation */
void beatconvert_init(BeatConvert* self, BeatLine* beat_line, double line_px)
{
	assert(self);
	
	self->beat_line = beat_line;	
	beatconvert_set_line_px(self, line_px);
}

/* implementation */
void patternviewstate_init(PatternViewState* self,
	PatternViewConfig* patconfig, KeyboardMiscConfig* keymiscconfig,
	psy_audio_Sequence* sequence, PatternCmds* cmds)
{
	psy_audio_sequencecursor_init(&self->cursor);
	psy_audio_blockselection_init(&self->selection);
	beatline_init(&self->beat_line, self->cursor.lpb);	
	self->sequence = sequence;	
	self->patconfig = patconfig;
	self->keymiscconfig = keymiscconfig;	
	self->cmds = cmds;
	self->chord = FALSE;
	self->chord_begin = 0;
	psy_audio_pattern_init(&self->patternpaste);	
	if (cmds) {
		cmds->patternpaste = &self->patternpaste;
	}
}

void patternviewstate_dispose(PatternViewState* self)
{	
	psy_audio_pattern_dispose(&self->patternpaste);
}

void patternviewstate_sync_cursor_to_sequence(PatternViewState* self)
{
	if (patternviewstate_sequence(self)) {
		patternviewstate_set_cursor(self,
			patternviewstate_sequence(self)->cursor);
	}
}
