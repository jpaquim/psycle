/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_SEQUENCECURSOR_H
#define psy_audio_SEQUENCECURSOR_H

/* local */
#include "sequenceselection.h"
/* dsp */
#include <dsptypes.h>
/* platform */
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_SequenceCursor
*/

struct psy_audio_SequenceEntry;

typedef struct psy_audio_SequenceCursor {
	/* index to the sequence entry */
	psy_audio_OrderIndex order_index;
	/* offset in sequence entry */
	psy_dsp_big_beat_t offset;
	/* channel track in the pattern */
	uintptr_t track;
	uintptr_t lpb;
	uintptr_t column;
	uintptr_t digit;
	uint8_t key;
	uintptr_t noteindex;
	/* abs values temporarily valid in some cases */
	psy_dsp_big_beat_t abs_offset;	
	uintptr_t abs_line;
} psy_audio_SequenceCursor;

void psy_audio_sequencecursor_init(psy_audio_SequenceCursor*);
void psy_audio_sequencecursor_init_all(psy_audio_SequenceCursor*,
	psy_audio_OrderIndex);

psy_audio_SequenceCursor psy_audio_sequencecursor_make_all(
	psy_audio_OrderIndex,
	uintptr_t track, psy_dsp_big_beat_t offset, uint8_t key);
psy_audio_SequenceCursor psy_audio_sequencecursor_make(
	psy_audio_OrderIndex,
	uintptr_t track, psy_dsp_big_beat_t offset);

INLINE uintptr_t psy_audio_sequencecursor_track(const
	psy_audio_SequenceCursor* self)
{
	return (self->track);
}

INLINE uintptr_t psy_audio_sequencecursor_column(const
	psy_audio_SequenceCursor* self)
{
	return (self->column);
}

INLINE uintptr_t psy_audio_sequencecursor_digit(const
	psy_audio_SequenceCursor* self)
{
	return (self->digit);
}

INLINE uintptr_t psy_audio_sequencecursor_key(const
	psy_audio_SequenceCursor* self)
{
	return (self->key);
}

INLINE uintptr_t psy_audio_sequencecursor_note_index(const
	psy_audio_SequenceCursor* self)
{
	return (self->noteindex);
}

/* compares two pattern edit positions, if they are equal */
bool psy_audio_sequencecursor_equal(psy_audio_SequenceCursor* lhs,
	psy_audio_SequenceCursor* rhs);
	
uintptr_t psy_audio_sequencecursor_pattern_id(const psy_audio_SequenceCursor*,
	const struct psy_audio_Sequence*);
psy_dsp_big_beat_t psy_audio_sequencecursor_seqoffset(
	const struct psy_audio_SequenceCursor*,
	const struct psy_audio_Sequence*);

INLINE void psy_audio_sequencecursor_set_offset(psy_audio_SequenceCursor* self,
	psy_dsp_big_beat_t offset)
{
	self->offset = offset;
}

INLINE psy_dsp_big_beat_t psy_audio_sequencecursor_offset(
	const psy_audio_SequenceCursor* self)
{
	return self->offset;
}

INLINE psy_dsp_big_beat_t psy_audio_sequencecursor_offset_abs(
	const psy_audio_SequenceCursor* self,
	const struct psy_audio_Sequence* sequence)
{
	return self->offset + psy_audio_sequencecursor_seqoffset(self, sequence);
}

INLINE uintptr_t psy_audio_sequencecursor_line(const psy_audio_SequenceCursor* self)
{
	return (uintptr_t)(self->offset * (double)self->lpb);
}

INLINE uintptr_t psy_audio_sequencecursor_line_abs(const psy_audio_SequenceCursor* self,
	const struct psy_audio_Sequence* sequence)
{
	return cast_decimal(psy_audio_sequencecursor_offset_abs(self, sequence) * self->lpb);
}

INLINE void psy_audio_sequencecursor_set_order_index(
	psy_audio_SequenceCursor* self, psy_audio_OrderIndex index)
{
	self->order_index = index;	
}

INLINE psy_audio_OrderIndex psy_audio_sequencecursor_order_index(
	const psy_audio_SequenceCursor* self)
{
	assert(self);

	return self->order_index;
}

INLINE psy_dsp_big_beat_t psy_audio_sequencecursor_bpl(
	const psy_audio_SequenceCursor* self)
{
	return (psy_dsp_big_beat_t)1.0 / (psy_dsp_big_beat_t)self->lpb;
}

INLINE void psy_audio_sequencecursor_set_lpb(psy_audio_SequenceCursor* self,
	uintptr_t lpb)
{
	self->lpb = lpb;	
}

INLINE uintptr_t psy_audio_sequencecursor_lpb(const psy_audio_SequenceCursor*
	self)
{
	return self->lpb;
}

void psy_audio_sequencecursor_trace(const psy_audio_SequenceCursor*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SEQUENCECURSOR_H */
