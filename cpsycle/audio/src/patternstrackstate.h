// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_TRACKSTATE_H
#define psy_audio_TRACKSTATE_H

#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_Table mute;	
	psy_Table record;
	bool soloactive;
	uintptr_t soloedtrack;	
} psy_audio_TrackState;

void psy_audio_trackstate_init(psy_audio_TrackState*);
void psy_audio_trackstate_dispose(psy_audio_TrackState*);
void psy_audio_trackstate_activatesolotrack(psy_audio_TrackState*, uintptr_t track);
void psy_audio_trackstate_setsolotrack(psy_audio_TrackState*, uintptr_t track);
void psy_audio_trackstate_deactivatesolotrack(psy_audio_TrackState*);
void psy_audio_trackstate_mutetrack(psy_audio_TrackState*, uintptr_t track);
void psy_audio_trackstate_unmutetrack(psy_audio_TrackState*, uintptr_t track);
bool psy_audio_trackstate_istrackmuted(const psy_audio_TrackState*, uintptr_t track);
bool psy_audio_trackstate_istracksoloed(const psy_audio_TrackState*, uintptr_t track);
uintptr_t psy_audio_trackstate_tracksoloed(const psy_audio_TrackState*);
void psy_audio_trackstate_armtrack(psy_audio_TrackState*, uintptr_t track);
void psy_audio_trackstate_unarmtrack(psy_audio_TrackState*, uintptr_t track);
bool psy_audio_trackstate_istrackarmed(const psy_audio_TrackState*, uintptr_t track);
uintptr_t psy_audio_trackstate_trackarmedcount(const psy_audio_TrackState*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_TRACKSTATE_H */
