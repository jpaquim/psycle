/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2007-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "luagraphics.h"
#include <lauxlib.h>
#include <lualib.h>

#include <uigraphics.h>
#include <stdlib.h>


static int psy_luaui_graphics_create(lua_State* L);
static int psy_luaui_graphics_gc(lua_State* L);
/*static int psy_luaui_point_setxy(lua_State* L) { LUAEXPORTM(L, meta, &ui::Point::set_xy); }
static int psy_luaui_point_setx(lua_State* L) { LUAEXPORTM(L, meta, &ui::Point::set_x); }
static int psy_luaui_point_x(lua_State* L) { LUAEXPORTM(L, meta, &ui::Point::x); }
static int psy_luaui_point_sety(lua_State* L) { LUAEXPORTM(L, meta, &ui::Point::set_y); }
static int psy_luaui_point_y(lua_State* L) { LUAEXPORTM(L, meta, &ui::Point::y); }
static int psy_luaui_point_offset(lua_State* L) { LUAEXPORTM(L, meta, &ui::Point::offset); }
static int psy_luaui_point_add(lua_State* L);
static int psy_luaui_point_sub(lua_State* L);
static int psy_luaui_point_mul(lua_State* L);
static int psy_luaui_point_div(lua_State* L);
*/

const char* psy_luaui_graphics_meta = "psygraphicsmeta";

int psy_luaui_graphics_open(lua_State *L)
{
	static const luaL_Reg pm_lib[] = {	
		{ NULL, NULL }
	};
  static const luaL_Reg pm_meta[] = {
    {"new", psy_luaui_graphics_create},
	{ "__gc", psy_luaui_graphics_gc },
    /*{"setxy", psy_luaui_point_setxy},
    {"setx", psy_luaui_point_setx},
    {"x", psy_luaui_point_x},
    {"sety", psy_luaui_point_sety},
    {"y", psy_luaui_point_y},
	{"offset", psy_luaui_point_offset},
    {"add", psy_luaui_point_add},
	{"sub", psy_luaui_point_sub},
	{"mul", psy_luaui_point_mul},
	{"div", psy_luaui_point_div},*/
    {NULL, NULL}
  };
  luaL_newmetatable(L, psy_luaui_graphics_meta);
  luaL_setfuncs(L, pm_meta, 0);
  lua_pop(L, 1);
  luaL_newlib(L, pm_lib);
  return 1;  
}

int psy_luaui_graphics_create(lua_State* L)
{  
  psy_ui_Point** udata;
  int n = lua_gettop(L);

  if (n == 1) {
	  udata = (psy_ui_Point**)lua_newuserdata(L, sizeof(psy_ui_Graphics*));
	  luaL_setmetatable(L, psy_luaui_graphics_meta);
  /*else if (n == 2) {
    Point::Ptr other = LuaHelper::check_sptr<Point>(L, 2, LuaPointBind::meta); 
    LuaHelper::new_shared_userdata<>(L, meta, new Point(*other.get()));
  } else
  if (n == 3) {
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    LuaHelper::new_shared_userdata<>(L, meta, new Point(x, y));*/
  } else {
    luaL_error(L, "Wrong number of arguments");
  }
  return 1;
}

int psy_luaui_graphics_gc(lua_State* L)
{
	psy_ui_Graphics* ptr;

	ptr = *(psy_ui_Graphics**)luaL_checkudata(L, 1, psy_luaui_graphics_meta);
	free(ptr);
	return 0;
}


/*
int psy_luaui_point_add(lua_State *L) {
	using namespace ui;
	Point::Ptr self = LuaHelper::check_sptr<Point>(L, 1, LuaPointBind::meta);
	Point::Ptr rhs = LuaHelper::test_sptr<Point>(L, 2, LuaPointBind::meta); 
	if (rhs) {
	  *self.get() += *rhs.get();
	} else {
	  double a = luaL_checknumber(L, 2);
	  *self.get() += Point(a, a);
	}
	return LuaHelper::chaining(L);
}

int psy_luaui_point_sub(lua_State *L) {
	using namespace ui;
	Point::Ptr self = LuaHelper::check_sptr<Point>(L, 1, LuaPointBind::meta);
	Point::Ptr rhs = LuaHelper::test_sptr<Point>(L, 2, LuaPointBind::meta); 
	if (rhs) {
	  *self.get() -= *rhs.get();
	} else {
	  double a = luaL_checknumber(L, 2);
	  *self.get() -= Point(a, a);
	}
	return LuaHelper::chaining(L);
}

int psy_luaui_point_mul(lua_State *L) {
	using namespace ui;
	Point::Ptr self = LuaHelper::check_sptr<Point>(L, 1, LuaPointBind::meta);
	Point::Ptr rhs = LuaHelper::test_sptr<Point>(L, 2, LuaPointBind::meta); 
	if (rhs) {
	  *self.get() *= *rhs.get();
	} else {
	  double a = luaL_checknumber(L, 2);
	  *self.get() *= Point(a, a);
	}
	return LuaHelper::chaining(L);
}

int psy_luaui_point_div(lua_State *L) {
	using namespace ui;
	Point::Ptr self = LuaHelper::check_sptr<Point>(L, 1, LuaPointBind::meta);
	Point::Ptr rhs = LuaHelper::test_sptr<Point>(L, 2, LuaPointBind::meta); 
	if (rhs) {
	  if (rhs->x() == 0 || rhs->y() == 0) {
	    return luaL_error(L, "Math error. Division by zero."); 
	  }
	  *self.get() /= *rhs.get();
	} else {
	  double a = luaL_checknumber(L, 2);
	  if (a == 0) {
	    return luaL_error(L, "Math error. Division by zero."); 
	  }
	  *self.get() /= Point(a, a);
	}
	return LuaHelper::chaining(L);
}
*/
