/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2007-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "luaopendialog.h"
/* ui */
#include <uiopendialog.h>
/* script */
#include <psyclescript.h>
#include <luaimport.h>
/* lua */
#include <lauxlib.h>
#include <lualib.h>
/* std */
#include <stdlib.h>


const char* psy_luaui_opendialog_meta = "psyfileopenmeta";

/* prototypes */
static int psy_luaui_opendialog_create(lua_State*);
static int psy_luaui_opendialog_gc(lua_State*);
static int psy_luaui_opendialog_show(lua_State*);

/* implementation */
int psy_luaui_opendialog_open(lua_State *L)
{
	static const luaL_Reg pm_lib[] = {	
		{ NULL, NULL }
	};
	static const luaL_Reg pm_meta[] = {
		{"new", psy_luaui_opendialog_create},
		{"__gc", psy_luaui_opendialog_gc},
		{"show", psy_luaui_opendialog_show},
		{NULL, NULL}
	};
	luaL_newmetatable(L, psy_luaui_opendialog_meta);
	luaL_setfuncs(L, pm_meta, 0);
	lua_pop(L, 1);
	luaL_newlib(L, pm_lib);
	return 1;  
}

int psy_luaui_opendialog_create(lua_State* L)
{  	
	psy_ui_OpenDialog** udata;

	int n = lua_gettop(L);
	if (n == 1) {
		udata = (psy_ui_OpenDialog**)lua_newuserdata(L, sizeof(psy_ui_OpenDialog*));
		*udata = psy_ui_opendialog_allocinit(NULL);
		luaL_setmetatable(L, psy_luaui_opendialog_meta);
	}
	return psyclescript_chaining(L);	
}

int psy_luaui_opendialog_gc(lua_State* L)
{
	psy_ui_OpenDialog* ptr;

	ptr = *(psy_ui_OpenDialog**)luaL_checkudata(L, 1, psy_luaui_opendialog_meta);
	free(ptr);
	return 0;
}

int psy_luaui_opendialog_show(lua_State* L)
{
	psy_ui_OpenDialog* dialog;

	dialog = psyclescript_checkself(L, 1, psy_luaui_opendialog_meta);
	return psyclescript_chaining(L);
	if (psy_ui_opendialog_execute(dialog)) {
		psy_LuaImport in;
		
		psy_luaimport_init(&in, L, NULL);
		if (psy_luaimport_open(&in, "onok")) {			
			psy_luaimport_pcall(&in, 0);			
		}
		psy_luaimport_dispose(&in);
	} else {
		psy_LuaImport in;

		psy_luaimport_init(&in, L, NULL);
		if (psy_luaimport_open(&in, "oncancel")) {
			psy_luaimport_pcall(&in, 0);
		}
		psy_luaimport_dispose(&in);
	}	
	return psyclescript_chaining(L);
}
