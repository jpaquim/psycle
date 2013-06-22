#include <psycle/host/detail/project.hpp>
#include "plugincatcher.hpp"
#include "Song.hpp"
#include "LuaHost.hpp"
#include "LuaPlugin.hpp"
#include <universalis/os/terminal.hpp>
#include "PlotterDlg.hpp"

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



universalis::os::terminal* LuaProxy::terminal = 0;

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

void LuaProxy::reload() {
	lock();
	try {
		lua_State* L = LuaHost::load_script(plug_->GetDllName());
		free_state();
		set_state(L);			
		call_run(plug_->samplesV);		
		call_init();
	}
	catch(std::exception &e) {
		std::string s = std::string("SCRIPT ERROR! OLD SCRIPT STILL RUNNING!\n") + e.what();
		AfxMessageBox(s.c_str());
	}
	unlock();
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



int LuaProxy::set_parameters(lua_State* L) {	
    luaL_checktype(L, 1, LUA_TTABLE); 	
    lua_getfield(L, 1, "__self");
    LuaProxy* ud = *(LuaProxy**)luaL_checkudata(L, -1, "psyhostusermeta");    	
	ud->num_parameter_ = lua_rawlen(L, 2);
	lua_pushvalue(L, 1);
	lua_pushvalue(L, 2);
	lua_setfield(L, -2, "params");
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
   size_t len;
   const char* plug_name = luaL_checklstring(L, 2, &len);   
   PSLuaPluginImport ** udata = (PSLuaPluginImport **)lua_newuserdata(L, sizeof(PSLuaPluginImport *));	
   luaL_setmetatable(L, PSLuaPluginImport::metaname.c_str());   
   *udata = new PSLuaPluginImport();
   try {
     (*udata)->load(plug_name);   
   } catch (std::exception &e) {
	  luaL_error(L, "plugin not found error");
   }
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

int LuaProxy::terminal_output(lua_State* L) {	
	size_t len = 0;
	const char* out = luaL_checklstring(L, 1, &len);	
	if (terminal == 0) {
	  terminal = new universalis::os::terminal();
	}
    terminal->output(universalis::os::loggers::levels::trace, out);			
	// AfxGetMainWnd()->PostMessage(0x0503, (WPARAM) out, 0);	
	return 0;
}

int LuaProxy::plotter_new(lua_State* L) {	
	CPlotterDlg ** udata = (CPlotterDlg **)lua_newuserdata(L, sizeof(CPlotterDlg *));	
	luaL_setmetatable(L, "psyplottermeta");	
    *udata = 0;
	AfxGetMainWnd()->SendMessage(0x0500, (WPARAM) udata, 0);
	return 1;
}

int LuaProxy::plotter_gc(lua_State* L) {
	CPlotterDlg* ud = *(CPlotterDlg**) luaL_checkudata(L, 1, "psyplottermeta");	
	AfxGetMainWnd()->SendMessage(0x0501, (WPARAM)ud, 0);
	return 0;
}

int LuaProxy::plotter_stem(lua_State* L) {
	CPlotterDlg* ud = *(CPlotterDlg**) luaL_checkudata(L, 1, "psyplottermeta");
	PSArray* x = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
	size_t len;
	AfxGetMainWnd()->SendMessage(0x0502, (WPARAM)ud, (LPARAM)x);
	return 0;
}

void LuaProxy::register_userdata(const luaL_Reg* funcs,
	                             const luaL_Reg* methods,
	                             const char* metaname,
								 const char* userdataname) {
  luaL_newmetatable(L, metaname);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_setfuncs(L, methods, 0);
  lua_pop(L,1);
  luaL_newlib(L, funcs);
  lua_setglobal(L, userdataname);
}

void LuaProxy::export_c_funcs() {
  static const luaL_Reg host_funcs[] = {	  
	  { NULL, NULL }
	};

  static const luaL_Reg host_methods[] = {
  	  {"setparameters", &set_parameters },
	  {"output", terminal_output },
	  {"info",dummy},
  	  { NULL, NULL }
  };
  
  static const luaL_Reg plotter_funcs[] = {
  	 {"new", plotter_new },
	 { NULL, NULL }
  };

  static const luaL_Reg plotter_methods[] = {
	 {"__gc", plotter_gc },
	 {"stem", plotter_stem },
	 { NULL, NULL }
  };

  static const luaL_Reg plugin_funcs[] = {
	 {"new", plugin_new},
	 { NULL, NULL }
  };

  static const luaL_Reg plugin_methods[] = {
  	  {"work", plugin_work },
	  {"channel", plugin_channel},
	  {"resize", plugin_resize},
	  {"param", plugin_param},
	  {"numparams", plugin_numparams},
	  {"paramdisplay", plugin_paramdisplay},
	  {"paramname", plugin_paramname},
	  {"setparameter", plugin_setparam},
	  {"setbuffer", plugin_setbuffer},
	  {"__gc", plugin_gc},
	  { NULL, NULL }
  };

  // register proxy, wrap userdata in table field (__self)
  // metatable of psyhost
  luaL_newmetatable(L, "psyhostmeta");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");   // self.__index = self
  luaL_setfuncs(L, host_methods, 0);
  lua_pop(L,1);
  // create public table for the global script
  lua_newtable(L);
  luaL_getmetatable(L, "psyhostmeta");
  lua_setmetatable(L, -2);
    // metatable only used to check and get the userdata
  // usage e.g in set_parameters
  LuaProxy ** ud = (LuaProxy **)lua_newuserdata(L, sizeof(LuaProxy *));
  *ud = this;
  luaL_newmetatable(L, "psyhostusermeta");
  lua_setmetatable(L, -2);
  lua_setfield(L, -2, "__self");  //
  // make table psyhost available to the script
  lua_setglobal(L, "psyhost");


  register_userdata(plugin_funcs, plugin_methods, "psypluginmeta", "psyplugin");
  register_userdata(plotter_funcs, plotter_methods, "psyplottermeta", "psyplotter");
}

void LuaProxy::push_proxy() {
  LuaProxy ** ud = (LuaProxy **)lua_newuserdata(L, sizeof(LuaProxy*));
  *ud = this;
  luaL_setmetatable(L, LuaProxy::meta_name.c_str());
}

PluginInfo LuaProxy::call_info() {		
  PluginInfo info;
  info.type = MACH_LUA;
  info.allow = true;							
  info.identifier = 0;							
  lua_getglobal(L, "psyhost");
  if (lua_isnil(L, -1)) {	 
	 lua_pop(L, 1);
	 throw psycle::host::exceptions::library_errors::loading_error("no host found");
  }
  lua_getfield(L, -1, "info");
  if (lua_isnil(L, -1)) {	  
	  lua_pop(L, 2);
	  throw psycle::host::exceptions::library_errors::loading_error("no info found");
  }
  lua_pushvalue(L, -2);
  int status = lua_pcall(L, 1, 1 ,0);  // call pc.info()  
  if (status) {
      const char* msg =lua_tostring(L, -1);
	  std::ostringstream s; s << "Failed: " << msg << std::endl;
	  throw psycle::host::exceptions::library_errors::loading_error(s.str());	
  }
  if (lua_istable(L,-1)) {
	size_t len;
	for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
	   const char* key = luaL_checklstring(L, -2, &len);
	   const char* value = luaL_checklstring(L, -1, &len);
	   if (strcmp(key, "numparameters") == 0) {
			 int v = atoi(value);			
			 //info->num_parameter_ = v;
	   } else
	   if (strcmp(key, "vendor") == 0) {
		   info.vendor = std::string(value);
		 } else
		 if (strcmp(key, "name") == 0) {
		    info.name = std::string(value);
		 } else
		 if (strcmp(key, "generator") == 0) {
		   if (std::string(value) == "1") {
			 info.mode = MACHMODE_GENERATOR;
		   }
		 } else
		 if (strcmp(key, "version") == 0) {
			 info.version = value;
		 } else
		 if (strcmp(key, "api") == 0) {
			 int v = atoi(value);
			 info.APIversion = v;
		 }
	  } 
  }
  std::ostringstream s;
  s << (info.mode == MACHMODE_GENERATOR ? "Lua instrument" : "Lua effect") << " by " << info.vendor;
  info.desc = s.str();
	
  return info;
}


void LuaProxy::call_run(std::vector<float*>& sample_buf) {
  // share samples
  // luaL_dostring(L, "parameter = require('parameter')");
  array_bind_.build_buffer(sample_buf, 256);    
  // run now the whole script at once
  int status = lua_pcall(L, 0, LUA_MULTRET, 0);
  if (status) {
      CString msg(lua_tostring(L, -1));
	  AfxMessageBox(msg);
  }  
}

void LuaProxy::call_init() {
  // call script, so it can do some init stuff
  lua_getglobal(L, "psyhost");
  lua_getfield(L, -1, "init");
  if (lua_isnil(L, -1)) {	  
	  lua_pop(L, 2);
	  return;
  }
  lua_pushvalue(L, -2);
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
    lua_getglobal(L, "psyhost");
    lua_getfield(L, -1, "seqtick");
	lua_pushvalue(L, -2);
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
	if (numSamples > 0) {
	  lock();
      array_bind_.update_num_samples(numSamples);
      lua_getglobal(L, "psyhost");
      lua_getfield(L, -1, "work");	
      if (lua_isnil(L, -1)) {	  
	    lua_pop(L, 2);
	    unlock();
	    return;
	  }
	  lua_pushvalue(L,-2);
      lua_pushnumber(L, numSamples);
      int status = lua_pcall(L, 2, 0 ,0);	// pc:work(num)
	  if (status) {
         std::string s(lua_tostring(L, -1));	
		 unlock();
		 try {
           throw std::runtime_error(s);
         } CATCH_WRAP_AND_RETHROW(*plug_)
	  }
	  unlock();
	}
}

void LuaProxy::call_stop() {
  lock();
  try {	
    lua_getglobal(L, "psyhost");
    lua_getfield(L, -1, "stop");
	if (lua_isnil(L, -1)) {
	  lua_pop(L, 2);
	  unlock();
	  return;
    }
	// todo translate keys to freq
	lua_pushvalue(L,-2);
    int status = lua_pcall(L, 1, 0 ,0);
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

void LuaProxy::call_parameter(int numparameter, double val) {  
  lock();
  lua_getglobal(L, "psyhost");
  lua_getfield(L, -1, "params");
  lua_rawgeti(L,-1, numparameter+1);
  lua_getfield(L, -1, "setnorm");
  if (lua_isnil(L, -1)) {
	  lua_pop(L, 4);
	  unlock();
	  return;
  }
  lua_pushvalue(L,-2);
  lua_pushnumber(L, val);
  int status = lua_pcall(L, 2, 0 ,0);   			
  try {
    if (status) {
        std::string s(lua_tostring(L, -1));	
		lua_pop(L,1);
		unlock();
        throw std::runtime_error(s);
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}


double LuaProxy::get_parameter_value(int numparam) {
  lock();
  lua_getglobal(L, "psyhost");
  lua_getfield(L, -1, "params");
  lua_rawgeti(L,-1, numparam+1);
  lua_getfield(L, -1, "getnorm");
  if (lua_isnil(L, -1)) {
	lua_pop(L, 4);
	unlock();
	return 0;
  }
  lua_pushvalue(L, -2);
  int status = lua_pcall(L, 1, 1 ,0);
  try {
    if (status) {
       std::string s(lua_tostring(L, -1));	
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
    if (!lua_isnumber(L, -1)) {
	   std::string s("function parameter:getvalue must return a number");	
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
   } CATCH_WRAP_AND_RETHROW(*plug_)
  double v = lua_tonumber(L, -1);
  lua_pop(L, 1);  // pop returned value
  unlock();
  return v;
}

void LuaProxy::get_parameter_range(int numparam,int &minval, int &maxval) {
  lock();
  lua_getglobal(L, "psyhost");
  lua_getfield(L, -1, "params");
  lua_rawgeti(L,-1, numparam+1);
  lua_getfield(L, -1, "getrange");
  if (lua_isnil(L, -1)) {
	lua_pop(L, 4);
	unlock();
	return;
  }
  lua_pushvalue(L, -2);
  int status = lua_pcall(L, 1, 3 ,0);
  try {
    if (status) {
       std::string s(lua_tostring(L, -1));	
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
	for (int test = -1; test >= -3; --test)
    if (!lua_isnumber(L, test)) {
	   std::string s("function parameter:getrange must return numbers");	
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
   } CATCH_WRAP_AND_RETHROW(*plug_)
  double mi = lua_tonumber(L, -3);
  double ma = lua_tonumber(L, -2);
  double st = lua_tonumber(L, -1);

  minval = 0;
  maxval = st;

  lua_pop(L, 3);  // pop returned value
  unlock();
}

std::string LuaProxy::get_parameter_name(int numparam) {   
  lock();
  lua_getglobal(L, "psyhost");
  lua_getfield(L, -1, "params");
  lua_rawgeti(L,-1, numparam+1);
  lua_getfield(L, -1, "getname");
  if (lua_isnil(L, -1)) {
	lua_pop(L, 4);
	unlock();
	return 0;
  }
  lua_pushvalue(L, -2);
  int status = lua_pcall(L, 1, 1 ,0);
  try {
    if (status) {
       std::string s(lua_tostring(L, -1));
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
	if (!lua_isstring(L, -1)) {
	   std::string s("function parameter:getname must return a string");	
	   unlock();
       throw std::runtime_error(s);
    }
   } CATCH_WRAP_AND_RETHROW(*plug_)
  std::string name(GetString());
  unlock();
  return name;
}

std::string LuaProxy::get_parameter_display(int numparam) {
  lock();
  lua_getglobal(L, "psyhost");
  lua_getfield(L, -1, "params");
  lua_rawgeti(L,-1, numparam+1);
  lua_getfield(L, -1, "getdisplay");
  if (lua_isnil(L, -1)) {
	lua_pop(L, 4);
	unlock();
	return 0;
  }
  lua_pushvalue(L, -2);
  int status = lua_pcall(L, 1, 1 ,0);
  try {
    if (status) {
       std::string s(lua_tostring(L, -1));
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
	if (!lua_isstring(L, -1)) {
	   std::string s("function parameter:getdisplay must return a string");	
	   unlock();
       throw std::runtime_error(s);
    }
   } CATCH_WRAP_AND_RETHROW(*plug_)
  std::string name(GetString());
  unlock();
  return name;
}

std::string LuaProxy::get_parameter_label(int numparam) {
  lock();
  lua_getglobal(L, "psyhost");
  lua_getfield(L, -1, "params");
  lua_rawgeti(L,-1, numparam+1);
  lua_getfield(L, -1, "getlabel");
  if (lua_isnil(L, -1)) {
	lua_pop(L, 4);
	unlock();
	return 0;
  }
  lua_pushvalue(L, -2);
  int status = lua_pcall(L, 1, 1 ,0);
  try {
    if (status) {
       std::string s(lua_tostring(L, -1));	
	   unlock();
       throw std::runtime_error(s);
    }
	if (!lua_isstring(L, -1)) {
	   std::string s("function parameter:getlabel must return a string");	
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
   } CATCH_WRAP_AND_RETHROW(*plug_)
  std::string name(GetString());  
  unlock();
  return name;
}

// helpers
int LuaProxy::GetRawParameter(const char* field, int index) {  
  lua_getglobal(L, "psyhost");
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
  std::string path1 = dir + "/?.lua;" + dir + "/" + fn + "/?.lua;"+dir + "/"+ "psycle/?.lua";
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

PluginInfo LuaHost::LoadInfo(const char * sName) {	
	LuaPlugin* plug = 0;
	PluginInfo info;	
	try { 
		lua_State* L = load_script(sName);
	    plug = new LuaPlugin(L, 0, false);	   
		info = plug->CallPluginInfo();		
	} catch(std::exception &e) {
	   delete plug;
	   AfxMessageBox(e.what());
	   throw e;
	}	
	delete plug;
    return info;
}


} // namespace
} // namespace
