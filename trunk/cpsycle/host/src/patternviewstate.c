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


/* implementation */
void patternviewstate_init(PatternViewState* self,
	PatternViewConfig* patconfig, KeyboardMiscConfig* keymiscconfig,
	psy_audio_Sequence* sequence, PatternCmds* cmds)
{
	psy_audio_sequencecursor_init(&self->cursor);
	psy_audio_blockselection_init(&self->selection);	
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

void patternviewstate_select_col(PatternViewState* self)
{
	assert(self);

	if (patternviewstate_pattern(self)) {
		psy_audio_blockselection_select(&self->selection,
			self->cursor.track, 1, 0.0,
			psy_audio_pattern_length(patternviewstate_pattern(self)));
		patternviewstate_invalidate(self);
	}
}

void patternviewstate_select_bar(PatternViewState* self)
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
		patternviewstate_invalidate(self);
	}
}

void patternviewstate_select_all(PatternViewState* self)
{
	assert(self);

	if (patternviewstate_pattern(self)) {
		psy_audio_blockselection_init(&self->selection);
		self->selection.topleft.key = psy_audio_NOTECOMMANDS_B9;
		psy_audio_blockselection_select(&self->selection,
			0, patternviewstate_num_song_tracks(self),
			0.0, psy_audio_pattern_length(
				patternviewstate_pattern(self)));
		patternviewstate_invalidate(self);
	}
}

void patternviewstate_sequencestart(PatternViewState* self,
	double startoffset, psy_audio_SequenceTrackIterator* rv)
{
	double offset;
	double seqoffset;
	double length;	

	assert(self);
	
	rv->pattern = patternviewstate_pattern(self);
	rv->patternnode = NULL;
	rv->patterns = patternviewstate_patterns(self);
	seqoffset = 0.0;
	length = rv->pattern->length;
	offset = startoffset;	
	if (!patternviewstate_single_mode(self) &&
			patternviewstate_sequence(self)) {		
		psy_audio_Sequence* sequence;

		sequence = patternviewstate_sequence(self);		
		psy_audio_sequence_begin(sequence,
			self->cursor.order_index.track,
			offset, rv);
		if (rv->sequencentrynode) {
			seqoffset = psy_audio_sequencetrackiterator_seqoffset(rv);
			if (rv->pattern) {
				length = rv->pattern->length;
			}
		} else {
			psy_audio_SequenceEntry* entry;
			psy_audio_Sequence* sequence;

			sequence = patternviewstate_sequence(self);	
			entry = psy_audio_sequence_entry(sequence,
				psy_audio_orderindex_make(self->cursor.order_index.track, 0));
			if (entry) {
				psy_audio_sequence_begin(sequence,
					self->cursor.order_index.track,
					psy_audio_sequenceentry_offset(entry),					
					rv);
			}
		}
	} else {
		psy_audio_Sequence* sequence;

		sequence = patternviewstate_sequence(self);
		rv->sequencentrynode = NULL;
		psy_audio_sequence_begin(sequence,
			self->cursor.order_index.track,
			psy_audio_sequencecursor_seqoffset(&self->cursor, self->sequence) +
			offset, rv);
		// rv->patternnode = psy_audio_pattern_greaterequal(
		//	patternviewstate_pattern(self), offset - seqoffset);
	}	
}

void patternviewstate_sync_cursor_to_sequence(PatternViewState* self)
{
	if (patternviewstate_sequence(self)) {
		patternviewstate_set_cursor(self,
			patternviewstate_sequence(self)->cursor);
	}
}
