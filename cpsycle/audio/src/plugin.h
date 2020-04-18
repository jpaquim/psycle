// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PLUGIN_H
#define psy_audio_PLUGIN_H

#include "custommachine.h"
#include "library.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_PluginMachineParam;

typedef struct psy_audio_PluginMachineParam {
	psy_audio_MachineParam machineparam;
	struct CMachineInterface* mi;
	struct CMachineInfo* cinfo;	
	uintptr_t index;
} psy_audio_PluginMachineParam;

void psy_audio_pluginmachineparam_init(psy_audio_PluginMachineParam*, struct CMachineInterface* mi,
	struct CMachineInfo* info,
	uintptr_t index);
void psy_audio_pluginmachineparam_dispose(psy_audio_PluginMachineParam*);
psy_audio_PluginMachineParam* psy_audio_pluginmachineparam_alloc(void);

struct psy_audio_Plugin;

typedef struct psy_audio_Plugin {
	psy_audio_CustomMachine custommachine;	
	struct CMachineInterface* mi;
	psy_Library library;	
	psy_audio_MachineInfo* plugininfo;
	unsigned int preventsequencerlinetick;
	psy_Table parameters;
} psy_audio_Plugin;

void psy_audio_plugin_init(psy_audio_Plugin*, MachineCallback, const char* path);
int psy_audio_plugin_psycle_test(const char* path, psy_audio_MachineInfo*);

INLINE psy_audio_Machine* psy_audio_plugin_base(psy_audio_Plugin* self)
{
	return &(self->custommachine.machine);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PLUGIN_H */
