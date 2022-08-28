/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNNAVIGATOR_H)
#define PATTERNNAVIGATOR_H

/* host */
#include "trackergridstate.h"
#include "pianokeyboardstate.h"


/* PatternLineNavigator */

typedef struct PatternLineNavigator {
	/* internal */
	PatternViewState* state;
	bool wrap;
	bool wrap_around;
} PatternLineNavigator;

void patternlinenavigator_init(PatternLineNavigator*, PatternViewState*,
	bool wraparound);

psy_audio_SequenceCursor patternlinenavigator_up(PatternLineNavigator*,
	uintptr_t lines, const psy_audio_SequenceCursor*);
psy_audio_SequenceCursor patternlinenavigator_down(PatternLineNavigator*,
	uintptr_t lines, const psy_audio_SequenceCursor*);
psy_audio_SequenceCursor patternlinenavigator_home(PatternLineNavigator*,
	const psy_audio_SequenceCursor*); /* ft2 home */
psy_audio_SequenceCursor patternlinenavigator_end(PatternLineNavigator*,
	const psy_audio_SequenceCursor*); /* ft2 end */

INLINE bool patternlinennavigator_wrap(const PatternLineNavigator* self)
{
	return self->wrap;
}

/* PatternColNavigator */

typedef struct PatternColNavigator {
	/* internal */
	TrackerState* state;
	bool wrap;
	bool wrap_around;
} PatternColNavigator;

void patterncolnavigator_init(PatternColNavigator*, TrackerState*,
	bool wraparound);

psy_audio_SequenceCursor patterncolnavigator_prev_track(PatternColNavigator*,
	const psy_audio_SequenceCursor*);
psy_audio_SequenceCursor patterncolnavigator_next_track(PatternColNavigator*,
	const psy_audio_SequenceCursor*);
psy_audio_SequenceCursor patterncolnavigator_prev_col(PatternColNavigator*,
	const psy_audio_SequenceCursor*);
psy_audio_SequenceCursor patterncolnavigator_next_col(PatternColNavigator*,
	const psy_audio_SequenceCursor*);
psy_audio_SequenceCursor patterncolnavigator_home(PatternColNavigator*,
	const psy_audio_SequenceCursor*);
psy_audio_SequenceCursor patterncolnavigator_end(PatternColNavigator*,
	const psy_audio_SequenceCursor*);


INLINE bool patterncolnavigator_wrap(const PatternColNavigator* self)
{
	return self->wrap;
}


/* PatternKeyNavigator */

typedef struct PatternKeyNavigator {
	/* internal */
	KeyboardState* state;
	bool wrap;
	bool wrap_around;
} PatternKeyNavigator;

void patternkeynavigator_init(PatternKeyNavigator*, KeyboardState*,
	bool wraparound);

psy_audio_SequenceCursor patternkeynavigator_up(PatternKeyNavigator*,
	uintptr_t lines, const psy_audio_SequenceCursor*);
psy_audio_SequenceCursor patternkeynavigator_down(PatternKeyNavigator*,
	uintptr_t lines, const psy_audio_SequenceCursor*);

INLINE bool patternkeynavigator_wrap(const PatternKeyNavigator* self)
{
	return self->wrap;
}


/* PatternSelect */

typedef struct PatternSelect {
	/* internal */
	PatternViewState* state;	
} PatternSelect;

void patternselect_init(PatternSelect*, PatternViewState*);

void patternselect_select_col(PatternSelect*);
void patternselect_select_bar(PatternSelect*);
void patternselect_select_all(PatternSelect*);

#endif /* PATTERNNAVIGATOR_H */
