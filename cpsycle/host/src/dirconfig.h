// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(DIRCONFIG_H)
#define DIRCONFIG_H

// container
#include <properties.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

// DirConfig

typedef struct DirConfig {
	// signals
	psy_Signal signal_changed;
	psy_Property* directories;
	// references
	psy_Property* parent;
	
} DirConfig;

void dirconfig_init(DirConfig*, psy_Property* parent);
void dirconfig_dispose(DirConfig*);

const char* dirconfig_songs(const DirConfig*);
const char* dirconfig_samples(const DirConfig*);
const char* dirconfig_plugins32(const DirConfig*);
const char* dirconfig_plugins64(const DirConfig*);
const char* dirconfig_pluginscurrplatform(const DirConfig*);
const char* dirconfig_luascripts(const DirConfig*);
const char* dirconfig_vsts32(const DirConfig*);
const char* dirconfig_vsts64(const DirConfig*);
const char* dirconfig_ladspas(const DirConfig*);
const char* dirconfig_skins(const DirConfig*);
const char* dirconfig_doc(const DirConfig*);
const char* dirconfig_config(const DirConfig*);
const char* dirconfig_userpresets(const DirConfig*);

bool dirconfig_onchanged(DirConfig*, psy_Property*);
bool dirconfig_hasproperty(const DirConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* DIRCONFIG_H */
