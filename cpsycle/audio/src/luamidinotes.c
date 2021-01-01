// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2020 members of the psycle project http://psycle.sourceforge.net

#include "luamidinotes.h"

#include <math.h>
#include <stdlib.h>

#include <lauxlib.h>
#include <lualib.h>
#include "patternevent.h"
#include <notestab.h>

#include "psyclescript.h"

#include "../../detail/stdint.h"


static int midinotes_gmpercussionnames(lua_State* L);
static int midinotes_gmpercussionname(lua_State* L);
static int midinotes_notename(lua_State* L);
static int midinotes_combine(lua_State* L);

int psy_audio_luabind_midinotes_open(lua_State* L)
{
	static const luaL_Reg funcs[] = {
	{"notename", midinotes_notename},
	{"gmpercussionname", midinotes_gmpercussionname},
	{"gmpercussionnames", midinotes_gmpercussionnames},
	{"combine", midinotes_combine},
	{NULL, NULL}
	};
	luaL_newlib(L, funcs);
	return 1;
}

int midinotes_notename(lua_State* L)
{	
	uint8_t note;
	
	note = (uint8_t)luaL_checkinteger(L, 1);
	lua_pushstring(L, psy_dsp_notetostr(note, psy_dsp_NOTESTAB_A440));
	return 1;
}

int midinotes_gmpercussionnames(lua_State* L)
{
	int i;

	lua_createtable(L, 127, 0);
	for (i = 1; i < 128; ++i) {
		lua_pushstring(L, (i > 34 && i < 82)
			? psy_dsp_notetostr(i , psy_dsp_NOTESTAB_GMPERCUSSION)
			: "");
		lua_rawseti(L, -2, i);
	}
	return 1;
}

int midinotes_gmpercussionname(lua_State* L)
{
	int note;
	
	note = (uint8_t)luaL_checkinteger(L, 1);
	lua_pushstring(L, (note > 34 && note < 82)
		? psy_dsp_notetostr(note, psy_dsp_NOTESTAB_GMPERCUSSION)
		: "");
	return 1;
}

int midinotes_combine(lua_State* L)
{
	uint8_t lsb = (uint8_t)luaL_checkinteger(L, 1);
	uint8_t msb = (uint8_t)luaL_checkinteger(L, 2);
	uint16_t val = (msb << 7) | lsb;
	lua_pushnumber(L, val);
	return 1;
}
