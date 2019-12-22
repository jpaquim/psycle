// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrument.h"
#include "instruments.h"
#include <string.h>
#include <stdlib.h>

static void instrument_disposeentries(psy_audio_Instrument*);

void instrumententry_init(psy_audio_InstrumentEntry* self)
{	
	self->sampleindex = sampleindex_make(NOSAMPLES_INDEX,
		NOSAMPLES_INDEX);
	parameterrange_init(&self->keyrange, 0, NOTECOMMANDS_RELEASE - 1, 0,
		NOTECOMMANDS_RELEASE - 1);
	parameterrange_init(&self->velocityrange, 0, 0xFF, 0, 0xFF);
}

psy_audio_InstrumentEntry* instrumententry_alloc(void)
{
	return (psy_audio_InstrumentEntry*) malloc(sizeof(psy_audio_InstrumentEntry));
}

psy_audio_InstrumentEntry* instrumententry_allocinit(void)
{
	psy_audio_InstrumentEntry* rv;

	rv = instrumententry_alloc();
	if (rv) {
		instrumententry_init(rv);
	}
	return rv;
}

int instrumententry_intersect(psy_audio_InstrumentEntry* self, uintptr_t key,
	uintptr_t velocity)
{
	return parameterrange_intersect(&self->keyrange, key) &&
		   parameterrange_intersect(&self->velocityrange, velocity);
}

void instrument_init(psy_audio_Instrument* self)
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

void instrument_dispose(psy_audio_Instrument* self)
{
	instrument_disposeentries(self);
	free(self->name);
}

void instrument_disposeentries(psy_audio_Instrument* self)
{
	psy_List* p;

	for (p = self->entries; p != 0; p = p->next) {
		free(p->entry);
	}
	psy_list_free(self->entries);
	self->entries = 0;
}

psy_audio_Instrument* instrument_alloc(void)
{
	return (psy_audio_Instrument*) malloc(sizeof(psy_audio_Instrument));
}

psy_audio_Instrument* instrument_allocinit(void)
{
	psy_audio_Instrument* rv;

	rv = instrument_alloc();
	if (rv) {
		instrument_init(rv);
	}
	return rv;
}

void instrument_load(psy_audio_Instrument* self, const char* path)
{	
}

void instrument_setname(psy_audio_Instrument* self, const char* name)
{
	free(self->name);
	self->name = _strdup(name);
}

void instrument_setindex(psy_audio_Instrument* self, uintptr_t index)
{
	psy_audio_InstrumentEntry entry;

	instrumententry_init(&entry);	
	entry.sampleindex = sampleindex_make(index, 0);	
	instrument_addentry(self, &entry);
	self->index = index;
}

uintptr_t instrument_index(psy_audio_Instrument* self)
{
	return self->index;
}

const char* instrument_name(psy_audio_Instrument* self)
{
	return self->name;
}

void instrument_setnna(psy_audio_Instrument* self, psy_audio_NewNoteAction nna)
{
	self->nna = nna;
}

psy_audio_NewNoteAction instrument_nna(psy_audio_Instrument* self)
{
	return self->nna;
}

psy_List* instrument_entriesintersect(psy_audio_Instrument* self, uintptr_t key,
	uintptr_t velocity)
{
	psy_List* rv = 0;

	if (self->entries) {
		psy_List* p;
		
		for (p = self->entries; p != 0; p = p->next) {
			psy_audio_InstrumentEntry* entry;

			entry = (psy_audio_InstrumentEntry*) p->entry;
			if (instrumententry_intersect(entry, key, velocity)) {
				psy_list_append(&rv, entry);
			}			
		}		
	}
	return rv;
}

void instrument_addentry(psy_audio_Instrument* self, const psy_audio_InstrumentEntry* entry)
{	
	if (entry) {
		psy_audio_InstrumentEntry* newentry;

		newentry = instrumententry_alloc();
		*newentry = *entry;
		psy_list_append(&self->entries, newentry);
	}
}

void instrument_removeentry(psy_audio_Instrument* self, uintptr_t numentry)
{
	psy_List* node;

	node = psy_list_at(self->entries, numentry);
	if (node) {
		free(node->entry);
		psy_list_remove(&self->entries, node);
	}
}

void instrument_clearentries(psy_audio_Instrument* self)
{	
	instrument_disposeentries(self);	
}

psy_audio_InstrumentEntry* instrument_entryat(psy_audio_Instrument* self, uintptr_t numentry)
{	
	psy_audio_InstrumentEntry* rv = 0;
	psy_List* node;

	node = psy_list_at(self->entries, numentry);
	if (node) {
		rv = (psy_audio_InstrumentEntry*) node->entry;
	}	
	return rv;
}

const psy_List* instrument_entries(psy_audio_Instrument* self)
{
	return self->entries;
}

