// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(LUAPLUGIN_H)
#define LUAPLUGIN_H

#include "custommachine.h"
#include "psyclescript.h"

typedef struct {
	CustomMachine custommachine;			
	PsycleScript script;
	Machine* client;
	MachineInfo* plugininfo;
} LuaPlugin;

void luaplugin_init(LuaPlugin*, MachineCallback, const char* path);
int plugin_luascript_test(const char* path, MachineInfo*);

#endif
