// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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
	psy_Table container;
} SamplesGroup;

typedef struct psy_audio_Samples {
	psy_Table groups;
	psy_Signal signal_insert;
	psy_Signal signal_removed;
} psy_audio_Samples;

void samples_init(psy_audio_Samples*);
void samples_dispose(psy_audio_Samples*);
void samples_insert(psy_audio_Samples*, psy_audio_Sample* sample, SampleIndex);
void samples_remove(psy_audio_Samples*, SampleIndex);
psy_audio_Sample* samples_at(psy_audio_Samples*, SampleIndex);
uintptr_t samples_groupsize(psy_audio_Samples*);

psy_TableIterator samples_begin(psy_audio_Samples*);
psy_TableIterator samples_groupbegin(psy_audio_Samples*, uintptr_t slot);

#endif
