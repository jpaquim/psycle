/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNVIEWSTATE_H)
#define PATTERNVIEWSTATE_H

/* host */
#include "patternviewskin.h"
#include "trackercmds.h"
#include "workspace.h"
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
	psy_audio_BlockSelection selection;
	psy_audio_SequenceCursor dragselectionbase;	
	bool singlemode;
	PatternDisplayMode display;
	/* references */
	psy_audio_Pattern* pattern;
	psy_audio_Song* song;
	PatternViewSkin* skin;
} PatternViewState;

void patternviewstate_init(PatternViewState*, psy_audio_Song*);
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

INLINE void patternviewstate_setcursor(PatternViewState* self,
	psy_audio_SequenceCursor cursor)
{
	self->cursor = cursor;
}

INLINE void patternviewstate_setsong(PatternViewState* self, psy_audio_Song* song)
{
	assert(self);

	self->song = song;
}

INLINE void patternviewstate_setpattern(PatternViewState* self, psy_audio_Pattern* pattern)
{
	assert(self);

	self->pattern = pattern;
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

#endif /* PATTERNVIEWSTATE_H */
