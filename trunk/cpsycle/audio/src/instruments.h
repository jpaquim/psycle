// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_INSTRUMENTS_H
#define psy_audio_INSTRUMENTS_H

// local
#include "instrument.h"
// container
#include <hashtbl.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

// psy_audio_Instruments
// 
// Aim: Container of grouped instruments
// Structure:
// psy_audio_Instruments <@>----- <psy_audio_InstrumentsGroup>
//                              *             <@>----- psy_audio_Instrument
//                                                   *
// default group index 0: Sampler PS1
//               index 1: Sampulse

// InstrumentIndex
//
// aim: Address a instrument inside psy_audio_Instruments
//      Separate sampulse and ps1 instruments
//
// Pair of two indexes (slot; subslot)
// 1. groupslot: selects a group
// 2. subslot:   selects a slot inside a group pointing
//	             to a instrument

typedef struct {
	uintptr_t groupslot;
	uintptr_t subslot;
} psy_audio_InstrumentIndex;

psy_audio_InstrumentIndex psy_audio_instrumentindex_make(uintptr_t groupslot,
	uintptr_t subslot);

INLINE uintptr_t psy_audio_instrumentindex_group(const psy_audio_InstrumentIndex* self)
{
	assert(self);

	return self->groupslot;
}

INLINE uintptr_t psy_audio_instrumentindex_subslot(
	const psy_audio_InstrumentIndex* self)
{
	assert(self);

	return self->subslot;
}

INLINE bool psy_audio_instrumentindex_invalid(const psy_audio_InstrumentIndex* self)
{
	assert(self);

	return (self->groupslot == psy_INDEX_INVALID) ||
		(self->subslot == psy_INDEX_INVALID);
}

typedef struct {
	char* name;
	psy_Table container;
} psy_audio_InstrumentsGroup;

void psy_audio_instrumentsgroup_init(psy_audio_InstrumentsGroup*);
void psy_audio_instrumentsgroup_dispose(psy_audio_InstrumentsGroup*);
psy_audio_InstrumentsGroup* psy_audio_instrumentsgroup_alloc(void);
psy_audio_InstrumentsGroup* psy_audio_instrumentsgroup_allocinit(void);

void psy_audio_instrumentsgroup_insert(psy_audio_InstrumentsGroup*,
psy_audio_Instrument*, uintptr_t slot);
void psy_audio_instrumentsgroup_remove(psy_audio_InstrumentsGroup*,
uintptr_t slot);
psy_audio_Instrument* psy_audio_instrumentsgroup_at(psy_audio_InstrumentsGroup*,
uintptr_t slot);
uintptr_t psy_audio_instrumentsgroup_size(const psy_audio_InstrumentsGroup*);


typedef struct psy_audio_Instruments {
	psy_Table groups;
	psy_audio_InstrumentIndex selected;
	psy_Signal signal_insert;
	psy_Signal signal_removed;
	psy_Signal signal_slotchange;
} psy_audio_Instruments;

void psy_audio_instruments_init(psy_audio_Instruments*);
void psy_audio_instruments_dispose(psy_audio_Instruments*);
void psy_audio_instruments_insert(psy_audio_Instruments*,
	psy_audio_Instrument*, psy_audio_InstrumentIndex);
void psy_audio_instruments_remove(psy_audio_Instruments*,
	psy_audio_InstrumentIndex);
void psy_audio_instruments_select(psy_audio_Instruments*,
	psy_audio_InstrumentIndex);
void psy_audio_instruments_dec(psy_audio_Instruments*);
void psy_audio_instruments_inc(psy_audio_Instruments*);

INLINE psy_audio_InstrumentIndex psy_audio_instruments_selected(
	psy_audio_Instruments* self)
{
	return self->selected;
}

psy_audio_Instrument* psy_audio_instruments_at(psy_audio_Instruments*,
	psy_audio_InstrumentIndex);
uintptr_t psy_audio_instruments_size(psy_audio_Instruments*, uintptr_t
	groupslot);
psy_TableIterator psy_audio_instruments_begin(psy_audio_Instruments*);
psy_TableIterator psy_audio_instruments_groupbegin(psy_audio_Instruments*,
	uintptr_t slot);
uintptr_t psy_audio_instruments_groupsize(psy_audio_Instruments*);
void psy_audio_instruments_insertgroup(psy_audio_Instruments*,
	psy_audio_InstrumentsGroup*, uintptr_t groupslot);
void psy_audio_instruments_removegroup(psy_audio_Instruments*,
	uintptr_t groupslot);
psy_audio_InstrumentsGroup* psy_audio_instruments_group_at(psy_audio_Instruments*,
	uintptr_t groupslot);
#ifdef __cplusplus
}
#endif

#endif /* psy_audio_INSTRUMENTS_H */
