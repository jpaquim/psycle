// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2020 members of the psycle project http://psycle.sourceforge.net

#include "luafilter.h"

#include "array.h"
#include "luaarray.h"

#include <filter.h>
#include <stdlib.h>

#include <lauxlib.h>
#include <lualib.h>

#include "psyclescript.h"

#include "../../detail/stdint.h"
#include "../../detail/portable.h"

static const char* luafilter_meta = "psyfiltermeta";
static int luafilter_create(lua_State* L);
static int luafilter_gc(lua_State* L);
static int luafilter_setcutoff(lua_State* L);
static int luafilter_cutoff(lua_State* L);
static int luafilter_setresonance(lua_State* L);
static int luafilter_resonance(lua_State* L);
static int luafilter_setfiltertype(lua_State* L);
static int luafilter_work(lua_State* L);
static int luafilter_tostring(lua_State* L);

// lua machine bind
int psy_audio_luabind_filter_open(lua_State *L)
{
	static const luaL_Reg methods[] = {
		{"new", luafilter_create},
		{"work", luafilter_work},
		{"setcutoff", luafilter_setcutoff},
		{"cutoff", luafilter_cutoff},
		{"setresonance", luafilter_setresonance},
		{"resonance", luafilter_resonance},
		{"settype", luafilter_setfiltertype},
		{"tostring", luafilter_tostring},
		{NULL, NULL}
	};
	static const char* const e[] =
	{ "LOWPASS", "HIGHPASS", "BANDPASS", "BANDREJECT", "NONE", "ITLOWPASS",
	  "MPTLOWPASS", "MPTHIGHPASS", "LOWPASS12E", "HIGHPASS12E", "BANDPASS12E",
	  "BANDREJECT12E" };

	psyclescript_open(L, luafilter_meta, methods,
		luafilter_gc, 0);
	psyclescript_buildenum(L, e, sizeof(e) / sizeof(e[0]), 0);
  return 1;
}

int luafilter_create(lua_State* L)
{	
	int n;
	int self = 1;	
	Filter** ud;	
	
	lua_pushvalue(L, self);
	n = lua_gettop(L);
	luaL_checktype(L, -1, LUA_TTABLE);  // self
	lua_newtable(L);  // new
	lua_pushvalue(L, self);
	lua_setmetatable(L, -2);
	lua_pushvalue(L, self);
	lua_setfield(L, self, "__index");
	ud = (void*) lua_newuserdata(L, sizeof(Filter*));	
	*ud = malloc(sizeof(Filter));			
	filter_init(*ud);	
	luaL_getmetatable(L, luafilter_meta);
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "__self");
	lua_remove(L, n);
	psyclescript_register_weakuserdata(L, ud);
	return 1;
}

int luafilter_gc(lua_State* L)
{
	Filter** ud = (Filter**)
		luaL_checkudata(L, 1, luafilter_meta);	
	filter_dispose(*ud);	
	free(*ud);
	return 0;
}

int luafilter_work(lua_State* L)
{
	int n = lua_gettop(L);
	if (n == 2 || n == 3 || n == 4) {
		float* vcfc = 0;
		float* vcfr = 0;
		Filter* filter;
		psy_audio_Array* x_input;
		uintptr_t num;
		float* data;
		uintptr_t i;

		filter = psyclescript_checkself(L, 1, luafilter_meta);
		x_input = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
		if (n > 2) {
			psy_audio_Array* arr = *(psy_audio_Array**)luaL_checkudata(L, 3, luaarraybind_meta);
			vcfc = psy_audio_array_data(arr);
		}
		if (n > 3) {
			psy_audio_Array* arr = *(psy_audio_Array**)luaL_checkudata(L, 3, luaarraybind_meta);
			vcfr = psy_audio_array_data(arr);
		}
		num = psy_audio_array_len(x_input);
		data = psy_audio_array_data(x_input);
		for (i = 0; i < num; ++i) {
			if (vcfc) {
				filter_setcutoff(filter, (int)psy_min(psy_max(0.0f, vcfc[i]), 127.0f));
			}
			if (vcfr) {
				filter_setressonance(filter, (int)psy_min(psy_max(0.0f, vcfr[i]), 127.0f));
			}
			data[i] = filter->vtable->work(filter, data[i]);
		}
		lua_pushvalue(L, 2);
	} else {
		luaL_error(L,
			"Got %d arguments expected 2 or 3(self, array filter input [, voltage control])", n);
	}
	return 1;
}

int luafilter_setcutoff(lua_State* L)
{
	Filter* filter;
	int cutoff;

	filter = psyclescript_checkself(L, 1, luafilter_meta);
	cutoff = (int)luaL_checkinteger(L, 2);
	filter_setcutoff(filter, cutoff);	
	return psyclescript_chaining(L);
}

int luafilter_cutoff(lua_State* L)
{
	Filter* filter;

	filter = psyclescript_checkself(L, 1, luafilter_meta);
	lua_pushinteger(L, filter_cutoff(filter));
	return 1;
}

int luafilter_setresonance(lua_State* L)
{
	Filter* filter;
	int q;

	filter = psyclescript_checkself(L, 1, luafilter_meta);
	q = (int)luaL_checkinteger(L, 2);
	filter_setressonance(filter, q);
	return psyclescript_chaining(L);	
}

int luafilter_resonance(lua_State* L) {
	Filter* filter;

	filter = psyclescript_checkself(L, 1, luafilter_meta);
	lua_pushinteger(L, filter_ressonance(filter));
	return 1;
}

int luafilter_setfiltertype(lua_State* L) {
	Filter* filter;
	int q;

	filter = psyclescript_checkself(L, 1, luafilter_meta);
	q = (int)luaL_checkinteger(L, 2);
	filter_settype(filter, (psy_dsp_FilterType) q);
	return psyclescript_chaining(L);
}

int luafilter_tostring(lua_State* L)
{
	Filter* filter;
	char text[256];

	filter = psyclescript_checkself(L, 1, luafilter_meta);
	psy_snprintf(text, 256, "c=%d", filter_cutoff(filter));	
	lua_pushfstring(L, text);
	return 1;
}