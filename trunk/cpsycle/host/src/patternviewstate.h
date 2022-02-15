/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNVIEWSTATE_H)
#define PATTERNVIEWSTATE_H

/* host */
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
	PatternCursorStepMode pgupdownstepmode;
	intptr_t pgupdownstep;
	bool movecursoronestep;
	psy_audio_BlockSelection selection;
	psy_audio_SequenceCursor dragselectionbase;	
	bool singlemode;
	PatternDisplayMode display;
	bool movecursorwhenpaste;
	bool ft2home;
	bool ft2delete;
	bool wraparound;
	psy_audio_Pattern patternpaste;
	/* references */
	psy_audio_Pattern* pattern;
	psy_audio_Song* song;	
	PatternCmds* cmds;
	PatternViewConfig* patconfig;
} PatternViewState;

void patternviewstate_init(PatternViewState*, PatternViewConfig*,
	psy_audio_Song*, PatternCmds* cmds);
void patternviewstate_dispose(PatternViewState*);

INLINE intptr_t patternviewstate_currpgupdownstep(const PatternViewState* self)
{
	if (self->pgupdownstepmode == PATTERNCURSOR_STEP_BEAT) {
		return self->cursor.lpb;
	} else if (self->pgupdownstepmode == PATTERNCURSOR_STEP_4BEAT) {
		return self->cursor.lpb * 4;
	}
	return self->pgupdownstep;
}

INLINE void patternviewstate_setpgupdownstep(PatternViewState* self, intptr_t step)
{
	self->pgupdownstep = step;
}

INLINE void patternviewstate_setpgupdown(PatternViewState* self,
	PatternCursorStepMode mode, intptr_t step)
{
	self->pgupdownstepmode = mode;
	self->pgupdownstep = step;
}

INLINE void patternviewstate_setcursor(PatternViewState* self,
	psy_audio_SequenceCursor cursor)
{
	self->cursor = cursor;
}

INLINE void patternviewstate_setsong(PatternViewState* self, psy_audio_Song* song)
{
	assert(self);

	self->song = song;
	if (self->cmds) {
		patterncmds_setsong(self->cmds, song);
	}
}

INLINE void patternviewstate_setpattern(PatternViewState* self, psy_audio_Pattern* pattern)
{
	assert(self);

	self->pattern = pattern;
	if (self->cmds) {
		patterncmds_setpattern(self->cmds, pattern);
	}
}

INLINE psy_audio_Pattern* patternviewstate_pattern(PatternViewState* self)
{
	assert(self);

	return self->pattern;
}

INLINE const psy_audio_Pattern* patternviewstate_pattern_const(const PatternViewState* self)
{
	assert(self);

	return self->pattern;
}

INLINE void patternviewstate_displaypattern(PatternViewState* self)
{
	self->singlemode = TRUE;
}

INLINE void patternviewstate_displaysequence(PatternViewState* self)
{
	self->singlemode = FALSE;
}

INLINE psy_audio_Sequence* patternviewstate_sequence(PatternViewState* self)
{
	assert(self);

	if (self->song) {
		return &self->song->sequence;
	}
	return NULL;
}

INLINE psy_audio_Sequence* patternviewstate_sequence_const(const PatternViewState* self)
{
	assert(self);

	if (self->song) {
		return &self->song->sequence;
	}
	return NULL;
}

INLINE psy_audio_Patterns* patternviewstate_patterns(PatternViewState* self)
{
	assert(self);

	if (self->song) {
		return &self->song->patterns;
	}
	return NULL;
}

INLINE psy_audio_Patterns* patternviewstate_patterns_const(const PatternViewState* self)
{
	assert(self);

	if (self->song) {
		return &self->song->patterns;
	}
	return NULL;
}


INLINE psy_dsp_big_beat_t patternviewstate_length(const PatternViewState* self)
{
	assert(self);

	if (self->singlemode) {
		if (self->pattern) {
			return psy_audio_pattern_length(self->pattern);
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

INLINE uintptr_t patternviewstate_numsongtracks(const PatternViewState* self)
{
	if (patternviewstate_patterns_const(self)) {
		return psy_audio_patterns_numtracks(
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

INLINE intptr_t patternviewstate_beattoline(const PatternViewState* self,
	psy_dsp_big_beat_t offset)
{
	assert(self);

	return cast_decimal(offset * patternviewstate_lpb(self));
}

INLINE uintptr_t patternviewstate_numlines(const PatternViewState* self)
{
	assert(self);
	
	return patternviewstate_beattoline(self,
		patternviewstate_length(self));
}

INLINE intptr_t patternviewstate_seqstartline(const PatternViewState* self)
{
	return patternviewstate_beattoline(self,
		((self->singlemode)
			? self->cursor.seqoffset
			: 0.0));
}

INLINE psy_dsp_big_beat_t patternviewstate_quantize(const PatternViewState*
	self, psy_dsp_big_beat_t position)
{
	assert(self);

	return patternviewstate_beattoline(self, position) *
		((psy_dsp_big_beat_t)1.0 / patternviewstate_lpb(self));
}

void patternviewstate_selectcol(PatternViewState*);
void patternviewstate_selectbar(PatternViewState*);
void patternviewstate_selectall(PatternViewState*);

INLINE double patternviewstate_preferredtrackwidth(const
	PatternViewState* self)
{	
	psy_ui_Style* style;

	style = psy_ui_style(STYLE_PV_TRACK_HEADER);
	return style->background.size.width;
}

INLINE bool patternviewstate_cursorposition_valid(PatternViewState* self)
{
	if (patternviewstate_pattern(self)) {
		return psy_audio_sequencecursor_offset(&self->cursor) <
			psy_audio_pattern_length(patternviewstate_pattern(self));
	}
	return psy_audio_sequencecursor_offset(&self->cursor) != 0.0;
}

INLINE void patternviewstate_invalidate(PatternViewState* self)
{
	if (patternviewstate_pattern(self)) {
		patternviewstate_pattern(self)->opcount++;
	}
}

INLINE bool patternviewstate_movecursorwhenpaste(PatternViewState* self)
{
	assert(self);

	if (self->movecursorwhenpaste && self->song) {
		psy_audio_SequenceCursor cursor;		
				
		cursor = self->cursor;
		cursor.track += self->patternpaste.maxsongtracks;
		cursor.offset += self->patternpaste.length;		
		if (cursor.offset >= psy_audio_pattern_length(patternviewstate_pattern(self))) {
			cursor.offset = psy_audio_pattern_length(patternviewstate_pattern(self)) -
				patternviewstate_bpl(self);
		}				
		psy_audio_sequence_setcursor(psy_audio_song_sequence(self->song),
			cursor);
		return TRUE;
	}
	return FALSE;
}

INLINE void patternviewstate_blockpaste(PatternViewState* self)
{
	assert(self);

	patterncmds_blockpaste(self->cmds, self->cursor, FALSE);
	patternviewstate_movecursorwhenpaste(self);	
	patternviewstate_invalidate(self);
}

INLINE void patternviewstate_blockmixpaste(PatternViewState* self)
{
	assert(self);

	patterncmds_blockpaste(self->cmds, self->cursor, TRUE);
	patternviewstate_movecursorwhenpaste(self);	
	patternviewstate_invalidate(self);
}

INLINE void patternviewstate_blockcopy(PatternViewState* self)
{
	assert(self);

	patterncmds_blockcopy(self->cmds, self->selection);
	patternviewstate_invalidate(self);
}

INLINE void patternviewstate_blockdelete(PatternViewState* self)
{
	assert(self);

	patterncmds_blockdelete(self->cmds, self->selection);
	patternviewstate_invalidate(self);
}

INLINE void patternviewstate_blockcut(PatternViewState* self)
{
	assert(self);

	patternviewstate_blockcopy(self);
	patternviewstate_blockdelete(self);
	patternviewstate_invalidate(self);
}

INLINE void patternviewstate_blockunmark(PatternViewState* self)
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

INLINE void patternviewstate_blocktranspose(PatternViewState* self, intptr_t offset)
{
	assert(self);

	patterncmds_blocktranspose(self->cmds, self->selection, self->cursor,
		offset);
	patternviewstate_invalidate(self);
}

void patternviewstate_configure_keyboard(PatternViewState*,
	KeyboardMiscConfig*);
void patternviewstate_configure(PatternViewState*);

// INLINE PatternViewSkin* patternviewstate_skin(PatternViewState* self)
// {
//	return &self->patconfig->skin;
// }

psy_audio_SequenceTrackIterator patternviewstate_sequencestart(PatternViewState*,
	double startoffset);

#endif /* PATTERNVIEWSTATE_H */
