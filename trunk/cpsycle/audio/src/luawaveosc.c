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
#include "../../detail/portable.h"

static const char* luawaveosc_meta = "psyoscmeta";

static int luawaveosc_create(lua_State*);
static int luawaveosc_gc(lua_State*);
static int luawaveosc_tostring(lua_State*);
static int luawaveosc_work(lua_State*);
static int luawaveosc_getbasefrequency(lua_State*);
static int luawaveosc_setbasefrequency(lua_State*);
static int luawaveosc_stop(lua_State*);
static int luawaveosc_start(lua_State*);
static int luawaveosc_isplaying(lua_State*);
static int luawaveosc_setgain(lua_State*);
static int luawaveosc_gain(lua_State*);
static int luawaveosc_setpw(lua_State*);
static int luawaveosc_pw(lua_State*);
static int luawaveosc_setshape(lua_State*);
static int luawaveosc_shape(lua_State*);
static int luawaveosc_setquality(lua_State*);
static int luawaveosc_quality(lua_State*);
static int luawaveosc_setsync(lua_State*);
static int luawaveosc_setsyncfadeout(lua_State*);
static int luawaveosc_phase(lua_State*);
static int luawaveosc_setphase(lua_State*);
static int luawaveosc_setpm(lua_State*);
static int luawaveosc_setam(lua_State*);
static int luawaveosc_setfm(lua_State*);
static int luawaveosc_setpwm(lua_State*);

int psy_audio_luabind_waveosc_open(lua_State *L)
{
	static const char* const e[] = { "SIN", "SAW", "SQR", "TRI", "PWM", "RND" };
	static const char* const f[] =
	{ "ZEROHOLD", "LINEAR", "SPLINE", "SINC", "SOXR" };
	static const luaL_Reg methods[] = {
		{"new", luawaveosc_create},		
		{"work", luawaveosc_work},
		{"frequency", luawaveosc_getbasefrequency},
		{"setfrequency", luawaveosc_setbasefrequency},
		{"setgain", luawaveosc_setgain},
		{"gain", luawaveosc_gain},
		{"setpw", luawaveosc_setpw},
		{"pw", luawaveosc_pw},
		{"work", luawaveosc_work},
		{"stop", luawaveosc_stop},
		{"start", luawaveosc_start},
		{"tostring", luawaveosc_tostring},
		{"isplaying", luawaveosc_isplaying},
		{"setshape", luawaveosc_setshape},
		{"shape", luawaveosc_shape},
		{"setquality", luawaveosc_setquality},
		{"quality", luawaveosc_setquality},
		{"setsync", luawaveosc_setsync},
		{"setsyncfadeout", luawaveosc_setsyncfadeout},
		{"phase", luawaveosc_phase},
		{"setphase", luawaveosc_setphase},
		{"setpm", luawaveosc_setpm},
		{"setam", luawaveosc_setam},
		{"setfm", luawaveosc_setfm},
		{"setpwm", luawaveosc_setpwm},
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
		int f;
		psy_audio_WaveOsc* osc;

		shape = (int) luaL_checkinteger(L, 2);
		if (n == 3) {
			f = (int) luaL_checknumber(L, 3);
		} else {
			f = 440;
		}
		osc = (psy_audio_WaveOsc*) malloc(sizeof(psy_audio_WaveOsc));
		psy_audio_waveosc_init(osc, (psy_audio_WaveShape) shape, f);
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
		if (n > 2 && (!lua_isnil(L, 3))) {
			psy_audio_Array* arr = *(psy_audio_Array**)luaL_checkudata(L, 3, luaarraybind_meta);
			fm = psy_audio_array_data(arr);
			psy_audio_waveosc_setfm(self, fm);
		}
		if (n > 3 && (!lua_isnil(L, 4))) {
			psy_audio_Array* arr = *(psy_audio_Array**)luaL_checkudata(L, 4, luaarraybind_meta);
			env = psy_audio_array_data(arr); 
			psy_audio_waveosc_setam(self, env);
		}
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
		psy_audio_waveosc_work(self, psy_audio_array_len(buffer),
			psy_audio_array_data(buffer));
		lua_pushinteger(L, psy_audio_array_len(buffer)); // return processed samples
		//psy_audio_waveosc_setfm(self, NULL);
		//psy_audio_waveosc_setam(self, NULL);
	} else {
		luaL_error(L, "Got %d arguments expected 2 or 3 (self, num, fm)", n);
	}	
	return 1;
}

int luawaveosc_getbasefrequency(lua_State* L)
{
	psy_audio_WaveOsc* self;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	lua_pushnumber(L, psy_audio_waveosc_basefrequency(self));
	return 1;
}

int luawaveosc_setbasefrequency(lua_State* L)
{
	psy_audio_WaveOsc* self;
	double frequency;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	frequency = luaL_checknumber(L, 2);
	psy_audio_waveosc_setfrequency(self, frequency);
	return psyclescript_chaining(L);
}

int luawaveosc_start(lua_State* L)
{
	psy_audio_WaveOsc* self;
	lua_Number phase = 0.0;
	int n;

	n = lua_gettop(L);	
	self = psyclescript_checkself(L, 1, luawaveosc_meta);	
	if (n == 2) {
		phase = luaL_checknumber(L, 2);
	}
	psy_audio_waveosc_start(self, phase);
	return psyclescript_chaining(L);
}

int luawaveosc_stop(lua_State* L)
{
	psy_audio_WaveOsc* self;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	psy_audio_waveosc_stop(self, 0);
	return psyclescript_chaining(L);
}

int luawaveosc_isplaying(lua_State* L)
{
	psy_audio_WaveOsc* self;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	lua_pushboolean(L, psy_audio_waveosc_playing(self));
	return 1;
}

int luawaveosc_setgain(lua_State* L)
{
	psy_audio_WaveOsc* self;
	lua_Number gain;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	gain = luaL_checknumber(L, 2);
	psy_audio_waveosc_setgain(self, gain);
	return psyclescript_chaining(L);
}

int luawaveosc_gain(lua_State* L)
{
	psy_audio_WaveOsc* self;
	
	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	lua_pushnumber(L, psy_audio_waveosc_gain(self));
	return 1;
}

int luawaveosc_setpw(lua_State* L)
{
	return psyclescript_chaining(L);
}

int luawaveosc_pw(lua_State* L)
{
	psy_audio_WaveOsc* self;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);

	return 1;
}

int luawaveosc_setshape(lua_State* L)
{
	psy_audio_WaveOsc* self;
	int shape;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	shape = (int)luaL_checkinteger(L, 2);
	psy_audio_waveosc_setshape(self, (psy_audio_WaveShape)shape);
	return psyclescript_chaining(L);
}

int luawaveosc_shape(lua_State* L)
{
	psy_audio_WaveOsc* self;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	lua_pushnumber(L, psy_audio_waveosc_shape(self));
	return 1;
}

int luawaveosc_setquality(lua_State* L)
{	
	psy_audio_WaveOsc* self;
	int quality;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	quality = (int)luaL_checkinteger(L, 2);
	psy_audio_waveosc_setquality(self, quality);
	return psyclescript_chaining(L);
}

int luawaveosc_quality(lua_State* L)
{
	psy_audio_WaveOsc* self;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	lua_pushinteger(L, psy_audio_waveosc_quality(self));
	return 1;
}

int luawaveosc_setsync(lua_State* L)
{
	return psyclescript_chaining(L);
}

int luawaveosc_setsyncfadeout(lua_State* L)
{
	return psyclescript_chaining(L);
}

int luawaveosc_phase(lua_State* L)
{
	psy_audio_WaveOsc* self;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	lua_pushnumber(L, 0.0);
	return 1;
}

int luawaveosc_setphase(lua_State* L)
{
	return psyclescript_chaining(L);
}

int luawaveosc_setpm(lua_State* L)
{
	return psyclescript_chaining(L);
}

int luawaveosc_setam(lua_State* L)
{
	psy_audio_WaveOsc* self;
	psy_audio_Array* am;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	if (lua_isnil(L, 2)) {
		psy_audio_waveosc_setam(self, NULL);
	} else {
		am = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
		psy_audio_waveosc_setam(self, psy_audio_array_data(am));
	}
	return psyclescript_chaining(L);
}

int luawaveosc_setfm(lua_State* L)
{
	psy_audio_WaveOsc* self;
	psy_audio_Array* fm;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	if (lua_isnil(L, 2)) {
		psy_audio_waveosc_setam(self, NULL);
	} else {
		fm = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
		psy_audio_waveosc_setfm(self, psy_audio_array_data(fm));
	}
	return psyclescript_chaining(L);
}

int luawaveosc_setpwm(lua_State* L)
{
	psy_audio_WaveOsc* self;
	psy_audio_Array* pm;

	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	if (lua_isnil(L, 2)) {
		psy_audio_waveosc_setam(self, NULL);
	} else {
		pm = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
		psy_audio_waveosc_setpm(self, psy_audio_array_data(pm));
	}
	return psyclescript_chaining(L);
}

int luawaveosc_tostring(lua_State* L)
{
	psy_audio_WaveOsc* self;
	char text[256];
	
	self = psyclescript_checkself(L, 1, luawaveosc_meta);
	psy_snprintf(text, 256, "f=", (int)self->basefrequency);
	lua_pushfstring(L, text);
	return 1;
}