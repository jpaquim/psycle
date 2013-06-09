#include <psycle/host/detail/project.hpp>
#include "plugincatcher.hpp"
#include "Song.hpp"
#include "LuaHost.hpp"
#include "LuaPlugin.hpp"

#include <boost/filesystem.hpp>

#include <lua.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>


namespace psycle { namespace host {


const std::string LuaProxy::meta_name = "psyhostcall";
const std::string LuaProxy::userdata_name = "psyhost";

const std::string PSLuaPluginImport::metaname = "psypluginmeta";

void PSLuaPluginImport::load(const char* name) {
   PluginCatcher* plug_catcher = static_cast<PluginCatcher*>(&Global::machineload());  
   PluginInfo* info = plug_catcher->info(name);
   if (info) {
     Song& song =  Global::song();
	 mac_ = song.CreateMachine(info->type, info->dllname.c_str(), 1024, 0);
	 mac_->Init();
	 build_buffer(mac_->samplesV, 256);	
   } else
   throw std::runtime_error("plugin not found error");
}

void PSLuaPluginImport::work(int samples) {
	update_num_samples(samples);	
	mac_->GenerateAudio(samples, false);	
}

void PSLuaPluginImport::build_buffer(std::vector<float*>& buf, int num) {
	    sampleV_.clear();
		std::vector<float*>::iterator it = buf.begin();
		for ( ; it != buf.end(); ++it) {
  		  sampleV_.push_back(PSArray(*it, num));
		}
	}

void PSLuaPluginImport::set_buffer(std::vector<float*>& buf) {
	mac_->change_buffer(buf);
	build_buffer(mac_->samplesV, 256);
}

void PSLuaPluginImport::update_num_samples(int num) {
		psybuffer::iterator it = sampleV_.begin();
		for ( ; it != sampleV_.end(); ++it) {
			(*it).set_len(num);
		}
	}


LuaProxy::LuaProxy(LuaPlugin* plug, lua_State* state) : 
        array_bind_(state),
        num_parameter_(0), 
		plug_(plug) {
  InitializeCriticalSection(&cs);
  set_state(state);
}

LuaProxy::~LuaProxy() {
  DeleteCriticalSection(&cs);
}

void LuaProxy::set_state(lua_State* state) { 
  L = state;
  export_c_funcs();
  info_.mode = MACHMODE_FX;
  array_bind_.set_state(state);
}

void LuaProxy::free_state() {
  if (L) {
    lua_close (L);
  }
  L = 0;
}

void LuaProxy::lock() const {
  ::EnterCriticalSection(&cs);
}

void LuaProxy::unlock() const {
  ::LeaveCriticalSection(&cs);
}

int LuaProxy::set_machine_info(lua_State* L) {
	LuaProxy** ud = (LuaProxy**) luaL_checkudata(L, 1, meta_name.c_str());
	if (lua_istable(L,-1)) {
	  size_t len;
	  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
	     const char* key = luaL_checklstring(L, -2, &len);
		 const char* value = luaL_checklstring(L, -1, &len);
	     if (strcmp(key, "numparameters") == 0) {
			 int v = atoi(value);			
			 (*ud)->num_parameter_ = v;
		 } else
		 if (strcmp(key, "vendor") == 0) {
			 (*ud)->info_.vendor = std::string(value);
		 } else
		 if (strcmp(key, "name") == 0) {
			 (*ud)->info_.name = std::string(value);
		 } else
		 if (strcmp(key, "generator") == 0) {
		   if (std::string(value) == "1") {
			  (*ud)->info_.mode = MACHMODE_GENERATOR;
		   }
		 } else
		 if (strcmp(key, "version") == 0) {
			 (*ud)->info_.version = value;
		 } else
		 if (strcmp(key, "api") == 0) {
			 int v = atoi(value);
			 (*ud)->info_.APIversion = v;
		 }
	  } 
	}	
	return 0;
}

int LuaProxy::message(lua_State* L) {
	size_t len;
	const char* msg = luaL_checklstring(L, 1, &len);
	CString cmsg(msg);	
	AfxMessageBox(cmsg);
	return 0;
}

int LuaProxy::plugin_new(lua_State* L) {
   PSLuaPluginImport** ud = (PSLuaPluginImport**) luaL_checkudata(L, 1, PSLuaPluginImport::metaname.c_str());
   size_t len;
   const char* plug_name = luaL_checklstring(L, 2, &len);   
   PSLuaPluginImport ** udata = (PSLuaPluginImport **)lua_newuserdata(L, sizeof(PSLuaPluginImport *));	
   *udata = new PSLuaPluginImport();
   try {
     (*udata)->load(plug_name);   
   } catch (std::exception &e) {
	  luaL_error(L, "plugin not found error");
   }

   luaL_setmetatable(L, PSLuaPluginImport::metaname.c_str());   
   return 1;
}

int LuaProxy::plugin_work(lua_State* L) {
	PSLuaPluginImport** ud = (PSLuaPluginImport**) luaL_checkudata(L, 1, PSLuaPluginImport::metaname.c_str());
	int num = luaL_checknumber (L, 2);
	(*ud)->work(num);
	return 0;
}

int LuaProxy::plugin_resize(lua_State* L) {
	PSLuaPluginImport** ud = (PSLuaPluginImport**) luaL_checkudata(L, 1, PSLuaPluginImport::metaname.c_str());
	int size = luaL_checknumber (L, 2);	
	(*ud)->update_num_samples(size);
	return 0;
}

int LuaProxy::plugin_channel(lua_State* L) {
	PSLuaPluginImport** ud = (PSLuaPluginImport**) luaL_checkudata(L, 1, PSLuaPluginImport::metaname.c_str());
	int idx = luaL_checknumber (L, 2);	
	PSArray ** udata = (PSArray **)lua_newuserdata(L, sizeof(PSArray *));	
	PSArray* a = (*ud)->channel(idx);
	*udata = new PSArray(a->data(), a->len());
    luaL_setmetatable(L, "array_meta");
	return 1;
}

int LuaProxy::plugin_param(lua_State* L) {
	PSLuaPluginImport** ud = (PSLuaPluginImport**) luaL_checkudata(L, 1, PSLuaPluginImport::metaname.c_str());
	int idx = luaL_checknumber (L, 2);	
	double val = (*ud)->mac()->GetParamValue(idx) / (double) 0xFFFF;
	lua_pushnumber(L, val);
	return 1;
}

int LuaProxy::plugin_numparams(lua_State* L) {
	PSLuaPluginImport** ud = (PSLuaPluginImport**) luaL_checkudata(L, 1, PSLuaPluginImport::metaname.c_str());	
	double val = (*ud)->mac()->GetNumParams();
	lua_pushnumber(L, val);
	return 1;
}


int LuaProxy::plugin_paramname(lua_State* L) {
	PSLuaPluginImport** ud = (PSLuaPluginImport**) luaL_checkudata(L, 1, PSLuaPluginImport::metaname.c_str());
	int idx = luaL_checknumber (L, 2);
	char buf[128];
	(*ud)->mac()->GetParamName(idx, buf);
	lua_pushstring(L, buf);
	return 1;
}

int LuaProxy::plugin_paramdisplay(lua_State* L) {
	PSLuaPluginImport** ud = (PSLuaPluginImport**) luaL_checkudata(L, 1, PSLuaPluginImport::metaname.c_str());
	int idx = luaL_checknumber (L, 2);
	char buf[128];
	(*ud)->mac()-> GetParamValue(idx, buf);
	lua_pushstring(L, buf);
	return 1;
}

int LuaProxy::plugin_setparam(lua_State* L) {
	PSLuaPluginImport** ud = (PSLuaPluginImport**) luaL_checkudata(L, 1, PSLuaPluginImport::metaname.c_str());
	int idx = luaL_checknumber (L, 2);
	double val = luaL_checknumber (L, 3);
	(*ud)->mac()->SetParameter(idx, val * 0xFFFF);
	return 0;
}

int LuaProxy::plugin_gc(lua_State* L) {
	PSLuaPluginImport* ud = *(PSLuaPluginImport**) luaL_checkudata(L, 1, PSLuaPluginImport::metaname.c_str());
	delete ud;
	return 0;
}

int LuaProxy::plugin_setbuffer(lua_State* L) {
	PSLuaPluginImport* ud = *(PSLuaPluginImport**) luaL_checkudata(L, 1, PSLuaPluginImport::metaname.c_str());
	luaL_checktype(L, 2, LUA_TTABLE);	
	std::vector<float*> sampleV;
	for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
  	     PSArray* v = *(PSArray **)luaL_checkudata(L, -1, "array_meta");
		 sampleV.push_back(v->data());		 
	}
	ud->set_buffer(sampleV);
	return 0;
}

void LuaProxy::export_c_funcs() {
  static const luaL_Reg putils_lib[] = {	  
	  { NULL, NULL }
	};
  static const luaL_Reg ph_methods[] = {
  	  {"setinfo", set_machine_info },
	  {"message", message },
  	  { NULL, NULL }
  };
  static const luaL_Reg pl_methods[] = {
  	  {"work", plugin_work },
	  {"new", plugin_new},
	  {"channel", plugin_channel},
	  {"resize", plugin_resize},
	  {"param", plugin_param},
	  {"numparams", plugin_numparams},
	  {"paramdisplay", plugin_paramdisplay},
	  {"paramname", plugin_paramname},
	  {"setparameter", plugin_setparam},
	  {"__gc", plugin_gc},
	  {"setbuffer", plugin_setbuffer},
  	  { NULL, NULL }
  };
  luaL_newmetatable(L, LuaProxy::meta_name.c_str());
  luaL_newlib(L, ph_methods);
  lua_setfield(L, -2, "__index");
  lua_setglobal(L, LuaProxy::meta_name.c_str());
  LuaProxy ** ud = (LuaProxy **)lua_newuserdata(L, sizeof(LuaProxy*));
  *ud = this;
  luaL_setmetatable(L, LuaProxy::meta_name.c_str());
  lua_setglobal(L, LuaProxy::userdata_name.c_str());  
  // pluginimport
  luaL_newmetatable(L, PSLuaPluginImport::metaname.c_str());
  luaL_newlib(L, pl_methods);
  lua_setfield(L, -2, "__index");
  lua_pop(L,1);
  PSLuaPluginImport** pud = (PSLuaPluginImport **)lua_newuserdata(L, sizeof(PSLuaPluginImport*));
  *pud = 0;
  luaL_setmetatable(L, PSLuaPluginImport::metaname.c_str());
  lua_setglobal(L, "psyplugin");
}

void LuaProxy::push_proxy() {
  LuaProxy ** ud = (LuaProxy **)lua_newuserdata(L, sizeof(LuaProxy*));
  *ud = this;
  luaL_setmetatable(L, LuaProxy::meta_name.c_str());
}

void LuaProxy::run_call_init(std::vector<float*>& sample_buf) {
  // share samples
  array_bind_.build_buffer(sample_buf, 256);    
  // run now whole script at once
  lua_pcall(L, 0, LUA_MULTRET, 0);
  // call script, so it can do some init stuff
  lua_getglobal(L, LuaProxy::meta_name.c_str());
  lua_getfield(L, -1, "init");
  if (lua_isnil(L, -1)) {	  
	  lua_pop(L, 2);
	  return;
  }
  push_proxy();
  int status = lua_pcall(L, 1, 0 ,0);  // call pc:init()
  if (status) {
      CString msg(lua_tostring(L, -1));
	  AfxMessageBox(msg);
  }
}
// call events
void LuaProxy::call_seqtick(int channel, int note, int ins, int cmd, int val) {
  lock();
  try {	
    lua_getglobal(L, LuaProxy::meta_name.c_str());
    lua_getfield(L, -1, "seqtick");
	push_proxy();
    lua_pushnumber( L, channel);
	lua_pushnumber( L, note);
	lua_pushnumber( L, ins);
	lua_pushnumber( L, val);
    int status = lua_pcall(L, 5, 0 ,0);    // pc:seqtick(channel, note, ins, cmd)
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

void LuaProxy::call_work(int numSamples) throw(psycle::host::exception) {
	lock();
    array_bind_.update_num_samples(numSamples);
    lua_getglobal(L, LuaProxy::meta_name.c_str());
    lua_getfield(L, -1, "work");
	if (lua_isnil(L, -1)) {	  
	  lua_pop(L, 2);
	  unlock();
	  return;
	}
	push_proxy();
    lua_pushnumber(L, numSamples);
    int status = lua_pcall(L, 2, 0 ,0);	// pc:work(num)
	try {
      if (status) {
         std::string s(lua_tostring(L, -1));	
		 unlock();
         throw std::runtime_error(s);
      }
    } CATCH_WRAP_AND_RETHROW(*plug_)
	unlock();
}

void LuaProxy::call_stop() {
  lock();
  try {	
    lua_getglobal(L, LuaProxy::meta_name.c_str());
    lua_getfield(L, -1, "stop");
	if (lua_isnil(L, -1)) {
	  lua_pop(L, 2);
	  unlock();
	  return;
    }
	// todo translate keys to freq
	push_proxy();
    int status = lua_pcall(L, 1, 0 ,0);
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

void LuaProxy::call_parameter(int numparameter, float val) {  
  lock();
  lua_getglobal(L, LuaProxy::meta_name.c_str());
  lua_getfield(L, -1, "setparameter");
  if (lua_isnil(L, -1)) {
	  lua_pop(L, 2);
	  unlock();
	  return;
  }
  push_proxy();
  lua_pushnumber(L, numparameter);
  lua_pushnumber(L, val);
  int status = lua_pcall(L, 3, 0 ,0);   			
  try {
    if (status) {
        std::string s(lua_tostring(L, -1));	
		unlock();
        throw std::runtime_error(s);
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

float LuaProxy::get_parameter(int numparam) {
  lock();
  if (GetRawParameter("getparameter", numparam)==0) {
	  return 0;
  }
  try {
    if (!lua_isnumber(L, -1)) {
	   std::string s("function getparameter must return a number");	
	   unlock();
       throw std::runtime_error(s);
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  float z = lua_tonumber(L, -1);
  lua_pop(L, 1);  // pop returned value
  unlock();
  return z;
}

const char* LuaProxy::get_parameter_name(int numparam) {   
  lock();
  if (GetRawParameter("getparametername", numparam)==0) {
	  return "";
  }
  try {
    if (!lua_isstring(L, -1)) {
	   std::string s("function getparametername must return a string");
	   unlock();
       throw std::runtime_error(s);
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  const char* name = GetString();
  unlock();
  return name;
}

const char* LuaProxy::get_parameter_display(int numparam) {
  lock();
  if (GetRawParameter("getparameterdisplay", numparam) == 0) {
	  unlock();
	  return "";
  }
  try {
    if (!lua_isstring(L, -1)) {
	   std::string s("function getparameterdisplay must return a string");	
	   unlock();
       throw std::runtime_error(s);
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  const char* name = GetString();
  unlock();
  return name;
}

const char* LuaProxy::get_parameter_label(int numparam) {
  lock();
  if (GetRawParameter("getparameterlabel", numparam) == 0) {
	  unlock();
	  return "";
  }
  try {
    if (!lua_isstring(L, -1)) {
	   std::string s("function getparameterlabel must return a string");	
	   unlock();
       throw std::runtime_error(s);
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  const char* name = GetString();
  unlock();
  return name;
}

// helpers
int LuaProxy::GetRawParameter(const char* field, int index) {  
  lua_getglobal(L, LuaProxy::meta_name.c_str());
  lua_getfield(L, -1, field);
  if (lua_isnil(L, -1)) {
	lua_pop(L, 2);
	return 0;
  }
  push_proxy();
  lua_pushnumber ( L, index );
  int status = lua_pcall(L, 2, 1 ,0);    // call Lua Work method with 1 param and 1 results   			
  try {
    if (status) {
       std::string s(lua_tostring(L, -1));	
	   unlock();
       throw std::runtime_error(s);
    }
   } CATCH_WRAP_AND_RETHROW(*plug_)
   return 1;
}

const char* LuaProxy::GetString() {	
	const char* name = lua_tostring(L, -1); 		  
	lua_pop(L, 1);  // pop returned value	
	return name;
}

// Host
lua_State* LuaHost::load_script(const char * sName) {	
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);
  // set search path for require
  std::string filename_noext;
  boost::filesystem::path p(sName);
  std::string dir = p.parent_path().string();
  std::string fn = p.stem().string();
  lua_getglobal(L, "package");
  std::string path1 = dir + "/?.lua;" + dir + "/" + fn + "/?.lua";
  std::replace(path1.begin(), path1.end(), '/', '\\' );
  lua_pushstring(L, path1.c_str());
  lua_setfield(L, -2, "path");

  std::string path = sName;
  /// This is needed to prevent loading problems
  std::replace(path.begin(), path.end(), '\\', '/');
  int status = luaL_loadfile(L, path.c_str());
  if (status) {
    const char* msg =lua_tostring(L, -1);
				std::ostringstream s; s
					<< "Failed: " << msg << std::endl;
					throw psycle::host::exceptions::library_errors::loading_error(s.str());			
  }
  return L;
}


LuaPlugin* LuaHost::LoadPlugin(const char * sName, int macIdx) {	
	lua_State* L = load_script(sName);
	LuaPlugin *plug = new LuaPlugin(L, macIdx);
	plug->dll_path_ = std::string(sName);
	return plug;
}


} // namespace
} // namespace
