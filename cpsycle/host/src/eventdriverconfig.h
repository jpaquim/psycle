/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(EVENTDRIVERCONFIG_H)
#define EVENTDRIVERCONFIG_H

/* audio */
#include <player.h>
/* container */
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

/*
** EventDriverConfig
**
** Configures and selects the eventdrivers
*/

typedef struct EventDriverConfig {	
	psy_Property* eventinputs;
	psy_Property* eventdriverconfigure;
	psy_Property* eventdriverconfigurations;
	/* internal */
	psy_Property* installeddriver;
	psy_Property* activedrivers;
	/* references */
	psy_audio_Player* player;
	psy_Property* config;	
} EventDriverConfig;

void eventdriverconfig_init(EventDriverConfig*, psy_Property* parent, psy_audio_Player*);
void eventdriverconfig_dispose(EventDriverConfig*);

void eventdriverconfig_register_event_drivers(EventDriverConfig*);
void eventdriverconfig_make(EventDriverConfig*);
void eventdriverconfig_show_active(EventDriverConfig*, intptr_t deviceid);
void eventdriverconfig_update_active(EventDriverConfig*);
void eventdriverconfig_config(EventDriverConfig*);
psy_EventDriver* eventdriverconfig_selected_driver(EventDriverConfig*);
intptr_t eventdriverconfig_current(EventDriverConfig*);
void eventdriverconfig_read(EventDriverConfig*);

#ifdef __cplusplus
}
#endif

#endif /* EVENTDRIVERCONFIG_H */
