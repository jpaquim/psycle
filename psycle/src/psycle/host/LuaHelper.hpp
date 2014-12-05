#pragma once
#include <lua.hpp>

namespace psycle { namespace host {
	struct LuaHelper {
		// mimics the bahaviour of luaL_checkudata for our own userdata structure
		template <class UserDataType>
		static UserDataType* check(lua_State* L, int index, const std::string& meta) {
			luaL_checktype(L, index, LUA_TTABLE); 
			lua_getfield(L, index, "__self");	
			UserDataType** ud = (UserDataType**) luaL_checkudata(L, -1, meta.c_str());
			luaL_argcheck(L, (*ud) != 0, 1, (meta+" expected").c_str());
			lua_pop(L, 1);
			return *ud;      
		}
		// creates our own userdata, that supports inheritance
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

    // new userdata needs to be on the top of the stack
    template <class UserDataType>
		static void register_userdata(lua_State* L, UserDataType* ud) {
       lua_getglobal(L, "psycle");
       lua_getfield(L, -1, "userdata");
       lua_pushlightuserdata(L, ud);
       lua_pushvalue(L, -4);
       lua_settable(L, -3);
       lua_pop(L, 2);   
    }

    // needs to be registered with register_userdata
    template <class UserDataType>
		static void find_userdata(lua_State* L, UserDataType* ud) {
      lua_getglobal(L, "psycle");
      lua_getfield(L, -1, "userdata");
      lua_pushlightuserdata(L, ud);
      lua_gettable(L, -2);
    }

		static int luaL_orderednext(lua_State *L)
		{
		  luaL_checkany(L, -1);                 // previous key
		  luaL_checktype(L, -2, LUA_TTABLE);    // self
		  luaL_checktype(L, -3, LUA_TFUNCTION); // iterator
		  lua_pop(L, 1);                        // pop the key since 
												// opair doesn't use it

		  // iter(self)
		  lua_pushvalue(L, -2);
		  lua_pushvalue(L, -2);
		  lua_call(L, 1, 2);

		  if(lua_isnil(L, -2))
		  {
			lua_pop(L, 2);
			return 0;
		  }
		  return 2;
		}

		static void get_proxy(lua_State* L) {
			lua_getglobal(L, "psycle");
			  if (lua_isnil(L, -1)) {	 
				 lua_pop(L, 1);				 
				 throw psycle::host::exceptions::library_errors::loading_error("no host found");
			  }
			  lua_getfield(L, -1, "proxy");
			  if (lua_isnil(L, -1)) {	 
				 lua_pop(L, 2);				 
				 throw psycle::host::exceptions::library_errors::loading_error("no proxy found");
			  }
		}

		template <class UserDataType>
		static int delete_userdata(lua_State* L, const char* meta) {
			UserDataType* ptr = *(UserDataType **)luaL_checkudata(L, 1, meta);
	        delete ptr;
			return 0;
		}

		template <class UserDataType>
		static UserDataType* check_tostring(lua_State* L, const char* meta) {
		    return (lua_isuserdata(L, 1))
	               ? *(UserDataType**) luaL_checkudata(L, 1, meta)
	               : LuaHelper::check<UserDataType>(L, 1, meta);
		}
		
				
		template <class UDT, class RT>
		static int getnumber(lua_State* L,
			                const char* meta,
						    RT (UDT::*pt2ConstMember)() const) { // function ptr
			int n = lua_gettop(L);
	        if (n ==1) {
		      UDT* m = LuaHelper::check<UDT>(L, 1, meta);
			  lua_pushnumber(L, (m->*pt2ConstMember)());		      		    			
			}  else {
               luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	        }
			return 1;
		}
		
		template <class UDT>
		static int call(lua_State* L,
			                  const char* meta,
						      void (UDT::*pt2Member)()) { // function ptr
			int n = lua_gettop(L); 
			if (n != 1) {
               luaL_error(L, "Got %d arguments expected 1 (self)", n); 
            }   
			UDT* ud = LuaHelper::check<UDT>(L, 1, meta);			
			(ud->*pt2Member)();
            return 0;
		}

		template <class UDT, class T>
		static int callstrict1(lua_State* L, const char* meta,
						       void (UDT::*pt2Member)(T)) { // function ptr
			int n = lua_gettop(L); 
			if (n != 2) {
               luaL_error(L, "Got %d arguments expected 2 (self, value)", n); 
            }   
			UDT* ud = LuaHelper::check<UDT>(L, 1, meta);
			T val = (T) luaL_checknumber(L, 2);
			(ud->*pt2Member)(val);
            return 0;
		}

		template <class UDT, class T, class T2>
		static int callstrict2(lua_State* L, const char* meta,
						       void (UDT::*pt2Member)(T, T2)) { // function ptr
			int n = lua_gettop(L); 
			if (n != 3) {
               luaL_error(L, "Got %d arguments expected 3 (self, value, value)", n); 
            }   
			UDT* ud = LuaHelper::check<UDT>(L, 1, meta);
			T val = luaL_checknumber(L, 2);
			T2 val2 = luaL_checknumber(L, 3);
			(ud->*pt2Member)(val, val2);
            return 0;
		}

		template <class UDT, class T>
		static int callopt1(lua_State* L, const char* meta,
						    void (UDT::*pt2Member)(T), T def) {
            int n = lua_gettop(L);
			UDT* ud = LuaHelper::check<UDT>(L, 1, meta);
			if (n == 1) {
				(ud->*pt2Member)(def);
			} else
			if (n == 2) {
			   T val = luaL_checknumber(L, 2);
			   (ud->*pt2Member)(val);
			} else {
               luaL_error(L, "Got %d arguments expected 1 or 2 (self [, value])", n); 
            }   						
            return 0;
		}
		
		// useful for debugging to see the stack state
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
