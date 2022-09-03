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
/* ui */
#include <uiapp.h>
/* audio */
#include <pattern.h>
#include <sequence.h>


typedef struct BeatLine {	
	uintptr_t lpb;
	psy_dsp_big_beat_t bpl;
} BeatLine;

void beatline_init(BeatLine*, uintptr_t lpb);

INLINE void beatline_set_lpb(BeatLine* self, uintptr_t lpb)
{
	assert(self);
	
	if (self->lpb != lpb) {
		self->lpb = lpb;
		self->bpl = 1.0 / (double)lpb;
	}
}

INLINE intptr_t beatline_beat_to_line(const BeatLine* self,
	psy_dsp_big_beat_t offset)
{
	assert(self);

	return cast_decimal(offset * self->lpb);
}

INLINE psy_dsp_big_beat_t beatline_quantize(const BeatLine* self,
	psy_dsp_big_beat_t position)
{		
	assert(self);
		
	return beatline_beat_to_line(self, position) *
		((psy_dsp_big_beat_t)1.0 / self->lpb);
}

typedef struct BeatConvert {
	BeatLine* beat_line;	
	double line_px;	
} BeatConvert;

void beatconvert_init(BeatConvert*, BeatLine* beat_line,  double line_px);

INLINE void beatconvert_set_line_px(BeatConvert* self, double line_px)
{
	assert(self);
	
	self->line_px = line_px;	
}

/* quantized */
INLINE double beatconvert_beat_to_px(const BeatConvert* self,
	psy_dsp_big_beat_t position)
{	
	assert(self);
	
	return self->line_px * beatline_beat_to_line(self->beat_line, position);
}

/* not quantized */
INLINE double beatconvert_raw_beat_to_px(const BeatConvert* self,
	psy_dsp_big_beat_t position)
{	
	assert(self);
	
	return self->line_px * (double)self->beat_line->lpb * position;		
}

/* quantized */
INLINE psy_dsp_big_beat_t beatconvert_px_to_beat(const BeatConvert* self,
	double px)
{		
	assert(self);
	
	return beatline_quantize(self->beat_line, (px / self->line_px) *
		(double)self->beat_line->bpl);
}

/* BeatClip */

typedef struct BeatClip {
	psy_dsp_big_beat_t begin;
	psy_dsp_big_beat_t end;	
} BeatClip;

void beatclip_init(BeatClip*, BeatConvert*, double begin_px, double end_px);


typedef enum {
	PATTERNCURSOR_STEP_BEAT,
	PATTERNCURSOR_STEP_4BEAT,
	PATTERNCURSOR_STEP_LINES
} PatternCursorStepMode;

/* PatternViewState */

typedef struct PatternViewState {
	BeatLine beat_line;
	/* internal */	
	psy_audio_SequenceCursor cursor;
	psy_audio_BlockSelection selection;	
	psy_audio_Pattern patternpaste;
	bool chord;
	uintptr_t chord_begin;
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
	beatline_set_lpb(&self->beat_line, cursor.lpb);
}

INLINE const psy_audio_SequenceCursor* patternviewstate_cursor(const
	PatternViewState* self)
{
	return &self->cursor;
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
	return psy_audio_sequence_pattern(self->sequence,
		psy_audio_sequencecursor_order_index(&self->cursor));
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

	if (!self->sequence) {
		return 0.0;
	}
	if (patternviewstate_single_mode(self)) {
		psy_audio_SequenceEntry* seqentry;
		
		seqentry = psy_audio_sequence_entry(self->sequence,
			self->cursor.order_index);
		if (seqentry) {
			return psy_audio_sequenceentry_length(seqentry);			
		}
	} else {
		psy_audio_SequenceTrack* track;

		track = psy_audio_sequence_track_at(self->sequence,
			self->cursor.order_index.track);
		if (track) {
			return psy_audio_sequencetrack_duration(track,
				patternviewstate_patterns_const(self));
		}		
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

INLINE uintptr_t patternviewstate_numlines(const PatternViewState* self)
{		
	assert(self);
		
	return beatline_beat_to_line(&self->beat_line,
		patternviewstate_length(self));
}

INLINE psy_dsp_big_beat_t patternviewstate_draw_offset(const PatternViewState*
	self, psy_dsp_big_beat_t absoffset)
{
	return absoffset -
		(patternviewstate_single_mode(self)
		? psy_audio_sequencecursor_seqoffset(&self->cursor, self->sequence)
		: 0.0);
}

INLINE void patternviewstate_invalidate(PatternViewState* self)
{
	if (patternviewstate_pattern(self)) {
		patternviewstate_pattern(self)->opcount++;
	}
}

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

INLINE PatternCursorStepMode patternviewstate_pgupdowntype(
	const PatternViewState* self)
{
	if (self->keymiscconfig) {
		return ((PatternCursorStepMode)keyboardmiscconfig_pgupdowntype(
			self->keymiscconfig));
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

INLINE intptr_t patternviewstate_curr_pgup_down_step(
	const PatternViewState* self)
{
	if (patternviewstate_pgupdowntype(self) == PATTERNCURSOR_STEP_BEAT) {
		return self->cursor.lpb;
	} else if (patternviewstate_pgupdowntype(self) ==
			PATTERNCURSOR_STEP_4BEAT) {
		return self->cursor.lpb * 4;
	}
	return patternviewstate_pgupdownstep(self);
}

INLINE bool patternviewstate_hasmovecursorwhenpaste(
	const PatternViewState* self)
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
		cursor.offset += self->patternpaste.length;
		/* todo abs */
		if (cursor.offset >= psy_audio_pattern_length(
				patternviewstate_pattern(self))) {
			cursor.offset = psy_audio_pattern_length(
				patternviewstate_pattern(self)) -
				psy_audio_sequencecursor_bpl(&cursor);
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
			patternviewstate_pattern(self), &self->selection);
		patternviewstate_invalidate(self);
	}
}

INLINE void patternviewstate_block_transpose(PatternViewState* self,
	intptr_t offset)
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
