// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PLUGINCATCHER_H
#define psy_audio_PLUGINCATCHER_H

#include "machinedefs.h"
#include "machineinfo.h"

#include <propertiesio.h>
#include <signal.h>
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_PluginCatcher {
	psy_Property* plugins;
	char* inipath;
	char* nativeroot;
	psy_Property* directories;
	psy_Signal signal_changed;
	psy_Signal signal_scanprogress;
	bool saveafterscan;
	bool hasplugincache;
} psy_audio_PluginCatcher;

void psy_audio_plugincatcher_init(psy_audio_PluginCatcher*);
void psy_audio_plugincatcher_dispose(psy_audio_PluginCatcher*);
void psy_audio_plugincatcher_clear(psy_audio_PluginCatcher*);
void psy_audio_plugincatcher_scan(psy_audio_PluginCatcher*);
int psy_audio_plugincatcher_load(psy_audio_PluginCatcher*);
int psy_audio_plugincatcher_save(psy_audio_PluginCatcher*);
void psy_audio_plugincatcher_setdirectories(psy_audio_PluginCatcher*, psy_Property*);
/*const psy_audio_MachineInfo* plugincatcher_machineinfo(psy_audio_PluginCatcher*, psy_audio_MachineType,
	const char* name);*/
char* psy_audio_plugincatcher_modulepath(psy_audio_PluginCatcher*, psy_audio_MachineType,
	int newgamefxblitzifversionunknown,
	const char* path,
	char* fullpath);
uintptr_t psy_audio_plugincatcher_extractshellidx(const char* path);
void psy_audio_plugincatcher_catchername(psy_audio_PluginCatcher*, const char* path,
	char* name, uintptr_t shellidx);
const char* psy_audio_plugincatcher_searchpath(psy_audio_PluginCatcher*, const char* name,
	int type);

INLINE bool psy_audio_plugincatcher_hascache(const psy_audio_PluginCatcher* self)
{
	assert(self);

	return self->hasplugincache;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PLUGINCATCHER_H */
