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
void patterncmds_init(PatternCmds* self, Workspace* workspace)
{
	assert(workspace);

	self->workspace = workspace;
	self->pattern = NULL;	
}

void patterncmds_setpattern(PatternCmds* self, psy_audio_Pattern* pattern)
{	
	self->pattern = pattern;
}

void patterncmds_blocktranspose(PatternCmds* self,
	psy_audio_BlockSelection selection,
	psy_audio_SequenceCursor cursor, int offset)
{	
	if (self->pattern && psy_audio_blockselection_valid(&selection)) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&blocktransposecommand_alloc(self->pattern,
				selection, cursor, offset, self->workspace)->command);
	}
}

void patterncmds_blockdelete(PatternCmds* self,
	psy_audio_BlockSelection selection)
{
	assert(self);

	if (self->pattern && psy_audio_blockselection_valid(&selection)) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&blockremovecommand_alloc(self->pattern,
				selection,
				self->workspace)->command);		
	}
}

void patterncmds_blockcopy(PatternCmds* self, psy_audio_BlockSelection selection)
{
	assert(self);

	if (self->pattern && psy_audio_blockselection_valid(&selection)) {
		psy_audio_pattern_blockcopy(&self->workspace->patternpaste,
			self->pattern, selection);
	}
}


void patterncmds_blockpaste(PatternCmds* self, psy_audio_SequenceCursor cursor,
	bool mix)
{
	assert(self);

	if (self->pattern && !psy_audio_pattern_empty(
			&self->workspace->patternpaste)) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&blockpastecommand_alloc(self->pattern,
				&self->workspace->patternpaste, cursor,
				1.0 / (double)cursor.lpb, mix, self->workspace)->command);		
	}
}

void patterncmds_changeinstrument(PatternCmds* self,
	psy_audio_BlockSelection selection)
{
	assert(self);

	if (self->pattern && workspace_song(self->workspace)) {
		psy_audio_pattern_changeinstrument(self->pattern,
			selection.topleft, selection.bottomright,
			psy_audio_instruments_selected(&workspace_song(
				self->workspace)->instruments).subslot);
	}
}

void patterncmds_changemachine(PatternCmds* self,
	psy_audio_BlockSelection selection)
{
	assert(self);

	if (self->pattern && workspace_song(self->workspace)) {
		psy_audio_pattern_changemachine(self->pattern,
			selection.topleft, selection.bottomright,
			psy_audio_machines_selected(&workspace_song(
				self->workspace)->machines));
	}
}
