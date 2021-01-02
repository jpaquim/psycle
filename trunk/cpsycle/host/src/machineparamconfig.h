// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEPARAMCONFIG_H)
#define MACHINEPARAMCONFIG_H

// container
#include <properties.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

// MachineParamConfig

enum {
	PROPERTY_ID_LOADCONTROLSKIN = 9000,
	PROPERTY_ID_DEFAULTCONTROLSKIN
};

typedef struct MachineParamConfig {
	// signals
	psy_Signal signal_changed;
	psy_Signal signal_themechanged;
	// internal data
	psy_Property* paramview;
	psy_Property* theme;	
	// references
	psy_Property* parent;
} MachineParamConfig;

void machineparamconfig_init(MachineParamConfig*, psy_Property* parent);
void machineparamconfig_dispose(MachineParamConfig*);

void machineparamconfig_resettheme(MachineParamConfig*);
void machineparamconfig_settheme(MachineParamConfig*, psy_Property* skin);
bool machineparamconfig_hasthemeproperty(const MachineParamConfig*,
	psy_Property*);
bool machineparamconfig_hasproperty(const MachineParamConfig*, psy_Property*);

bool machineparamconfig_showfloated(const MachineParamConfig*);
const char* machineparamconfig_dialbpm(const MachineParamConfig*);
void machineparamconfig_setdialbpm(MachineParamConfig*, const char* filename);

bool machineparamconfig_onchanged(MachineParamConfig*, psy_Property*);
bool machineparamconfig_onthemechanged(MachineParamConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* MACHINEPARAMCONFIG_H */
