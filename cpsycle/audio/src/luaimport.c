// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "luaimport.h"

#include <assert.h>

static int psy_luaimport_get_method_optional(psy_LuaImport*,
	const char*  method);
static int psy_luaimport_error_handler(lua_State* L);

void psy_luaimport_init(psy_LuaImport* self, lua_State* L, void* target)
{
	self->L_ = L,        
	// lh_(lh),
	self->begin_top_ = 0;
	self->num_ret_ = 0;
	self->is_open_ = 0;
	self->lasterr = 0;
	self->target_ = target;
}

void psy_luaimport_dispose(psy_LuaImport* self)
{
    psy_luaimport_close(self);  
}

void psy_luaimport_settarget(psy_LuaImport* self, void* target)
{    
    psy_luaimport_close(self);
    self->target_ = target;
}

int psy_luaimport_open(psy_LuaImport* self, const char* method)
{
    if (self->is_open_) {
		psy_luaimport_close(self);
    }    
    if (!self->target_) {
      return 0;
    }
    // if (lh_) {
      //lh_->lock();
    //}    
    lua_pushcfunction(self->L_, psy_luaimport_error_handler);
    self->begin_top_ = lua_gettop(self->L_);
    if (method == 0) {
      self->is_open_ = 1; 
      return 1;
    };
    if (psy_luaimport_get_method_optional(self, method)) {
      self->is_open_= 1;      
    } else {
      lua_pop(self->L_, 1); // pop error_handler func
      //if (lh_) {
        //lh_->unlock();
      //}
      self->is_open_ = 0;
    }
    return self->is_open_;
  }

void psy_luaimport_pcall(psy_LuaImport* self, int numret)
{
	int curr_top;
	int argsnum;
	int status;

	if (!self->is_open_) {
		static const char* msg = "LUAIMPORT NOT OPEN";

		self->lasterr = psy_LUAIMPORT_ERR_NOT_OPEN;
		self->errmsg = msg;
		return;
	}
	curr_top = lua_gettop(self->L_);
	self->num_ret_ = numret;
	argsnum = curr_top - self->begin_top_ - 1;
	assert(argsnum > 0);    
//	#ifdef LUAERRORHANDLER
	status = lua_pcall(self->L_, argsnum, numret, -argsnum - 2);
//	#else
//	int status = lua_pcall(L_, argsnum, numret, 0);
//	#endif
	if (status) {        
	  const char* msg = lua_tostring(self->L_, -1);
	  self->errmsg = msg;
	  self->lasterr = psy_LUAIMPORT_ERR_PCALL;
//	  if (lh_) {
//		lh_->unlock();      
//	  }
	  self->is_open_ = 0;
	//  throw std::runtime_error(msg);        
	}
}

void psy_luaimport_close(psy_LuaImport* self)
{
	int end_top;

	if (self->is_open_) {
		if (self->num_ret_ > 0) {
			lua_pop(self->L_, self->num_ret_);
		}
		end_top = lua_gettop(self->L_);      
		lua_pop(self->L_, 1); // pop error_handler func
	//      if (lh_) {
	//        lh_->unlock(); 
	//      } 
		if (end_top != self->begin_top_) {
			const char* msg = "LuaImport close Lua stack incorrect";
			//throw std::runtime_error(msg);
		}             
		self->is_open_ = 0;
	}
}  
//  lua_State* L() const { return L_; }
//  int begin_top() const { return begin_top_; }
//  const LockIF* lh() const { return lh_; }  
//  void* target() const { return target_; }
  
void psy_luaimport_findweakuserdata(psy_LuaImport* self)
{
	lua_getglobal(self->L_, "psycle");
	if (!lua_isnil(self->L_, -1)) {
		lua_getfield(self->L_, -1, "weakuserdata");
		if (!lua_isnil(self->L_, -1)) {
			lua_pushlightuserdata(self->L_, self->target_);
			lua_gettable(self->L_, -2);
			if (lua_istable(self->L_, -1)) {
				lua_remove(self->L_, -2);
			} 
			lua_remove(self->L_, -2);
		} else {
			assert(0);
		}
	} else {
		assert(0);
	}  
}

int psy_luaimport_get_method_optional(psy_LuaImport* self, 
	  const char*  method)
{	
	psy_luaimport_findweakuserdata(self);
	if (lua_isnil(self->L_, -1)) {
		static const char* msg = "LUAIMPORT NO PROXY";

		assert(0);
		lua_pop(self->L_, 1);
		self->lasterr = psy_LUAIMPORT_ERR_NO_PROXY;
		self->errmsg = msg;
		return 0;
	}    
	lua_getfield(self->L_, -1, method);
	if (lua_isnil(self->L_, -1) || lua_iscfunction(self->L_, -1)) {
		lua_pop(self->L_, 2);
		return 0;
	}   
	lua_insert(self->L_, -2);    
	return 1;	
}

int psy_luaimport_error_handler(lua_State* L)
{  
	const char* msg = lua_tostring(L, -1);	
    return 1; // error in error handler
}
