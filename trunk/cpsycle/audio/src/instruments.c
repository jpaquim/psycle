// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instruments.h"
#include <stdlib.h>

void instruments_init(Instruments* self)
{
	table_init(&self->container);
	self->slot = 0;		
	signal_init(&self->signal_insert);
	signal_init(&self->signal_slotchange);
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
	signal_dispose(&self->signal_insert);
	signal_dispose(&self->signal_slotchange);
}

void instruments_insert(Instruments* self, Instrument* instrument, int slot)
{
	table_insert(&self->container, slot, instrument);
	signal_emit(&self->signal_insert, self, 1, slot);
}

void instruments_changeslot(Instruments* self, int slot)
{
	self->slot = slot;	
	signal_emit(&self->signal_slotchange, self, 1, slot);
}

int instruments_slot(Instruments* self)
{
	return self->slot;
}

Instrument* instruments_at(Instruments* self, int slot)
{
	return table_at(&self->container, slot);
}
