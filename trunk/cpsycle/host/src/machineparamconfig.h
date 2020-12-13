// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEPARAMCONFIG_H)
#define MACHINEPARAMCONFIG_H

// container
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// MachineParamConfig
//
enum {
	PROPERTY_ID_LOADCONTROLSKIN = 9000
};

typedef struct MachineParamConfig {
	psy_Property* paramview;
	psy_Property* paramtheme;	
	// references
	psy_Property* parent;	
} MachineParamConfig;

void machineparamconfig_init(MachineParamConfig*, psy_Property* parent);
void machineparamconfig_makeparamtheme(MachineParamConfig*, psy_Property* parent);

bool machineparamconfig_showfloated(const MachineParamConfig*);

#ifdef __cplusplus
}
#endif

#endif /* MACHINEPARAMCONFIG_H */
