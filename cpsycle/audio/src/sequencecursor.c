/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sequencecursor.h"
/* platform */
#include "sequencer.h" /* calculate duration */
/* platform */
#include "../../detail/portable.h"

/* SequenceCursor */

/* implementation */
void psy_audio_sequencecursor_init(psy_audio_SequenceCursor* self)
{	
	self->order_index = psy_audio_orderindex_make(0, 0);	
	self->key = psy_audio_NOTECOMMANDS_MIDDLEC;
	self->track = 0;
	self->offset = 0.0;	
	self->lpb = 4;
	self->column = 0;
	self->digit = 0;	
	self->noteindex = 0;	
}

void psy_audio_sequencecursor_init_all(psy_audio_SequenceCursor* self,
	psy_audio_OrderIndex order_index)
{
	psy_audio_sequencecursor_init(self);
	self->order_index = order_index;
}

psy_audio_SequenceCursor psy_audio_sequencecursor_make_all(
	psy_audio_OrderIndex order_index,
	uintptr_t track, psy_dsp_big_beat_t offset, uint8_t key)
{
	psy_audio_SequenceCursor rv;
	
	psy_audio_sequencecursor_init(&rv);
	rv.order_index = order_index;
	rv.track = track;
	rv.offset = offset;
	rv.key = key;	
	return rv;
}

psy_audio_SequenceCursor psy_audio_sequencecursor_make(
	psy_audio_OrderIndex order_index,
	uintptr_t track, psy_dsp_big_beat_t offset)
{
	psy_audio_SequenceCursor rv;

	psy_audio_sequencecursor_init(&rv);
	rv.order_index = order_index;
	rv.track = track;
	rv.offset = offset;	
	return rv;
}

bool psy_audio_sequencecursor_equal(psy_audio_SequenceCursor* lhs,
	psy_audio_SequenceCursor* rhs)
{
	assert(lhs && rhs);

	return (psy_audio_orderindex_equal(&lhs->order_index, rhs->order_index) &&
		rhs->offset == lhs->offset &&
		rhs->column == lhs->column &&
		rhs->digit == lhs->digit &&
		rhs->track == lhs->track &&		
		rhs->noteindex == lhs->noteindex &&
		rhs->lpb == lhs->lpb &&
		rhs->key == lhs->key);		
}

uintptr_t psy_audio_sequencecursor_pattern_id(
	const psy_audio_SequenceCursor* self,
	const psy_audio_Sequence* sequence)
{	
	const psy_audio_SequenceEntry* entry;

	assert(self);	
		
	entry = psy_audio_sequence_entry_const(sequence, self->order_index);
	if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
		return ((psy_audio_SequencePatternEntry*)entry)->patternslot;
	}
	return psy_INDEX_INVALID;
}

psy_dsp_big_beat_t psy_audio_sequencecursor_seqoffset(
	const psy_audio_SequenceCursor* self,
	const psy_audio_Sequence* sequence)
{
	const psy_audio_SequenceEntry* seqentry;
	
	assert(self);
		
	seqentry = psy_audio_sequence_entry_const(sequence, self->order_index);	
	if (seqentry) {
		return psy_audio_sequenceentry_offset(seqentry);
	}
	return 0.0;
}

void psy_audio_sequencecursor_trace(const psy_audio_SequenceCursor* self)
{
	printf("trk: %u  order: %u pos %f\n",
		(unsigned int)self->order_index.track,
		(unsigned int)self->order_index.order,
		(float)self->offset);
		
}

