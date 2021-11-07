/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_SEQUENCECURSOR_H
#define psy_audio_SEQUENCECURSOR_H

/* local */
#include "patterncursor.h"
#include "sequenceselection.h"

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_SequenceCursor
*/

typedef struct psy_audio_SequenceCursor {
	psy_audio_PatternCursor cursor;
	psy_audio_OrderIndex orderindex;
	psy_dsp_big_beat_t seqoffset;
} psy_audio_SequenceCursor;

void psy_audio_sequencecursor_init(psy_audio_SequenceCursor*);
void psy_audio_sequencecursor_init_all(psy_audio_SequenceCursor*,
	psy_audio_OrderIndex orderindex);

void psy_audio_sequencecursor_updateseqoffset(psy_audio_SequenceCursor*,
	const struct psy_audio_Sequence*);
uintptr_t psy_audio_sequencecursor_patternid(const psy_audio_SequenceCursor*,
	const struct psy_audio_Sequence*);
psy_dsp_big_beat_t psy_audio_sequencecursor_seqoffset(
	const struct psy_audio_SequenceCursor*);
uintptr_t psy_audio_sequencecursor_line(const psy_audio_SequenceCursor*);
uintptr_t psy_audio_sequencecursor_track(const psy_audio_SequenceCursor*);
uintptr_t psy_audio_sequencecursor_column(const psy_audio_SequenceCursor*);
uintptr_t psy_audio_sequencecursor_digit(const psy_audio_SequenceCursor*);

INLINE psy_dsp_big_beat_t psy_audio_sequencecursor_offset_abs(const psy_audio_SequenceCursor* self)
{
	return self->cursor.offset + (
		(self->cursor.absolute)
		? 0.0
		: self->seqoffset);
}

INLINE psy_dsp_big_beat_t psy_audio_sequencecursor_offset(const psy_audio_SequenceCursor* self)
{
	return self->cursor.offset;
}

INLINE psy_audio_OrderIndex psy_audio_sequencecursor_orderindex(
	const psy_audio_SequenceCursor* self)
{
	assert(self);

	return self->orderindex;
}

INLINE uintptr_t psy_audio_sequencecursor_lpb(const psy_audio_SequenceCursor*
	self)
{
	return self->cursor.lpb;
}

INLINE psy_dsp_big_beat_t psy_audio_sequencecursor_bpl(
	const psy_audio_SequenceCursor* self)
{
	return (psy_dsp_big_beat_t)1.0 / (psy_dsp_big_beat_t)self->cursor.lpb;
}

INLINE void psy_audio_sequencecursor_setlpb(psy_audio_SequenceCursor* self,
	uintptr_t lpb)
{
	self->cursor.lpb = lpb;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SEQUENCECURSOR_H */
