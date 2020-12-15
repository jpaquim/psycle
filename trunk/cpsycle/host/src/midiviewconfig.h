// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(MIDIVIEWCONFIG_H)
#define MIDIVIEWCONFIG_H

// audio
#include <player.h>
// container
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// MidiViewConfig
//

enum {
	PROPERTY_ID_ADDCONTROLLERMAP = 4000,
	PROPERTY_ID_REMOVECONTROLLERMAP
};

typedef struct MidiViewConfig {
	// signals
	psy_Signal signal_changed;
	psy_Property* controllers;
	// references
	psy_Property* parent;
	psy_audio_Player* player;
} MidiViewConfig;

void midiviewconfig_init(MidiViewConfig*, psy_Property* parent,
	psy_audio_Player* player);
void midiviewconfig_dispose(MidiViewConfig*);
void midiviewconfig_makecontrollers(MidiViewConfig*);
void midiviewconfig_makecontrollersave(MidiViewConfig*);

bool midiviewconfig_onchanged(MidiViewConfig*, psy_Property*);
bool midiviewconfig_hasproperty(const MidiViewConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* MIDIVIEWCONFIG_H */
