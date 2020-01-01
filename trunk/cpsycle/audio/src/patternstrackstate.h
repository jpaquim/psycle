// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNSTRACKSTATE_H)
#define PATTERNSTRACKSTATE_H

#include <hashtbl.h>

typedef struct {
	psy_Table mute;	
	psy_Table record;
	int soloactive;
	uintptr_t soloedtrack;	
} psy_audio_PatternsTrackState;

void patternstrackstate_init(psy_audio_PatternsTrackState*);
void patternstrackstate_dispose(psy_audio_PatternsTrackState*);
void patternstrackstate_activatesolotrack(psy_audio_PatternsTrackState*, uintptr_t track);
void patternstrackstate_deactivatesolotrack(psy_audio_PatternsTrackState*);
void patternstrackstate_mutetrack(psy_audio_PatternsTrackState*, uintptr_t track);
void patternstrackstate_unmutetrack(psy_audio_PatternsTrackState*, uintptr_t track);
int patternstrackstate_istrackmuted(psy_audio_PatternsTrackState*, uintptr_t track);
int patternstrackstate_istracksoloed(psy_audio_PatternsTrackState*, uintptr_t track);

#endif
