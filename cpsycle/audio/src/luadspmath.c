// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2020 members of the psycle project http://psycle.sourceforge.net

#include "luadspmath.h"

#include <math.h>
#include <stdlib.h>

#include <lauxlib.h>
#include <lualib.h>
#include "patternevent.h"

#include "psyclescript.h"

#include "../../detail/stdint.h"


static int freqtonote(lua_State*);
static int notetofreq(lua_State*);

int psy_audio_luabind_dspmath_open(lua_State* L)
{
	static const luaL_Reg funcs[] = {
	{"notetofreq", notetofreq},
	{"freqtonote", freqtonote},
	{NULL, NULL}
	};
	luaL_newlib(L, funcs);
	return 1;
}

int notetofreq(lua_State* L)
{
	double note = luaL_checknumber(L, 1);
	int n = lua_gettop(L);
	lua_Integer base = NOTECOMMANDS_MIDDLEA;
	if (n == 2) {
		base = luaL_checkinteger(L, 2);
	}
	lua_pushnumber(L, 440 * pow(2.0, (note - base) / 12.0));
	return 1;
}

int freqtonote(lua_State* L)
{
	double f = luaL_checknumber(L, 1);
	int n = lua_gettop(L);
	lua_Integer base = NOTECOMMANDS_MIDDLEA;
	double note;

	if (n == 2) {
		base = luaL_checkinteger(L, 2);
	}
	note = 12 * log10(f / 440.0) / log10(2.0) + base;
	lua_pushnumber(L, note);
	return 1;
}
