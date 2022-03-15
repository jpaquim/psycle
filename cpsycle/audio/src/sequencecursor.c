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
	self->orderindex = psy_audio_orderindex_make(0, 0);
	self->seqoffset = (psy_dsp_big_beat_t)0.0;	
	self->key = psy_audio_NOTECOMMANDS_MIDDLEC;
	self->track = 0;
	self->absoffset = 0.0;	
	self->lpb = 4;
	self->column = 0;
	self->digit = 0;
	self->patternid = 0;
	self->noteindex = 0;
	psy_audio_sequencecursor_updatecache(self);
}

void psy_audio_sequencecursor_init_all(psy_audio_SequenceCursor* self,
	psy_audio_OrderIndex orderindex)
{
	psy_audio_sequencecursor_init(self);
	self->orderindex = orderindex;
	self->seqoffset = 0.0;
}

psy_audio_SequenceCursor psy_audio_sequencecursor_make_all(
	uintptr_t track, psy_dsp_big_beat_t absoffset, uint8_t key)
{
	psy_audio_SequenceCursor rv;
	
	psy_audio_sequencecursor_init(&rv);
	rv.track = track;
	rv.absoffset = absoffset;
	rv.key = key;
	psy_audio_sequencecursor_updatecache(&rv);
	return rv;
}

psy_audio_SequenceCursor psy_audio_sequencecursor_make(
	uintptr_t track, psy_dsp_big_beat_t absoffset)
{
	psy_audio_SequenceCursor rv;

	psy_audio_sequencecursor_init(&rv);
	rv.track = track;
	rv.absoffset = absoffset;
	psy_audio_sequencecursor_updatecache(&rv);
	return rv;
}

bool psy_audio_sequencecursor_equal(psy_audio_SequenceCursor* lhs,
	psy_audio_SequenceCursor* rhs)
{
	assert(lhs && rhs);

	return
		(rhs->column == lhs->column &&
		rhs->digit == lhs->digit &&
		rhs->track == lhs->track &&
		rhs->absoffset == lhs->absoffset &&
		rhs->patternid == lhs->patternid &&
		rhs->noteindex == lhs->noteindex &&
		rhs->lpb == lhs->lpb &&		
		rhs->seqoffset == lhs->seqoffset &&
		rhs->orderindex.order == lhs->orderindex.order &&
		rhs->orderindex.track == lhs->orderindex.track &&
		rhs->key == lhs->key);
}

void psy_audio_sequencecursor_updateseqoffset(psy_audio_SequenceCursor* self,
	const psy_audio_Sequence* sequence)
{
	const psy_audio_SequenceEntry* entry;

	assert(self);
	assert(sequence);

	entry = psy_audio_sequence_entry_const(sequence, self->orderindex);
	if (entry) {
		self->seqoffset = psy_audio_sequenceentry_offset(entry);
	} else {
		self->seqoffset = 0.0;
	}	
}

void psy_audio_sequencecursor_update_order(psy_audio_SequenceCursor* self,
	const psy_audio_Sequence* sequence)
{
	assert(self);
	assert(sequence);

	self->orderindex.order = psy_audio_sequence_order(sequence,
		self->orderindex.track,
		psy_audio_sequencecursor_offset_abs(self));	
	psy_audio_sequencecursor_updateseqoffset(self, sequence);	
}


uintptr_t psy_audio_sequencecursor_patternid(
	const psy_audio_SequenceCursor* self,
	const psy_audio_Sequence* sequence)
{
	uintptr_t rv;
	const psy_audio_SequenceEntry* entry;

	assert(self);	
	
	rv = psy_INDEX_INVALID;	
	entry = psy_audio_sequence_entry_const(sequence, self->orderindex);
	if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
		rv = ((psy_audio_SequencePatternEntry*)entry)->patternslot;
	}
	return rv;
}

psy_dsp_big_beat_t psy_audio_sequencecursor_seqoffset(
	const psy_audio_SequenceCursor* self)
{
	return self->seqoffset;
}

uintptr_t psy_audio_sequencecursor_track(const psy_audio_SequenceCursor* self)
{
	return (self->track);
}

uintptr_t psy_audio_sequencecursor_column(const psy_audio_SequenceCursor* self)
{
	return (self->column);
}

uintptr_t psy_audio_sequencecursor_digit(const psy_audio_SequenceCursor* self)
{
	return (self->digit);
}

uintptr_t psy_audio_sequencecursor_noteindex(const psy_audio_SequenceCursor* self)
{
	return (self->noteindex);
}

void psy_audio_sequencecursor_updatecache(const psy_audio_SequenceCursor* self)
{	
	((psy_audio_SequenceCursor*)self)->linecache = /* mutable */
		cast_decimal(self->absoffset * self->lpb);
}
