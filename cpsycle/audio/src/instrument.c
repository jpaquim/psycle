// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrument.h"
#include "instruments.h"
#include <string.h>
#include <stdlib.h>

void psy_audio_frequencyrange_init(psy_audio_FrequencyRange* self,
	double low, double high, double min, double max)
{
	self->low = low;
	self->high = high;
	self->min = min;
	self->max = high;
}

int frequencyrange_intersect(psy_audio_FrequencyRange* self, double value)
{
	return value >= self->low && value <= self->high;
}

static void psy_audio_instrument_disposeentries(psy_audio_Instrument*);

void psy_audio_instrumententry_init(psy_audio_InstrumentEntry* self)
{	
	self->sampleindex = sampleindex_make(UINTPTR_MAX, UINTPTR_MAX);
	psy_audio_parameterrange_init(&self->keyrange, 0, psy_audio_NOTECOMMANDS_RELEASE - 1, 0,
		psy_audio_NOTECOMMANDS_RELEASE - 1);
	psy_audio_parameterrange_init(&self->velocityrange, 0, 0xFF, 0, 0xFF);
	psy_audio_frequencyrange_init(&self->freqrange, 0, 0, 0, 0);	
	self->use_keyrange = 1;
	self->use_velrange = 0;
	self->use_freqrange = 0;
}

psy_audio_InstrumentEntry* psy_audio_instrumententry_alloc(void)
{
	return (psy_audio_InstrumentEntry*) malloc(sizeof(psy_audio_InstrumentEntry));
}

psy_audio_InstrumentEntry* psy_audio_instrumententry_allocinit(void)
{
	psy_audio_InstrumentEntry* rv;

	rv = psy_audio_instrumententry_alloc();
	if (rv) {
		psy_audio_instrumententry_init(rv);
	}
	return rv;
}

int psy_audio_instrumententry_intersect(psy_audio_InstrumentEntry* self, uintptr_t key,
	uintptr_t velocity, double frequency)
{
	if (self->use_keyrange && !psy_audio_parameterrange_intersect(&self->keyrange, key)) {
		return 0;
	}
	if (self->use_velrange && !psy_audio_parameterrange_intersect(&self->velocityrange, velocity)) {
		return 0;
	}
	if (self->use_freqrange && !frequencyrange_intersect(&self->freqrange, frequency)) {
		return 0;
	}
	return 1;
}

void psy_audio_instrument_init(psy_audio_Instrument* self)
{	
	self->index = UINTPTR_MAX;
	self->entries = NULL;	
	self->name = strdup("");
	self->loop = FALSE;
	self->lines = 16;
	self->nna = psy_audio_NNA_STOP;
	self->globalvolume = (psy_dsp_amp_t) 1.0f;
	self->randompan = 0;
	self->filtertype = F_NONE;
	adsr_settings_initdefault(&self->volumeenvelope);
	adsr_settings_init(&self->filterenvelope, 0.005f, 0.370f, 0.5f, 0.370f);
	self->filtermodamount = 1.0f;
	self->filtercutoff = 1.f;
	self->filterres = 0.f;
	psy_signal_init(&self->signal_namechanged);
}

void psy_audio_instrument_dispose(psy_audio_Instrument* self)
{	
	psy_audio_instrument_disposeentries(self);
	psy_signal_dispose(&self->signal_namechanged);
	free(self->name);
}

void psy_audio_instrument_disposeentries(psy_audio_Instrument* self)
{
	psy_list_deallocate(&self->entries, (psy_fp_disposefunc)NULL);
}

psy_audio_Instrument* psy_audio_instrument_alloc(void)
{
	return (psy_audio_Instrument*) malloc(sizeof(psy_audio_Instrument));
}

psy_audio_Instrument* psy_audio_instrument_allocinit(void)
{
	psy_audio_Instrument* rv;

	rv = psy_audio_instrument_alloc();
	if (rv) {
		psy_audio_instrument_init(rv);
	}
	return rv;
}

void psy_audio_instrument_load(psy_audio_Instrument* self, const char* path)
{	
}

void psy_audio_instrument_setname(psy_audio_Instrument* self, const char* name)
{
	free(self->name);
	self->name = strdup(name);
	psy_signal_emit(&self->signal_namechanged, self, 0);
}

void psy_audio_instrument_setindex(psy_audio_Instrument* self, uintptr_t index)
{
	psy_audio_InstrumentEntry entry;

	psy_audio_instrumententry_init(&entry);	
	entry.sampleindex = sampleindex_make(index, 0);	
	psy_audio_instrument_addentry(self, &entry);
	self->index = index;
}

uintptr_t psy_audio_instrument_index(psy_audio_Instrument* self)
{
	return self->index;
}

const char* psy_audio_instrument_name(psy_audio_Instrument* self)
{
	return self->name;
}

void psy_audio_instrument_setnna(psy_audio_Instrument* self, psy_audio_NewNoteAction nna)
{
	self->nna = nna;
}

psy_audio_NewNoteAction psy_audio_instrument_nna(psy_audio_Instrument* self)
{
	return self->nna;
}

psy_List* psy_audio_instrument_entriesintersect(psy_audio_Instrument* self, uintptr_t key,
	uintptr_t velocity, double frequency)
{
	psy_List* rv = 0;

	if (self->entries) {
		psy_List* p;
		
		for (p = self->entries; p != NULL; psy_list_next(&p)) {
			psy_audio_InstrumentEntry* entry;

			entry = (psy_audio_InstrumentEntry*)psy_list_entry(p);
			if (psy_audio_instrumententry_intersect(entry, key, velocity, frequency)) {
				psy_list_append(&rv, entry);
			}			
		}		
	}
	return rv;
}

void psy_audio_instrument_addentry(psy_audio_Instrument* self, const psy_audio_InstrumentEntry* entry)
{	
	if (entry) {
		psy_audio_InstrumentEntry* newentry;

		newentry = psy_audio_instrumententry_alloc();
		*newentry = *entry;
		psy_list_append(&self->entries, newentry);
	}
}

void psy_audio_instrument_removeentry(psy_audio_Instrument* self, uintptr_t numentry)
{
	psy_List* node;

	node = psy_list_at(self->entries, numentry);
	if (node) {
		free(node->entry);
		psy_list_remove(&self->entries, node);
	}
}

void psy_audio_instrument_clearentries(psy_audio_Instrument* self)
{	
	psy_audio_instrument_disposeentries(self);	
}

psy_audio_InstrumentEntry* psy_audio_instrument_entryat(psy_audio_Instrument* self, uintptr_t numentry)
{	
	psy_audio_InstrumentEntry* rv = 0;
	psy_List* node;

	node = psy_list_at(self->entries, numentry);
	if (node) {
		rv = (psy_audio_InstrumentEntry*) node->entry;
	}	
	return rv;
}

const psy_List* psy_audio_instrument_entries(psy_audio_Instrument* self)
{
	return self->entries;
}

