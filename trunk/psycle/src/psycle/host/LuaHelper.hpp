// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <lua.hpp>
#include "Machine.hpp" // for universalis exception
#include "canvas.hpp"

#undef GetGValue
#define GetGValue(rgb) (LOBYTE((rgb) >> 8))

namespace psycle {
namespace host {
namespace LuaHelper {
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

    static int check_argnum(lua_State* L, int num, const std::string& names) {    
      int n = lua_gettop(L);  // Number of arguments
      if (n != num) {
        std::stringstream s;
        s << "Got " << num << " arguments expected ("<< names <<")"; 
        return luaL_error(L, s.str().c_str(), n); 
      } else {
        return 0;
      }
    }

    // mimics the bahaviour of luaL_testudata for our own userdata structure
		template <class UserDataType>
		static UserDataType* test(lua_State* L, int index, const std::string& meta) {
			luaL_checktype(L, index, LUA_TTABLE); 
			lua_getfield(L, index, "__self");	
			UserDataType** ud = (UserDataType**) luaL_testudata(L, -1, meta.c_str());
      if (ud == NULL) {
        lua_pop(L, 1);
        return 0;
      }
			luaL_argcheck(L, (*ud) != 0, 1, (meta+" expected").c_str());
			lua_pop(L, 1);
			return *ud;      
		}

    template <class UDT>
    static void require(lua_State* L, const std::string& name)  {      
      luaL_requiref(L, name.c_str(), UDT::open, 1);
      lua_pop(L, 1);
    }
    
    static int open(lua_State* L,
                    const std::string& meta,
                    const luaL_Reg methods[],
                    lua_CFunction gc=0,
                    lua_CFunction tostring=0) {
      luaL_newmetatable(L, meta.c_str());
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

    static int openmeta(lua_State* L,
                        const std::string& meta,                        
                        lua_CFunction gc=0,
                        lua_CFunction tostring=0) {
      luaL_newmetatable(L, meta.c_str());
      if (gc) {
        lua_pushcclosure(L, gc, 0);
        lua_setfield(L,-2, "__gc");
      }
      if (tostring) {
        lua_pushcclosure(L, tostring, 0);
        lua_setfield(L,-2, "__tostring");
      }      
      return 1;
    }

    template <class Base>
    static int openex(lua_State* L,
                      const std::string& meta,
                      lua_CFunction setmethods=0,
                      lua_CFunction gc=0,
                      lua_CFunction tostring=0) {
       openmeta(L, meta, gc);
       lua_newtable(L);
       Base::setmethods(L);
       setmethods(L);
       return 1;
    }

		// creates userdata able to support inheritance
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
    template <class UDT>
		static void register_userdata(lua_State* L, UDT* ud) {
       lua_getglobal(L, "psycle");
       lua_getfield(L, -1, "userdata");
       lua_pushlightuserdata(L, ud);
       lua_pushvalue(L, -4);
       lua_settable(L, -3);
       lua_pop(L, 2);   
    }

    // new userdata needs to be on the top of the stack
    template <class UserDataType>
		static void register_weakuserdata(lua_State* L, UserDataType* ud) {
       lua_getglobal(L, "psycle");
       lua_getfield(L, -1, "weakuserdata");
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
      if (!lua_isnil(L, -1)) {
        lua_pushlightuserdata(L, ud);
        lua_gettable(L, -2);                 
        lua_remove(L, -2);
        lua_remove(L, -2);
      } else {
        assert(0);         
      }
    }

    // needs to be registered with register_weakuserdata
    template <class UserDataType>
		static void find_weakuserdata(lua_State* L, UserDataType* ud) {
      lua_getglobal(L, "psycle");
      lua_getfield(L, -1, "weakuserdata");      
      if (!lua_isnil(L, -1)) {
        lua_pushlightuserdata(L, ud);
        lua_gettable(L, -2);                 
        lua_remove(L, -2);
        lua_remove(L, -2);
      } else {
        assert(0);         
      }
    }

    static int chaining(lua_State* L) {
      lua_pushvalue(L, 1);
      return 1;
    }

    // needs to be registered with register_userdata
    template <class UserDataType>
		static void unregister_userdata(lua_State* L, UserDataType* ud) {
      lua_getglobal(L, "psycle");
      lua_getfield(L, -1, "userdata");             
      lua_pushlightuserdata(L, ud);
      lua_pushnil(L);
      lua_settable(L, -3);
      lua_pop(L, 2);
    }

    // build enum table
    static int buildenum(lua_State* L, const char* const e[], int len, int startidx = 0) {      
      for (int i=0; i < len; ++i) {
        lua_pushnumber(L, i+startidx);
        lua_setfield(L, -2, e[i]);
      }
      return 0;
    }

 
    // c iterator for orderedtable
		static int luaL_orderednext(lua_State *L) {
		  luaL_checkany(L, -1);                 // previous key
		  luaL_checktype(L, -2, LUA_TTABLE);    // self
		  luaL_checktype(L, -3, LUA_TFUNCTION); // iterator
		  lua_pop(L, 1);                        // pop the key since 
												                    // opair doesn't use it
		  // iter(self)
		  lua_pushvalue(L, -2);
		  lua_pushvalue(L, -2);
		  lua_call(L, 1, 2);

		  if(lua_isnil(L, -2)) {
			  lua_pop(L, 2);
			  return 0;
		  }
		  return 2;
		}

    // push argb
    static int push_argb(lua_State* L, COLORREF cr) {
      int r, g, b;
      r = GetRValue(cr);
      g = GetGValue(cr);
      b = GetBValue(cr);     
      int a = 0;
      using namespace ui;
      ARGB argb = (((ARGB) (b) << 0)  |
                   ((ARGB) (g) << 8)  |
                   ((ARGB) (r) << 16) |
                   ((ARGB) (a) << 24));
       lua_pushnumber(L, argb);       
       return 1;
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
		static int delete_userdata(lua_State* L, const std::string& meta) {
			UserDataType* ptr = *(UserDataType **)luaL_checkudata(L, 1, meta.c_str());
	    delete ptr;
			return 0;
		}

		template <class UserDataType>
		static UserDataType* check_tostring(lua_State* L, const std::string& meta) {
		    return (lua_isuserdata(L, 1))
	               ? *(UserDataType**) luaL_checkudata(L, 1, meta.c_str())
	               : LuaHelper::check<UserDataType>(L, 1, meta.c_str());
		}
		
    template <class UDT>
		static int getbool(lua_State* L,
			                const std::string& meta,
						    bool (UDT::*pt2ConstMember)() const) { // function ptr
			int n = lua_gettop(L);
      if (n ==1) {
		    UDT* m = LuaHelper::check<UDT>(L, 1, meta.c_str());
			  lua_pushboolean(L, (m->*pt2ConstMember)());		      		    			
			}  else {
        luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	    }
			return 1;
		}

    template <class UDT>
		static int getstring(lua_State* L,
			                const std::string& meta,
						    const std::string& (UDT::*pt2ConstMember)() const) { // function ptr
			int n = lua_gettop(L);
      if (n ==1) {
		    UDT* m = LuaHelper::check<UDT>(L, 1, meta.c_str());
			  lua_pushstring(L, (m->*pt2ConstMember)().c_str());		      		    			
			}  else {
        luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	    }
			return 1;
		}

				
		template <class UDT, class RT>
		static int getnumber(lua_State* L,
			                const std::string& meta,
						    RT (UDT::*pt2ConstMember)() const) { // function ptr
			int n = lua_gettop(L);
      if (n ==1) {
		    UDT* m = LuaHelper::check<UDT>(L, 1, meta.c_str());
			  lua_pushnumber(L, (m->*pt2ConstMember)());		      		    			
			}  else {
        luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	    }
			return 1;
		}

    template <class UDT, class RT>
		static int get2number2(lua_State* L,
			                const std::string& meta,
						    void (UDT::*pt2ConstMember)(RT&, RT&) const) { // function ptr
			int n = lua_gettop(L);
      if (n ==1) {
		    UDT* m = LuaHelper::check<UDT>(L, 1, meta.c_str());
        RT v1, v2;
        (m->*pt2ConstMember)(v1, v2);
			  lua_pushnumber(L, v1);
        lua_pushnumber(L, v2);
			}  else {
        luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	    }
			return 2;
		}

    template <class UDT, class RT>
		static int get4numbers(lua_State* L,
			                const std::string& meta,
						    void (UDT::*pt2ConstMember)(RT&, RT&, RT&, RT&) const) { // function ptr
			int n = lua_gettop(L);
      if (n ==1) {
		    UDT* m = LuaHelper::check<UDT>(L, 1, meta.c_str());
        RT v1, v2, v3, v4;
        (m->*pt2ConstMember)(v1, v2, v3, v4);
			  lua_pushnumber(L, v1);
        lua_pushnumber(L, v2);
        lua_pushnumber(L, v3);
        lua_pushnumber(L, v4);
			}  else {
        luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	    }
			return 4;
		}

    template <class UDT, class CT, class RT>
		static int getnumber1(lua_State* L,
			                    const std::string& meta,                          
						              RT (UDT::*pt2ConstMember)(CT) const) { // function ptr
			int n = lua_gettop(L);
      if (n == 2) {
		    UDT* m = LuaHelper::check<UDT>(L, 1, meta.c_str());
        RT val = luaL_checknumber(L, 2);
			  lua_pushnumber(L, (m->*pt2ConstMember)(val));
			}  else {
        luaL_error(L, "Got %d arguments expected 2 (self)", n); 
	    }
			return 1;
		}

    template <class UDT, class RT1, class RT2>
		static int get2numbers(lua_State* L,
			                     const std::string& meta,                          
						               RT1 (UDT::*pt2ConstMember1)() const,
                           RT2 (UDT::*pt2ConstMember2)() const) { // function ptr			
      int n = lua_gettop(L);  // Number of arguments
      if (n != 1) {
        return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
      }      
      UDT* m = LuaHelper::check<UDT>(L, 1, meta.c_str());
      lua_pushnumber(L, (m->*pt2ConstMember1)());
      lua_pushnumber(L, (m->*pt2ConstMember2)());
      return 2;
		}
		
		template <class UDT>
		static UDT* call(lua_State* L,
			                  const std::string& meta,
						      void (UDT::*pt2Member)()) { // function ptr
			int n = lua_gettop(L); 
			if (n != 1) {
        luaL_error(L, "Got %d arguments expected 1 (self)", n); 
      }   
			UDT* ud = LuaHelper::check<UDT>(L, 1, meta.c_str());			
			(ud->*pt2Member)();
      return ud;
		}

		template <class UDT, class T>
		static int callstrict1(lua_State* L, const std::string& meta,
						       void (UDT::*pt2Member)(T), bool dec1=false) { // function ptr
			int n = lua_gettop(L); 
			if (n != 2) {
        luaL_error(L, "Got %d arguments expected 2 (self, value)", n); 
      }   
			UDT* ud = LuaHelper::check<UDT>(L, 1, meta.c_str());
			T val = (T) luaL_checknumber(L, 2);
      if (dec1) val--;
			(ud->*pt2Member)(val);
      return 0;
		}

		template <class UDT, class T, class T2>
		static int callstrict2(lua_State* L, const std::string& meta,
						       void (UDT::*pt2Member)(T, T2), bool dec1 = false, bool dec2 = false) { // function ptr
			int n = lua_gettop(L); 
			if (n != 3) {
        luaL_error(L, "Got %d arguments expected 3 (self, value, value)", n); 
      }   
			UDT* ud = LuaHelper::check<UDT>(L, 1, meta.c_str());
			T val = luaL_checknumber(L, 2);
			T2 val2 = luaL_checknumber(L, 3);
      if (dec1) val--;
      if (dec2) val2--;
			(ud->*pt2Member)(val, val2);
      return 0;
		}

		template <class UDT, class T>
		static int callopt1(lua_State* L, const std::string& meta,
						    void (UDT::*pt2Member)(T), T def) {
            int n = lua_gettop(L);
			UDT* ud = LuaHelper::check<UDT>(L, 1, meta.c_str());
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

    template <class UDT>
		static int callstrictstr(lua_State* L, const std::string& meta,
						       void (UDT::*pt2Member)(const std::string&), bool dec1=false) { // function ptr
			int n = lua_gettop(L); 
			if (n != 2) {
        luaL_error(L, "Got %d arguments expected 2 (self, string)", n); 
      }   
			UDT* ud = LuaHelper::check<UDT>(L, 1, meta.c_str());
			const char* str = luaL_checkstring(L, 2);      
			(ud->*pt2Member)(str);
      return 0;
		}
		
		// useful for debugging to see the stack state
		static void stackDump (lua_State *L) {			
			int top = lua_gettop(L);
			for (int i = 1; i <= top; ++i) { // repeat for each level
				int t = lua_type(L, i);
				switch (t) {
				  case LUA_TSTRING: // strings
					  OutputDebugString(lua_tostring(L, i));					
					break;
				  case LUA_TBOOLEAN: // booleans
					  OutputDebugString(lua_toboolean(L, i) ? "true" : "false");
					break;
				  case LUA_TNUMBER: // numbers
					  OutputDebugString("number"); // %g", lua_tonumber(L, i));
					break;
				  default: // other values
					  OutputDebugString(lua_typename(L, t));
					break;
				}
				OutputDebugString("  "); // put a separator
			}
			OutputDebugString("\n"); // end the listing
		}	     

	};
}  // namespace
}  // namespace
