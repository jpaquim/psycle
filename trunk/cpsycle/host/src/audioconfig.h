// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(AUDIOCONFIG_H)
#define AUDIOCONFIG_H

// audio
#include <player.h>
// container
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	PROPERTY_ID_AUDIODRIVERS = 2000,
	PROPERTY_ID_ENABLEAUDIO
};

// AudioConfig
//
// Configures and selects the audiodriver

typedef struct AudioConfig {
	psy_Property* inputoutput;
	psy_Property* driverconfigure;
	psy_Property* driverconfigurations;
	// internal
	bool audioenabled;
	// references
	psy_audio_Player* player;
	psy_Property* parent;	
} AudioConfig;

void audioconfig_init(AudioConfig*, psy_Property* parent, psy_audio_Player*);
void audioconfig_enableaudio(AudioConfig*, bool enable);
void audioconfig_driverconfigure_section(AudioConfig*);
void audioconfig_onaudiodriverselect(AudioConfig*, bool enabled);
void audioconfig_oneditaudiodriverconfiguration(AudioConfig*, bool enabled);

bool audioconfig_onpropertychanged(AudioConfig*, psy_Property*);

const char* audioconfig_driverpath(AudioConfig*);
const char* audioconfig_driverkey(AudioConfig*);

#ifdef __cplusplus
}
#endif

#endif /* AUDIOCONFIG_H */
