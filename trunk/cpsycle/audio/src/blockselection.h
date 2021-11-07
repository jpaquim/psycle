/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_BLOCKSELECTION_H
#define psy_audio_BLOCKSELECTION_H

/* local */
#include "sequencecursor.h"
#include "patternentry.h"
/* container */
#include <signal.h>
/* platform */
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_BlockSelection
**
*/

#define psy_audio_GLOBALPATTERN INT32_MAX - 1
#define psy_audio_GLOBALPATTERN_TIMESIGTRACK 0
#define psy_audio_GLOBALPATTERN_LOOPTRACK 1

typedef struct psy_audio_BlockSelection {
	psy_audio_SequenceCursor topleft;
	psy_audio_SequenceCursor bottomright;
	bool valid;
} psy_audio_BlockSelection;

void psy_audio_blockselection_init(psy_audio_BlockSelection*);
void psy_audio_blockselection_init_all(psy_audio_BlockSelection*,
	psy_audio_PatternCursor topleft, psy_audio_PatternCursor bottomright);
psy_audio_BlockSelection psy_audio_blockselection_make(
	psy_audio_PatternCursor topleft, psy_audio_PatternCursor bottomright);

INLINE bool psy_audio_blockselection_valid(
	const psy_audio_BlockSelection* self)
{
	return self->valid;
}

INLINE void psy_audio_blockselection_enable(psy_audio_BlockSelection* self)
{
	self->valid = TRUE;
}

INLINE void psy_audio_blockselection_disable(
	psy_audio_BlockSelection* self)
{
	self->valid = FALSE;
}

INLINE bool psy_audio_blockselection_test(psy_audio_BlockSelection* self,
	uintptr_t track, psy_dsp_big_beat_t offset)
{
	return psy_audio_blockselection_valid(self) &&
		track >= self->topleft.cursor.track &&
		track < self->bottomright.cursor.track&&
		offset >= self->topleft.cursor.offset /* + self->topleft.seqoffset */ &&
		offset < self->bottomright.cursor.offset /* + self->bottomright.seqoffset */;
}

void psy_audio_blockselection_startdrag(psy_audio_BlockSelection*,
	psy_audio_SequenceCursor dragselectionbase,
	psy_audio_SequenceCursor cursor, double bpl);
void psy_audio_blockselection_drag(psy_audio_BlockSelection*,
	psy_audio_SequenceCursor dragselectionbase,
	psy_audio_SequenceCursor cursor, double bpl);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_BLOCKSELECTION_H */
