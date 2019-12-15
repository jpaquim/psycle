// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "samples.h"
#include <stdlib.h>

static void samplesgroup_init(SamplesGroup*);
static void samplesgroup_dispose(SamplesGroup*);
static SamplesGroup* samplesgroup_alloc(void);
static SamplesGroup* samplesgroup_allocinit(void);
static void samplesgroup_insert(SamplesGroup*, Sample* sample, uintptr_t slot);
static void samplesgroup_remove(SamplesGroup*, uintptr_t slot);
static Sample* samplesgroup_at(SamplesGroup*, uintptr_t slot);

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
	table_init(&self->container);
}

void samplesgroup_dispose(SamplesGroup* self)
{
	TableIterator it;

	for (it = table_begin(&self->container);
			!tableiterator_equal(&it, table_end()); tableiterator_inc(&it)) {
		Sample* sample;
		
		sample = (Sample*)tableiterator_value(&it);
		sample_dispose(sample);
		free(sample);
	}
	table_dispose(&self->container);	
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

void samplesgroup_insert(SamplesGroup* self, Sample* sample, uintptr_t slot)
{
	if (sample) {		
		table_insert(&self->container, slot, sample);
	}
}

void samplesgroup_remove(SamplesGroup* self, uintptr_t slot)
{
	Sample* sample;
	
	sample = table_at(&self->container, slot);
	if (sample) {
		table_remove(&self->container, slot);
		sample_dispose(sample);
		free(sample);
	}
}

Sample* samplesgroup_at(SamplesGroup* self, uintptr_t slot)
{
	return table_at(&self->container, slot);
}

uintptr_t samplesgroup_size(SamplesGroup* self)
{
	return table_size(&self->container);
}

// Samples
void samples_init(Samples* self)
{
	table_init(&self->groups);
}

void samples_dispose(Samples* self)
{	
	TableIterator it;

	for (it = table_begin(&self->groups);
			!tableiterator_equal(&it, table_end()); tableiterator_inc(&it)) {
		SamplesGroup* group;
		
		group = (SamplesGroup*) tableiterator_value(&it);
		samplesgroup_dispose(group);
		free(group);
	}	
	table_dispose(&self->groups);
}

void samples_insert(Samples* self, Sample* sample, SampleIndex index)
{
	SamplesGroup* group;

	group = table_at(&self->groups, index.slot);
	if (!group) {
		group = samplesgroup_allocinit();
		if (group) {
			table_insert(&self->groups, index.slot, group);
		}
	}
	if (group) {
		samplesgroup_insert(group, sample, index.subslot);
	}
}

void samples_remove(Samples* self, SampleIndex index)
{
	SamplesGroup* group;

	group = table_at(&self->groups, index.slot);
	if (group) {
		samplesgroup_remove(group, index.subslot);
		if (samplesgroup_size(group) == 0) {
			table_remove(&self->groups, index.slot);
			samplesgroup_dispose(group);
			free(group);
		}
	}	
}

Sample* samples_at(Samples* self, SampleIndex index)
{
	SamplesGroup* group;

	group = table_at(&self->groups, index.slot);
	if (group) {
		return samplesgroup_at(group, index.subslot);	
	}
	return 0;
}

uintptr_t samples_groupsize(Samples* self)
{		
	return table_size(&self->groups);
}

TableIterator samples_begin(Samples* self)
{
	return table_begin(&self->groups);
}

TableIterator samples_groupbegin(Samples* self, uintptr_t slot)
{
	SamplesGroup* group;

	group = table_at(&self->groups, slot);
	if (group) {
		return table_begin(&group->container);
	}
	return tableend;
}