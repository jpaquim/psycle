/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PSYCLESCRIPT_H
#define psy_audio_PSYCLESCRIPT_H

#include "machineinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../../detail/psydef.h"

typedef struct {
	int id;
	lua_State* L;
	char* modulepath;
	char* errstr;
} psy_audio_PsycleScript;

int psyclescript_init(psy_audio_PsycleScript*);
int psyclescript_load(psy_audio_PsycleScript*, const char* path);
int psyclescript_loadstring(psy_audio_PsycleScript*, const char* script);
int psyclescript_preparestate(psy_audio_PsycleScript*, const luaL_Reg methods[],
	void* host);
int psyclescript_run(psy_audio_PsycleScript*);
int psyclescript_start(psy_audio_PsycleScript*);
int psyclescript_dispose(psy_audio_PsycleScript*);

INLINE bool psyclescript_empty(psy_audio_PsycleScript* self)
{
	return self->L == NULL || self->modulepath == NULL;
}

INLINE const char* psyclescript_modulepath(psy_audio_PsycleScript* self)
{
	return self->modulepath;
}

int psyclescript_machineinfo(psy_audio_PsycleScript*, psy_audio_MachineInfo*);
int psyclescript_parse_machineinfo(psy_audio_PsycleScript*, psy_audio_MachineInfo*);
int psyclescript_open(lua_State*, const char* meta, const luaL_Reg methods[],
	lua_CFunction gc, lua_CFunction tostring);
void psyclescript_require(psy_audio_PsycleScript*, const char* name, lua_CFunction openf);
void psyclescript_register_weakuserdata(lua_State*, void* ud);
void* psyclescript_checkself(lua_State*, int index, const char* meta);
int psyclescript_createuserdata(lua_State*, int index, const char* meta, void* ud);
int psyclescript_buildenum(lua_State*, const char* const e[], int len, int startidx);
INLINE int psyclescript_chaining(lua_State* L)
{
	lua_pushvalue(L, 1);
	return 1;
}
void* psyclescript_host(lua_State*);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PSYCLESCRIPT_H */
