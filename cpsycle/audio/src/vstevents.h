// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_VSTEVENTS_H
#define psy_audio_VSTEVENTS_H

// local
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AEffect;
struct VstEvents;
struct VstEvent;

typedef struct psy_audio_VstEvents {
	struct VstEvents* events;
	uintptr_t eventcap;
	uintptr_t counter;
} psy_audio_VstEvents;

void psy_audio_vstevents_init(psy_audio_VstEvents*, uintptr_t capacity);
void psy_audio_vstevents_dispose(psy_audio_VstEvents*);
void psy_audio_vstevents_append(psy_audio_VstEvents*, struct VstEvent*);
void psy_audio_vstevents_append_midi_control(psy_audio_VstEvents*,
	uint8_t channel, uint8_t cc, uint8_t data);
void psy_audio_vstevents_append_noteon(psy_audio_VstEvents*,
	uint8_t channel, uint8_t note);
void psy_audio_vstevents_append_noteoff(psy_audio_VstEvents*,
	uint8_t channel, uint8_t note);
void psy_audio_vstevents_append_midi(psy_audio_VstEvents*,
	uint8_t byte0, uint8_t byte1, uint8_t byte2);
void psy_audio_vstevents_clear(psy_audio_VstEvents*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_VSTEVENTS_H */
