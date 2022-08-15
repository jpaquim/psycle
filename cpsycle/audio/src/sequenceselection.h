/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_SEQUENCESELECTION_H
#define psy_audio_SEQUENCESELECTION_H

/* container */
#include <list.h>
#include <signal.h>
#include <command.h>

#include "../../detail/psydef.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_OrderIndex
**
** Addresses a SequenceEntry inside psy_audio_Sequence
*/

typedef struct psy_audio_OrderIndex {
	uintptr_t track;
	uintptr_t order;
} psy_audio_OrderIndex;

INLINE psy_audio_OrderIndex psy_audio_orderindex_make(uintptr_t track,
	uintptr_t order)
{
	psy_audio_OrderIndex rv;

	rv.track = track;
	rv.order = order;
	return rv;
}

psy_audio_OrderIndex* psy_audio_orderindex_alloc(void);

INLINE psy_audio_OrderIndex psy_audio_orderindex_make_invalid(void)
{
	return psy_audio_orderindex_make(psy_INDEX_INVALID, psy_INDEX_INVALID);
}

INLINE bool psy_audio_orderindex_valid(const psy_audio_OrderIndex* self)
{
	return (self->order != psy_INDEX_INVALID &&
		    self->track != psy_INDEX_INVALID);
}

INLINE bool psy_audio_orderindex_invalid(const psy_audio_OrderIndex* self)
{
	return (!psy_audio_orderindex_valid(self));
}

INLINE bool psy_audio_orderindex_equal(const psy_audio_OrderIndex* self,
	psy_audio_OrderIndex other)
{
	return (self->order == other.order) && (self->track == other.track);
}

/*
** psy_audio_Order
** A pair of OrderIndex and SequenceEntrySequence
*/

struct psy_audio_Sequence;

/*
** psy_audio_SequenceSelection
**
** Selection of SequenceEntries stored as OrderIndexes
*/

typedef psy_List* psy_audio_SequenceSelectionIterator;

typedef struct psy_audio_SequenceSelection {
	/* signals */
	psy_Signal signal_clear;
	psy_Signal signal_select;
	psy_Signal signal_deselect;
	// psy_Signal signal_update;
	/* internal */
	psy_List* entries; /* list of order indexes */	
} psy_audio_SequenceSelection;

void psy_audio_sequenceselection_init(psy_audio_SequenceSelection*);
void psy_audio_sequenceselection_dispose(psy_audio_SequenceSelection*);

void psy_audio_sequenceselection_clear(psy_audio_SequenceSelection*);
void psy_audio_sequenceselection_copy(psy_audio_SequenceSelection*,
	psy_audio_SequenceSelection* other);

INLINE psy_audio_SequenceSelectionIterator psy_audio_sequenceselection_begin(
	psy_audio_SequenceSelection* self)
{
	return self->entries;
}

psy_audio_OrderIndex psy_audio_sequenceselection_first(
	const psy_audio_SequenceSelection*);
void psy_audio_sequenceselection_select(psy_audio_SequenceSelection*,
	psy_audio_OrderIndex);
void psy_audio_sequenceselection_select_first(psy_audio_SequenceSelection*,
	psy_audio_OrderIndex);
void psy_audio_sequenceselection_deselect(psy_audio_SequenceSelection*,
	psy_audio_OrderIndex);
void psy_audio_sequenceselection_deselect_all(psy_audio_SequenceSelection*);
bool psy_audio_sequenceselection_is_selected(const psy_audio_SequenceSelection*,
	psy_audio_OrderIndex);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SEQUENCESELECTION_H */
