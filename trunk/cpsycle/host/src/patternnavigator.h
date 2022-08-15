/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNNAVIGATOR_H)
#define PATTERNNAVIGATOR_H

/* host */
#include "trackergridstate.h"


typedef struct PatternLineNavigator {
	PatternViewState* state;
	bool wrap;
	bool wraparound;
} PatternLineNavigator;

void patternlinenavigator_init(PatternLineNavigator*, PatternViewState*,
	bool wraparound);

psy_audio_SequenceCursor patternlinenavigator_up(PatternLineNavigator*,
	uintptr_t lines, psy_audio_SequenceCursor);
psy_audio_SequenceCursor patternlinenavigator_down(PatternLineNavigator*,
	uintptr_t lines, psy_audio_SequenceCursor);
psy_audio_SequenceCursor patternlinenavigator_home(PatternLineNavigator*,
	psy_audio_SequenceCursor); /* ft2 home */
psy_audio_SequenceCursor patternlinenavigator_end(PatternLineNavigator*,
	psy_audio_SequenceCursor); /* ft2 end */

INLINE bool patternlinennavigator_wrap(const PatternLineNavigator* self)
{
	return self->wrap;
}

typedef struct PatternColNavigator {
	TrackerState* state;
	bool wrap;
	bool wraparound;
} PatternColNavigator;

void patterncolnavigator_init(PatternColNavigator*, TrackerState*,
	bool wraparound);

psy_audio_SequenceCursor patterncolnavigator_prev_track(PatternColNavigator*,
	psy_audio_SequenceCursor);
psy_audio_SequenceCursor patterncolnavigator_next_track(PatternColNavigator*,
	psy_audio_SequenceCursor);
psy_audio_SequenceCursor patterncolnavigator_prev_col(PatternColNavigator*,
	psy_audio_SequenceCursor);
psy_audio_SequenceCursor patterncolnavigator_next_col(PatternColNavigator*,
	psy_audio_SequenceCursor);
psy_audio_SequenceCursor patterncolnavigator_home(PatternColNavigator*,
	psy_audio_SequenceCursor);
psy_audio_SequenceCursor patterncolnavigator_end(PatternColNavigator*,
	psy_audio_SequenceCursor);


INLINE bool patterncolnavigator_wrap(const PatternColNavigator* self)
{
	return self->wrap;
}

#endif /* PATTERNNAVIGATOR_H */
