/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNVIEWSTATE_H)
#define PATTERNVIEWSTATE_H

/* host */
#include "keyboardmiscconfig.h"
#include "patternviewconfig.h"
#include "patternhostcmds.h"
#include "styles.h"
#include "trackercmds.h"
#include "workspace.h"
/* ui */
#include <uiapp.h>
/* audio */
#include <pattern.h>
#include <sequence.h>

typedef enum {
	PATTERNCURSOR_STEP_BEAT,
	PATTERNCURSOR_STEP_4BEAT,
	PATTERNCURSOR_STEP_LINES
} PatternCursorStepMode;

/* PatternViewState */
typedef struct PatternViewState {
	/* internal */	
	psy_audio_SequenceCursor cursor;
	psy_audio_BlockSelection selection;
	psy_audio_SequenceCursor dragselectionbase;
	psy_audio_Pattern patternpaste;	
	/* references */	
	psy_audio_Sequence* sequence;	
	PatternCmds* cmds;
	PatternViewConfig* patconfig;
	KeyboardMiscConfig* keymiscconfig;
} PatternViewState;

void patternviewstate_init(PatternViewState*, PatternViewConfig*,
	KeyboardMiscConfig*, psy_audio_Sequence*, PatternCmds*);
void patternviewstate_dispose(PatternViewState*);

INLINE void patternviewstate_set_cursor(PatternViewState* self,
	psy_audio_SequenceCursor cursor)
{
	self->cursor = cursor;
}

INLINE psy_audio_SequenceCursor patternviewstate_cursor(const
	PatternViewState* self)
{
	return self->cursor;
}

void patternviewstate_sync_cursor_to_sequence(PatternViewState* self);

INLINE void patternviewstate_set_sequence(PatternViewState* self,
	psy_audio_Sequence* sequence)
{
	assert(self);
	
	self->sequence = sequence;
	if (self->cmds) {
		patterncmds_set_sequence(self->cmds, self->sequence);
	}
}

INLINE void patternviewstate_displaypattern(PatternViewState* self)
{
	if (self->patconfig) {
		patternviewconfig_display_single_pattern(self->patconfig);
	}	
}

INLINE void patternviewstate_displaysequence(PatternViewState* self)
{
	if (self->patconfig) {
		patternviewconfig_display_sequence(self->patconfig);
	}
}

INLINE psy_audio_Sequence* patternviewstate_sequence(PatternViewState* self)
{
	assert(self);

	return self->sequence;
}

INLINE psy_audio_Sequence* patternviewstate_sequence_const(
	const PatternViewState* self)
{
	assert(self);

	return self->sequence;
}

INLINE psy_audio_Pattern* patternviewstate_pattern(PatternViewState* self)
{
	assert(self);	

	if (!self->sequence) {
		return NULL;
	}
	return psy_audio_sequence_pattern(
		self->sequence,
		self->cursor.orderindex);
}

INLINE const psy_audio_Pattern* patternviewstate_pattern_const(
	const PatternViewState* self)
{
	assert(self);

	return patternviewstate_pattern((PatternViewState*)self);
}

INLINE psy_audio_Patterns* patternviewstate_patterns(PatternViewState* self)
{
	assert(self);

	if (self->sequence) {
		return psy_audio_sequence_patterns(self->sequence);
	}
	return NULL;
}

INLINE const psy_audio_Patterns* patternviewstate_patterns_const(
	const PatternViewState* self)
{
	assert(self);

	if (self->sequence) {
		return psy_audio_sequence_patterns_const(self->sequence);
	}
	return NULL;
}

INLINE bool patternviewstate_single_mode(const PatternViewState* self)
{
	if (self->patconfig) {
		return patternviewconfig_single_mode(self->patconfig);
	}
	return TRUE;
}

INLINE psy_dsp_big_beat_t patternviewstate_length(const PatternViewState* self)
{
	assert(self);

	if (patternviewstate_single_mode(self)) {
		const psy_audio_Pattern* pattern;

		pattern = patternviewstate_pattern_const(self);
		if (pattern) {
			return psy_audio_pattern_length(pattern);
		}
	} else if (patternviewstate_sequence_const(self)) {
		psy_audio_SequenceTrack* track;

		track = psy_audio_sequence_track_at(
			patternviewstate_sequence_const(self), self->cursor.orderindex.track);
		if (track) {
			return psy_audio_sequencetrack_duration(track,
				patternviewstate_patterns_const(self));
		}
		return psy_audio_sequence_duration(patternviewstate_sequence_const(self));
	}
	return 0.0;
}

INLINE uintptr_t patternviewstate_num_song_tracks(const PatternViewState* self)
{
	if (patternviewstate_patterns_const(self)) {
		return psy_audio_patterns_num_tracks(
			patternviewstate_patterns_const(self));
	}
	return 0;
}

INLINE uintptr_t patternviewstate_lpb(const PatternViewState* self)
{
	return self->cursor.lpb;
}

INLINE psy_dsp_big_beat_t patternviewstate_bpl(const PatternViewState* self)
{
	return (psy_dsp_big_beat_t)1.0 / self->cursor.lpb;
}

INLINE intptr_t patternviewstate_beat_to_line(const PatternViewState* self,
	psy_dsp_big_beat_t offset)
{
	assert(self);

	return cast_decimal(offset * patternviewstate_lpb(self));
}

INLINE uintptr_t patternviewstate_numlines(const PatternViewState* self)
{
	assert(self);
	
	return patternviewstate_beat_to_line(self,
		patternviewstate_length(self));
}

INLINE intptr_t patternviewstate_seqstartline(const PatternViewState* self)
{	
	return patternviewstate_beat_to_line(self,
		(patternviewstate_single_mode(self)
		? self->cursor.seqoffset
		: 0.0));
}

INLINE psy_dsp_big_beat_t patternviewstate_quantize(const PatternViewState*
	self, psy_dsp_big_beat_t position)
{
	assert(self);

	return patternviewstate_beat_to_line(self, position) *
		((psy_dsp_big_beat_t)1.0 / patternviewstate_lpb(self));
}


INLINE psy_dsp_big_beat_t patternviewstate_draw_offset(const PatternViewState*
	self, psy_dsp_big_beat_t absoffset)
{
	return absoffset -
		(patternviewstate_single_mode(self)
		? self->cursor.seqoffset
		: 0.0);
}

void patternviewstate_select_col(PatternViewState*);
void patternviewstate_select_bar(PatternViewState*);
void patternviewstate_select_all(PatternViewState*);

INLINE double patternviewstate_preferredtrackwidth(const
	PatternViewState* self)
{	
	psy_ui_Style* style;

	style = psy_ui_style(STYLE_PV_TRACK_HEADER);
	return style->background.size.width;
}

INLINE void patternviewstate_invalidate(PatternViewState* self)
{
	if (patternviewstate_pattern(self)) {
		patternviewstate_pattern(self)->opcount++;
	}
}

void patternviewstate_sequencestart(PatternViewState*,
	double startoffset, psy_audio_SequenceTrackIterator* rv);


INLINE bool patternviewstate_ft2home(const PatternViewState* self)
{
	if (self->keymiscconfig) {
		return keyboardmiscconfig_ft2home(self->keymiscconfig);
	}
	return TRUE;
}

INLINE bool patternviewstate_ft2delete(const PatternViewState* self)
{
	if (self->keymiscconfig) {
		return keyboardmiscconfig_ft2delete(self->keymiscconfig);
	}
	return TRUE;
}

INLINE bool patternviewstate_move_cursor_one_step(const PatternViewState* self)
{
	if (self->keymiscconfig) {
		return keyboardmiscconfig_movecursoronestep(self->keymiscconfig);
	}
	return TRUE;
}

INLINE PatternCursorStepMode patternviewstate_pgupdowntype(const PatternViewState* self)
{
	if (self->keymiscconfig) {
		return ((PatternCursorStepMode)keyboardmiscconfig_pgupdowntype(self->keymiscconfig));
	}
	return TRUE;
}

INLINE intptr_t patternviewstate_pgupdownstep(const PatternViewState* self)
{
	if (self->keymiscconfig) {
		return keyboardmiscconfig_pgupdownstep(self->keymiscconfig);
	}
	return 4;
}

INLINE intptr_t patternviewstate_curr_pgup_down_step(const PatternViewState* self)
{
	if (patternviewstate_pgupdowntype(self) == PATTERNCURSOR_STEP_BEAT) {
		return self->cursor.lpb;
	} else if (patternviewstate_pgupdowntype(self) == PATTERNCURSOR_STEP_4BEAT) {
		return self->cursor.lpb * 4;
	}
	return patternviewstate_pgupdownstep(self);
}

INLINE bool patternviewstate_hasmovecursorwhenpaste(const PatternViewState* self)
{
	if (self->patconfig) {
		return patternviewconfig_ismovecursorwhenpaste(self->patconfig);
	}
	return TRUE;
}

INLINE bool patternviewstate_movecursorwhenpaste(PatternViewState* self)
{
	assert(self);
	
	if (self->sequence && patternviewstate_hasmovecursorwhenpaste(self)) {
		psy_audio_SequenceCursor cursor;

		cursor = self->cursor;
		cursor.track += self->patternpaste.maxsongtracks;
		cursor.absoffset += self->patternpaste.length;
		if (cursor.absoffset >= psy_audio_pattern_length(patternviewstate_pattern(self))) {
			cursor.absoffset = psy_audio_pattern_length(patternviewstate_pattern(self)) -
				patternviewstate_bpl(self);
		}
		psy_audio_sequence_set_cursor(self->sequence, cursor);
		return TRUE;
	}
	return FALSE;
}

INLINE void patternviewstate_block_paste(PatternViewState* self)
{
	assert(self);

	patterncmds_block_paste(self->cmds, self->cursor, FALSE);
	patternviewstate_movecursorwhenpaste(self);
	patternviewstate_invalidate(self);
}

INLINE void patternviewstate_block_mixpaste(PatternViewState* self)
{
	assert(self);

	patterncmds_block_paste(self->cmds, self->cursor, TRUE);
	patternviewstate_movecursorwhenpaste(self);
	patternviewstate_invalidate(self);
}

INLINE void patternviewstate_block_copy(PatternViewState* self)
{
	assert(self);

	patterncmds_block_copy(self->cmds, self->selection);
	patternviewstate_invalidate(self);
}

INLINE void patternviewstate_block_delete(PatternViewState* self)
{
	assert(self);

	if (psy_audio_blockselection_valid(&self->selection)) {
		patterncmds_block_delete(self->cmds, self->selection);
		patternviewstate_invalidate(self);
	}
}

INLINE void patternviewstate_block_cut(PatternViewState* self)
{
	assert(self);

	patternviewstate_block_copy(self);
	patternviewstate_block_delete(self);
	patternviewstate_invalidate(self);
}

INLINE void patternviewstate_block_unmark(PatternViewState* self)
{
	assert(self);

	psy_audio_blockselection_disable(&self->selection);
	patternviewstate_invalidate(self);
}

INLINE void patternviewstate_interpolatelinear(PatternViewState* self)
{
	if (patternviewstate_pattern(self)) {
		psy_audio_pattern_blockinterpolatelinear(
			patternviewstate_pattern(self),
			&self->selection);
		patternviewstate_invalidate(self);
	}
}

INLINE void patternviewstate_block_transpose(PatternViewState* self, intptr_t offset)
{
	assert(self);

	//	patterncmds_blocktranspose(self->cmds, self->selection, self->cursor,
	//		offset);
	if (psy_audio_blockselection_valid(&self->selection)) {
		psy_audio_sequence_blocktranspose(self->sequence, self->selection,
			offset);
	}
	patternviewstate_invalidate(self);
}


#endif /* PATTERNVIEWSTATE_H */
