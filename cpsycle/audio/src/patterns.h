// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNS_H)
#define PATTERNS_H

#include <hashtbl.h>
#include <list.h>
#include "pattern.h"
#include "patternstrackstate.h"

typedef struct {	
	Table slots;
	uintptr_t songtracks;
	unsigned char sharetracknames;
	PatternsTrackState trackstate;
} Patterns;

void patterns_init(Patterns*);
void patterns_dispose(Patterns*);
void patterns_insert(Patterns*,uintptr_t slot, Pattern*);
int patterns_append(Patterns*, Pattern*);
Pattern* patterns_at(Patterns*, uintptr_t slot);
void patterns_clear(Patterns*);
void patterns_erase(Patterns*, uintptr_t slot);
void patterns_remove(Patterns*, uintptr_t slot);
uintptr_t patterns_size(Patterns*);
void patterns_activatesolotrack(Patterns*, uintptr_t track);
void patterns_deactivatesolotrack(Patterns*);
void patterns_mutetrack(Patterns*, uintptr_t track);
void patterns_unmutetrack(Patterns* self, uintptr_t track);
int patterns_istrackmuted(Patterns*, uintptr_t track);
int patterns_istracksoloed(Patterns*, uintptr_t track);
void patterns_setsongtracks(Patterns*, uintptr_t trackcount);
uintptr_t patterns_songtracks(Patterns*);


#endif
