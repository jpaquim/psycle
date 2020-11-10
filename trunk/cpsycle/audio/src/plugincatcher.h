// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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

typedef struct {
	psy_Property* plugins;
	char* inipath;
	char* nativeroot;
	psy_Property* dirconfig;
	psy_Signal signal_changed;
	psy_Signal signal_scanprogress;
} psy_audio_PluginCatcher;

void plugincatcher_init(psy_audio_PluginCatcher*, psy_Property* dirconfig);
void plugincatcher_dispose(psy_audio_PluginCatcher*);
void plugincatcher_clear(psy_audio_PluginCatcher*);
void plugincatcher_scan(psy_audio_PluginCatcher*);
int plugincatcher_load(psy_audio_PluginCatcher*);
void plugincatcher_save(psy_audio_PluginCatcher*);
/*const psy_audio_MachineInfo* plugincatcher_machineinfo(psy_audio_PluginCatcher*, MachineType,
	const char* name);*/
char* plugincatcher_modulepath(psy_audio_PluginCatcher*, MachineType,
	int newgamefxblitzifversionunknown,
	const char* path,
	char* fullpath);
uintptr_t plugincatcher_extractshellidx(const char* path);
void plugincatcher_catchername(psy_audio_PluginCatcher*, const char* path, char* name, uintptr_t shellidx);
const char* plugincatcher_searchpath(psy_audio_PluginCatcher*, const char* name,
	int type);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PLUGINCATCHER_H */
