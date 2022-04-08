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
			self->cursor.absoffset,
			4.0);
		if (self->cursor.absoffset > patternviewstate_pattern(self)->length) {
			self->cursor.absoffset = patternviewstate_pattern(self)->length;
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

void patternviewstate_configure(PatternViewState* self)
{	
	if (patternviewconfig_issinglepatterndisplay(self->patconfig)) {
		patternviewstate_displaypattern(self);
	} else {
		patternviewstate_displaysequence(self);
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
		psy_audio_SequenceTrackNode* tracknode;
		psy_audio_Sequence* sequence;

		sequence = patternviewstate_sequence(self);
		tracknode = psy_list_at(sequence->tracks,
			self->cursor.orderindex.track);
		if (!tracknode) {
			tracknode = sequence->tracks;
		}
		psy_audio_sequence_begin(sequence,
			tracknode ? (psy_audio_SequenceTrack*)tracknode->entry : NULL,
			offset, rv);
		if (rv->sequencentrynode) {
			seqoffset = psy_audio_sequencetrackiterator_seqoffset(rv);
			if (rv->pattern) {
				length = rv->pattern->length;
			}
		}
	} else {
		rv->sequencentrynode = NULL;
		rv->patternnode = psy_audio_pattern_greaterequal(
			patternviewstate_pattern(self), offset - seqoffset);
	}	
}

void patternviewstate_sync_cursor_to_sequence(PatternViewState* self)
{
	if (patternviewstate_sequence(self)) {
		patternviewstate_set_cursor(self,
			patternviewstate_sequence(self)->cursor);
	}
}