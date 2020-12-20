// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2020 members of the psycle project http://psycle.sourceforge.net

#include "luaenvelope.h"

#include "array.h"
#include "luaarray.h"

#include <envelope.h>
#include <stdlib.h>

#include <lauxlib.h>
#include <lualib.h>

#include "psyclescript.h"

#include "../../detail/stdint.h"

static const char* luaenvelope_meta = "psyenvelopemeta";
static int luaenvelope_create(lua_State* L);
static int luaenvelope_gc(lua_State* L);
static int work(lua_State* L);
static int release(lua_State* L);
static int start(lua_State* L);
static int isplaying(lua_State* L);
static int setpeak(lua_State* L);
static int peak(lua_State* L);
static int tostring(lua_State* L);
static int setstagetime(lua_State* L);
static int time(lua_State* L);
static int setstartvalue(lua_State* L);
static int lastvalue(lua_State* L);

// lua machine bind
int psy_audio_luabind_envelope_open(lua_State *L)
{
	static const luaL_Reg methods[] = {
		{"new", luaenvelope_create},
		{"work", work },
		{"release", release},
		{"isplaying", isplaying},
		{"start", start },
		{"setpeak", setpeak},
		{"setstartvalue", setstartvalue},
		{"lastvalue", lastvalue},
		{"peak", peak},
		{"settime", setstagetime},
		{"time", time},
		{"tostring", tostring},
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
	psy_dsp_Envelope* env;
	lua_Integer suspos;
	lua_Number startpeak;

	n = lua_gettop(L);  // Number of arguments
	if (n != 3) {
		return luaL_error(L,
			"Got %d arguments expected 3 (self, points, sustainpos)", n);
	}
	env = (psy_dsp_Envelope*)malloc(sizeof(psy_dsp_Envelope));
	if (!env) {
		return luaL_error(L, "Memory Error");
	}
	psy_dsp_envelope_init(env);
	if (lua_istable(L, 2)) {
		psy_dsp_EnvelopeSettings* settings;

		settings = &env->settings;
		lua_Unsigned len = lua_rawlen(L, 2);
		for (size_t i = 1; i <= len; ++i) {
			double t;
			double peak;
			lua_Unsigned argnum;

			lua_rawgeti(L, 2, i); // get point {time, peak}
			argnum = lua_rawlen(L, 4);
			lua_rawgeti(L, 4, 1); // get time
			t = luaL_checknumber(L, 5);
						
			lua_pop(L, 1);
			lua_rawgeti(L, 4, 2); // get peak
			peak = luaL_checknumber(L, 5);			
			lua_pop(L, 1);
			if (argnum == 3) {
				lua_rawgeti(L, 4, 3); // get type
				if (!lua_isnil(L, 5)) {
					lua_Integer type;

					type = luaL_checkinteger(L, 5);
				}
				lua_pop(L, 1);
			}
			psy_dsp_envelopesettings_append(settings,
				psy_dsp_envelopepoint_make_all((psy_dsp_seconds_t)t,
					(psy_dsp_amp_t)peak, 0, 0, 0, 1));
			lua_pop(L, 1);
		}
	}
	suspos = luaL_checkinteger(L, 3) - 1;
	startpeak = 0;
	if (n == 4) {
		startpeak = luaL_checknumber(L, 4);
	}
	env->startpeak = (psy_dsp_amp_t)startpeak;	
	env->settings.sustainbegin= (int)suspos;
	psyclescript_createuserdata(L, 1, luaenvelope_meta, env);	
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

int work(lua_State* L)
{
	int n = lua_gettop(L);
	if (n == 2) {		
		psy_dsp_Envelope* self;
		uintptr_t i;

		self = psyclescript_checkself(L, 1, luaenvelope_meta);
		uintptr_t num = (uintptr_t)luaL_checkinteger(L, 2);
		psy_audio_Array** rv = (psy_audio_Array**)lua_newuserdata(L, sizeof(psy_audio_Array*));
		*rv = malloc(sizeof(psy_audio_Array));
		psy_audio_array_init_len(*rv, num, 0.0);
		for (i = 0; i < num; ++i) {
			psy_dsp_amp_t value;

			value = psy_dsp_envelope_tick(self);
			psy_audio_array_set(*rv, i, value);
		}		
		luaL_setmetatable(L, luaarraybind_meta);
	} else {
		return luaL_error(L, "Got %d arguments expected 2 (self, num)", n);
	}
	return 1;
}

int release(lua_State* L)
{
	psy_dsp_Envelope* self;

	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	psy_dsp_envelope_release(self);
	return psyclescript_chaining(L);
}

int start(lua_State* L)
{
	psy_dsp_Envelope* self;

	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	psy_dsp_envelope_start(self);
	return psyclescript_chaining(L);
}

int isplaying(lua_State* L)
{
	psy_dsp_Envelope* self;
	bool playing;

	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	playing = psy_dsp_envelope_playing(self);
	lua_pushboolean(L, playing);
	return 1;
}

int setpeak(lua_State* L)
{
	psy_dsp_Envelope* self;
	psy_List* p;	
	lua_Integer stage;

	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	stage = luaL_checkinteger(L, 2);
	if (stage > 0) {
		p = psy_list_at(self->settings.points, (uintptr_t)stage - 1);
		if (p) {
			psy_dsp_EnvelopePoint* pt;
			lua_Number peak;

			pt = (psy_dsp_EnvelopePoint*)psy_list_entry(p);
			peak = luaL_checknumber(L, 3);
			pt->value = (psy_dsp_amp_t)peak;
		}
	}
	return psyclescript_chaining(L);
}

int peak(lua_State* L)
{
	lua_pushnumber(L, 0.0);
	return 1;
}

int tostring(lua_State* L)
{
	lua_pushfstring(L, "env");
	return 1;
}

int setstagetime(lua_State* L)
{
	psy_dsp_Envelope* self;
	psy_List* p;
	lua_Integer stage;

	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	stage = luaL_checkinteger(L, 2);
	if (stage >= 0) {
		p = psy_list_at(self->settings.points, (uintptr_t)stage - 1);
		if (p) {
			psy_dsp_EnvelopePoint* pt;
			lua_Number time;

			pt = (psy_dsp_EnvelopePoint*)psy_list_entry(p);
			time = luaL_checknumber(L, 3);
			pt->time = (psy_dsp_beat_t)time;
		}
	}
	return psyclescript_chaining(L);
}

int time(lua_State* L)
{
	lua_pushnumber(L, 0.0);
	return 1;
}

int setstartvalue(lua_State* L)
{
	return psyclescript_chaining(L);
}

int lastvalue(lua_State* L)
{
	lua_pushnumber(L, 0.0);
	return 1;
}
