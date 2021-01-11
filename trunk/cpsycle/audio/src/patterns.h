// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PATTERNS_H
#define psy_audio_PATTERNS_H

#include "pattern.h"
#include "patternstrackstate.h"

#include <hashtbl.h>
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {	
	psy_Table slots;
	uintptr_t songtracks;
	bool sharetracknames;
	psy_audio_TrackState trackstate;
	psy_Signal signal_namechanged;
	psy_Signal signal_lengthchanged;
} psy_audio_Patterns;

void psy_audio_patterns_init(psy_audio_Patterns*);
void psy_audio_patterns_dispose(psy_audio_Patterns*);
psy_audio_Pattern* psy_audio_patterns_insert(psy_audio_Patterns*,
	uintptr_t slot, psy_audio_Pattern*);
uintptr_t psy_audio_patterns_append(psy_audio_Patterns*, psy_audio_Pattern*);
psy_audio_Pattern* psy_audio_patterns_at(psy_audio_Patterns*, uintptr_t slot);

INLINE psy_TableIterator psy_audio_patterns_begin(psy_audio_Patterns* self)
{
	assert(self);

	return psy_table_begin(&self->slots);
}

void psy_audio_patterns_clear(psy_audio_Patterns*);
void psy_audio_patterns_erase(psy_audio_Patterns*, uintptr_t slot);
void psy_audio_patterns_remove(psy_audio_Patterns*, uintptr_t slot);
uintptr_t psy_audio_patterns_size(psy_audio_Patterns*);
void psy_audio_patterns_activatesolotrack(psy_audio_Patterns*, uintptr_t track);
void psy_audio_patterns_deactivatesolotrack(psy_audio_Patterns*);
void psy_audio_patterns_mutetrack(psy_audio_Patterns*, uintptr_t track);
void psy_audio_patterns_unmutetrack(psy_audio_Patterns*, uintptr_t track);
int psy_audio_patterns_istrackmuted(const psy_audio_Patterns*, uintptr_t track);
int psy_audio_patterns_istracksoloed(const psy_audio_Patterns*, uintptr_t track);
void psy_audio_patterns_setsongtracks(psy_audio_Patterns*, uintptr_t trackcount);
uintptr_t psy_audio_patterns_songtracks(psy_audio_Patterns*);
uintptr_t psy_audio_patterns_slot(psy_audio_Patterns*, psy_audio_Pattern*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PATTERNS_H */
