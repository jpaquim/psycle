// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SAMPLES_H
#define psy_audio_SAMPLES_H

#include "sample.h"

#include <hashtbl.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

// psy_audio_Samples, psy_audio_SamplesGroup
// 
// Aim: Container of samples with pregrouping for sampler, sampulse and ft2
// Structure:
// psy_audio_Samples <@>----- psy_audio_SamplesGroup <@>----- psy_audio_Sample
//                          *                               *
// SampleIndex
//
// aim: Address a sample inside psy_audio_Samples
//
// Pair of two indexes (slot; subslot)
// 1. slot:    selects a group
// 2. subslot: selects a slot inside a group pointing
//	           to a sample
// Example
//
// Samples
// Groups	Group Samples
// 0 Drums  0 Hi Hat
//          1 Bass
// 1 Piano  0 lower keys
//          1 middle keys
//          2 upper keys
// 2 Guitar 0 Guitar
//
// sampleindex_make(1, 2) addresses the sample 'upper keys'

typedef struct {
	uintptr_t slot;
	uintptr_t subslot;
} psy_audio_SampleIndex;

psy_audio_SampleIndex sampleindex_make(uintptr_t slot, uintptr_t subslot);

typedef struct {
	char* name;
	psy_Table container;
} psy_audio_SamplesGroup;

typedef struct psy_audio_Samples {
	psy_Table groups;
	psy_Signal signal_insert;
	psy_Signal signal_removed;
} psy_audio_Samples;

void psy_audio_samples_init(psy_audio_Samples*);
void psy_audio_samples_dispose(psy_audio_Samples*);
void psy_audio_samples_insert(psy_audio_Samples*, psy_audio_Sample*,
	psy_audio_SampleIndex);
void psy_audio_samples_remove(psy_audio_Samples*, psy_audio_SampleIndex);
psy_audio_Sample* psy_audio_samples_at(psy_audio_Samples*, psy_audio_SampleIndex);
uintptr_t psy_audio_samples_size(psy_audio_Samples*, uintptr_t slot);

uintptr_t psy_audio_samples_groupsize(psy_audio_Samples*);
psy_TableIterator psy_audio_samples_begin(psy_audio_Samples*);
psy_TableIterator psy_audio_samples_groupbegin(psy_audio_Samples*,
	uintptr_t slot);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SAMPLES_H */
