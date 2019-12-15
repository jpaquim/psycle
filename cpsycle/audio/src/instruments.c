// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instruments.h"
#include <stdlib.h>

void instruments_init(Instruments* self)
{
	table_init(&self->container);
	self->slot = 0;		
	psy_signal_init(&self->signal_insert);
	psy_signal_init(&self->signal_removed);
	psy_signal_init(&self->signal_slotchange);
}

void instruments_dispose(Instruments* self)
{	
	TableIterator it;

	for (it = table_begin(&self->container);
			!tableiterator_equal(&it, table_end()); tableiterator_inc(&it)) {
		Instrument* instrument;
		
		instrument = (Instrument*)tableiterator_value(&it);
		instrument_dispose(instrument);
		free(instrument);
	}
	table_dispose(&self->container);
	psy_signal_dispose(&self->signal_insert);
	psy_signal_dispose(&self->signal_removed);
	psy_signal_dispose(&self->signal_slotchange);
}

void instruments_insert(Instruments* self, Instrument* instrument, uintptr_t slot)
{
	table_insert(&self->container, slot, instrument);
	instrument_setindex(instrument, slot);
	psy_signal_emit(&self->signal_insert, self, 1, slot);
}

void instruments_remove(Instruments* self, uintptr_t slot)
{
	Instrument* instrument;
	
	instrument = instruments_at(self, slot);
	if (instrument) {
		table_remove(&self->container, slot);
		instrument_dispose(instrument);
		free(instrument);
		psy_signal_emit(&self->signal_removed, self, 1, slot);
	}
}

void instruments_changeslot(Instruments* self, uintptr_t slot)
{
	self->slot = slot;	
	psy_signal_emit(&self->signal_slotchange, self, 1, slot);
}

uintptr_t instruments_slot(Instruments* self)
{
	return self->slot;
}

Instrument* instruments_at(Instruments* self, uintptr_t slot)
{
	return table_at(&self->container, slot);
}

size_t instruments_size(Instruments* self)
{
	return table_size(&self->container);
}
