// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEVIEWCONFIG_H)
#define MACHINEVIEWCONFIG_H

// container
#include <properties.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

// MachineViewConfig

enum {
	PROPERTY_ID_DRAWVUMETERS = 6000,
};

typedef struct MachineViewConfig {
	// signals
	psy_Signal signal_changed;
	psy_Signal signal_themechanged;
	psy_Property* machineview;
	psy_Property* theme;	
	// references
	psy_Property* parent;	
} MachineViewConfig;

void machineviewconfig_init(MachineViewConfig*, psy_Property* parent);
void machineviewconfig_dispose(MachineViewConfig*);
void machineviewconfig_resettheme(MachineViewConfig*);
void machineviewconfig_settheme(MachineViewConfig*, psy_Property* skin);
bool machineviewconfig_hasthemeproperty(const MachineViewConfig*,
	psy_Property*);
bool machineviewconfig_hasproperty(const MachineViewConfig*, psy_Property*);

bool machineviewconfig_machineindexes(const MachineViewConfig*);
bool machineviewconfig_wirehover(const MachineViewConfig*);
bool machineviewconfig_vumeters(const MachineViewConfig*);
bool machineviewconfig_virtualgenerators(const MachineViewConfig*);

bool machineviewconfig_onchanged(MachineViewConfig*, psy_Property*);
bool machineviewconfig_onthemechanged(MachineViewConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* MACHINEVIEWCONFIG_H */
