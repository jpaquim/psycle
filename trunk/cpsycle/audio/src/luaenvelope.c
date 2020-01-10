// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2020 members of the psycle project http://psycle.sourceforge.net

#include "luaenvelope.h"

#include <envelope.h>
#include <stdlib.h>

#include <lauxlib.h>
#include <lualib.h>

#include "psyclescript.h"

#include "../../detail/stdint.h"

static const char* luaenvelope_meta = "psyenvelopemeta";
static int luaenvelope_create(lua_State* L);
static int luaenvelope_gc(lua_State* L);

// lua machine bind
int psy_audio_luabind_envelope_open(lua_State *L)
{
	static const luaL_Reg methods[] = {
		{"new", luaenvelope_create},
		{NULL, NULL}
	};
	psyclescript_open(L, luaenvelope_meta, methods, 
		luaenvelope_gc, 0);  
  return 1;
}

int luaenvelope_create(lua_State* L)
{	
	int n;
	int self = 1;	
	psy_dsp_Envelope** ud;	
	
	lua_pushvalue(L, self);
	n = lua_gettop(L);
	luaL_checktype(L, -1, LUA_TTABLE);  // self
	lua_newtable(L);  // new
	lua_pushvalue(L, self);
	lua_setmetatable(L, -2);
	lua_pushvalue(L, self);
	lua_setfield(L, self, "__index");
	ud = (void*) lua_newuserdata(L, sizeof(psy_dsp_Envelope*));	
	*ud = malloc(sizeof(psy_dsp_Envelope));			
	psy_dsp_envelope_init(*ud);	
	luaL_getmetatable(L, luaenvelope_meta);
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "__self");
	lua_remove(L, n);
	psyclescript_register_weakuserdata(L, ud);
	return 1;
}

int luaenvelope_gc(lua_State* L)
{
	psy_dsp_Envelope** ud = (psy_dsp_Envelope**)
		luaL_checkudata(L, 1, luaenvelope_meta);	
	psy_dsp_envelope_dispose(*ud);	
	free(*ud);
	return 0;
}
