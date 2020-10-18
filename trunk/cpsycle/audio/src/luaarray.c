// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2020 members of the psycle project http://psycle.sourceforge.net

#include "luaarray.h"

#include "array.h"
#include "psyclescript.h"

#include <stdlib.h>
#include <string.h>

#include <lauxlib.h>
#include <lualib.h>

#include "../../detail/stdint.h"
#include "../../detail/portable.h"

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif


const char* luaarraybind_meta = "array_meta";

static int psy_audio_luabind_array_new(lua_State*);
static int psy_audio_luabind_array_arange(lua_State*);
static int psy_audio_luabind_array_random(lua_State*);
static int psy_audio_luabind_array_gc(lua_State*);
static int psy_audio_luabind_array_index(lua_State*);
static int psy_audio_luabind_array_new_index(lua_State*);
static psy_audio_Array* create_copy_array(lua_State*, int idx);

static int array_mul(lua_State*);
static int array_div(lua_State*);
static int array_add(lua_State*);
static int array_sub(lua_State*);
static int array_sin(lua_State*);
static int array_cos(lua_State*);
static int array_tan(lua_State*);
static int array_pow(lua_State*);
static int array_mod(lua_State*);
static int array_sqrt(lua_State*);
static int array_unm(lua_State*);
static int array_concat(lua_State*);
static int array_tostring(lua_State*);
static int array_rsum(lua_State*);

static int array_method_mul(lua_State*);
static int array_method_div(lua_State*);
static int array_method_add(lua_State*);
static int array_method_sub(lua_State*);
static int array_method_mix(lua_State*);
static int array_method_random(lua_State*);
static int array_method_sqrt(lua_State*);
static int array_method_sin(lua_State*);
static int array_method_cos(lua_State*);
static int array_method_tan(lua_State*);
static int array_method_ceil(lua_State*);
static int array_method_floor(lua_State*);
static int array_method_fabs(lua_State*);
static int array_method_rsum(lua_State*);
static int array_method_min(lua_State*);
static int array_method_max(lua_State*);
static int array_method_band(lua_State*);
static int array_method_bor(lua_State*);
static int array_method_bxor(lua_State*);
static int array_method_sleft(lua_State*);
static int array_method_sright(lua_State*);
static int array_method_bnot(lua_State*);
static int array_method_size(lua_State*);
static int array_copy(lua_State*);
static int array_method_fillzero(lua_State*);
static int array_method_fill(lua_State*);
static int array_method_resize(lua_State*);
static int array_method_margin(lua_State*);
static int array_method_clearmargin(lua_State*);
static int array_method_from_table(lua_State*);
static int array_method_to_table(lua_State*);

int array_method_mix(lua_State* L)
{
	psy_audio_Array** ud;

	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	if (lua_isuserdata(L, 2)) {
		psy_audio_Array** ud_other;

		ud_other = (psy_audio_Array**) luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L,
			psy_audio_array_len(*ud) == psy_audio_array_len(*ud_other), 2,
			"size not compatible");
		psy_audio_array_mix(*ud, *ud_other, (float) luaL_checknumber (L, 3));
	} else {
		luaL_error(L, "array mix: expected array as second argument");
	}
	lua_pushvalue(L, 1);
	return 1;
}

int array_method_mul(lua_State* L)
{
	psy_audio_Array** ud;

	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	if (lua_isuserdata(L, 2)) {
		psy_audio_Array** ud_other;

		ud_other = (psy_audio_Array**) luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L,
			psy_audio_array_len(*ud) == psy_audio_array_len(*ud_other), 2,
			"size not compatible");
		psy_audio_array_mul_array(*ud, *ud_other);
	} else {		
		psy_audio_array_mul_constant(*ud, (float) luaL_checknumber (L, 2));
	}
	lua_pushvalue(L, 1);
	return 1;
}

int array_method_div(lua_State* L)
{
	psy_audio_Array** ud;

	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	if (lua_isuserdata(L, 2)) {
		psy_audio_Array** ud_other;

		ud_other = (psy_audio_Array**) luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L,
			psy_audio_array_len(*ud) == psy_audio_array_len(*ud_other), 2,
			"size not compatible");
		psy_audio_array_div_array(*ud, *ud_other);
	} else {		
		psy_audio_array_div_constant(*ud, (float) luaL_checknumber (L, 2));
	}
	lua_pushvalue(L, 1);
	return 1;
}

int array_method_add(lua_State* L)
{
	psy_audio_Array** ud;

	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	if (lua_isuserdata(L, 2)) {
		psy_audio_Array** ud_other;

		ud_other = (psy_audio_Array**) luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L,
			psy_audio_array_len(*ud) == psy_audio_array_len(*ud_other), 2,
			"size not compatible");
		psy_audio_array_add_array(*ud, *ud_other);
	} else {		
		psy_audio_array_add_constant(*ud, (float) luaL_checknumber (L, 2));
	}
	lua_pushvalue(L, 1);
	return 1;
}

int array_method_sub(lua_State* L)
{
	psy_audio_Array** ud;

	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	if (lua_isuserdata(L, 2)) {
		psy_audio_Array** ud_other;

		ud_other = (psy_audio_Array**) luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L,
			psy_audio_array_len(*ud) == psy_audio_array_len(*ud_other), 2,
			"size not compatible");
		psy_audio_array_sub_array(*ud, *ud_other);
	} else {		
		psy_audio_array_sub_constant(*ud, (float) luaL_checknumber (L, 2));
	}
	lua_pushvalue(L, 1);
	return 1;
}

int array_method_random(lua_State* L)
{
	psy_audio_Array** ud;
	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_random(*ud);
	lua_pushvalue(L, 1);
	return 1;
}

int array_method_sqrt(lua_State* L)
{
	psy_audio_Array** ud;
	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_sqrt(*ud);
	lua_pushvalue(L, 1);
	return 1;
}

int array_method_sin(lua_State* L)
{
	psy_audio_Array** ud;
	ud = (psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_sin(*ud);
	lua_pushvalue(L, 1);
	return 1;
}

int array_method_cos(lua_State* L)
{
	psy_audio_Array** ud;
	ud = (psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_cos(*ud);
	lua_pushvalue(L, 1);
	return 1;
}

int array_method_tan(lua_State* L)
{
	psy_audio_Array** ud;
	ud = (psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_tan(*ud);
	lua_pushvalue(L, 1);
	return 1;
}

int array_method_ceil(lua_State* L)
{
	psy_audio_Array** ud;
	ud = (psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_ceil(*ud);
	lua_pushvalue(L, 1);
	return 1;
}

int array_method_floor(lua_State* L)
{
	psy_audio_Array** ud;
	ud = (psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_floor(*ud);
	lua_pushvalue(L, 1);
	return 1;
}

int array_method_fabs(lua_State* L)
{
	psy_audio_Array** ud;
	ud = (psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_fabs(*ud);
	lua_pushvalue(L, 1);
	return 1;
}

int array_method_sgn(lua_State* L)
{
	psy_audio_Array** ud;
	ud = (psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_sgn(*ud);
	lua_pushvalue(L, 1);
	return 1;
}

int psy_audio_luabind_array_open(lua_State* L)
{
	static const luaL_Reg pm_lib[] = {
		{ "new", psy_audio_luabind_array_new },
		{ "arange", psy_audio_luabind_array_arange },
		{ "random", psy_audio_luabind_array_random },
		{ "sin", array_sin },
		{ "cos", array_cos },
		{ "tan", array_tan },
		{ "sqrt", array_sqrt },
//		{ "sum", array_sum },
		{ "rsum", array_rsum },
		{ "pow", array_pow },
		{ NULL, NULL }
	};
	static const luaL_Reg pm_meta[] = {
		{ "random", array_method_random},
		{ "sqrt", array_method_sqrt},
		{ "sin", array_method_sin},
		{ "cos", array_method_cos },
		{ "tan", array_method_tan },		
		{ "ceil", array_method_ceil},
		{ "floor", array_method_floor},
		{ "abs", array_method_fabs},
		{ "add", array_method_add},
		{ "sub", array_method_sub},
		{ "mix", array_method_mix},
		{ "mul", array_method_mul},
		{ "div", array_method_div},
		{ "rsum", array_method_rsum},
		{ "sgn", array_method_sgn},
		{ "max", array_method_max},
		{ "min", array_method_min},
		{ "band", array_method_band},
		{ "bor", array_method_bor},
		{ "bxor", array_method_bxor},
		{ "bleft", array_method_sleft},
		{ "bright", array_method_sright},
		{ "bnot", array_method_bnot},
		{ "size", array_method_size},
		{ "resize", array_method_resize},
		{ "copy", array_copy},
		{ "fillzero", array_method_fillzero},
		{ "fill", array_method_fill},
		{ "tostring", array_tostring },
		{ "margin", array_method_margin},
		{ "clearmargin", array_method_clearmargin },
		{ "fromtable", array_method_from_table},
		{ "table", array_method_to_table},
		{ "__newindex", psy_audio_luabind_array_new_index },
		{ "__index", psy_audio_luabind_array_index },
		{ "__gc", psy_audio_luabind_array_gc },
		{ "__tostring", array_tostring },
		{ "__add", array_add },
		{ "__sub", array_sub },
		{ "__mul", array_mul },
		{ "__div", array_div },
		{ "__mod", array_mod },
		{ "__pow", array_pow },
		{ "__unm", array_unm },
		{ "__concat", array_concat},
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
	int n;
	psy_audio_Array** udata;
	
	n = lua_gettop(L); 
	udata = (psy_audio_Array**) lua_newuserdata(L, sizeof(psy_audio_Array *));
	luaL_setmetatable(L, luaarraybind_meta);	
	switch (n) {
		case 0:
			*udata = psy_audio_array_alloc();
			psy_audio_array_init(*udata);
			break;
		case 1:
			if (lua_istable(L, 1)) {
				uintptr_t len;
				float* ptr;
				uintptr_t i;

				len = (uintptr_t) lua_rawlen(L, 1);
				*udata = psy_audio_array_alloc();
				psy_audio_array_init_len(*udata, len, 0);				
				ptr = psy_audio_array_data(*udata);
				for (i = 1; i <= len; ++i) {
					lua_rawgeti(L, 1, i);
					*ptr++ = (float) luaL_checknumber(L, -1);
					lua_pop(L,1);
				}
			} else {
				uintptr_t len;
				
				len = (uintptr_t) luaL_checkinteger(L, 1);
				*udata = psy_audio_array_alloc();
				psy_audio_array_init_len(*udata, len, 0);
			}
		break;
		case 2:
		{
			uintptr_t len;
			lua_Number value;

			len = (uintptr_t) luaL_checkinteger(L, 1);
			value = luaL_checknumber(L, 2);
			*udata = psy_audio_array_alloc();
			psy_audio_array_init_len(*udata, len, (float) value);
		}
		break;
		default:
			luaL_error(L, "Got %d arguments expected max 2 (none, size, size+val)", n);
		break;
	}
	return 1;
}

int psy_audio_luabind_array_arange(lua_State* L)
{
	psy_audio_Array** udata;
	lua_Number start;
	lua_Number stop;
	lua_Number step;

	udata = (psy_audio_Array**) lua_newuserdata(L, sizeof(psy_audio_Array*));
	start = luaL_checknumber(L, 1);
	stop = luaL_checknumber(L, 2);
	step = luaL_checknumber(L, 3);
	*udata = psy_audio_array_alloc();
	psy_audio_array_init_arange(*udata, (float) start, (float) stop,
		(float) step);
	luaL_setmetatable(L, luaarraybind_meta);
	return 1;
}

int psy_audio_luabind_array_random(lua_State* L)
{
	psy_audio_Array** udata;
	uintptr_t size;

	udata = (psy_audio_Array**) lua_newuserdata(L, sizeof(psy_audio_Array*));	
	size = (uintptr_t) luaL_checkinteger(L, 1);
	*udata = psy_audio_array_alloc();
	psy_audio_array_init_len(*udata, size, 0);
	psy_audio_array_random(*udata);		
	return 1;
}

psy_audio_Array* create_copy_array(lua_State* L, int idx)
{
	psy_audio_Array* udata;
	psy_audio_Array** rv;
	
	udata = *(psy_audio_Array**) luaL_checkudata(L, idx, luaarraybind_meta);
	rv = (psy_audio_Array**) lua_newuserdata(L, sizeof(psy_audio_Array*));
	luaL_setmetatable(L, luaarraybind_meta);
	*rv = psy_audio_array_alloc();
	psy_audio_array_init_len(*rv, psy_audio_array_len(udata), 0);
	psy_audio_array_copy(*rv, udata);	
	return *rv;
}

int psy_audio_luabind_array_gc(lua_State *L)
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
		if (index < 0 && index >= (intptr_t) psy_audio_array_len(*ud)) {
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
	  size_t len;
	  const char* key;

	  luaL_checkudata(L, 1, luaarraybind_meta);
	  key = luaL_checklstring(L, 2, &len);
	  lua_getmetatable(L, 1);
	  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
	    const char* method = luaL_checklstring(L, -2, &len);
	    int result = strcmp(key, method);
	    if (result == 0) {
	      return 1;
	    }
	  }
	}
	// no method found
	return 0;
}

int psy_audio_luabind_array_new_index(lua_State *L)
{
	if (lua_isnumber(L, 2)) {
		intptr_t index;
		float value;

		psy_audio_Array** ud = (psy_audio_Array**) luaL_checkudata(L, 1,
			luaarraybind_meta);
		index = (intptr_t) luaL_checkinteger(L, 2);
		value = (float) luaL_checknumber(L, 3);
		if (!(0 <= index && index < (intptr_t) psy_audio_array_len(*ud))) {
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

int array_rsum(lua_State* L)
{
	psy_audio_Array* v = *(psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_Array** rv = (psy_audio_Array**)lua_newuserdata(L, sizeof(psy_audio_Array*));
	*rv = malloc(sizeof(psy_audio_Array));
	psy_audio_array_init_len(*rv, psy_audio_array_len(v), 0);
	luaL_setmetatable(L, luaarraybind_meta);
	psy_audio_array_rsum(*rv, 0);
	return 1;
}

int array_method_rsum(lua_State* L)
{  
  psy_audio_Array* rv = *(psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
  int n = lua_gettop(L);
  double sum = 0;
  if (n==2) {
    sum = luaL_checknumber(L, 2);
  }
  psy_audio_array_rsum(rv, luaL_checknumber(L, 2));
  return psyclescript_chaining(L);
}

int array_method_max(lua_State* L)
{
	psy_audio_Array* rv;
	
	rv = *(psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	if (lua_isuserdata(L, 2)) {
		psy_audio_Array* v = *(psy_audio_Array **) luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L,
			psy_audio_array_len(rv) == psy_audio_array_len(v), 2,
			"size not compatible");
		psy_audio_array_max_array(rv, v);    
	} else {    
		psy_audio_array_max_constant(rv, (float)luaL_checknumber(L, 2));
	}
	return psyclescript_chaining(L);
}

int array_method_min(lua_State* L)
{
	psy_audio_Array* rv;
	
	rv = *(psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	if (lua_isuserdata(L, 2)) {
		psy_audio_Array* v = *(psy_audio_Array **) luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L,
			psy_audio_array_len(rv) == psy_audio_array_len(v), 2,
			"size not compatible");
		psy_audio_array_min_array(rv, v);    
	} else {    
		psy_audio_array_min_constant(rv, (float)luaL_checknumber(L, 2));
	}
	return psyclescript_chaining(L);
}

int array_method_band(lua_State* L)
{
	psy_audio_Array* rv;
	
	rv = *(psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	if (lua_isuserdata(L, 2)) {
		psy_audio_Array* v = *(psy_audio_Array **)luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L,
			psy_audio_array_len(rv) == psy_audio_array_len(v), 2,
			"size not compatible");
		psy_audio_array_band_array(rv, v);
	} else {    
		psy_audio_array_band_constant(rv, (float)luaL_checknumber(L, 2));
	}
	return psyclescript_chaining(L);
}

int array_method_bor(lua_State* L)
{
	psy_audio_Array* rv;
	
	rv = *(psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	if (lua_isuserdata(L, 2)) {
		psy_audio_Array* v = *(psy_audio_Array **)luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L,
			psy_audio_array_len(rv) == psy_audio_array_len(v), 2,
			"size not compatible");
		psy_audio_array_bor_array(rv, v);
	} else {    
		psy_audio_array_bor_constant(rv, (float)luaL_checknumber(L, 2));
	}
	return psyclescript_chaining(L);
}

int array_method_bxor(lua_State* L)
{
	psy_audio_Array* rv;
	
	rv = *(psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	if (lua_isuserdata(L, 2)) {
		psy_audio_Array* v = *(psy_audio_Array **)luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L,
			psy_audio_array_len(rv) == psy_audio_array_len(v), 2,
			"size not compatible");
		psy_audio_array_bxor_array(rv, v);
	} else {    
		psy_audio_array_bxor_constant(rv, (float)luaL_checknumber(L, 2));
	}
	return psyclescript_chaining(L);
}

int array_method_sleft(lua_State* L)
{
	psy_audio_Array* rv;
	
	rv = *(psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	if (lua_isuserdata(L, 2)) {
		psy_audio_Array* v = *(psy_audio_Array **)luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L,
			psy_audio_array_len(rv) == psy_audio_array_len(v), 2,
			"size not compatible");
		psy_audio_array_sleft_array(rv, v);
	} else {    
		psy_audio_array_sleft_constant(rv, (float)luaL_checknumber(L, 2));
	}
	return psyclescript_chaining(L);
}

int array_method_sright(lua_State* L)
{
	psy_audio_Array* rv;
	
	rv = *(psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	if (lua_isuserdata(L, 2)) {
		psy_audio_Array* v = *(psy_audio_Array **)luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L,
			psy_audio_array_len(rv) == psy_audio_array_len(v), 2,
			"size not compatible");
		psy_audio_array_sright_array(rv, v);
	} else {    
		psy_audio_array_sright_constant(rv, (float)luaL_checknumber(L, 2));
	}
	return psyclescript_chaining(L);
}

int array_method_bnot(lua_State* L)
{
	psy_audio_Array** ud;

	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_bnot(*ud);
	return psyclescript_chaining(L);
}

int array_method_size(lua_State* L)
{
	psy_audio_Array** ud;

	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	lua_pushinteger(L, psy_audio_array_len(*ud));
	return 1;
}

int array_copy(lua_State* L)
{
	int n;
	psy_audio_Array* dest;
	
	n = lua_gettop(L);
	dest = *(psy_audio_Array **)luaL_checkudata(L, 1, luaarraybind_meta);
	if (n==3) {
		uintptr_t pos = (uintptr_t) luaL_checknumber (L, 2);
		psy_audio_Array* src = *(psy_audio_Array **)luaL_checkudata(L, 3, luaarraybind_meta);
		
		if (!psy_audio_array_copyfrom(dest, src, pos)) {
			luaL_error(L, "Array out of range error");
		}
	} else {
		psy_audio_Array* src = *(psy_audio_Array **)luaL_checkudata(L, 2, luaarraybind_meta);
		if (!psy_audio_array_copy(dest, src)) {
			luaL_error(L, "Array out of range error");
		}
	}
	return psyclescript_chaining(L);
}

int array_method_fillzero(lua_State* L)
{
	psy_audio_Array** ud;

	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_fillzero(*ud);
	return psyclescript_chaining(L);
}

int array_method_fill(lua_State* L)
{
	psy_audio_Array** ud;

	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_fill(*ud, (float) luaL_checknumber(L, 2));
	return psyclescript_chaining(L);
}

int array_method_resize(lua_State* L)
{
	psy_audio_Array** ud;

	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_resize(*ud, (int) luaL_checknumber(L, 2));
	return psyclescript_chaining(L);
}

int array_tostring(lua_State* L)
{
	char* str;
	uintptr_t i;

	psy_audio_Array* ud = *(psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	str = malloc(ud->len_ * 22 + 3);
	if (str) {
		strcpy(str, "[");
		for (i = 0; i < ud->len_; ++i) {
			char strval[20];
			psy_snprintf(strval, 20, "%f", ud->ptr_[i]);
			strcat(str, strval);
			strcat(str, " ");
		}
		strcat(str, "]");
		lua_pushfstring(L, str);
		free(str);
		return 1;
	}
	return 0;
}

int array_method_margin(lua_State* L)
{
	psy_audio_Array** ud;

	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_margin(*ud, (int) luaL_checkinteger(L, 2), (int) luaL_checkinteger(L, 3));
	return psyclescript_chaining(L);
}

int array_method_clearmargin(lua_State* L)
{
	psy_audio_Array** ud;

	ud = (psy_audio_Array**) luaL_checkudata(L, 1, luaarraybind_meta);
	psy_audio_array_clearmargin(*ud);
	return psyclescript_chaining(L);
}

// static methods
int array_mul(lua_State* L)
{
	if ((lua_isuserdata(L, 1)) && (lua_isuserdata(L, 2))) {
		psy_audio_Array* rv;
		psy_audio_Array* v;

		rv = create_copy_array(L, 1);
		 v = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L, psy_audio_array_len(rv) == psy_audio_array_len(v), 2, "size not compatible");
		psy_audio_array_mul_array(rv, v);
	}
	else {
		psy_audio_Array* rv;

		rv = create_copy_array(L, 1);
		if ((lua_isuserdata(L, 1)) && (lua_isnumber(L, 2))) {
			lua_Number param1;

			param1 = luaL_checknumber(L, 2);
			psy_audio_array_mul_constant(rv, (float) param1);
		} else {
			lua_Number param1;

			param1 = luaL_checknumber(L, 1);
			rv = create_copy_array(L, 2);
			psy_audio_array_mul_constant(rv, (float) param1);
		}
	}
	return 1;
}

int array_div(lua_State* L)
{
	if ((lua_isuserdata(L, 1)) && (lua_isuserdata(L, 2))) {
		psy_audio_Array* rv;
		psy_audio_Array* v;

		rv = create_copy_array(L, 1);
		v = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L, psy_audio_array_len(rv) == psy_audio_array_len(v), 2, "size not compatible");
		psy_audio_array_div_array(rv, v);
	}
	else {
		psy_audio_Array* rv;

		rv = create_copy_array(L, 1);
		if ((lua_isuserdata(L, 1)) && (lua_isnumber(L, 2))) {
			lua_Number param1;

			param1 = luaL_checknumber(L, 2);
			psy_audio_array_div_constant(rv, (float) param1);
		}
		else {
			lua_Number param1;

			param1 = luaL_checknumber(L, 1);
			rv = create_copy_array(L, 2);
			psy_audio_array_div_constant(rv, (float) param1);
		}
	}
	return 1;
}

int array_add(lua_State* L)
{
	if ((lua_isuserdata(L, 1)) && (lua_isuserdata(L, 2))) {
		psy_audio_Array* rv;
		psy_audio_Array* v;

		rv = create_copy_array(L, 1);
		v = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L, psy_audio_array_len(rv) == psy_audio_array_len(v), 2, "size not compatible");
		psy_audio_array_add_array(rv, v);
	}
	else {
		psy_audio_Array* rv;

		rv = create_copy_array(L, 1);
		if ((lua_isuserdata(L, 1)) && (lua_isnumber(L, 2))) {
			lua_Number param1;

			param1 = luaL_checknumber(L, 2);
			psy_audio_array_add_constant(rv, (float) param1);
		}
		else {
			lua_Number param1;

			param1 = luaL_checknumber(L, 1);
			rv = create_copy_array(L, 2);
			psy_audio_array_add_constant(rv, (float) param1);
		}
	}
	return 1;
}

int array_sub(lua_State* L)
{
	if ((lua_isuserdata(L, 1)) && (lua_isuserdata(L, 2))) {
		psy_audio_Array* rv;
		psy_audio_Array* v;

		rv = create_copy_array(L, 1);
		v = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L, psy_audio_array_len(rv) == psy_audio_array_len(v), 2, "size not compatible");
		psy_audio_array_sub_array(rv, v);
	}
	else {
		psy_audio_Array* rv;

		rv = create_copy_array(L, 1);
		if ((lua_isuserdata(L, 1)) && (lua_isnumber(L, 2))) {
			lua_Number param1;

			param1 = luaL_checknumber(L, 2);
			psy_audio_array_sub_constant(rv, (float) param1);
		}
		else {
			lua_Number param1;

			param1 = luaL_checknumber(L, 1);
			rv = create_copy_array(L, 2);
			psy_audio_array_sub_constant(rv, (float) param1);
		}
	}
	return 1;
}

int array_sin(lua_State* L)
{
	psy_audio_Array* rv;

	rv = create_copy_array(L, 1);
	psy_audio_array_sin(rv);	
	return 1;
}

int array_cos(lua_State* L)
{
	psy_audio_Array* rv;

	rv = create_copy_array(L, 1);
	psy_audio_array_cos(rv);
	return 1;
}

int array_tan(lua_State* L)
{
	psy_audio_Array* rv;

	rv = create_copy_array(L, 1);
	psy_audio_array_tan(rv);
	return 1;
}

int array_sqrt(lua_State* L)
{
	psy_audio_Array* rv;

	rv = create_copy_array(L, 1);
	psy_audio_array_sqrt(rv);	
	return 1;
}

int array_unm(lua_State* L)
{
	psy_audio_Array* rv;

	rv = create_copy_array(L, 1);
	psy_audio_array_mul_constant(rv, -1);
	return 1;
}

int array_concat(lua_State* L)
{
	if ((lua_isuserdata(L, 1)) && (lua_isuserdata(L, 2))) {
		psy_audio_Array* rv;
		psy_audio_Array* v;

		rv = create_copy_array(L, 1);
		v = *(psy_audio_Array**) luaL_checkudata(L, 2, luaarraybind_meta);
		psy_audio_array_concat(rv, v);
	} else {
		luaL_error(L, "No array userdata error");
	}
	return 1;
}

int array_pow(lua_State* L)
{
	if ((lua_isuserdata(L, 1)) && (lua_isuserdata(L, 2))) {
		psy_audio_Array* rv;
		psy_audio_Array* v;

		rv = create_copy_array(L, 1);
		v = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L, psy_audio_array_len(rv) == psy_audio_array_len(v), 2, "size not compatible");
		psy_audio_array_pow_array(rv, v);
	}
	else {
		psy_audio_Array* rv;

		rv = create_copy_array(L, 1);
		if ((lua_isuserdata(L, 1)) && (lua_isnumber(L, 2))) {
			lua_Number param1;

			param1 = luaL_checknumber(L, 2);
			psy_audio_array_pow_array_constant(rv, (float)param1);
		}
		else {
			lua_Number param1;

			param1 = luaL_checknumber(L, 1);
			rv = create_copy_array(L, 2);
			psy_audio_array_pow_constant_array(rv, (float)param1);
		}
	}
	return 1;
}

int array_mod(lua_State* L)
{
	if ((lua_isuserdata(L, 1)) && (lua_isuserdata(L, 2))) {
		psy_audio_Array* rv;
		psy_audio_Array* v;

		rv = create_copy_array(L, 1);
		v = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
		luaL_argcheck(L, psy_audio_array_len(rv) == psy_audio_array_len(v), 2, "size not compatible");
		psy_audio_array_fmod_array(rv, v);
	} else {
		psy_audio_Array* rv;

		rv = create_copy_array(L, 1);
		if ((lua_isuserdata(L, 1)) && (lua_isnumber(L, 2))) {
			lua_Number param1;

			param1 = luaL_checknumber(L, 2);
			psy_audio_array_fmod_array_constant(rv, (float)param1);
		} else {
			lua_Number param1;

			param1 = luaL_checknumber(L, 1);
			rv = create_copy_array(L, 2);
			psy_audio_array_fmod_constant_array(rv, (float)param1);
		}
	}
	return 1;
}

int array_method_from_table(lua_State* L)
{
	psy_audio_Array* rv = *(psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	lua_Unsigned len = lua_rawlen(L, 2);
	float* ptr = psy_audio_array_data(rv);
	size_t i;

	for (i = 1; i <= len; ++i) {
		lua_rawgeti(L, 2, i);
		*ptr++ = (float) luaL_checknumber(L, -1);
		lua_pop(L, 1);
	}
	return psyclescript_chaining(L);
}

int array_method_to_table(lua_State* L)
{
	psy_audio_Array* rv = *(psy_audio_Array**)luaL_checkudata(L, 1, luaarraybind_meta);
	size_t i;

	lua_createtable(L, psy_audio_array_len(rv), 0);
	for (i = 0; i < psy_audio_array_len(rv); ++i) {
		lua_pushnumber(L, psy_audio_array_at(rv, i));
		lua_rawseti(L, 2, i + 1);
	}
	return 1;
}

#ifdef _WIN32
#pragma warning( pop )
#endif