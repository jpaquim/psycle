// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2020 members of the psycle project http://psycle.sourceforge.net

#include "luaarray.h"

#include "array.h"
#include <stdlib.h>

#include <lauxlib.h>
#include <lualib.h>

#include "../../detail/stdint.h"

#define MAKE_EXPORT_SELF(x) static int array_method_ ## x (lua_State* L)\
{\
	psy_audio_Array** ud;\
	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);\
	psy_audio_array_ ## x (*ud);\
	lua_pushvalue(L, 1);\
	return 1;\
}
	
static const char* luaarraybind_meta = "array_meta";

static int psy_audio_luabind_array_new(lua_State*);
static int psy_audio_luabind_array_gc(lua_State*);
static int psy_audio_luabind_array_index(lua_State*);
static int psy_audio_luabind_array_new_index(lua_State*);

MAKE_EXPORT_SELF(sqrt)
MAKE_EXPORT_SELF(sin)
MAKE_EXPORT_SELF(cos)
MAKE_EXPORT_SELF(tan)
MAKE_EXPORT_SELF(ceil)
MAKE_EXPORT_SELF(floor)
MAKE_EXPORT_SELF(fabs)

int psy_audio_luabind_array_open(lua_State* L)
{
	static const luaL_Reg pm_lib[] = {
		{ "new", psy_audio_luabind_array_new },
/*		{ "arange", array_arange },
		{ "random", array_random },
		{ "sin", array_sin },
		{ "cos", array_cos },
		{ "tan", array_tan },
		{ "sqrt", array_sqrt },
		{ "sum", array_sum },
		{ "rsum", array_rsum },
		{ "pow", array_pow },*/
		{ NULL, NULL }
	};
	static const luaL_Reg pm_meta[] = {
//		{ "random", array_method_random},
		{ "sqrt", array_method_sqrt},
		{ "sin", array_method_sin},
		{ "cos", array_method_cos },
		{ "tan", array_method_tan },		
		{ "ceil", array_method_ceil},
		{ "floor", array_method_floor},
		{ "abs", array_method_fabs},
/*		{ "add", array_method_add},
		{ "sub", array_method_sub},
		{ "mix", array_method_mix},
		{ "mul", array_method_mul},
		{ "div", array_method_div},
		{ "rsum", array_method_rsum},
		{ "sgn", array_method_sgn},

		{ "max", array_method_max},
		{ "min", array_method_min},
		{ "band", array_method_and},
		{ "bor", array_method_or},
		{ "bxor", array_method_xor},
		{ "bleft", array_method_sleft},
		{ "bright", array_method_sright},
		{ "bnot", array_method_bnot},
		{ "size", array_size},
		{ "resize", array_resize},
		{ "copy", array_copy},
		{ "fillzero", array_fillzero},
		{ "fill", array_method_fill},
		{ "tostring", array_tostring },
		{ "margin", array_margin},
		{ "fromtable", array_method_from_table},
		{ "table", array_method_to_table},
		{ "clearmargin", array_clearmargin }, */
		{ "__newindex", psy_audio_luabind_array_new_index },
		{ "__index", psy_audio_luabind_array_index },
		{ "__gc", psy_audio_luabind_array_gc },
		/*{ "__tostring", array_tostring },
		{ "__add", array_add },
		{ "__sub", array_sub },
		{ "__mul", array_mul },
		{ "__div", array_div },
		{ "__mod", array_mod },
		{ "__pow", array_op_pow },
		{ "__unm", array_unm },
		{ "__concat", array_concat},*/
		{ NULL, NULL }
	};
	luaL_newmetatable(L, luaarraybind_meta);
	luaL_setfuncs(L, pm_meta, 0);
	lua_pop(L, 1);
	luaL_newlib(L, pm_lib);
	return 1;
}

int psy_audio_luabind_array_new(lua_State *L)
{
//	int size;
//	double val;
	int n = lua_gettop(L);
	psy_audio_Array ** udata = (psy_audio_Array **)
		lua_newuserdata(L, sizeof(psy_audio_Array *));
	luaL_setmetatable(L, luaarraybind_meta);	
	switch (n) {
		case 0:
			*udata = malloc(sizeof(psy_audio_Array));
			psy_audio_array_init(*udata);
			break;
		case 1 : {
			/*if (lua_istable(L, 1)) {
				size_t len = lua_rawlen(L, 1);
				*udata = malloc(sizeof(psy_audio_Array))
				psy_audio_array_init_size(*udata, len, 0);				
				float* ptr = (*udata)->data();
				for (size_t i = 1; i <= len; ++i) {
					lua_rawgeti(L, 1, i);
					*ptr++ = luaL_checknumber(L, -1);
					lua_pop(L,1);
				}
			} else {
				size = luaL_checknumber (L, 1);
				*udata = new PSArray(size, 0);
			}*/
		}
		break;
		case 2:
/*			size = luaL_checknumber (L, 1);
			val = luaL_checknumber (L, 2);
			*udata = new PSArray(size, val);*/
		break;
		default:
			luaL_error(L, "Got %d arguments expected max 2 (none, size, size+val)", n);
		break;
	}
	return 1;
}

int psy_audio_luabind_array_gc (lua_State *L)
{
  psy_audio_Array* ptr;
  
  ptr = *(psy_audio_Array **)luaL_checkudata(L, 1, luaarraybind_meta);
  psy_audio_array_dispose(ptr);
  free(ptr);
  return 0;
}

int psy_audio_luabind_array_index(lua_State *L)
{
	if (lua_isnumber(L, 2)) {
		intptr_t index;
		psy_audio_Array** ud = (psy_audio_Array**)
		luaL_checkudata(L, 1, luaarraybind_meta);
		index = (intptr_t) luaL_checknumber(L, 2);
		if (index <0 && index >= psy_audio_array_len(*ud)) {
			luaL_error(L, "index out of range");
		}
		lua_pushnumber(L, *(psy_audio_array_data(*ud) + index));
		return 1;
	} else
	if (lua_istable(L, 2)) {
	/*
	psy_audio_Array** ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	std::vector<int> p;
	for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
	p.push_back(luaL_checknumber(L, -1));
	}
	psy_audio_Array** rv = (psy_audio_Array**)
	  lua_newuserdata(L, sizeof(psy_audio_Array*));
	if (p[1]-p[0] == 0) {
	*rv = new PSArray();
	} else {
	if (!((0 <= p[0] && p[0] <= (*ud)->len()) &&
		  (0 <= p[1] && p[1] <= (*ud)->len()) &&
		  p[0] < p[1])) {
	  // std::ostringstream o;
	  // o << "index" << p[0] << "," << p[1] << "," << " out of range (max)" << (*ud)->len();
	  // luaL_error(L, o.str().c_str());
	}
	*rv = new psy_audio_Array(p[1]-p[0], 0);
	struct {float* p; float* v; int s; float operator()(int y) {return v[y+s];}} f;
	f.s = p[0];
	f.v = (*ud)->data();
	(*rv)->do_op(f);
	}
	luaL_setmetatable(L, meta);*/
	return 1;
	} else {
	/*  size_t len;
	luaL_checkudata(L, 1, meta);
	const char* key = luaL_checklstring(L, 2, &len);
	lua_getmetatable(L, 1);
	for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
	const char* method = luaL_checklstring(L, -2, &len);
	int result = strcmp(key, method);
	if (result == 0) {
	  return 1;
	}
	}*/
	}
	// no method found
	return 0;
}

int psy_audio_luabind_array_new_index(lua_State *L)
{
	if (lua_isnumber(L, 2)) {
		int index;
		float value;

		psy_audio_Array** ud = (psy_audio_Array**) luaL_checkudata(L, 1,
			luaarraybind_meta);
		index = (int) luaL_checkinteger(L, 2);
		value = (float) luaL_checknumber(L, 3);
		if (!(0 <= index && index < psy_audio_array_len(*ud))) {
			luaL_error(L, "index out of range");
		}
		*(psy_audio_array_data(*ud) + index) = value;
			return 0;
		} else {
		//error
		lua_error(L);
		return 0;
	}
}