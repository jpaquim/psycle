// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <lua.hpp>

#undef GetGValue
#define GetGValue(rgb) (LOBYTE((rgb) >> 8))

#define LUASIZECHKBEG int stack_size_begin__ = lua_gettop(L);
#define LUASIZECHKEND int stack_size_end__ = lua_gettop(L); assert(stack_size_begin__ == stack_size_end__);

#define LUAERRORHANDLER

#define LUAEXPORT(L, F) { int ret = 0; try { ret = LuaHelper::bind(L, T::type(), F); } \
   catch(const std::exception &e)   \
   {   \
     return luaL_error(L, e.what());   \
   } \
   return ret!=0  ? ret : LuaHelper::chaining(L); }

#define LUAEXPORTM(L, M, F) { int ret = 0; try { ret = LuaHelper::bind(L, M, F); } \
   catch(const std::exception &e)   \
   {   \
     return luaL_error(L, e.what());   \
   } \
   return ret!=0  ? ret : LuaHelper::chaining(L); }

namespace psycle {
namespace host {  

namespace luaerrhandler
{
int error_handler(lua_State* L);
}

class LockIF {
  public:
  LockIF() {}
  virtual ~LockIF() = 0;
  virtual void lock() const = 0;
  virtual void unlock() const = 0;
};

inline LockIF::~LockIF() { }

struct LuaImport {
  LuaImport(lua_State* L, void* target, const LockIF* lh)
      : L_(L),        
        lh_(lh),
        begin_top_(0),
        num_ret_(0),
        is_open_(false),
        target_(target) {
  }

  ~LuaImport() {
    close();
  }
  
  void set_target(void* target) {    
    close();
    target_ = target;
  }

  bool open(const std::string& method = "") {
    if (is_open_) {
      close();
    }    
    if (!target_) {
      return false;
    }
    if (lh_) {
      lh_->lock();
    }    
    lua_pushcfunction(L_, luaerrhandler::error_handler);
    begin_top_ = lua_gettop(L_);
    if (method =="") {
      is_open_ = true; 
      return true;
    };
    if (get_method_optional(method)) {
      is_open_= true;      
    } else {
      lua_pop(L_, 1); // pop error_handler func
      if (lh_) {
        lh_->unlock();
      }
      is_open_ = false;
    }
    return is_open_;
  }

  void pcall(int numret) {
    if (!is_open_) {
      throw std::runtime_error("LuaImport not open");
    }
    int curr_top = lua_gettop(L_);
    num_ret_ = numret;
    int argsnum = curr_top - begin_top_ - 1;
    assert(argsnum > 0);    
#ifdef LUAERRORHANDLER
    int status = lua_pcall(L_, argsnum, numret, -argsnum - 2);
#else
    int status = lua_pcall(L_, argsnum, numret, 0);
#endif
    if (status) {        
      const char* msg = lua_tostring(L_, -1);
      if (lh_) {
        lh_->unlock();      
      }
      is_open_ = false;
      throw std::runtime_error(msg);        
    }
  }
  void close() {
    if (is_open_) {
      if (num_ret_ > 0) {
        lua_pop(L_, num_ret_);
      }
      const int end_top = lua_gettop(L_);      
      lua_pop(L_, 1); // pop error_handler func
      if (lh_) {
        lh_->unlock(); 
      }        
      assert(end_top == begin_top_);
      is_open_ = false;
    }
  }
  lua_State* L() const { return L_; }
  int begin_top() const { return begin_top_; }
  const LockIF* lh() const { return lh_; }  
  void* target() const { return target_; }
  void find_weakuserdata() {
      lua_getglobal(L_, "psycle");
      lua_getfield(L_, -1, "weakuserdata");
      if (!lua_isnil(L_, -1)) {
        lua_pushlightuserdata(L_, target_);
        lua_gettable(L_, -2);
        if (lua_istable(L_, -1)) {
          lua_remove(L_, -2);
        }        
        lua_remove(L_, -2);
      } else {
        assert(0);
      }
    }
 private:
  bool get_method_optional(const std::string& method) {    
    find_weakuserdata();
    if (lua_isnil(L_, -1)) {
      lua_pop(L_, 1);
      throw std::runtime_error("no proxy found");
    }    
    lua_getfield(L_, -1, method.c_str());        
    if (lua_isnil(L_, -1) || lua_iscfunction(L_, -1)) {
      lua_pop(L_, 2);
      return false;
    }   
    lua_insert(L_, -2);    
    return true;
  }    
  int begin_top_, num_ret_;  
  lua_State* L_;
  const LockIF* lh_;
  int is_open_;  
  void* target_;
};

struct pcall { 
  pcall(int numret) : numret(numret) { }
  void operator()(LuaImport& im) const { im.pcall(numret); }
 private:
  int numret;    
};

struct getparam { 
  getparam(int idx, const std::string& method) : idx(idx), method_(method) {}
   void operator()(LuaImport& import) const { 
     lua_State* L = import.L();     
     if (idx < 0) {       
       throw std::runtime_error("index less then zero");
     }    
     import.find_weakuserdata();
     if (lua_isnil(L, -1)) {
       lua_pop(L, 1);
       throw std::runtime_error("no proxy found");
     }
     lua_getfield(L, -1, "params");
     if (lua_isnil(L, -1)) {
       lua_pop(L, 2);       
       throw std::runtime_error("params not found");
     }    
     lua_rawgeti(L,-1, idx+1);    
    lua_getfield(L, -1, method_.c_str());
    if (lua_isnil(L, -1)) {
      lua_pop(L, 4);
      throw std::runtime_error("no param found");
    }    
    lua_replace(L, -4);
    lua_replace(L, -2);        
   }          
   private:
    int idx;  
    const std::string method_;
  };

template<class T>
inline LuaImport& operator<< (LuaImport &out, const T &val) {
  val(out);
  return out;
}

inline LuaImport& operator<< (LuaImport& import, const bool& val) {
  lua_pushboolean(import.L(), val);  
  return import;
}

inline LuaImport& operator<< (LuaImport& import, const int& val) {
  lua_pushinteger(import.L(), val);  
  return import;
}

inline LuaImport& operator<< (LuaImport& import, const double& val) {
  lua_pushnumber(import.L(), val);  
  return import;
}

inline LuaImport& operator<< (LuaImport& import, const std::string& val) {
  lua_pushstring(import.L(), val.c_str());
  return import;
}

inline LuaImport& operator<< (LuaImport& import, const char* str) {
  lua_pushstring(import.L(), str);
  return import;
}


template<class T>
inline LuaImport& operator>> (LuaImport &out, T &val) {
  val(out);
  return out;
}

inline LuaImport& operator>> (LuaImport& import, int& val) {
  val = static_cast<int>(luaL_checknumber(import.L(), -1));
  return import;
}

inline LuaImport& operator>> (LuaImport& import, double& val) {
  val = luaL_checknumber(import.L(), -1);  
  return import;
}

inline LuaImport& operator>> (LuaImport& import, std::string& str) {
  const char* s = luaL_checkstring(import.L(), -1);
  if (s) {
    str = std::string(s);
  }
  return import;
}


namespace LuaHelper {            	
    // check for shared_ptr
		template <class T>
		static boost::shared_ptr<T> check_sptr(lua_State* L, int index, const std::string& meta) {
			luaL_checktype(L, index, LUA_TTABLE);
			lua_getfield(L, index, "__self");
      typedef boost::shared_ptr<T> SPtr;
			SPtr* ud = (SPtr*) luaL_checkudata(L, -1, meta.c_str());
      luaL_argcheck(L, (ud) != 0, 1, (meta+" expected").c_str());
			lua_pop(L, 1);
			//luaL_argcheck(L, (*SPTR) != 0, 1, (meta+" expected").c_str());			
			return *ud;
		}

    // mimics the bahaviour of luaL_testudata for our own userdata structure
		template <class T>
		static boost::shared_ptr<T> test_sptr(lua_State* L, int index, const std::string& meta) {
			luaL_checktype(L, index, LUA_TTABLE);
			lua_getfield(L, index, "__self");			
      typedef boost::shared_ptr<T> SPtr;
      SPtr* ud = (SPtr*) luaL_testudata(L, -1, meta.c_str());
      if (ud == NULL) {
        lua_pop(L, 1);
        return 0;
      }
			luaL_argcheck(L, (ud) != 0, 1, (meta+" expected").c_str());
			lua_pop(L, 1);
			return *ud;
		}

    static int check_argnum(lua_State* L, int num, const std::string& names) {
      int n = lua_gettop(L);
      if (n != num) {
        std::stringstream s;
        s << "Got " << num << " arguments expected ("<< names <<")";
        return luaL_error(L, s.str().c_str(), n);
      } else {
        return 0;
      }
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
		template <class T>
		static boost::shared_ptr<T> new_shared_userdata(lua_State* L, 
                                    const std::string& meta,
                                    T* ud,
                                    int self=1) {
			lua_pushvalue(L, self);
			int n = lua_gettop(L);
			luaL_checktype(L, -1, LUA_TTABLE);  // self
			lua_newtable(L);  // new
			lua_pushvalue(L, self);
			lua_setmetatable(L, -2);
			lua_pushvalue(L, self);
			lua_setfield(L, self, "__index");
      typedef boost::shared_ptr<T> SPtr;
			SPtr* p = (SPtr*)lua_newuserdata(L, sizeof(SPtr));      
      new(p) SPtr(ud);      
			luaL_getmetatable(L, meta.c_str());
			lua_setmetatable(L, -2);
			lua_setfield(L, -2, "__self");
			lua_remove(L, n);			
      LuaHelper::register_weakuserdata(L, ud);
      return *p;
		}

    template <class T>
		static int delete_shared_userdata(lua_State* L, const std::string& meta) {
      typedef boost::shared_ptr<T> SPtr;
			SPtr* ud = (SPtr*) luaL_checkudata(L, 1, meta.c_str());      
      ud->~SPtr();      
			return 0;
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
        if (lua_istable(L, -1)) {
          lua_remove(L, -2);
        }
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
        if (lua_istable(L, -1)) {
          lua_remove(L, -2);
        }        
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

    // needs to be registered with register_userdata
    template <class UserDataType>
		static void unregister_weakuserdata(lua_State* L, UserDataType* ud) {
      lua_getglobal(L, "psycle");
      lua_getfield(L, -1, "weakuserdata");
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

 		static void get_proxy(lua_State* L) {
			lua_getglobal(L, "psycle");      
			  if (lua_isnil(L, -1)) {
				 lua_pop(L, 1);
				 throw std::runtime_error("no host found");
			  }
			  lua_getfield(L, -1, "proxy");
			  if (lua_isnil(L, -1)) {
				 lua_pop(L, 2);
				 throw std::runtime_error("no proxy found");
			  }
		}

		static void numargcheck(lua_State* L, int numargs) {
      int n = lua_gettop(L);
			if (n != numargs) {
        std::stringstream what;
        what << "Got" << n << "arguments expected" << numargs;
        throw std::runtime_error(what.str());        
      }      
    }

    // void -> void
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(void)) {
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      (ud.get()->*ptmember)();
      return 0;
    }

    // bool -> void
    template <class T>
    static int bind(lua_State* L, const std::string& meta, bool (T::*ptmember)() const) {
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      bool val = (ud.get()->*ptmember)();
      lua_pushboolean(L, val);
      return 1;
    }
    
    // void -> int
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(int)) {
      numargcheck(L, 2);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      int val = luaL_checkinteger(L, 2);
      (ud.get()->*ptmember)(val);
      return 0;
    }
       
    // int -> void
    template <class T>
    static int bind(lua_State* L, const std::string& meta, int (T::*ptmember)()) {
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      int val = (ud.get()->*ptmember)();
      lua_pushinteger(L, val);
      return 1;
    }

    // int -> void const
    template <class T>
    static int bind(lua_State* L, const std::string& meta, int (T::*ptmember)() const) {
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      int val = (ud.get()->*ptmember)();
      lua_pushinteger(L, val);
      return 1;
    }
    

    // int -> void  (const )
    template <class T>
    static int bindinc1(lua_State* L, const std::string& meta, int (T::*ptmember)() const) {      
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      int val = (ud.get()->*ptmember)() + 1;
      lua_pushinteger(L, val);
      return 1;
    }


    // int -> int (const)
    template <class T>
    static int bind(lua_State* L, const std::string& meta, int (T::*ptmember)(int)) {      
      numargcheck(L, 2);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      int v = luaL_checkinteger(L, 2);
      int r = (ud.get()->*ptmember)(v);
      lua_pushinteger(L, r);      
      return 1;
    }

    // int -> int (const)
    template <class T>
    static int bind(lua_State* L, const std::string& meta, int (T::*ptmember)(int) const) {      
      numargcheck(L, 2);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      int v = luaL_checkinteger(L, 2);
      int r = (ud.get()->*ptmember)(v);
      lua_pushinteger(L, r);      
      return 1;
    }
       
    // void -> int X int
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(int, int)) {
      numargcheck(L, 3);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      int val1 = luaL_checkinteger(L, 2);
      int val2 = luaL_checkinteger(L, 3);
      (ud.get()->*ptmember)(val1, val2);      
      return 0;
    }

     // void -> int X int X int X int
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(int, int, int, int)) {
      numargcheck(L, 5);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      int val1 = luaL_checkinteger(L, 2);
      int val2 = luaL_checkinteger(L, 3);
      int val3 = luaL_checkinteger(L, 2);
      int val4 = luaL_checkinteger(L, 3);
      (ud.get()->*ptmember)(val1, val2, val3, val4);      
      return 0;
    }

    // void -> bool X int
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(bool, int)) {
      numargcheck(L, 3);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      bool val1 = luaL_checkinteger(L, 2);
      int val2 = luaL_checkinteger(L, 3);
      (ud.get()->*ptmember)(val1, val2);            
      return 0;
    }

    // void -> int X bool
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(int, bool)) {
      numargcheck(L, 3);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      int val1 = luaL_checkintger(L, 2);
      bool val2 = luaL_checkinteger(L, 3);
      (ud.get()->*ptmember)(val1, val2);      
      return 0;
    }

    // void -> int& X int& (const)
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(int&, int&) const) {
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      int v1(0);
      int v2(0);
      (ud.get()->*ptmember)(v1, v2);
      lua_pushinteger(L, v1);
      lua_pushinteger(L, v2);      
      return 2;
    }

    // void -> int& X int&
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(int&, int&)) {
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      int v1(0);
      int v2(0);
      (ud.get()->*ptmember)(v1, v2);
      lua_pushinteger(L, v1);
      lua_pushinteger(L, v2);      
      return 2;
    }

     // void -> int& X int& X int& X int& (const)
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(int&, int&, int&, int&)) {
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      int v1(0);
      int v2(0);
      int v3(0);
      int v4(0);
      (ud.get()->*ptmember)(v1, v2, v3, v4);
      lua_pushinteger(L, v1);
      lua_pushinteger(L, v2);
      lua_pushinteger(L, v3);
      lua_pushinteger(L, v4);      
      return 4;
    }

    // void -> double
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(double)) {
      numargcheck(L, 2);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      double val = luaL_checknumber(L, 2);
      (ud.get()->*ptmember)(val);      
      return 0;
    }

     // double -> void
    template <class T>
    static int bind(lua_State* L, const std::string& meta, double (T::*ptmember)() const) {      
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      double val = (ud.get()->*ptmember)();
      lua_pushnumber(L, val);    
      return 1;
    }

    // double -> int (const)
    template <class T>
    static int bind(lua_State* L, const std::string& meta, double (T::*ptmember)(int) const) {
      numargcheck(L, 2);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      int v = luaL_checkinteger(L, 2);
      int r = (ud.get()->*ptmember)(v);
      lua_pushnumber(L, r);      
      return 1;
    }

    // void -> double X double
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(double, double)) {
      numargcheck(L, 3);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      double val1 = luaL_checknumber(L, 2);
      double val2 = luaL_checknumber(L, 3);
      (ud.get()->*ptmember)(val1, val2);      
      return 0;
    }

    // void -> double X double X double X double
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(double, double, double, double)) {
      numargcheck(L, 5);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      double val1 = luaL_checknumber(L, 2);
      double val2 = luaL_checknumber(L, 3);
      double val3 = luaL_checknumber(L, 4);
      double val4 = luaL_checknumber(L, 5);
      (ud.get()->*ptmember)(val1, val2, val3, val4);      
      return 0;
    }

     // void -> double& X double& (const)
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(double&, double&) const) {
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      double v1(0);
      double v2(0);
      (ud.get()->*ptmember)(v1, v2);
      lua_pushnumber(L, v1);
      lua_pushnumber(L, v2);      
      return 2;
    }

     // void -> double& X double& (const)
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(double&, double&, double&, double&) const) {
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      double v1(0);
      double v2(0);
      double v3(0);
      double v4(0);
      (ud.get()->*ptmember)(v1, v2, v3, v4);
      lua_pushnumber(L, v1);
      lua_pushnumber(L, v2);                
      lua_pushnumber(L, v3);
      lua_pushnumber(L, v4);      
      return 4;
    }

    // void -> float
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(float)) {
      numargcheck(L, 2);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      float val = luaL_checknumber(L, 2);
      (ud.get()->*ptmember)(val);      
      return 0;
    }

     // float -> void
    template <class T>
    static int bind(lua_State* L, const std::string& meta, float (T::*ptmember)() const) {      
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      float val = (ud.get()->*ptmember)();
      lua_pushnumber(L, val);      
      return 1;
    }

    // void -> float X float
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(float, float)) {
      numargcheck(L, 3);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      float val1 = luaL_checknumber(L, 2);
      float val2 = luaL_checknumber(L, 3);
      (ud.get()->*ptmember)(val1, val2);            
      return 0;
    }

    // void -> float X float X float X float
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(float, float, float, float)) {
      numargcheck(L, 5);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      float val1 = luaL_checknumber(L, 2);
      float val2 = luaL_checknumber(L, 3);
      float val3 = luaL_checknumber(L, 4);
      float val4 = luaL_checknumber(L, 5);
      (ud.get()->*ptmember)(val1, val2, val3, val4);      
      return 0;
    }

     // void -> float& X float& (const)
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(float&, float&) const) {
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      float v1(0);
      float v2(0);
      (ud.get()->*ptmember)(v1, v2);
      lua_pushnumber(L, v1);
      lua_pushnumber(L, v2);      
      return 2;
    }

     // void -> float& X float& (const)
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(float&, float&, float&, float&) const) {
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      float v1(0);
      float v2(0);
      float v3(0);
      float v4(0);
      (ud.get()->*ptmember)(v1, v2, v3, v4);
      lua_pushnumber(L, v1);
      lua_pushnumber(L, v2);                
      lua_pushnumber(L, v3);
      lua_pushnumber(L, v4);                      
      return 4;
    }

    // void -> const std::string&
    template <class T>
    static int bind(lua_State* L, const std::string& meta, void (T::*ptmember)(const std::string&)) {
      numargcheck(L, 2);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      const char* str = luaL_checkstring(L, 2);
      (ud.get()->*ptmember)(str);      
      return 0;
    }

    // const std::string& -> void
    template <class T>
    static int bind(lua_State* L, const std::string& meta, const std::string& (T::*ptmember)() const) { 
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      const std::string& str = (ud.get()->*ptmember)();
      lua_pushstring(L, str.c_str());      
      return 1;
    }

    // std::string -> void
    template <class T>
    static int bind(lua_State* L, const std::string& meta, std::string (T::*ptmember)() const) { 
      numargcheck(L, 1);      
      boost::shared_ptr<T> ud = check_sptr<T>(L, 1, meta.c_str());
      std::string str = (ud.get()->*ptmember)();
      lua_pushstring(L, str.c_str());      
      return 1;
    }

		template <class UDT, class T>
		static int callstrict1(lua_State* L, const std::string& meta,
						       void (UDT::*pt2Member)(T), bool dec1=false) { // function ptr
			int n = lua_gettop(L);
			if (n != 2) {
        luaL_error(L, "Got %d arguments expected 2 (self, value)", n);
      }
			boost::shared_ptr<UDT> ud = LuaHelper::check_sptr<UDT>(L, 1, meta.c_str());
			T val = (T) luaL_checknumber(L, 2);
      if (dec1) val--;
			(ud.get()->*pt2Member)(val);
      return 0;
		}

    
		template <class UDT, class T, class T2>
		static int callstrict2(lua_State* L, const std::string& meta,
						       void (UDT::*pt2Member)(T, T2), bool dec1 = false, bool dec2 = false) { // function ptr
			int n = lua_gettop(L);
			if (n != 3) {
        luaL_error(L, "Got %d arguments expected 3 (self, value, value)", n);
      }
			boost::shared_ptr<UDT> ud = LuaHelper::check_sptr<UDT>(L, 1, meta.c_str());
			T val = luaL_checknumber(L, 2);
			T2 val2 = luaL_checknumber(L, 3);
      if (dec1) val--;
      if (dec2) val2--;
			(ud.get()->*pt2Member)(val, val2);
      return 0;
		}

		template <class UDT, class T>
		static int callopt1(lua_State* L, const std::string& meta,
						    void (UDT::*pt2Member)(T), T def) {
            int n = lua_gettop(L);
			boost::shared_ptr<UDT> ud = LuaHelper::check_sptr<UDT>(L, 1, meta.c_str());
			if (n == 1) {
				(ud.get()->*pt2Member)(def);
			} else
			if (n == 2) {
			  T val = luaL_checknumber(L, 2);
			  (ud.get()->*pt2Member)(val);
			} else {
        luaL_error(L, "Got %d arguments expected 1 or 2 (self [, value])", n);
      }
      return 0;
		}

    template <class UDT, class RT>
		static int getnumber(lua_State* L,
			                const std::string& meta,
						    RT (UDT::*pt2ConstMember)() const) { // function ptr
			int n = lua_gettop(L);
      if (n ==1) {
        typedef boost::shared_ptr<UDT> S;
		    S m = LuaHelper::check_sptr<UDT>(L, 1, meta.c_str());
			  lua_pushnumber(L, (m.get()->*pt2ConstMember)());
			}  else {
        luaL_error(L, "Got %d arguments expected 1 (self)", n);
	    }
			return 1;
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