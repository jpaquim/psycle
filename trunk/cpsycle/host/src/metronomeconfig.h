/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(METRONOMECONFIG_H)
#define METRONOMECONFIG_H

/* audio */
#include <machinefactory.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MetronomeConfig */

enum {
	PROPERTY_ID_METRONOME = 60000,
	PROPERTY_ID_SHOWMETRONOME
};

struct psy_audio_Player;

typedef struct MetronomeConfig {
	/* signals */
	psy_Signal signal_changed;
	psy_Property* metronome;
	/* references */
	psy_Property* parent;
	struct psy_audio_Player* player;
} MetronomeConfig;

void metronomeconfig_init(MetronomeConfig*, psy_Property* parent,
	struct psy_audio_Player*);
void metronomeconfig_dispose(MetronomeConfig*);

bool metronomeconfig_showmetronomebar(const MetronomeConfig*);
uint8_t metronomeconfig_note(const MetronomeConfig* self);
uintptr_t metronomeconfig_machine(const MetronomeConfig*);

uintptr_t metronomeconfig_onchanged(MetronomeConfig*, psy_Property*);
bool metronomeconfig_hasproperty(const MetronomeConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* METRONOMECONFIG_H */
