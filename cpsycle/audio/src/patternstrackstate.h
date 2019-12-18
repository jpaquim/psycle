// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNSTRACKSTATE_H)
#define PATTERNSTRACKSTATE_H

#include <hashtbl.h>

typedef struct {
	Table mute;	
	Table record;
	int soloactive;
	uintptr_t soloedtrack;	
} PatternsTrackState;

void patternstrackstate_init(PatternsTrackState*);
void patternstrackstate_dispose(PatternsTrackState*);
void patternstrackstate_activatesolotrack(PatternsTrackState*, uintptr_t track);
void patternstrackstate_deactivatesolotrack(PatternsTrackState*);
void patternstrackstate_mutetrack(PatternsTrackState*, uintptr_t track);
void patternstrackstate_unmutetrack(PatternsTrackState*, uintptr_t track);
int patternstrackstate_istrackmuted(PatternsTrackState*, uintptr_t track);
int patternstrackstate_istracksoloed(PatternsTrackState*, uintptr_t track);

#endif
