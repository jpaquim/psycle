// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTRUMENTS_H)
#define INSTRUMENTS_H

#include "instrument.h"
#include <hashtbl.h>
#include <signal.h>

typedef struct Instruments {
	IntHashTable container;
	int slot;
	Signal signal_insert;
	Signal signal_slotchange;
} Instruments;

void instruments_init(Instruments*);
void instruments_dispose(Instruments*);
void instruments_insert(Instruments*, Instrument*, int slot);
void instruments_changeslot(Instruments* self, int slot);
int instruments_slot(Instruments* self);
Instrument* instruments_at(Instruments* self, int slot);

#endif