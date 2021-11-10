/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternhostcmds.h"
/* host*/
#include "patterncmds.h"
/* audio */
#include <exclusivelock.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

/* implementation */
void patterncmds_init(PatternCmds* self, psy_audio_Song* song,
	psy_UndoRedo* undoredo, psy_audio_Pattern* patternpaste)
{
	assert(self);
	assert(undoredo);	
	
	self->pattern = NULL;
	self->song = song;
	self->undoredo = undoredo;
	self->patternpaste = patternpaste;
}

void patterncmds_setsong(PatternCmds* self, psy_audio_Song* song)
{
	self->song = song;
}

void patterncmds_setpattern(PatternCmds* self, psy_audio_Pattern* pattern)
{	
	self->pattern = pattern;
}

void patterncmds_blocktranspose(PatternCmds* self,
	psy_audio_BlockSelection selection,
	psy_audio_SequenceCursor cursor, intptr_t offset)
{	
	if (self->pattern && psy_audio_blockselection_valid(&selection)) {
		psy_undoredo_execute(self->undoredo,
			&blocktransposecommand_alloc(self->pattern,
				selection, cursor, offset, self->song)->command);
	}
}

void patterncmds_blockdelete(PatternCmds* self, psy_audio_BlockSelection
	selection)
{
	assert(self);

	if (self->pattern && psy_audio_blockselection_valid(&selection)) {
		psy_undoredo_execute(self->undoredo,
			&blockremovecommand_alloc(self->pattern,
				selection, self->song)->command);
	}
}

void patterncmds_blockcopy(PatternCmds* self, psy_audio_BlockSelection selection)
{
	assert(self);

	if (self->pattern && psy_audio_blockselection_valid(&selection)) {
		psy_audio_pattern_blockcopy(self->patternpaste,
			self->pattern, selection);
	}
}


void patterncmds_blockpaste(PatternCmds* self, psy_audio_SequenceCursor cursor,
	bool mix)
{
	assert(self);

	if (self->pattern && !psy_audio_pattern_empty(
			self->patternpaste)) {
		psy_undoredo_execute(self->undoredo,
			&blockpastecommand_alloc(self->pattern,
				self->patternpaste, cursor,
				1.0 / (double)cursor.lpb, mix)->command);		
	}
}

void patterncmds_changeinstrument(PatternCmds* self,
	psy_audio_BlockSelection selection)
{
	assert(self);

	if (self->pattern && self->song) {
		psy_audio_pattern_changeinstrument(self->pattern,
			selection.topleft, selection.bottomright,
			psy_audio_instruments_selected(&self->song->instruments).subslot);
	}
}

void patterncmds_changemachine(PatternCmds* self,
	psy_audio_BlockSelection selection)
{
	assert(self);

	if (self->pattern && self->song) {
		psy_audio_pattern_changemachine(self->pattern,
			selection.topleft, selection.bottomright,
			psy_audio_machines_selected(&self->song->machines));
	}
}
