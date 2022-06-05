/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2007-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "luacomponent.h"
#include <lauxlib.h>
#include <lualib.h>

/* ui */
#include <uicomponent.h>


static int psy_luaui_component_create(lua_State* L);
static int psy_luaui_component_gc(lua_State* L);

const char* psy_luaui_component_meta = "psycomponentmeta";

int psy_luaui_component_open(lua_State *L)
{
	static const luaL_Reg pm_lib[] = {	
		{ NULL, NULL }
	};
  static const luaL_Reg pm_meta[] = {
    {"new", psy_luaui_component_create},
	{ "__gc", psy_luaui_component_gc },    
    {NULL, NULL}
  };
  luaL_newmetatable(L, psy_luaui_component_meta);
  luaL_setfuncs(L, pm_meta, 0);
  lua_pop(L, 1);
  luaL_newlib(L, pm_lib);
  return 1;  
}

int psy_luaui_component_create(lua_State* L)
{  
	psy_ui_Component** udata;
	int n = lua_gettop(L);

	if (n == 1) {
		udata = (psy_ui_Component**)lua_newuserdata(L, sizeof(psy_ui_Component*));		
		luaL_setmetatable(L, psy_luaui_component_meta);
	} else {
	luaL_error(L, "Wrong number of arguments");
	}
	return 1;
}

int psy_luaui_component_gc(lua_State* L)
{
	psy_ui_Component* ptr;

	ptr = *(psy_ui_Component**)luaL_checkudata(L, 1, psy_luaui_component_meta);	
	return 0;
}
