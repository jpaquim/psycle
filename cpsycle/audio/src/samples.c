// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "samples.h"
#include <stdlib.h>

static void samplesgroup_init(SamplesGroup*);
static void samplesgroup_dispose(SamplesGroup*);
static SamplesGroup* samplesgroup_alloc(void);
static SamplesGroup* samplesgroup_allocinit(void);
static void samplesgroup_insert(SamplesGroup*, psy_audio_Sample* sample, uintptr_t slot);
static void samplesgroup_remove(SamplesGroup*, uintptr_t slot);
static psy_audio_Sample* samplesgroup_at(SamplesGroup*, uintptr_t slot);

SampleIndex sampleindex_make(uintptr_t slot, uintptr_t subslot)
{
	SampleIndex rv;

	rv.slot = slot;
	rv.subslot = subslot;
	return rv;
}

// SamplesGroup
void samplesgroup_init(SamplesGroup* self)
{
	psy_table_init(&self->container);	
}

void samplesgroup_dispose(SamplesGroup* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->container);
			!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
		psy_audio_Sample* sample;
		
		sample = (psy_audio_Sample*)psy_tableiterator_value(&it);
		sample_dispose(sample);
		free(sample);
	}
	psy_table_dispose(&self->container);
}

SamplesGroup* samplesgroup_alloc(void)
{	
	return (SamplesGroup*) malloc(sizeof(SamplesGroup));
}

SamplesGroup* samplesgroup_allocinit(void)
{	
	SamplesGroup* rv;

	rv = samplesgroup_alloc();
	if (rv) {
		samplesgroup_init(rv);
	}
	return rv;
}

void samplesgroup_insert(SamplesGroup* self, psy_audio_Sample* sample, uintptr_t slot)
{
	if (sample) {		
		psy_table_insert(&self->container, slot, sample);
	}
}

void samplesgroup_remove(SamplesGroup* self, uintptr_t slot)
{
	psy_audio_Sample* sample;
	
	sample = psy_table_at(&self->container, slot);
	if (sample) {
		psy_table_remove(&self->container, slot);
		sample_dispose(sample);
		free(sample);
	}
}

psy_audio_Sample* samplesgroup_at(SamplesGroup* self, uintptr_t slot)
{
	return psy_table_at(&self->container, slot);
}

uintptr_t samplesgroup_size(SamplesGroup* self)
{
	return psy_table_size(&self->container);
}

// psy_audio_Samples
void samples_init(psy_audio_Samples* self)
{
	psy_table_init(&self->groups);
	psy_signal_init(&self->signal_insert);
	psy_signal_init(&self->signal_removed);
}

void samples_dispose(psy_audio_Samples* self)
{	
	psy_TableIterator it;

	for (it = psy_table_begin(&self->groups);
			!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
		SamplesGroup* group;
		
		group = (SamplesGroup*) psy_tableiterator_value(&it);
		samplesgroup_dispose(group);
		free(group);
	}	
	psy_table_dispose(&self->groups);
	psy_signal_dispose(&self->signal_insert);
	psy_signal_dispose(&self->signal_removed);
}

void samples_insert(psy_audio_Samples* self, psy_audio_Sample* sample, SampleIndex index)
{
	SamplesGroup* group;

	group = psy_table_at(&self->groups, index.slot);
	if (!group) {
		group = samplesgroup_allocinit();
		if (group) {
			psy_table_insert(&self->groups, index.slot, group);
		}
	}
	if (group) {
		samplesgroup_insert(group, sample, index.subslot);		
		psy_signal_emit(&self->signal_insert, self, 1, &index);
	}
}

void samples_remove(psy_audio_Samples* self, SampleIndex index)
{
	SamplesGroup* group;

	group = psy_table_at(&self->groups, index.slot);
	if (group) {
		samplesgroup_remove(group, index.subslot);
		if (samplesgroup_size(group) == 0) {
			psy_table_remove(&self->groups, index.slot);
			samplesgroup_dispose(group);
			free(group);
			psy_signal_emit(&self->signal_removed, self, 1, &index);
		}
	}	
}

psy_audio_Sample* samples_at(psy_audio_Samples* self, SampleIndex index)
{
	SamplesGroup* group;

	group = psy_table_at(&self->groups, index.slot);
	if (group) {
		return samplesgroup_at(group, index.subslot);	
	}
	return 0;
}

uintptr_t samples_groupsize(psy_audio_Samples* self)
{		
	return psy_table_size(&self->groups);
}

psy_TableIterator samples_begin(psy_audio_Samples* self)
{
	return psy_table_begin(&self->groups);
}

psy_TableIterator samples_groupbegin(psy_audio_Samples* self, uintptr_t slot)
{
	SamplesGroup* group;

	group = psy_table_at(&self->groups, slot);
	if (group) {
		return psy_table_begin(&group->container);
	}
	return tableend;
}