// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "psyclescript.h"
#include "lauxlib.h"
#include "lualib.h"
#if defined DIVERSALIS__OS__MICROSOFT    
#include <windows.h>
#endif

int psyclescript_init(PsycleScript* self)
{
	self->L = luaL_newstate();
	luaL_openlibs(self->L);
	return 0;
}

int psyclescript_load(PsycleScript* self, const char* path)
{
  int status;  
// set search path for require  
//  lua_pushstring(L, "?.lua");
//  lua_setfield(L, -2, "path");
      
  status = luaL_loadfile(self->L, path);
  if (status) {
    const char* msg =lua_tostring(self->L, -1);
#if defined DIVERSALIS__OS__MICROSOFT    
	MessageBox(0, msg, "Lua Error", MB_ICONERROR);
#endif    
  }
  return status;
}

int psyclescript_preparestate(PsycleScript* self)
{
  PsycleScript** ud;

  static const luaL_Reg methods[] = {{NULL, NULL}};
  lua_newtable(self->L);
  luaL_setfuncs(self->L, methods, 0);
  ud = (PsycleScript **)lua_newuserdata(self->L, sizeof(PsycleScript *));
  luaL_newmetatable(self->L, "psyhostmeta");
  lua_setmetatable(self->L, -2);
  *ud = self;
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

int psyclescript_run(PsycleScript* self)
{ 
  int status;

  status = lua_pcall(self->L, 0, LUA_MULTRET, 0);
  if (status) {      
    const char* msg = lua_tostring(self->L, -1);
  }
  return status;
}

int psyclescript_start(PsycleScript* self)
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

int psyclescript_dispose(PsycleScript* self)
{
  int err = 0;

  if (self->L) {
    lua_close(self->L);	
  }
  self->L = 0;
  return err;
}
