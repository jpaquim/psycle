// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "instruments.h"

void instruments_init(Instruments* self)
{
	InitIntHashTable(&self->container, 256);
	self->slot = 0;		
	signal_init(&self->signal_insert);
	signal_init(&self->signal_slotchange);
}

void instruments_dispose(Instruments* self)
{
	signal_dispose(&self->signal_insert);
	signal_dispose(&self->signal_slotchange);
	DisposeIntHashTable(&self->container);
}

void instruments_insert(Instruments* self, Instrument* instrument, int slot)
{
	InsertIntHashTable(&self->container, slot, instrument);
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
	return SearchIntHashTable(&self->container, slot);
}



