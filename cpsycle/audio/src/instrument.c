// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrument.h"
// local
#include "instruments.h"
// platform
#include "../../detail/portable.h"

void psy_audio_frequencyrange_init(psy_audio_FrequencyRange* self,
	double low, double high, double min, double max)
{
	assert(self);

	self->low = low;
	self->high = high;
	self->min = min;
	self->max = high;
}

int frequencyrange_intersect(psy_audio_FrequencyRange* self, double value)
{
	assert(self);

	return value >= self->low && value <= self->high;
}

static void psy_audio_instrument_disposeentries(psy_audio_Instrument*);

void psy_audio_instrumententry_init(psy_audio_InstrumentEntry* self)
{	
	assert(self);

	self->sampleindex = psy_audio_sampleindex_make(psy_INDEX_INVALID, psy_INDEX_INVALID);
	self->fixedkey = psy_audio_NOTECOMMANDS_EMPTY; // disabled
	psy_audio_parameterrange_init(&self->keyrange,
		0, psy_audio_NOTECOMMANDS_RELEASE - 1,
		0, psy_audio_NOTECOMMANDS_RELEASE - 1);
	psy_audio_parameterrange_init(&self->velocityrange, 0, 0xFF, 0, 0xFF);
	psy_audio_frequencyrange_init(&self->freqrange, 0, 0, 0, 0);
	self->use_keyrange = TRUE;
	self->use_velrange = FALSE;
	self->use_freqrange = FALSE;
	self->use_loop = FALSE;	
	self->zone.tune = 0;
	self->zone.zoneset = psy_audio_ZONESET_NONE;
}

psy_audio_InstrumentEntry* psy_audio_instrumententry_alloc(void)
{
	return (psy_audio_InstrumentEntry*)malloc(sizeof(psy_audio_InstrumentEntry));
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

bool psy_audio_instrumententry_intersect(psy_audio_InstrumentEntry* self, uintptr_t key,
	uintptr_t velocity, double frequency)
{
	assert(self);

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

psy_audio_Sample* psy_audio_instrumententry_sample(psy_audio_InstrumentEntry* self,
	psy_audio_Samples* samples)
{
	return psy_audio_samples_at(samples, self->sampleindex);
}

int16_t psy_audio_instrumententry_tune(psy_audio_InstrumentEntry* self,
	psy_audio_Samples* samples)
{
	if ((self->zone.zoneset & psy_audio_ZONESET_TUNE) == psy_audio_ZONESET_TUNE) {
		return self->zone.tune;
	}
	if (samples) {
		psy_audio_Sample* sample;

		sample = psy_audio_samples_at(samples, self->sampleindex);
		if (sample) {
			return sample->zone.tune;
		}
	}
	return 0;
}

// psy_audio_Instrument
void psy_audio_instrument_init(psy_audio_Instrument* self)
{	
	assert(self);

	self->enabled = TRUE;
	self->name = strdup("");
	self->lines = 16;
	self->entries = NULL;		
	self->loop = FALSE;		
	self->globalvolume = (psy_dsp_amp_t)1.0f;
	self->volumefadespeed = 0.f;
	self->panenabled = FALSE;
	self->initpan = 0.5f;
	self->surround = FALSE;
	self->notemodpancenter = psy_audio_NOTECOMMANDS_MIDDLEC;
	self->notemodpansep = 0;	
	self->filtertype = F_NONE;
	self->randomvolume = 0.f;
	self->randompanning = 0.f;
	self->randomcutoff = 0.f;
	self->randomresonance = 0.f;

	self->nna = psy_audio_NNA_STOP;
	self->dct = psy_audio_DUPECHECK_NONE;
	self->dca = psy_audio_NNA_STOP;

	psy_dsp_envelope_init_adsr(&self->volumeenvelope);	
	psy_dsp_envelope_init_adsr(&self->filterenvelope);
	psy_dsp_envelope_settimeandvalue(&self->filterenvelope, 		
		1, 0.005f, 1.f);
	psy_dsp_envelope_settimeandvalue(&self->filterenvelope,
		2, 0.005f + 0.370f, 0.5f);
	psy_dsp_envelope_settimeandvalue(&self->filterenvelope,
		3, 0.005f + 0.370f + 0.370f, 0.0f);
	self->filtermodamount = 1.0f;
	self->filtercutoff = 1.f;	
	self->filterres = 0.f;	
	psy_dsp_envelope_init_adsr(&self->panenvelope);
	psy_dsp_envelope_init_adsr(&self->pitchenvelope);
	psy_signal_init(&self->signal_namechanged);
}

void psy_audio_instrument_dispose(psy_audio_Instrument* self)
{	
	assert(self);

	psy_dsp_envelope_init_dispose(&self->volumeenvelope);
	psy_dsp_envelope_init_dispose(&self->panenvelope);
	psy_dsp_envelope_init_dispose(&self->pitchenvelope);
	psy_dsp_envelope_init_dispose(&self->filterenvelope);	
	psy_audio_instrument_disposeentries(self);
	psy_signal_dispose(&self->signal_namechanged);
	free(self->name);
	self->name = NULL;
}

void psy_audio_instrument_disposeentries(psy_audio_Instrument* self)
{
	assert(self);

	psy_list_deallocate(&self->entries, (psy_fp_disposefunc)NULL);
}

psy_audio_Instrument* psy_audio_instrument_alloc(void)
{
	return (psy_audio_Instrument*)malloc(sizeof(psy_audio_Instrument));
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

void psy_audio_instrument_deallocate(psy_audio_Instrument* self)
{
	psy_audio_instrument_dispose(self);
	free(self);
}

void psy_audio_instrument_load(psy_audio_Instrument* self, const char* path)
{
	assert(self);
}

void psy_audio_instrument_setname(psy_audio_Instrument* self, const char* name)
{
	assert(self);

	psy_strreset(&self->name, name);	
	self->name = strdup(name);
	psy_signal_emit(&self->signal_namechanged, self, 0);
}

void psy_audio_instrument_setindex(psy_audio_Instrument* self, uintptr_t index)
{
	psy_audio_InstrumentEntry entry;

	assert(self);

	psy_audio_instrumententry_init(&entry);	
	entry.sampleindex = psy_audio_sampleindex_make(index, 0);	
	psy_audio_instrument_addentry(self, &entry);
}

const char* psy_audio_instrument_name(psy_audio_Instrument* self)
{
	assert(self);

	return self->name;
}

psy_List* psy_audio_instrument_entriesintersect(psy_audio_Instrument* self, uintptr_t key,
	uintptr_t velocity, double frequency)
{
	psy_List* rv;

	assert(self);

	rv = NULL;
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
	assert(self);

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

	assert(self);

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
	psy_List* node;

	assert(self);

	node = psy_list_at(self->entries, numentry);
	if (node) {
		return (psy_audio_InstrumentEntry*)psy_list_entry(node);
	}	
	return NULL;
}

const psy_List* psy_audio_instrument_entries(psy_audio_Instrument* self)
{
	assert(self);

	return self->entries;
}

const char* psy_audio_instrument_tostring(const psy_audio_Instrument* self)
{
	assert(self);

	return NULL;
}

void psy_audio_instrument_fromstring(psy_audio_Instrument* self, const char* str)
{
	assert(self);
}
