// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_LUAPLUGIN_H
#define psy_audio_LUAPLUGIN_H

#include "custommachine.h"
#include "psyclescript.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_audio_Machine* machine;
	psy_audio_BufferContext* bc;
	uintptr_t numparameters_;
	int numcols_;
	int numprograms_;
	bool shared_;
	bool bcshared_;
} psy_audio_LuaMachine;

void psy_audio_luamachine_init(psy_audio_LuaMachine*);
void psy_audio_luamachine_init_shared(psy_audio_LuaMachine*,
	psy_audio_Machine*);
void psy_audio_luamachine_init_machine(psy_audio_LuaMachine*,
	psy_audio_Machine*);
void psy_audio_luamachine_dispose(psy_audio_LuaMachine*);

INLINE bool luamachine_shared(psy_audio_LuaMachine* self)
{
	return self->shared_;
}

INLINE void luamachine_setnumparameters(psy_audio_LuaMachine* self, uintptr_t num)
{
	self->numparameters_ = num;
}

INLINE uintptr_t luamachine_numparameters(psy_audio_LuaMachine* self)
{
	return self->numparameters_;
}

struct psy_audio_LuaPlugin;

typedef struct psy_audio_LuaPluginMachineParam {
	psy_audio_CustomMachineParam custommachineparam;
	struct psy_audio_LuaPlugin* plugin;
	uintptr_t index;
} psy_audio_LuaPluginMachineParam;

void psy_audio_luapluginmachineparam_init(psy_audio_LuaPluginMachineParam*, struct psy_audio_LuaPlugin* plugin,
	uintptr_t index);
void psy_audio_luapluginmachineparam_dispose(psy_audio_LuaPluginMachineParam*);
psy_audio_LuaPluginMachineParam* psy_audio_luapluginmachineparam_alloc(void);

struct psy_audio_Lock;

typedef struct psy_audio_LuaPlugin {
	psy_audio_CustomMachine custommachine;
	psy_audio_PsycleScript script;
	psy_audio_LuaMachine* client;
	psy_audio_MachineInfo* plugininfo;
	struct psy_audio_Lock* lock;
	psy_audio_LuaPluginMachineParam parameter;
	bool usenoteon;
} psy_audio_LuaPlugin;

void psy_audio_luaplugin_init(psy_audio_LuaPlugin*, psy_audio_MachineCallback*,
	const char* path);
int psy_audio_plugin_luascript_test(const char* path, psy_audio_MachineInfo*);

INLINE psy_audio_Machine* psy_audio_luaplugin_base(psy_audio_LuaPlugin* self)
{
	return &(self->custommachine.machine);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_LUAPLUGIN_H */
