/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2007-2020 members of the psycle project http://psycle.sourceforge.net
*/

#include "luaplayer.h"

#include "array.h"
#include "luaarray.h"
#include "player.h"

#include <envelope.h>
#include <stdlib.h>

#include <lauxlib.h>
#include <lualib.h>

#include "psyclescript.h"

#include "../../detail/stdint.h"

static const char* luaplayer_meta = "psyplayermeta";
static psy_audio_Player* player = NULL;

static int luaplayer_create(lua_State* L);
static int luaplayer_gc(lua_State* L);
static int luaplayer_samplerate(lua_State* L);
/*static int luaenvelope_create(lua_State* L);
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
static int lastvalue(lua_State* L);*/

// lua machine bind
int psy_audio_luabind_player_open(lua_State *L)
{
	static const luaL_Reg methods[] = {
		{"new", luaplayer_create},
		{"samplerate", luaplayer_samplerate},
		{NULL, NULL}
	};
	psyclescript_open(L, luaplayer_meta, methods,
		luaplayer_gc, 0);  
	return 1;	
}

int luaplayer_create(lua_State* L)
{
	psyclescript_createuserdata(L, 1, luaplayer_meta, player);
	return 1;
}

int luaplayer_gc(lua_State* L)
{
	psy_audio_Player** ud = (psy_audio_Player**)
		luaL_checkudata(L, 1, luaplayer_meta);	
	return 0;
}

void psy_audio_luabind_setplayer(psy_audio_Player* theplayer)
{
	player = theplayer;
}

int luaplayer_samplerate(lua_State* L)
{
	psy_audio_Player** ud = (psy_audio_Player**)
		luaL_checkudata(L, 1, luaplayer_meta);
	lua_pushinteger(L, (int)psy_audio_player_samplerate(*ud));
	return 1;
}
