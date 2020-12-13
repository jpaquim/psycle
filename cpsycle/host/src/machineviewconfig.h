// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEVIEWCONFIG_H)
#define MACHINEVIEWCONFIG_H

// container
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// MachineViewConfig

enum {
	PROPERTY_ID_DRAWVUMETERS = 6000,
};

typedef struct MachineViewConfig {
	psy_Property* machineview;
	psy_Property* theme;	
	// references
	psy_Property* parent;
	
} MachineViewConfig;

void machineviewconfig_init(MachineViewConfig*, psy_Property* parent);
void machineviewconfig_maketheme(MachineViewConfig* self, psy_Property* parent);
bool machineviewconfig_machineindexes(const MachineViewConfig*);
bool machineviewconfig_wirehover(const MachineViewConfig*);
bool machineviewconfig_vumeters(const MachineViewConfig*);

#ifdef __cplusplus
}
#endif

#endif /* MACHINEVIEWCONFIG_H */
