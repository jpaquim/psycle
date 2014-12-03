#include <psycle/host/detail/project.hpp>
#include "LuaHost.hpp"
#include "LuaInternals.hpp"
#include "LuaPlugin.hpp"
#include "Player.hpp"
#include "plugincatcher.hpp"
#include "Song.hpp"
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <lua.hpp>
#include "LuaHelper.hpp"

#if defined LUASOCKET_SUPPORT && !defined WINAMP_PLUGIN
extern "C" {
#include <luasocket/luasocket.h>
#include <luasocket/mime.h>
}
#endif //defined LUASOCKET_SUPPORT && !defined WINAMP_PLUGIN

#include <universalis/os/terminal.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>

namespace psycle { namespace host {

universalis::os::terminal* LuaProxy::terminal = 0;

LuaProxy::LuaProxy(LuaPlugin* plug, lua_State* state) : plug_(plug) {
  InitializeCriticalSection(&cs);
  set_state(state);
}

LuaProxy::~LuaProxy() {
  DeleteCriticalSection(&cs);
}

void LuaProxy::set_state(lua_State* state) { 
  L = state;
  export_c_funcs();
  // requiref our c modules  
  luaL_requiref(L, "psycle.dsp.resampler", LuaResamplerBind::open, 1);
  lua_pop(L, 1);
  luaL_requiref(L, "psycle.dsp.math", LuaDspMathHelper::open, 1);  
  lua_pop(L, 1);
  luaL_requiref(L, "psycle.midi", LuaMidiHelper::open, 1);  
  lua_pop(L, 1);
  LuaArrayBind::register_module(L);  
  luaL_requiref(L, "psycle.dsp.filter", LuaDspFilterBind::open, 1);
  lua_pop(L, 1);
#if !defined WINAMP_PLUGIN
  luaL_requiref(L, "psycle.plotter", LuaPlotterBind::open, 1);
  lua_pop(L, 1);
#endif //!defined WINAMP_PLUGIN
  luaL_requiref(L, "psycle.menu", LuaMenuBind::open, 1);
  lua_pop(L, 1);
  luaL_requiref(L, "psycle.delay", LuaDelayBind::open, 1);
  lua_pop(L, 1);
  luaL_requiref(L, "psycle.machine", LuaMachineBind::open, 1);
  lua_pop(L, 1);
  luaL_requiref(L, "psycle.osc", LuaWaveOscBind::open, 1);
  lua_pop(L, 1);
  luaL_requiref(L, "psycle.dsp.wavedata", LuaWaveDataBind::open, 1);  
  lua_pop(L, 1);
  luaL_requiref(L, "psycle.envelope", LuaEnvelopeBind::open, 1);
  lua_pop(L, 1);
  luaL_requiref(L, "psycle.player", LuaPlayerBind::open, 1);
  lua_pop(L, 1);
#if defined LUASOCKET_SUPPORT && !defined WINAMP_PLUGIN
	luaL_requiref(L, "socket", luaopen_socket_core, 1);
	lua_pop(L, 1);
    luaL_requiref(L, "mime", luaopen_mime_core, 1);
    lua_pop(L, 1);
#endif  //LUASOCKET_SUPPORT
  info_.mode = MACHMODE_FX;  
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
	lua_State* old_state = L;	
	lua_State* new_state = 0;
	try {
		new_state = LuaHost::load_script(plug_->GetDllName());
		set_state(new_state);				
		call_run();
		call_init();
		if (old_state) {
		  lua_close(old_state);
		}
	} catch(std::exception &e) {
		if (new_state) {
		  lua_close(new_state);
	}
		L = old_state;
		std::string s = std::string("Reload Error, old script still running!\n") + e.what();
		AfxMessageBox(s.c_str());
	}
	unlock();
}

int LuaProxy::message(lua_State* L) {
	size_t len;
	const char* msg = luaL_checklstring(L, 1, &len);
	CString cmsg(msg);	
	AfxMessageBox(cmsg);
	return 0;
}

int LuaProxy::terminal_output(lua_State* L) {
	if (terminal == 0) {
	  terminal = new universalis::os::terminal();
	}
	size_t len = 0;
	int n = lua_gettop(L);  /* number of arguments */
	const char* out = 0;
	if (lua_isboolean(L, 1)) {
		int v = lua_toboolean(L, 1);
		if (v==1) out = "true"; else out = "false";
	} else {	
	  int i;
	  lua_getglobal(L, "tostring");
      for (i=1; i<=n; i++) {
        const char *s;
        size_t l;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tolstring(L, -1, &l);  /* get result */
		if (s == NULL)
		  return luaL_error(L,
			LUA_QL("tostring") " must return a string to " LUA_QL("print"));			
		lua_pop(L, 1);  /* pop result */
		terminal->output(universalis::os::loggers::levels::trace, s);			
	  }  
	}	    
	return 0;
}

int LuaProxy::call_filedialog(lua_State* L) {  
	//char szFilters[]= "Text Files (*.NC)|*.NC|Text Files (*.txt)|*.txt|All Files (*.*)|*.*||";
// Create an Open dialog; the default file name extension is ".my".
  //CFileDialog* m_pFDlg = new CFileDialog(TRUE, "txt", "*.txt",
    //  OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters, AfxGetMainWnd());   
   //m_pFDlg->Create(40000,AfxGetMainWnd());
   //m_pFDlg->ShowWindow(SW_SHOW);
  
  return 0;
}

int LuaProxy::set_machine(lua_State* L) {
	lua_getglobal(L, "psycle");
	lua_getfield(L, -1, "__self");
	LuaProxy* proxy = *(LuaProxy**)luaL_checkudata(L, -1, "psyhostmeta");
	luaL_checktype(L, 1, LUA_TTABLE); 
    lua_getfield(L, 1, "__self");	
    LuaMachine** ud = (LuaMachine**) luaL_checkudata(L, -1, "psypluginmeta");
	(*ud)->set_mac(proxy->plug_);
	proxy->plugimport_ = *ud;
	lua_pushvalue(L, 1);
	lua_setfield(L, 2, "proxy");
	// share samples  
    (*ud)->build_buffer(proxy->plug_->samplesV, 256);
	return 0;
}

void LuaProxy::export_c_funcs() {
  static const luaL_Reg methods[] = {
	  {"output", terminal_output },
	  {"setmachine", set_machine},
	  {"filedialog", call_filedialog},
  	  { NULL, NULL }
  };  
  lua_newtable(L); 
  luaL_setfuncs(L, methods, 0);
  LuaProxy** ud = (LuaProxy **)lua_newuserdata(L, sizeof(LuaProxy *));
  luaL_newmetatable(L, "psyhostmeta");
  lua_setmetatable(L, -2);
  *ud = this;
  lua_setfield(L, -2, "__self");
  lua_setglobal(L, "psycle");
}

bool LuaProxy::get_param(lua_State* L, int index, const char* method) {
  try {
    LuaHelper::get_proxy(L);
  } catch(std::exception &e) {	
	throw psycle::host::exceptions::library_errors::loading_error(e.what());
  }
  lua_getfield(L, -1, "params");
  if (lua_isnil(L, -1)) {	 
	 lua_pop(L, 1);
	 throw psycle::host::exceptions::library_errors::loading_error("no param found");
  }
  lua_rawgeti(L,-1, index+1);
  lua_getfield(L, -1, method);
  if (lua_isnil(L, -1)) {
	  lua_pop(L, 5);
	  return false;
  }
  lua_remove(L, -3);
  lua_remove(L, -3);
  lua_remove(L, -3);
  lua_pushvalue(L, -2);
  lua_remove(L,-3);
  return true;
}

void LuaProxy::get_method_strict(lua_State* L, const char* method) {
  try {
    LuaHelper::get_proxy(L);
  } catch(std::exception &e) {	
	throw psycle::host::exceptions::library_errors::loading_error(e.what());
  }
  lua_getfield(L, -1, method);
  if (lua_isnil(L, -1)) {	  
	  lua_pop(L, 3);
	  throw psycle::host::exceptions::library_errors::loading_error("no "+std::string(method)+" found");
  }
  lua_remove(L, -3);
  lua_pushvalue(L, -2);
  lua_remove(L,-3);
}

bool LuaProxy::get_method_optional(lua_State* L, const char* method) {
  try {
    LuaHelper::get_proxy(L);
  } catch(std::exception &e) {	
	throw psycle::host::exceptions::library_errors::loading_error(e.what());
  }
  lua_getfield(L, -1, method);
  if (lua_isnil(L, -1)) {	  
	  lua_pop(L, 3);
	  return false;
  }
  if (lua_iscfunction(L, -1)) {
	lua_pop(L, 3);	
	return false;
  }
  lua_remove(L, -3);
  lua_pushvalue(L, -2);
  lua_remove(L,-3);
  return true;
}

PluginInfo LuaProxy::call_info() {		
  PluginInfo info;
  info.type = MACH_LUA;
  info.mode = MACHMODE_FX;
  info.allow = true;							
  info.identifier = 0;							
  get_method_strict(L, "info");
  int status = lua_pcall(L, 1, 1 ,0);
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
		 } else
		 if (strcmp(key, "noteon") == 0) {
			 int v = atoi(value);
			 info.flags = v;
		 }
	  } 
  }
  std::ostringstream s;
  s << (info.mode == MACHMODE_GENERATOR ? "Lua instrument" : "Lua effect") << " by " << info.vendor;
  info.desc = s.str();	
  return info;
}

void LuaProxy::call_run() {
  int status = lua_pcall(L, 0, LUA_MULTRET, 0);
  if (status) {
      CString msg(lua_tostring(L, -1));
	  throw std::runtime_error(msg.GetString());
  }  
}

void LuaProxy::call_init() {
  get_method_strict(L, "init");
  lua_pushnumber(L, Global::player().SampleRate());
  int status = lua_pcall(L, 2, 0 ,0);
  if (status) {
      CString msg(lua_tostring(L, -1));
	  throw std::runtime_error(msg.GetString());
  }
}
// call events
void LuaProxy::call_newline() {
  lock();
  try {	
    if (!get_method_optional(L, "sequencertick")) {
		unlock();
		return;
	}
    int status = lua_pcall(L, 1, 0 ,0);    // pc:sequencertick()
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

void LuaProxy::call_seqtick(int channel, int note, int ins, int cmd, int val) {
  lock();
  try {	
    if (!get_method_optional(L, "seqtick")) {
		unlock();
		return;
	}
    lua_pushnumber( L, channel);
	lua_pushnumber( L, note);
	lua_pushnumber( L, ins);
	lua_pushnumber( L, cmd);
	lua_pushnumber( L, val);
    int status = lua_pcall(L, 6, 0, 0); // seqtick(channel, note, ins, cmd, val)
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

void LuaProxy::call_command(int lastnote, int inst, int cmd, int val) {
  lock();
  try {	
    if (!get_method_optional(L, "command")) {
		unlock();
		return;
	}    
	if (lastnote != notecommands::empty) {
	  lua_pushnumber(L, lastnote);
	} else {
	  lua_pushnil(L);
	}
    lua_pushnumber(L, inst);
	lua_pushnumber(L, cmd);
	lua_pushnumber(L, val);
    int status = lua_pcall(L, 5, 0 ,0);  // command
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

void LuaProxy::call_noteon(int note, int lastnote, int inst, int cmd, int val) {
  lock();
  try {	
    if (!get_method_optional(L, "noteon")) {
		unlock();
		return;
	}
	lua_pushnumber(L, note);
	if (lastnote != notecommands::empty) {
		lua_pushnumber(L, lastnote);
	} else {
	  lua_pushnil(L);
	}
	lua_pushnumber(L, inst);
	lua_pushnumber(L, cmd);
	lua_pushnumber(L, val);
    int status = lua_pcall(L, 6, 0 ,0); // noteon
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

void LuaProxy::call_noteoff(int note, int lastnote, int inst, int cmd, int val) {
  lock();
  try {	
    if (!get_method_optional(L, "noteoff")) {
		unlock();
		return;
	}
	lua_pushnumber(L, note);
	if (lastnote != notecommands::empty) {
		lua_pushnumber(L, lastnote);
	} else {
	  lua_pushnil(L);
	}
	lua_pushnumber(L, inst);
	lua_pushnumber(L, cmd);
	lua_pushnumber(L, val);
    int status = lua_pcall(L, 6, 0 ,0); // noteoff
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

std::string LuaProxy::call_help() {
  lock();
  try {	
    if (!get_method_optional(L, "help")) {
		unlock();
		return "no help found";
	}
    int status = lua_pcall(L, 1, 1 ,0);    // pc:seqtick(channel, note, ins, cmd, val)
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
	if (!lua_isstring(L, -1)) {
	   std::string s("call_help must return a string");	
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
	unlock();
	return GetString();

  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
  return "no help found";
}

void LuaProxy::call_data(byte* data) {	
	lock();
	std::string str;
	try {	
    if (!get_method_optional(L, "data")) {
		unlock();
		str = "";
		return;
	}
    int status = lua_pcall(L, 1, 1 ,0);
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
	if (!lua_isstring(L, -1)) {
	   std::string s("data must return a string");	
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
	str = GetString();	
	std::copy(str.begin(), str.end(), data);
	unlock();
	return;
  } CATCH_WRAP_AND_RETHROW(*plug_)  
  unlock();
  return;
}

uint32_t LuaProxy::call_data_size() {
	lock();
	std::string str;
	try {	
    if (!get_method_optional(L, "data")) {
		unlock();
		str = "";
		return 0;
	}
    int status = lua_pcall(L, 1, 1 ,0);
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
	if (!lua_isstring(L, -1)) {
	   std::string s("data must return a string");	
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
	str = GetString();
	unlock();
	return str.size();
  } CATCH_WRAP_AND_RETHROW(*plug_)  
  unlock();
  return 0;
}

void LuaProxy::call_putdata(byte* data, int size) {
  lock();
  try {	
    if (!get_method_optional(L, "putdata")) {
		unlock();
		return;
	}
	std::string s(reinterpret_cast<char const*>(data), size);
	lua_pushstring(L, s.c_str());
    int status = lua_pcall(L, 2, 0 ,0);
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

void LuaProxy::call_work(int numSamples, int offset) {	
	if (numSamples > 0) {
	  lock();	  
  	  if (offset >0) {
		plugimport_->offset(offset);
	  }
	  plugimport_->update_num_samples(numSamples);
	  if (!get_method_optional(L, "work")) {
		unlock();
		return;
	  }	  
      lua_pushnumber(L, numSamples);
      int status = lua_pcall(L, 2, 0 ,0);
   	  if (offset >0) {
		plugimport_->offset(-offset);
	  }
	  if (status) {
		 int n = lua_gettop(L);
		 std::ostringstream o;
		 o << lua_tostring(L, -1) << ", stack count :" << n;
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
    if (!get_method_optional(L, "stop")) {
		unlock();
		return;
	}
    int status = lua_pcall(L, 1, 0 ,0);
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

void LuaProxy::call_sr_changed(int rate) {
  lock();
  try {	
	WaveOscTables::getInstance()->set_samplerate(rate);
	LuaDspFilterBind::setsamplerate(rate);
	LuaWaveOscBind::setsamplerate(rate);
    if (!get_method_optional(L, "onsrchanged")) {
		unlock();
		return;
	}
	lua_pushnumber(L, rate);
    int status = lua_pcall(L, 2, 0 ,0);
    if (status) {
      CString msg(lua_tostring(L, -1));
	  unlock();
	  throw std::runtime_error(msg.GetString());
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

void LuaProxy::call_aftertweaked(int numparameter) {
 lock();
  if (!get_param(L, numparameter, "afternotify")) {
	  unlock();
	  return;
  }
  int status = lua_pcall(L, 1, 0 ,0);   			
  try {
    if (status) {
        std::string s(lua_tostring(L, -1));	
		lua_pop(L,1);
		unlock();
		#if !defined WINAMP_PLUGIN
		ReleaseCapture();
		while (ShowCursor(TRUE) < 0);
		#endif // #if !defined WINAMP_PLUGIN
        throw std::runtime_error(s);
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

// Parameter tweak range is [0..1]
void LuaProxy::call_parameter(int numparameter, double val) {  
  lock();
  if (!get_param(L, numparameter, "setnorm")) {
	  unlock();
	  return;
  }
  lua_pushnumber(L, val);
  int status = lua_pcall(L, 2, 0 ,0);   			
  try {
    if (status) {
        std::string s(lua_tostring(L, -1));	
		lua_pop(L,1);
		unlock();
		#if !defined WINAMP_PLUGIN
		ReleaseCapture();
		while (ShowCursor(TRUE) < 0);
		#endif // #if !defined WINAMP_PLUGIN
        throw std::runtime_error(s);
    }
  } CATCH_WRAP_AND_RETHROW(*plug_)
  unlock();
}

double LuaProxy::get_parameter_value(int numparam) {
  lock();
  if (!get_param(L, numparam, "norm")) {
	  unlock();
	  return 0;
  }
  int status = lua_pcall(L, 1, 1 ,0);
  try {
    if (status) {
       std::string s(lua_tostring(L, -1));	
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
    if (!lua_isnumber(L, -1)) {
	   std::string s("function parameter:val must return a number");	
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
  if (!get_param(L, numparam, "range")) {
	  unlock();
	  return;
  }
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
	   std::string s("function parameter:range must return numbers");	
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
   } CATCH_WRAP_AND_RETHROW(*plug_) 
  double st = lua_tonumber(L, -1);
  minval = 0;
  maxval = st;
  lua_pop(L, 3);  // pop returned value
  unlock();
}

std::string LuaProxy::get_parameter_name(int numparam) {   
  lock();
  if (!get_param(L, numparam, "name")) {
	  unlock();
	  return "";
  }
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
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
   } CATCH_WRAP_AND_RETHROW(*plug_)
  std::string name(GetString());
  unlock();
  return name;
}

std::string LuaProxy::get_parameter_id(int numparam) {   
  lock();
  if (!get_param(L, numparam, "id")) {
	  unlock();
	  return "";
  }
  int status = lua_pcall(L, 1, 1 ,0);
  try {
    if (status) {
       std::string s(lua_tostring(L, -1));
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
	if (!lua_isstring(L, -1)) {
	   std::string s("function parameter:id must return a string");	
	   lua_pop(L, 1);
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
  if (!get_param(L, numparam, "display")) {
	  unlock();
	  return "";
  }
  int status = lua_pcall(L, 1, 1 ,0);
  try {
    if (status) {
       std::string s(lua_tostring(L, -1));
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
	if (!lua_isstring(L, -1)) {
	   std::string s("function parameter:display must return a string");	
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
  if (!get_param(L, numparam, "label")) {
	  unlock();
	  return "";
  }
  int status = lua_pcall(L, 1, 1 ,0);
  try {
    if (status) {
       std::string s(lua_tostring(L, -1));	
	   unlock();
       throw std::runtime_error(s);
    }
	if (!lua_isstring(L, -1)) {
	   std::string s("function parameter:label must return a string");	
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
   } CATCH_WRAP_AND_RETHROW(*plug_)
  std::string name(GetString());  
  unlock();
  return name;
}

int LuaProxy::get_parameter_type(int numparam) {
  lock();
  if (!get_param(L, numparam, "mpf")) {
	  unlock();
	  return 0;
  }
  int status = lua_pcall(L, 1, 1 ,0);
  try {
    if (status) {
       std::string s(lua_tostring(L, -1));	
	   unlock();
       throw std::runtime_error(s);
    }
	if (!lua_isnumber(L, -1)) {
	   std::string s("function parameter:mpf must return a number");	
	   lua_pop(L, 1);
	   unlock();
       throw std::runtime_error(s);
    }
   } CATCH_WRAP_AND_RETHROW(*plug_)
  int mpf = luaL_checknumber(L, -1);
  lua_pop(L, 1);
  unlock();
  return mpf;
}

std::string LuaProxy::GetString() {	
	std::string name(lua_tostring(L, -1));
	lua_pop(L, 1);  // pop returned value	
	return name;
}

void LuaProxy::build_tree(menu& t) {	
	luaL_checktype(L, -1, LUA_TTABLE);
	// t:opairs()
	lua_getfield(L, -1, "opairs");
	lua_pushvalue(L, -2);
	lua_call(L, 1, 2);
	size_t len;
	// iter, self (t), nil
	for(lua_pushnil(L); LuaHelper::luaL_orderednext(L);)
	{
		std::string s = luaL_checklstring(L, -2, &len);
		menu t1;
		lua_getfield(L, -1, "__self");
		menuitem* ptr = *(menuitem **)luaL_checkudata(L, -1, "psymenumeta");		
		t1.object = ptr;
		t1.object->id = s;		
		lua_pop(L, 1);
		lua_getfield(L, -1, "menus");
		build_tree(t1);		
        t.nodes.push_back(t1);
        lua_pop(L, 1);
		lua_pop(L, 1);
	}
	lua_pop(L, 2);
}

bool LuaProxy::get_menu_tbl() {
  try {
    LuaHelper::get_proxy(L);
  } catch(std::exception &e) {	  
	unlock();
	throw psycle::host::exceptions::library_errors::loading_error(e.what());
  }  
  lua_getfield(L, -1, "__menus");
  if (lua_isnil(L, -1)) {	 
	 lua_pop(L, 1);
	 unlock();
	 return false;	 
  }  
  return true;
}

menu LuaProxy::get_menu_tree() {
  lock(); 
  menu t;
  if (!get_menu_tbl()) return t;
  build_tree(t);
  lua_pop(L, 3);  
  unlock();
  return t;
}

void LuaProxy::call_menu(const std::string& id) {
  lock();  
  if (!get_menu_tbl()) return;
  typedef boost::tokenizer<boost::char_separator<char> > 
    tokenizer;
  boost::char_separator<char> sep(".");
  tokenizer tokens(id, sep);  
  std::vector<std::string> fields;
  for (tokenizer::iterator tok_iter = tokens.begin();
       tok_iter != tokens.end(); ++tok_iter) {
		   fields.push_back(*tok_iter);
  }    
  int n = 0;
  for (std::vector<std::string>::iterator it = fields.begin();
	     it!=fields.end(); ++it) {
	std::string field = *it;
	lua_getfield(L, -1, field.c_str());
	++n;
	bool is_last = (it != fields.end()) && (it + 1 == fields.end());
	if (!is_last) {
	  lua_getfield(L, -1, "menus");
	  ++n;
	}
  }
  lua_getfield(L, -1, "notify");
  lua_pushvalue(L, -2); // self	
  int status = lua_pcall(L, 1, 0, 0);
  if (status) {
     std::string s(lua_tostring(L, -1));	
	 lua_pop(L, 1);
	 unlock();
     throw std::runtime_error(s);  
  }  
  lua_pop(L, 3+n);  
  unlock();
}

// Host
lua_State* LuaHost::load_script(const std::string& dllpath) {	
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);
  // set search path for require
  std::string filename_noext;
  boost::filesystem::path p(dllpath);
  std::string dir = p.parent_path().string();
  std::string fn = p.stem().string();
  lua_getglobal(L, "package");
  std::string path1 = dir + "/?.lua;" + dir + "/" + fn + "/?.lua;"+dir + "/"+ "psycle/?.lua";
  std::replace(path1.begin(), path1.end(), '/', '\\' );
  lua_pushstring(L, path1.c_str());
  lua_setfield(L, -2, "path");

  std::string path = dllpath;
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

LuaPlugin* LuaHost::LoadPlugin(const std::string& dllpath, int macIdx) {	
	lua_State* L = load_script(dllpath);
	LuaPlugin *plug = new LuaPlugin(L, macIdx);
	plug->dll_path_ = dllpath;
	PluginInfo info = plug->CallPluginInfo();
	plug->_mode = info.mode;
	plug->usenoteon_ = info.flags;
	strncpy(plug->_editName, info.name.c_str(),sizeof(plug->_editName)-1);
	return plug;
}

PluginInfo LuaHost::LoadInfo(const std::string& dllpath) {	
	LuaPlugin* plug = 0;
	PluginInfo info;	
	try { 
		lua_State* L = load_script(dllpath);
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


}} // namespace
