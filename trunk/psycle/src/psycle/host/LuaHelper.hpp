#pragma once
#include <lua.hpp>

namespace psycle { namespace host {
	struct LuaHelper {
		template <class UserDataType>
		static UserDataType* check(lua_State* L, int index, const std::string& meta) {
			luaL_checktype(L, index, LUA_TTABLE); 
			lua_getfield(L, index, "__self");	
			UserDataType** ud = (UserDataType**) luaL_checkudata(L, -1, meta.c_str());
			luaL_argcheck(L, (*ud) != 0, 1, (meta+" expected").c_str());
			lua_pop(L, 1);
			return *ud;      
		}
		template <class UserDataType>
		static UserDataType* new_userdata(lua_State* L, const std::string& meta, UserDataType* ud, int self=1) {
			lua_pushvalue(L, self);
			int n = lua_gettop(L);
			luaL_checktype(L, -1, LUA_TTABLE);  // self
			lua_newtable(L);  // new 
			lua_pushvalue(L, self);
			lua_setmetatable(L, -2);
			lua_pushvalue(L, self);
			lua_setfield(L, self, "__index");  
			UserDataType ** udata = (UserDataType **)lua_newuserdata(L, sizeof(UserDataType *));	
			*udata = ud;
			luaL_getmetatable(L, meta.c_str());
			lua_setmetatable(L, -2);
			lua_setfield(L, -2, "__self");    
			lua_remove(L, n);		
			return ud;
		}

		static void stackDump (lua_State *L) {
			int i;
			int top = lua_gettop(L);
			for (i = 1; i <= top; i++) {  /* repeat for each level */
				int t = lua_type(L, i);
				switch (t) {

				case LUA_TSTRING:  /* strings */
					OutputDebugString(lua_tostring(L, i));
					//printf("`%s'", lua_tostring(L, i));
					break;

				case LUA_TBOOLEAN:  /* booleans */
					OutputDebugString(lua_toboolean(L, i) ? "true" : "false");
					break;

				case LUA_TNUMBER:  /* numbers */
					OutputDebugString("number"); // %g", lua_tonumber(L, i));
					break;

				default:  /* other values */
					OutputDebugString(lua_typename(L, t));
					break;

				}
				OutputDebugString("  ");  /* put a separator */
			}
			OutputDebugString("\n");  /* end the listing */
		}	


	};
}  // namespace
}  // namespace
