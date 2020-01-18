// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PATTERNS_H
#define psy_audio_PATTERNS_H

#include <hashtbl.h>
#include <list.h>
#include "pattern.h"
#include "patternstrackstate.h"

typedef struct {	
	psy_Table slots;
	uintptr_t songtracks;
	unsigned char sharetracknames;
	psy_audio_PatternsTrackState trackstate;
} psy_audio_Patterns;

void patterns_init(psy_audio_Patterns*);
void patterns_dispose(psy_audio_Patterns*);
void patterns_insert(psy_audio_Patterns*,uintptr_t slot, psy_audio_Pattern*);
int patterns_append(psy_audio_Patterns*, psy_audio_Pattern*);
psy_audio_Pattern* patterns_at(psy_audio_Patterns*, uintptr_t slot);
void patterns_clear(psy_audio_Patterns*);
void patterns_erase(psy_audio_Patterns*, uintptr_t slot);
void patterns_remove(psy_audio_Patterns*, uintptr_t slot);
uintptr_t patterns_size(psy_audio_Patterns*);
void patterns_activatesolotrack(psy_audio_Patterns*, uintptr_t track);
void patterns_deactivatesolotrack(psy_audio_Patterns*);
void patterns_mutetrack(psy_audio_Patterns*, uintptr_t track);
void patterns_unmutetrack(psy_audio_Patterns* self, uintptr_t track);
int patterns_istrackmuted(psy_audio_Patterns*, uintptr_t track);
int patterns_istracksoloed(psy_audio_Patterns*, uintptr_t track);
void patterns_setsongtracks(psy_audio_Patterns*, uintptr_t trackcount);
uintptr_t patterns_songtracks(psy_audio_Patterns*);

#endif /* psy_audio_PATTERNS_H */
