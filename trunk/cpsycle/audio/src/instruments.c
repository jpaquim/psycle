// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instruments.h"
#include <stdlib.h>

psy_audio_InstrumentIndex instrumentindex_make(uintptr_t slot, uintptr_t subslot)
{
	psy_audio_InstrumentIndex rv;

	rv.slot = slot;
	rv.subslot = subslot;
	return rv;
}

// InstrumentsGroup
void instrumentsgroup_init(psy_audio_InstrumentsGroup* self)
{
	psy_table_init(&self->container);
}

void instrumentsgroup_dispose(psy_audio_InstrumentsGroup* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->container);
		!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
		psy_audio_Instrument* instrument;

		instrument = (psy_audio_Instrument*)psy_tableiterator_value(&it);
		psy_audio_instrument_dispose(instrument);
		free(instrument);
	}
	psy_table_dispose(&self->container);
}

psy_audio_InstrumentsGroup* instrumentsgroup_alloc(void)
{
	return (psy_audio_InstrumentsGroup*)malloc(sizeof(psy_audio_InstrumentsGroup));
}

psy_audio_InstrumentsGroup* instrumentsgroup_allocinit(void)
{
	psy_audio_InstrumentsGroup* rv;

	rv = instrumentsgroup_alloc();
	if (rv) {
		instrumentsgroup_init(rv);
	}
	return rv;
}

void instrumentsgroup_insert(psy_audio_InstrumentsGroup* self, psy_audio_Instrument* instrument, uintptr_t slot)
{
	if (instrument) {
		psy_table_insert(&self->container, slot, instrument);
	}
}

void instrumentsgroup_remove(psy_audio_InstrumentsGroup* self, uintptr_t slot)
{
	psy_audio_Instrument* instrument;

	instrument = psy_table_at(&self->container, slot);
	if (instrument) {
		psy_table_remove(&self->container, slot);
		psy_audio_instrument_dispose(instrument);
		free(instrument);
	}
}

psy_audio_Instrument* instrumentsgroup_at(psy_audio_InstrumentsGroup* self, uintptr_t slot)
{
	return psy_table_at(&self->container, slot);
}

uintptr_t instrumentsgroup_size(psy_audio_InstrumentsGroup* self)
{
	return psy_table_size(&self->container);
}

// psy_audio_Instruments
void instruments_init(psy_audio_Instruments* self)
{
	psy_table_init(&self->groups);
	self->slot = instrumentindex_make(0, 0);
	psy_signal_init(&self->signal_insert);
	psy_signal_init(&self->signal_removed);
	psy_signal_init(&self->signal_slotchange);	
}

void instruments_dispose(psy_audio_Instruments* self)
{	
	psy_TableIterator it;

	for (it = psy_table_begin(&self->groups);
		!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
		psy_audio_InstrumentsGroup* group;

		group = (psy_audio_InstrumentsGroup*)psy_tableiterator_value(&it);
		instrumentsgroup_dispose(group);
		free(group);
	}
	psy_table_dispose(&self->groups);
	psy_signal_dispose(&self->signal_insert);
	psy_signal_dispose(&self->signal_removed);
	psy_signal_dispose(&self->signal_slotchange);
}

void instruments_insert(psy_audio_Instruments* self, psy_audio_Instrument* instrument,
	psy_audio_InstrumentIndex index)
{
	psy_audio_InstrumentsGroup* group;

	group = psy_table_at(&self->groups, index.slot);
	if (!group) {
		group = instrumentsgroup_allocinit();
		if (group) {
			psy_table_insert(&self->groups, index.slot, group);
		}
	}
	if (group) {
		instrumentsgroup_insert(group, instrument, index.subslot);
		psy_signal_emit(&self->signal_insert, self, 1, &index);
	}
}

void instruments_remove(psy_audio_Instruments* self, psy_audio_InstrumentIndex index)
{	
	psy_audio_InstrumentsGroup* group;

	group = psy_table_at(&self->groups, index.slot);
	if (group) {
		instrumentsgroup_remove(group, index.subslot);
		if (instrumentsgroup_size(group) == 0) {
			psy_table_remove(&self->groups, index.slot);
			instrumentsgroup_dispose(group);
			free(group);
			psy_signal_emit(&self->signal_removed, self, 1, &index);
		}
	}
}

void instruments_changeslot(psy_audio_Instruments* self, psy_audio_InstrumentIndex index)
{
	self->slot = index;	
	psy_signal_emit(&self->signal_slotchange, self, 1, &index);
}

psy_audio_InstrumentIndex instruments_slot(psy_audio_Instruments* self)
{
	return self->slot;
}

psy_audio_Instrument* instruments_at(psy_audio_Instruments* self,
	psy_audio_InstrumentIndex index)
{
	psy_audio_InstrumentsGroup* group;

	group = psy_table_at(&self->groups, index.slot);
	if (group) {
		return instrumentsgroup_at(group, index.subslot);
	}
	return NULL;
}

uintptr_t instruments_size(psy_audio_Instruments* self, uintptr_t slot)
{
	psy_audio_InstrumentsGroup* group;

	group = psy_table_at(&self->groups, slot);
	if (group) {
		return psy_table_size(&group->container);
	}
	return 0;
}

uintptr_t psy_audio_instruments_groupsize(psy_audio_Instruments* self)
{
	return psy_table_size(&self->groups);
}

psy_TableIterator psy_audio_instruments_begin(psy_audio_Instruments* self)
{
	return psy_table_begin(&self->groups);
}

psy_TableIterator psy_audio_instruments_groupbegin(psy_audio_Instruments* self,
	uintptr_t slot)
{
	psy_audio_InstrumentsGroup* group;

	group = psy_table_at(&self->groups, slot);
	if (group) {
		return psy_table_begin(&group->container);
	}
	return tableend;
}
