/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
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
	PatternViewConfig* patconfig, psy_audio_Song* song, PatternCmds* cmds)
{
	psy_audio_sequencecursor_init(&self->cursor);
	psy_audio_blockselection_init(&self->selection);	
	self->pgupdownstepmode = PATTERNCURSOR_STEP_BEAT;
	self->pgupdownstep = 4;	
	self->singlemode = TRUE;
	self->song = song;
	self->pattern = NULL;	
	self->patconfig = patconfig;
	self->display = PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER;
	self->cmds = cmds;
	self->ft2home = TRUE;
	self->ft2delete = TRUE;
	self->movecursorwhenpaste = TRUE;
	self->movecursoronestep = FALSE;
	self->wraparound = TRUE;
	psy_audio_pattern_init(&self->patternpaste);	
	if (cmds) {
		cmds->patternpaste = &self->patternpaste;
	}
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
		patternviewstate_invalidate(self);
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
		patternviewstate_invalidate(self);
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
		patternviewstate_invalidate(self);
	}
}

void patternviewstate_configure_keyboard(PatternViewState* self,
	KeyboardMiscConfig* config)
{
	self->ft2home = keyboardmiscconfig_ft2home(config);
	self->ft2delete = keyboardmiscconfig_ft2delete(config);
	self->movecursoronestep = keyboardmiscconfig_movecursoronestep(config);
	patternviewstate_setpgupdown(self,
		(PatternCursorStepMode)keyboardmiscconfig_pgupdowntype(config),
		keyboardmiscconfig_pgupdownstep(config));
}

void patternviewstate_configure(PatternViewState* self)
{
	self->movecursorwhenpaste = patternviewconfig_ismovecursorwhenpaste(
		self->patconfig);
	self->wraparound = patternviewconfig_wraparound(self->patconfig);
	if (patternviewconfig_issinglepatterndisplay(self->patconfig)) {
		patternviewstate_displaypattern(self);
	} else {
		patternviewstate_displaysequence(self);
	}
}

psy_audio_SequenceTrackIterator patternviewstate_sequencestart(PatternViewState* self,
	double startoffset)
{
	double offset;
	double seqoffset;
	double length;
	psy_audio_SequenceTrackIterator rv;

	assert(self);

	rv.pattern = self->pattern;
	rv.patternnode = NULL;
	rv.patterns = patternviewstate_patterns(self);
	seqoffset = 0.0;
	length = rv.pattern->length;
	offset = startoffset;
	if (!self->singlemode && patternviewstate_sequence(self)) {
		psy_audio_SequenceTrackNode* tracknode;
		psy_audio_Sequence* sequence;

		sequence = patternviewstate_sequence(self);
		tracknode = psy_list_at(sequence->tracks,
			self->cursor.orderindex.track);
		if (!tracknode) {
			tracknode = sequence->tracks;
		}
		rv = psy_audio_sequence_begin(sequence,
			tracknode ? (psy_audio_SequenceTrack*)tracknode->entry : NULL,
			offset);
		if (rv.sequencentrynode) {
			seqoffset = psy_audio_sequencetrackiterator_seqoffset(&rv);
			if (rv.pattern) {
				length = rv.pattern->length;
			}
		}
	} else {
		rv.sequencentrynode = NULL;
		rv.patternnode = psy_audio_pattern_greaterequal(
			patternviewstate_pattern(self), offset - seqoffset);
	}	
	return rv;
}
