// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSYCLESCRIPT_H)
#define PSYCLESCRIPT_H

#include "machineinfo.h"
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

typedef struct {
	int id;
	lua_State* L;
} PsycleScript;

int psyclescript_init(PsycleScript*);
int psyclescript_load(PsycleScript*, const char* path);
int psyclescript_preparestate(PsycleScript* self, const luaL_Reg methods[],
	void* host);
int psyclescript_run(PsycleScript*);
int psyclescript_start(PsycleScript*);
int psyclescript_dispose(PsycleScript*);

int psyclescript_machineinfo(PsycleScript*, MachineInfo*);
int psyclescript_open(lua_State*, const char* meta, const luaL_Reg methods[],
	lua_CFunction gc, lua_CFunction tostring);
void psyclescript_require(PsycleScript*, const char* name, lua_CFunction openf);
void psyclescript_register_weakuserdata(lua_State*, void* ud);

#endif