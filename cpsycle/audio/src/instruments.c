// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instruments.h"
#include <stdlib.h>

void instruments_init(psy_audio_Instruments* self)
{
	psy_table_init(&self->container);
	self->slot = 0;		
	psy_signal_init(&self->signal_insert);
	psy_signal_init(&self->signal_removed);
	psy_signal_init(&self->signal_slotchange);	
}

void instruments_dispose(psy_audio_Instruments* self)
{	
	psy_TableIterator it;

	for (it = psy_table_begin(&self->container);
			!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
		psy_audio_Instrument* instrument;
		
		instrument = (psy_audio_Instrument*)psy_tableiterator_value(&it);
		instrument_dispose(instrument);
		free(instrument);
	}
	psy_table_dispose(&self->container);
	psy_signal_dispose(&self->signal_insert);
	psy_signal_dispose(&self->signal_removed);
	psy_signal_dispose(&self->signal_slotchange);
}

void instruments_insert(psy_audio_Instruments* self, psy_audio_Instrument* instrument, uintptr_t slot)
{
	psy_table_insert(&self->container, slot, instrument);
	instrument_setindex(instrument, slot);
	psy_signal_emit(&self->signal_insert, self, 1, slot);
}

void instruments_remove(psy_audio_Instruments* self, uintptr_t slot)
{
	psy_audio_Instrument* instrument;
	
	instrument = instruments_at(self, slot);
	if (instrument) {
		psy_table_remove(&self->container, slot);
		instrument_dispose(instrument);
		free(instrument);
		psy_signal_emit(&self->signal_removed, self, 1, slot);
	}
}

void instruments_changeslot(psy_audio_Instruments* self, uintptr_t slot)
{
	self->slot = slot;	
	psy_signal_emit(&self->signal_slotchange, self, 1, slot);
}

uintptr_t instruments_slot(psy_audio_Instruments* self)
{
	return self->slot;
}

psy_audio_Instrument* instruments_at(psy_audio_Instruments* self, uintptr_t slot)
{
	return psy_table_at(&self->container, slot);
}

size_t instruments_size(psy_audio_Instruments* self)
{
	return psy_table_size(&self->container);
}
