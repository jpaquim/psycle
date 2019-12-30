// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"
#include "../../detail/stdint.h"

#include "psyclescript.h"
#include "machinedefs.h"

#include <lauxlib.h>
#include <lualib.h>
#if defined DIVERSALIS__OS__MICROSOFT    
#include <windows.h>
#endif

static int psyclescript_parse_machineinfo(psy_audio_PsycleScript*, psy_audio_MachineInfo*);
static void psyclescript_setsearchpath(psy_audio_PsycleScript*, const char* modulepath);

int psyclescript_init(psy_audio_PsycleScript* self)
{
	self->L = luaL_newstate();
	luaL_openlibs(self->L);
	return 0;
}

int psyclescript_dispose(psy_audio_PsycleScript* self)
{
  int err = 0;

  if (self->L) {
    lua_close(self->L);	
  }
  self->L = 0;
  return err;
}

int psyclescript_load(psy_audio_PsycleScript* self, const char* path)
{
  int status;  
// set search path for require  
//  lua_pushstring(L, "?.lua");
//  lua_setfield(L, -2, "path");
  psyclescript_setsearchpath(self, path);
  status = luaL_loadfile(self->L, "nonsense");
  if (status) {
    const char* msg =lua_tostring(self->L, -1);
#if defined DIVERSALIS__OS__MICROSOFT    
	MessageBox(0, msg, "Lua Error", MB_ICONERROR);
#endif    
  }
  return status;
}

int psyclescript_preparestate(psy_audio_PsycleScript* self, const luaL_Reg methods[],
	void* host)
{
  psy_audio_PsycleScript** ud;

  lua_newtable(self->L);
  luaL_setfuncs(self->L, methods, 0);
  ud = (psy_audio_PsycleScript **)lua_newuserdata(self->L, sizeof(psy_audio_PsycleScript *));
  luaL_newmetatable(self->L, "psyhostmeta");
  lua_setmetatable(self->L, -2);
  *ud = host;
  lua_setfield(self->L, -2, "__self");
  lua_setglobal(self->L, "psycle");
  lua_getglobal(self->L, "psycle");  
  lua_newtable(self->L); // table for canvasdata
  lua_setfield(self->L, -2, "userdata");
  lua_newtable(self->L); // table for canvasdata
  lua_newtable(self->L); // metatable
  lua_pushstring(self->L, "kv");
  lua_setfield(self->L, -2, "__mode");
  lua_setmetatable(self->L, -2);
  lua_setfield(self->L, -2, "weakuserdata");
  lua_pop(self->L, 1);
  return 0;
}

int psyclescript_run(psy_audio_PsycleScript* self)
{ 
  int status;

  status = lua_pcall(self->L, 0, LUA_MULTRET, 0);
  if (status) {      
    const char* msg = lua_tostring(self->L, -1);
  }
  return status;
}

int psyclescript_start(psy_audio_PsycleScript* self)
{
  lua_getglobal(self->L, "psycle");
  if (lua_isnil(self->L, -1)) {
    // throw std::runtime_error("Psycle not available.");
	  return 1;
  }
  lua_getfield(self->L, -1, "start");
  if (!lua_isnil(self->L, -1)) {
    int status = lua_pcall(self->L, 0, 0, 0);
    if (status) {         
      const char* msg = lua_tostring(self->L, -1);      
      return 1;
    }
  }
  return 0;
}

int psyclescript_machineinfo(psy_audio_PsycleScript* self, psy_audio_MachineInfo* rv)
{
	int err = 0;
	lua_getglobal(self->L, "psycle");
	if (lua_isnil(self->L, -1)) {
		err = 1; // throw std::runtime_error("Psycle not available.");
		return err;
	}
	lua_getfield(self->L, -1, "info");
	if (!lua_isnil(self->L, -1)) {
		int status = lua_pcall(self->L, 0, 1, 0);    
		if (status) {         
			const char* msg = lua_tostring(self->L, -1); 
			// ui::error(msg);
			err = 1; // throw std::runtime_error(msg);
			return err;
		}		
		psyclescript_parse_machineinfo(self, rv);
	} else {
		err = 1;
		// throw std::runtime_error("no info found");
	}
	return err;
}

int psyclescript_parse_machineinfo(psy_audio_PsycleScript* self, psy_audio_MachineInfo* rv)
{  
	char* name = 0;
	char* vendor = 0;	
	int mode = MACHMODE_FX;
	int version = 0;
	int apiversion = 0;
	int noteon = 0;
	int err = 0;
	
	if (lua_istable(self->L, -1)) {
		size_t len;
		for (lua_pushnil(self->L); lua_next(self->L, -2);
				lua_pop(self->L, 1)) {
			const char* key = luaL_checklstring(self->L, -2, &len);
			if (strcmp(key, "vendor") == 0) {
				const char* value = luaL_checklstring(self->L, -1, &len);
				if (value) {
					vendor = strdup(value);
				}        
			} else
			if (strcmp(key, "name") == 0) {
				const char* value = luaL_checklstring(self->L, -1, &len);
				if (value) {
					name = strdup(value);
				}          
			} else
			if (strcmp(key, "mode") == 0) {
				int64_t value;

				if (lua_isnumber(self->L, -1) == 0) {
					err = 1;
					break;
					// throw std::runtime_error("info mode not a number"); 
				}			
				value = luaL_checkinteger(self->L, -1);
				switch (value) {
					case 0:
						mode = MACHMODE_GENERATOR;
					break;
					case 3:
						// mode = MACHMODE_HOST;
						mode = MACHMODE_FX;
					break;
					default:
						mode = MACHMODE_FX;
					break;
				}
			} else
			if (strcmp(key, "generator") == 0) {
				// deprecated, use mode instead
				int64_t value = luaL_checkinteger(self->L, -1);
				mode = (value == 1) ? MACHMODE_GENERATOR : MACHMODE_FX;
			} else
			if (strcmp(key, "version") == 0) {
				version = (int) luaL_checkinteger(self->L, -1);
			} else
			if (strcmp(key, "api") == 0) {
				apiversion = (int) luaL_checkinteger(self->L, -1);			
			} else
			if (strcmp(key, "noteon") == 0) {
				noteon = (int) luaL_checkinteger(self->L, -1);			
			}
		}
	}	     
	// std::ostringstream s;
	// s << (result.mode == MACHMODE_GENERATOR ? "Lua instrument"
	//                                          : "Lua effect")
	//    << " by "
	//    << result.vendor;
	// result.desc = s.str();
	machineinfo_set(rv, vendor, "", 0, mode, name, name, (short) apiversion, 
		(short) version, MACH_LUA, "", 0);
	free(name);
	free(vendor);
	return err;
}

int psyclescript_open(lua_State* L,
		const char* meta,
		const luaL_Reg methods[],
		lua_CFunction gc,
		lua_CFunction tostring)
{
	luaL_newmetatable(L, meta);
    if (gc) {
        lua_pushcclosure(L, gc, 0);
        lua_setfield(L,-2, "__gc");
    }
    if (tostring) {
        lua_pushcclosure(L, tostring, 0);
        lua_setfield(L,-2, "__tostring");
    }
    luaL_newlib(L, methods);
    return 1;
}

void psyclescript_require(psy_audio_PsycleScript* self, const char* name,
	lua_CFunction openf)
{
	luaL_requiref(self->L, name, openf, 1);
    lua_pop(self->L, 1);
}

void psyclescript_setsearchpath(psy_audio_PsycleScript* self, const char* modulepath)
{	
  const char* path =
	  "c:/programme/psycle/luascripts/?.lua;"
	  "c:/programme/psycle/luascripts/effecttest?.lua;";
  lua_getglobal(self->L, "package");
  lua_pushstring(self->L, path);
  lua_setfield(self->L, -2, "path");
}

void psyclescript_register_weakuserdata(lua_State* L, void* ud)
{
	lua_getglobal(L, "psycle");
	lua_getfield(L, -1, "weakuserdata");
	lua_pushlightuserdata(L, ud);
	lua_pushvalue(L, -4);
	lua_settable(L, -3);
	lua_pop(L, 2);
}

void* psyclescript_checkself(lua_State* L, int index, const char* meta)
{
	void** ud;

	luaL_checktype(L, index, LUA_TTABLE);
	lua_getfield(L, index, "__self");      
	ud = luaL_checkudata(L, -1, meta);
	luaL_argcheck(L, (ud) != 0, 1, meta);
	lua_pop(L, 1);
	// luaL_argcheck(L, (*SPTR) != 0, 1, (meta+" expected").c_str());     
	return *ud;
}
