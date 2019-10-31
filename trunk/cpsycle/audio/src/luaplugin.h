// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(PLUGIN_H)
#define PLUGIN_H

#include "machine.h"
#include <lua.h>

typedef struct {
	Machine machine;		
	lua_State* L;
} LuaPlugin;

void luaplugin_init(LuaPlugin* self, MachineCallback, const char* path);
MachineInfo* plugin_lua_test(const char* path);

#endif
