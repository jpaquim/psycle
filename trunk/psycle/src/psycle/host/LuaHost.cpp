// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

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
#include "Canvas.hpp"
#include "NewMachine.hpp"
#include <algorithm>

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
    // if (terminal) { delete terminal; terminal = NULL; }    
    DeleteCriticalSection(&cs);
  }

  void LuaProxy::set_state(lua_State* state) { 
    L = state;
    export_c_funcs();
    // require c modules  
    LuaHelper::require<LuaResamplerBind>(L, "psycle.dsp.resampler");
    LuaHelper::require<LuaDspMathHelper>(L, "psycle.dsp.math");
    LuaHelper::require<LuaDspFilterBind>(L, "psycle.dsp.filter");
    LuaHelper::require<LuaMidiHelper>(L, "psycle.midi");
    LuaHelper::require<LuaArrayBind>(L, "psycle.array");        
    LuaHelper::require<LuaDelayBind>(L, "psycle.delay");
    LuaHelper::require<LuaMachineBind>(L, "psycle.machine");
    LuaHelper::require<LuaWaveOscBind>(L, "psycle.osc");
    LuaHelper::require<LuaWaveDataBind>(L, "psycle.dsp.wavedata");
    LuaHelper::require<LuaEnvelopeBind>(L, "psycle.envelope");
    LuaHelper::require<LuaPlayerBind>(L, "psycle.player");
    LuaHelper::require<LuaPatternDataBind>(L, "psycle.pattern");
    LuaHelper::require<LuaConfigBind>(L, "psycle.config");    
    // ui binds    
    LuaHelper::require<LuaActionListenerBind>(L, "psycle.ui.hostactionlistener");
    LuaHelper::require<LuaMenuBarBind>(L, "psycle.ui.menubar");
    LuaHelper::require<LuaMenuBind>(L, "psycle.ui.menu");
    LuaHelper::require<LuaMenuItemBind>(L, "psycle.ui.menuitem");    
    LuaHelper::require<LuaDialogBind>(L, "psycle.ui.dialog");        
    LuaHelper::require<LuaCanvasBind>(L, "psycle.ui.canvas");
    LuaHelper::require<LuaGroupBind>(L, "psycle.ui.canvas.group");    
    LuaHelper::require<LuaItemBind>(L, "psycle.ui.canvas.item");
    LuaHelper::require<LuaRectBind>(L, "psycle.ui.canvas.rect");    
    LuaHelper::require<LuaLineBind>(L, "psycle.ui.canvas.line");    
    LuaHelper::require<LuaTextBind>(L, "psycle.ui.canvas.text");
    // LuaHelper::require<LuaPixBind>(L, "psycle.ui.canvas.pix");
    LuaHelper::require<LuaGraphicsBind>(L, "psycle.ui.canvas.graphics");
#if !defined WINAMP_PLUGIN
    LuaHelper::require<LuaPlotterBind>(L, "psycle.plotter");    
#endif //!defined WINAMP_PLUGIN    
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
      lua_close(L);
    }
    L = 0;
  }

  void LuaProxy::reload() {        
    plug_->Mute(true);
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
      plug_->Mute(false);
    } catch(std::exception &e) {      
      if (new_state) {
        lua_close(new_state);
      }
      L = old_state;
      std::string s = std::string("Reload Error, old script still running!\n") + e.what();
      plug_->set_crashed(true);
      unlock();      
      throw std::exception(e);
  //    AfxMessageBox(s.c_str());      
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
    int n = lua_gettop(L);  // number of arguments
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
    char szFilters[]= "Text Files (*.NC)|*.NC|Text Files (*.txt)|*.txt|All Files (*.*)|*.*||";
    // Create an Open dialog; the default file name extension is ".my".
    CFileDialog* m_pFDlg = new CFileDialog(TRUE, "txt", "*.txt",
      OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters, AfxGetMainWnd());   
    m_pFDlg->DoModal();
    //m_pFDlg->Create(40000,AfxGetMainWnd());
    //m_pFDlg->ShowWindow(SW_SHOW);
    /*lua_getglobal(L, "psycle");
    lua_getfield(L, -1, "__self");
    LuaProxy* proxy = *(LuaProxy**)luaL_checkudata(L, -1, "psyhostmeta");
    proxy->test.OnStart();*/
    return 0;
  }

  int LuaProxy::call_selmachine(lua_State* L) {  
    CNewMachine dlg(AfxGetMainWnd());
    dlg.DoModal();
    if (dlg.Outputmachine >= 0) {
      std::string filename = dlg.psOutputDll;
      boost::filesystem::path p(filename);
      lua_pushstring(L, p.stem().string().c_str());
    } else {
       lua_pushnil(L);
    }
    //m_pFDlg->Create(40000,AfxGetMainWnd());
    //m_pFDlg->ShowWindow(SW_SHOW);
    /*lua_getglobal(L, "psycle");
    lua_getfield(L, -1, "__self");
    LuaProxy* proxy = *(LuaProxy**)luaL_checkudata(L, -1, "psyhostmeta");
    proxy->test.OnStart();*/
    return 1;
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
    proxy->plugimport_->setproxy(proxy);
    lua_pushvalue(L, 1);
    LuaHelper::register_weakuserdata(L, proxy->plugimport_);
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
      {"selmachine", call_selmachine},
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
    lua_getglobal(L, "psycle");    
    lua_newtable(L); // table for canvasdata    
    lua_setfield(L, -2, "userdata");
    lua_newtable(L); // table for canvasdata
    lua_newtable(L); // metatable
    lua_pushstring(L, "kv");
    lua_setfield(L, -2, "__mode"); 
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "weakuserdata");
    lua_pop(L, 1);    
  }

  bool LuaProxy::get_param(lua_State* L, int index, const char* method) {
    if (index < 0) return false;
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
        if (strcmp(key, "vendor") == 0) {
          const char* value = luaL_checklstring(L, -1, &len);
          info.vendor = std::string(value);
        } else
          if (strcmp(key, "name") == 0) {
            const char* value = luaL_checklstring(L, -1, &len);
            info.name = std::string(value);
          } else 
            if (strcmp(key, "mode") == 0) {
              int value = luaL_checknumber(L, -1);
              switch (value) {
                case 0 : info.mode = MACHMODE_GENERATOR; break;
                case 3 : info.mode = MACHMODE_LUAUIEXT; break;                
                default: info.mode = MACHMODE_FX; break;
              }
            } else
            if (strcmp(key, "generator") == 0) {
              int value = luaL_checknumber(L, -1);
              if (value==0) info.mode = MACHMODE_GENERATOR; else info.mode = MACHMODE_FX;
            } else
              if (strcmp(key, "version") == 0) {
                const char* value = luaL_checklstring(L, -1, &len);
                info.version = value;
              } else
                if (strcmp(key, "api") == 0) {
                  int value = luaL_checknumber(L, -1);                  
                  info.APIversion = value;
                } else
                  if (strcmp(key, "noteon") == 0) {
                    int value = luaL_checknumber(L, -1);
                    info.flags = value;
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
  void LuaProxy::call_execute() {
    lock();
    try {	
      if (!get_method_optional(L, "onexecute")) {
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

  int LuaProxy::call_data(unsigned char **ptr, bool all) {	
    lock();
    std::string str;
    try {	
      if (!get_method_optional(L, "data")) {
        unlock();
        str = "";
        return 0;
      }
      lua_pushboolean(L, all);
      int status = lua_pcall(L, 2, 1 ,0);
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
      *ptr = new unsigned char[str.size()];
      std::copy(str.begin(), str.end(), *ptr);
      unlock();
      return str.size();
    } CATCH_WRAP_AND_RETHROW(*plug_)  
      unlock();
    return 0;
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

  void LuaProxy::call_putdata(unsigned char* data, int size) {
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
      lua_gc(L, LUA_GCSTEP, 5);
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

  // call events
  void LuaProxy::call_setprogram(int idx) {
    lock();
    try {	
      if (!get_method_optional(L, "setprogram")) {
        unlock();
        return;
      }
      lua_pushnumber(L, idx);
      int status = lua_pcall(L, 2, 0 ,0);    // pc:sequencertick()
      if (status) {
        CString msg(lua_tostring(L, -1));
        unlock();
        throw std::runtime_error(msg.GetString());
      }
    } CATCH_WRAP_AND_RETHROW(*plug_)
      unlock();
  }

  // call events
  int LuaProxy::get_curr_program() {
    lock();
    int n1 = lua_gettop(L);
    try {	
      if (!get_method_optional(L, "getprogram")) {
        unlock();
        return 0;
      }      
      int status = lua_pcall(L, 1, 1, 0);    // pc:sequencertick()
      if (status) {
        CString msg(lua_tostring(L, -1));
        unlock();
        throw std::runtime_error(msg.GetString());
      }
      int prg = luaL_checknumber(L, -1);
      lua_pop(L, 1);
      int n2 = lua_gettop(L);
      unlock();
      assert(n1==n2);
      return prg;
    } CATCH_WRAP_AND_RETHROW(*plug_)
	//Usually the catch will rethrow, but the debugger can bypass it so we woul reach this point
    unlock();
    return NULL;
  }

  // call events
  int LuaProxy::call_numprograms() {
     return plugimport_->numprograms();   
  }

  std::string LuaProxy::get_program_name(int bnkidx, int idx) {
    lock();
    int n1 = lua_gettop(L);
    try {	
      if (!get_method_optional(L, "programname")) {
        unlock();
        return "";
      }      
      lua_pushnumber(L, idx);
      int status = lua_pcall(L, 2, 1 ,0);    // pc:sequencertick()
      if (status) {
        CString msg(lua_tostring(L, -1));
        unlock();
        throw std::runtime_error(msg.GetString());
      }
      const char* name = luaL_checkstring(L, -1);
      lua_pop(L, 1);
      int n2 = lua_gettop(L);
      unlock();
      assert(n1==n2);
      return std::string(name);
    } CATCH_WRAP_AND_RETHROW(*plug_)
	//Usually the catch will rethrow, but the debugger can bypass it so we woul reach this point
    unlock();
    return NULL;
  }
  
  std::string LuaProxy::GetString() {	
    std::string name(lua_tostring(L, -1));
    lua_pop(L, 1);  // pop returned value	
    return name;
  }

  void LuaProxy::update_menu(void* hnd) {
    if (hnd) {
      lock();    
      CFrameWnd* cwnd = (CFrameWnd*) hnd;
      cwnd->DrawMenuBar();
      unlock();
    }
  }
  
  LuaMenuBar* LuaProxy::get_menu(LuaMenu* menu) {
    lock();
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
      return 0;
    } else {  
      LuaMenuBar* menubar = LuaHelper::check<LuaMenuBar>(L, -1, LuaMenuBarBind::meta);
      menubar->append(menu);      
      unlock();
      return menubar;
    }
  }
  
  void LuaProxy::call_menu(UINT id) {
    lock();    
    std::map<std::uint16_t, LuaMenuItem*>::iterator it = LuaMenuItem::menuItemIdMap.find(id);
    if (it != LuaMenuItem::menuItemIdMap.end()) {
      LuaHelper::find_userdata<>(L, it->second);
      lua_getfield(L, -1, "notify");
      lua_pushvalue(L, -2); // self	
      int status = lua_pcall(L, 1, 0, 0);
      if (status) {
        std::string s(lua_tostring(L, -1));	
        lua_pop(L, 1);
        unlock();
        throw std::runtime_error(s);  
      }  
      lua_pop(L, 1);
    }    
    unlock();
  }

  LuaCanvas* LuaProxy::call_canvas() {    
    try {	
      lock();
      if (!get_method_optional(L, "canvas")) {
        unlock();
        return 0; // no canvas found;
      }
      int status = lua_pcall(L, 1, 1 ,0); // machine:canvas()
      if (status) {
        CString msg(lua_tostring(L, -1));
        unlock();
        throw std::runtime_error(msg.GetString());
      }            
      if (lua_isnumber(L, -1) or lua_isnil(L, -1)) {
        unlock();
        return 0;
      }
      LuaCanvas* canvas = LuaHelper::check<LuaCanvas>(L, -1, LuaCanvasBind::meta);
      lua_pop(L, 1);
      unlock();
      return canvas;
    } CATCH_WRAP_AND_RETHROW(*plug_)
    return 0;
  }

  bool LuaProxy::call_event(canvas::Event* ev) {
    lock();    
    bool res = false;
    try {	      
      if (!get_method_optional(L, "canvas")) {
        unlock();
        return false; // no canvas found;
      }
      int status = lua_pcall(L, 1, 1 ,0); // machine:canvas()
      if (status) {
        CString msg(lua_tostring(L, -1));
        unlock();
        throw std::runtime_error(msg.GetString());
      }            
      if (lua_isnumber(L, -1) || lua_isnil(L, -1)) {
        unlock();
        return false;
      }      
      canvas::Canvas* canvas = LuaHelper::check<canvas::Canvas>(L, -1, LuaCanvasBind::meta);      
      res = canvas->OnEvent(ev);      
      lua_pop(L, 1);
      unlock();            
    } CATCH_WRAP_AND_RETHROW(*plug_)    
    return res;
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
