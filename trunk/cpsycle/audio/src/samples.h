// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLES_H)
#define SAMPLES_H

#include "sample.h"
#include <hashtbl.h>
#include <signal.h>

#define NOSAMPLES_INDEX UINTPTR_MAX

typedef struct {
	uintptr_t slot;
	uintptr_t subslot;
} SampleIndex;

SampleIndex sampleindex_make(uintptr_t slot, uintptr_t subslot);

typedef struct {
	char* name;
	Table container;
} SamplesGroup;

typedef struct Samples {
	Table groups;
	psy_Signal signal_insert;
	psy_Signal signal_removed;
} Samples;

void samples_init(Samples*);
void samples_dispose(Samples*);
void samples_insert(Samples*, Sample* sample, SampleIndex);
void samples_remove(Samples*, SampleIndex);
Sample* samples_at(Samples*, SampleIndex);
uintptr_t samples_groupsize(Samples*);

TableIterator samples_begin(Samples*);
TableIterator samples_groupbegin(Samples*, uintptr_t slot);

#endif
