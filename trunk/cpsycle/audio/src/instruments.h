// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_INSTRUMENTS_H
#define psy_audio_INSTRUMENTS_H

#include "instrument.h"

#include <hashtbl.h>
#include <signal.h>

#include "../../detail/stdint.h"

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

typedef struct {
	char* name;
	psy_Table container;
} psy_audio_InstrumentsGroup;

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

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_INSTRUMENTS_H */
