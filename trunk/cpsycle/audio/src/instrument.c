// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrument.h"
#include "instruments.h"
#include <string.h>
#include <stdlib.h>

static void instrument_disposeentries(Instrument*);

void instrumententry_init(InstrumentEntry* self)
{	
	self->sampleindex = sampleindex_make(NOSAMPLES_INDEX,
		NOSAMPLES_INDEX);
	parameterrange_init(&self->keyrange, 0, NOTECOMMANDS_RELEASE - 1, 0,
		NOTECOMMANDS_RELEASE - 1);
	parameterrange_init(&self->velocityrange, 0, 0xFF, 0, 0xFF);
}

InstrumentEntry* instrumententry_alloc(void)
{
	return (InstrumentEntry*) malloc(sizeof(InstrumentEntry));
}

InstrumentEntry* instrumententry_allocinit(void)
{
	InstrumentEntry* rv;

	rv = instrumententry_alloc();
	if (rv) {
		instrumententry_init(rv);
	}
	return rv;
}

int instrumententry_intersect(InstrumentEntry* self, uintptr_t key,
	uintptr_t velocity)
{
	return parameterrange_intersect(&self->keyrange, key) &&
		   parameterrange_intersect(&self->velocityrange, velocity);
}

void instrument_init(Instrument* self)
{	
	self->index = NOINSTRUMENT_INDEX;
	self->entries = 0;	
	self->name = _strdup("");
	self->nna = NNA_STOP;
	self->filtertype = F_NONE;
	adsr_settings_initdefault(&self->volumeenvelope);
	adsr_settings_init(&self->filterenvelope, 0.005f, 0.370f, 0.5f, 0.370f);
	self->filtermodamount = 1.0f;
}

void instrument_dispose(Instrument* self)
{
	instrument_disposeentries(self);
	free(self->name);
}

void instrument_disposeentries(Instrument* self)
{
	List* p;

	for (p = self->entries; p != 0; p = p->next) {
		free(p->entry);
	}
	list_free(self->entries);
	self->entries = 0;
}

Instrument* instrument_alloc(void)
{
	return (Instrument*) malloc(sizeof(Instrument));
}

Instrument* instrument_allocinit(void)
{
	Instrument* rv;

	rv = instrument_alloc();
	if (rv) {
		instrument_init(rv);
	}
	return rv;
}

void instrument_load(Instrument* self, const char* path)
{	
}

void instrument_setname(Instrument* self, const char* name)
{
	free(self->name);
	self->name = _strdup(name);
}

void instrument_setindex(Instrument* self, uintptr_t index)
{
	InstrumentEntry entry;

	instrumententry_init(&entry);	
	entry.sampleindex = sampleindex_make(index, 0);	
	instrument_addentry(self, &entry);
	self->index = index;
}

uintptr_t instrument_index(Instrument* self)
{
	return self->index;
}

const char* instrument_name(Instrument* self)
{
	return self->name;
}

void instrument_setnna(Instrument* self, NewNoteAction nna)
{
	self->nna = nna;
}

NewNoteAction instrument_nna(Instrument* self)
{
	return self->nna;
}

SampleIndex instrument_sample(Instrument* self, uintptr_t key,
	uintptr_t velocity)
{
	SampleIndex rv;

	if (self->entries) {
		List* p;

		p = self->entries;
		while (p != 0) {
			InstrumentEntry* entry;

			entry = (InstrumentEntry*) p->entry;
			if (instrumententry_intersect(entry, key, velocity)) {
				rv = entry->sampleindex;
				break;
			}
			p = p->next;
		}
		if (p == 0) {
			rv = sampleindex_make(self->index, 0);
		}
	}
	return rv;
}

void instrument_addentry(Instrument* self, const InstrumentEntry* entry)
{	
	if (entry) {
		InstrumentEntry* newentry;

		newentry = instrumententry_alloc();
		*newentry = *entry;
		list_append(&self->entries, newentry);
	}
}

