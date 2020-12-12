// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(COMPATIBILITYCONFIG_H)
#define COMPATIBILITYCONFIG_H

// audio
#include <machinefactory.h>

#ifdef __cplusplus
extern "C" {
#endif

// CompatConfig

typedef struct CompatConfig {
	psy_Property* compatibility;
	// references
	psy_Property* parent;
	psy_audio_MachineFactory* machinefactory;
} CompatConfig;

void compatconfig_init(CompatConfig*, psy_Property* parent,
	psy_audio_MachineFactory*);

void compatconfig_setloadnewblitz(CompatConfig*, bool mode);
bool compatconfig_loadnewblitz(const CompatConfig*);

#ifdef __cplusplus
}
#endif

#endif /* COMPATIBILITYCONFIG_H */
