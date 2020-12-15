// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(EVENTDRIVERCONFIG_H)
#define EVENTDRIVERCONFIG_H

// audio
#include <player.h>
// container
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	PROPERTY_ID_ADDEVENTDRIVER = 1000,
	PROPERTY_ID_REMOVEEVENTDRIVER,
	PROPERTY_ID_EVENTDRIVERCONFIGDEFAULTS,
	PROPERTY_ID_EVENTDRIVERCONFIGLOAD,
	PROPERTY_ID_EVENTDRIVERCONFIGKEYMAPSAVE,
	PROPERTY_ID_ACTIVEEVENTDRIVERS
};

// EventDriverConfig
//
// Configures and selects the eventdrivers

typedef struct EventDriverConfig {
	// signals
	psy_Signal signal_changed;
	psy_Property* eventinputs;
	psy_Property* eventdriverconfigure;
	psy_Property* eventdriverconfigurations;
	// internal data
	psy_Property* installeddriver;
	psy_Property* activedrivers;
	// references
	psy_audio_Player* player;
	psy_Property* config;	
} EventDriverConfig;

void eventdriverconfig_init(EventDriverConfig*, psy_Property* parent, psy_audio_Player*);
void eventdriverconfig_dispose(EventDriverConfig*);
void eventdriverconfig_makeeventinput(EventDriverConfig*);
void eventdriverconfig_makeeventdriverlist(EventDriverConfig*);
void eventdriverconfig_registereventdrivers(EventDriverConfig*);
void eventdriverconfig_makeeventdriverconfigurations(EventDriverConfig*);
void eventdriverconfig_showactiveeventdriverconfig(EventDriverConfig*, int deviceid);
void eventdriverconfig_updateactiveeventdriverlist(EventDriverConfig*);
void eventdriverconfig_configeventdrivers(EventDriverConfig*);
psy_EventDriver* eventdriverconfig_selectedeventdriver(EventDriverConfig*);
void eventdriverconfig_reseteventdriverconfiguration(EventDriverConfig*);
void eventdriverconfig_loadeventdriverconfiguration(EventDriverConfig*);
void eventdriverconfig_saveeventdriverconfiguration(EventDriverConfig*);
int eventdriverconfig_curreventdriverconfiguration(EventDriverConfig*);
const char* eventdriverconfig_eventdriverpath(EventDriverConfig*);
void eventdriverconfig_readeventdriverconfigurations(EventDriverConfig*);
psy_EventDriver* eventdriverconfig_selectedeventdriver(EventDriverConfig*);

bool eventdriverconfig_onchanged(EventDriverConfig*, psy_Property*);
bool eventdriverconfig_hasproperty(const EventDriverConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* EVENTDRIVERCONFIG_H */
