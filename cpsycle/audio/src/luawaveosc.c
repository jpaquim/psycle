// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2020 members of the psycle project http://psycle.sourceforge.net

#include "luawaveosc.h"

#include "waveosc.h"
#include <stdlib.h>

#include <lauxlib.h>
#include <lualib.h>

#include "psyclescript.h"
#include "luaarray.h"

#include "../../detail/stdint.h"

static const char* luawaveosc_meta = "psyoscmeta";
static int luawaveosc_create(lua_State*);
static int luawaveosc_gc(lua_State*);
static int luawaveosc_work(lua_State*);

int psy_audio_luabind_waveosc_open(lua_State *L)
{
	static const char* const e[] = { "SIN", "SAW", "SQR", "TRI", "PWM", "RND" };
	static const char* const f[] =
	{ "ZEROHOLD", "LINEAR", "SPLINE", "SINC", "SOXR" };
	static const luaL_Reg methods[] = {
		{"new", luawaveosc_create},
		{"work", luawaveosc_work},
		{NULL, NULL}
	};
	psyclescript_open(L, luawaveosc_meta, methods, 
		luawaveosc_gc, 0);
	psyclescript_buildenum(L, e, sizeof(e) / sizeof(e[0]), 1);
	psyclescript_buildenum(L, f, sizeof(f) / sizeof(f[0]), 1);
  return 1;
}

int luawaveosc_create(lua_State* L)
{	
	int n = lua_gettop(L);  // Number of arguments
	if (n != 2 && n != 3) {
		return luaL_error(L,
			"Got %d arguments expected 2[,3] (self, shape [, f])",
			n);
	} else {
		int shape;
		psy_audio_WaveOsc* osc;

		shape = (int) luaL_checkinteger(L, 2);
		osc = (psy_audio_WaveOsc*) malloc(sizeof(psy_audio_WaveOsc));
		psy_audio_waveosc_init(osc, (psy_audio_WaveShape) shape);
		psyclescript_createuserdata(L, 1, luawaveosc_meta, osc);
		return 1;
	}
	return 0;
}

int luawaveosc_gc(lua_State* L)
{
	psy_audio_WaveOsc** ud = (psy_audio_WaveOsc**)
		luaL_checkudata(L, 1, luawaveosc_meta);	
	psy_audio_waveosc_dispose(*ud);	
	free(*ud);
	return 0;
}

int luawaveosc_work(lua_State* L)
{
	int n;
	
	n = lua_gettop(L);
	if (n == 2 || n == 3 || n == 4 || n == 5) {
		psy_audio_WaveOsc* self;
		psy_audio_Array* buffer;
		float* fm = 0;
		float* env = 0;

		self = psyclescript_checkself(L, 1, luawaveosc_meta);
		buffer = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);		
		// float* pwm = 0;
		//if (n > 2 && (!lua_isnil(L, 3))) {
		//	psy_audio_Array* arr = *(psy_audio_Array**)luaL_checkudata(L, 3, luaarraybind_meta);
		//	fm = arr->data();
		//}
		//if (n > 3 && (!lua_isnil(L, 4))) {
		//	psy_audio_Array* arr = *(psy_audio_Array**)luaL_checkudata(L, 4, luaarraybind_meta);
		//	env = arr->data();
		//}
		// check for master
		//std::auto_ptr<LuaSingleWorker> master;
		//lua_getfield(L, 1, "sync");
		//if (lua_isnil(L, -1)) {
		//	lua_pop(L, 1);
		//}
		//else {
		//	lua_pushvalue(L, 1);
		//	master.reset(new LuaSingleWorker(L));
		//}
		psy_audio_waveosc_work(self, psy_audio_array_len(buffer), psy_audio_array_data(buffer));
		lua_pushinteger(L, psy_audio_array_len(buffer)); // return processed samples
	} else {
		luaL_error(L, "Got %d arguments expected 2 or 3 (self, num, fm)", n);
	}
	return 1;
}