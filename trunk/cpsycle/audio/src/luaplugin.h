// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_LUAPLUGIN_H
#define psy_audio_LUAPLUGIN_H

#include "custommachine.h"
#include "psyclescript.h"

typedef struct {
	psy_audio_Machine* machine;
	psy_audio_BufferContext* bc;
} psy_audio_LuaMachine;

typedef struct {
	psy_audio_CustomMachine custommachine;
	psy_audio_PsycleScript script;
	psy_audio_LuaMachine* client;
	psy_audio_MachineInfo* plugininfo;
} psy_audio_LuaPlugin;

void psy_audio_luaplugin_init(psy_audio_LuaPlugin*, MachineCallback,
	const char* path);
int psy_audio_plugin_luascript_test(const char* path, psy_audio_MachineInfo*);

#endif /* psy_audio_LUAPLUGIN_H */
