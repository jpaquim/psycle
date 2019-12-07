// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTRUMENTS_H)
#define INSTRUMENTS_H

#include "instrument.h"
#include <hashtbl.h>
#include <signal.h>
#include "../../detail/stdint.h"

#define NOINSTRUMENT_INDEX UINTPTR_MAX

typedef struct Instruments {
	Table container;
	uintptr_t slot;
	Signal signal_insert;
	Signal signal_removed;
	Signal signal_slotchange;
} Instruments;

void instruments_init(Instruments*);
void instruments_dispose(Instruments*);
void instruments_insert(Instruments*, Instrument*, uintptr_t slot);
void instruments_remove(Instruments*, uintptr_t slot);
void instruments_changeslot(Instruments*, uintptr_t slot);
uintptr_t instruments_slot(Instruments*);
Instrument* instruments_at(Instruments*, uintptr_t slot);
uintptr_t instruments_size(Instruments*);

#endif
