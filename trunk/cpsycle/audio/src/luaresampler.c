/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2007-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "luaresampler.h"
/* local */
#include "luaarray.h"
#include "psyclescript.h"
#include "sample.h"
#include "waveosc.h"
/* lua */
#include <lauxlib.h>
#include <lualib.h>
/* platform */
#include "../../detail/portable.h"


void luaresampler_init(LuaResampler* self)
{
	self->rsvd = 0;
}

void luaresampler_dispose(LuaResampler* self)
{	
}

int luaresampler_work(LuaResampler* self, int numSamples, float* pSamplesL,
	float* pSamplesR, void* master)
{
	return numSamples;
}


static const char* luaresampler_meta = "psyresamplermeta";

static int create(lua_State*);
static int createwavetable(lua_State*);
static int gc(lua_State*);
static int tostring(lua_State*);
static int work(lua_State*);
static int noteoff(lua_State*);
static int isplaying(lua_State*);
static int start(lua_State*);
static int stop(lua_State*);
static int set_frequency(lua_State*);
static int frequency(lua_State*);
static int set_wave_data(lua_State*);
static int set_quality(lua_State*);
static int quality(lua_State*);
static int set_sync(lua_State*);
static int set_sync_fadeout(lua_State*);
static int phase(lua_State*);
static int setphase(lua_State*);
static int setpm(lua_State*);
static int setam(lua_State*);
static int setfm(lua_State*);

int psy_audio_luabind_resampler_open(lua_State *L)
{	
	static const luaL_Reg methods[] = {
		{"new", create},
		{"newwavetable", createwavetable},
		{"work", work},
		{"noteoff", noteoff},
		{"setfrequency", set_frequency},
		{"frequency", frequency},
		{"start", start},
		{"stop", stop },
		{"phase", phase},
		{"setphase", setphase},
		{"isplaying", isplaying},
		{"setwavedata", set_wave_data},
		{"setquality", set_quality},
		{"quality", set_quality},
		{"setsync", set_sync},
		{"setsyncfadeout", set_sync_fadeout},
		{"setpm", setpm},
		{"setam", setam},
		{"setfm", setfm},
		{NULL, NULL}
	};

	psyclescript_open(L, luaresampler_meta, methods, gc, 0);
	lua_pushnumber(L, 1);
	lua_setfield(L, -2, "ZEROHOLD");
	lua_pushnumber(L, 2);
	lua_setfield(L, -2, "LINEAR");
	lua_pushnumber(L, 3);
	lua_setfield(L, -2, "SINC");
	return 1;
}

int create(lua_State* L)
{	
	LuaResampler* rwrap;
	int n;  /* Number of arguments */
	psy_audio_Sample* wave;

	rwrap = 0;
	n = lua_gettop(L);
	if (n != 2) {
		return luaL_error(L, "Got %d arguments expected 2 (self, wave)", n);
	}
	wave = *(psy_audio_Sample**)luaL_checkudata(L, 4, "psywavedatameta");
	// WaveList<float>::Type waves;
	// waves[range<double>(0, 96000)] = wave.get();
	// rwrap = new ResamplerWrap<float, 1>(waves);
	rwrap = (LuaResampler*)malloc(sizeof(LuaResampler));
	luaresampler_init(rwrap); // , (psy_audio_WaveShape)shape, f);
	psyclescript_createuserdata(L, 1, luaresampler_meta, rwrap);
	lua_pushnil(L);
	lua_setfield(L, -2, "sync");
	return 1;
}

int createwavetable(lua_State* L)
{
	int n;
	LuaResampler* rwrap;
	
	n = lua_gettop(L);  /* Number of arguments */
	if (n != 2) {
		return luaL_error(L, "Got %d arguments expected 2 (self, wavetable)", n);
	}
	rwrap = (LuaResampler*)malloc(sizeof(LuaResampler));
	luaresampler_init(rwrap);
	psyclescript_createuserdata(L, 1, luaresampler_meta, rwrap);
	// LuaHelper::new_shared_userdata<>(L, meta,
	//	new ResamplerWrap<float, 1>(check_wavelist(L)));
	return 1;
}

int gc(lua_State* L)
{
	LuaResampler** ud = (LuaResampler**)
		luaL_checkudata(L, 1, luaresampler_meta);
	luaresampler_dispose(*ud);	
	free(*ud);
	return 0;
}

int work(lua_State* L)
{
	int n;
	LuaResampler* rwrap;
	psy_audio_Array* l;
	psy_audio_Array* r;
	float* fm;
	float* env;
	int processed;

	n = lua_gettop(L);
	if (n != 3 && n != 4 && n != 5 && n != 6) {
		return luaL_error(L, "Got %d arguments expected 2 (self, left, right [,fm])", n);
	}
	rwrap = psyclescript_checkself(L, 1, luaresampler_meta);
	if (!luaresampler_playing(rwrap)) {
		lua_pushnumber(L, 0);
		return 1;
	}
	
	l = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
	r = *(psy_audio_Array**)luaL_checkudata(L, 3, luaarraybind_meta);	

	fm = NULL;
	env = NULL;
	/* frequence modulation -- optional */
	if (n > 3 && (!lua_isnil(L, 4))) {
		psy_audio_Array* arr;
		
		arr = *(psy_audio_Array**)luaL_checkudata(L, 4, luaarraybind_meta);
		fm = psy_audio_array_data(arr);
	}
	/* volume envelope -- optional */
	if (n > 4 && !lua_isnil(L, 5)) {
		psy_audio_Array* arr;

		arr = *(psy_audio_Array**)luaL_checkudata(L, 5, luaarraybind_meta);		
		env = psy_audio_array_data(arr);		
	}
	/* check for master */
	//std::auto_ptr<LuaSingleWorker> master;
	lua_getfield(L, 1, "sync");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
	} else {
		lua_pushvalue(L, 1);
		//	master.reset(new LuaSingleWorker(L));
	}
	processed = luaresampler_work(rwrap, (int)psy_audio_array_len(l),
		psy_audio_array_data(l), psy_audio_array_data(r), NULL); // master.get());
	/* return total number of resampled samples */
	lua_pushnumber(L, processed); 
	return 1;	
}

int tostring(lua_State* L)
{
	LuaResampler* self;
	char text[256];
	
	self = psyclescript_checkself(L, 1, luaresampler_meta);
	psy_snprintf(text, 256, "%s", "luaresampler");
	lua_pushfstring(L, text);
	return 1;
}

int noteoff(lua_State* L)
{
	return 0;
}

int isplaying(lua_State* L)
{
	return 0;
}

int start(lua_State* L)
{
	return 0;
}

int stop(lua_State* L)
{
	return 0;
}

int set_frequency(lua_State* L)
{
	return psyclescript_chaining(L);
}

int frequency(lua_State* L)
{
	return 0;
}

int set_wave_data(lua_State* L)
{
	return 0;
}

int set_quality(lua_State* L)
{
	return 0;
}

int quality(lua_State* L)
{
	return 0;
}

int set_sync(lua_State* L)
{
	return 0;
}

int set_sync_fadeout(lua_State* L)
{
	return 0;
}

int phase(lua_State* L)
{
	return 0;
}

int setphase(lua_State* L)
{
	return 0;
}

int setpm(lua_State* L)
{
	int n;

	n = lua_gettop(L);
	if (n == 2) {
		LuaResampler* rwrap;

		rwrap = psyclescript_checkself(L, 1, luaresampler_meta);
		if (lua_isnil(L, 2)) {
			// rwrap->setpm(0);
		} else {
			psy_audio_Array* v;

			v = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
			// rwrap->setpm(v->data());
		}
	} else {
		luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
	}
	return psyclescript_chaining(L);
}

int setfm(lua_State* L)
{
	int n;

	n = lua_gettop(L);
	if (n == 2) {
		LuaResampler* rwrap;

		rwrap = psyclescript_checkself(L, 1, luaresampler_meta);
		if (lua_isnil(L, 2)) {
			// rwrap->setfm(0);
		} else {
			psy_audio_Array* v;

			v = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
			// rwrap->setfm(v->data());
		}
	} else {
		luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
	}
	return psyclescript_chaining(L);
}

int setam(lua_State* L)
{
	int n;	
	
	n = lua_gettop(L);
	if (n == 2) {
		LuaResampler* rwrap;

		rwrap = psyclescript_checkself(L, 1, luaresampler_meta);		
		if (lua_isnil(L, 2)) {
			// rwrap->setam(0);
		} else {
			psy_audio_Array* v;

			v = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
			// rwrap->setam(v->data());
		}
	} else {
		luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
	}
	return psyclescript_chaining(L);
}
