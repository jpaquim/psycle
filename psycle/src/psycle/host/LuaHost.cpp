// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include <psycle/host/detail/project.hpp>
#include "LuaHost.hpp"
#include "LuaInternals.hpp"
#include "LuaPlugin.hpp"
#include "Player.hpp"
#include "LockIF.hpp"
#include "plugincatcher.hpp"
#include "Song.hpp"
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <lua.hpp>
#include "LuaHelper.hpp"
#include "LuaGui.hpp"
#include "CanvasItems.hpp"
#include "NewMachine.hpp"
#include "MainFrm.hpp"
#include "MfcUi.hpp"
#include "ChildView.hpp"
#include <algorithm>
#include "resources\resources.hpp"

#if defined LUASOCKET_SUPPORT && !defined WINAMP_PLUGIN
extern "C" {
#include <luasocket/luasocket.h>
#include <luasocket/mime.h>
}
#endif //defined LUASOCKET_SUPPORT && !defined WINAMP_PLUGIN

#include <algorithm>
#include <iostream>
#include <sstream>

namespace psycle { 
namespace host {

extern CPsycleApp theApp;
  
using namespace ui;

LuaControl::LuaControl() : L(0), LM(0) {
  invokelater_.reset(new ui::Commands());
  InitializeCriticalSection(&cs);  
}
LuaControl::~LuaControl() {
  Free();
  DeleteCriticalSection(&cs);  
}

void LuaControl::Load(const std::string& filename) {  
  L = LuaGlobal::load_script(filename);
} 

void LuaControl::Run() { 
  int status = lua_pcall(L, 0, LUA_MULTRET, 0);
  if (status) {      
    const char* msg = lua_tostring(L, -1);    
    throw std::runtime_error(msg);       
  }   
}

void LuaControl::yield() {
  lua_yieldk(L, 0, 0, 0);
}

void LuaControl::resume() {
  lua_resume(L, 0, 0);
}

void LuaControl::Start() {
  lua_getglobal(L, "psycle");
  if (lua_isnil(L, -1)) {
    throw std::runtime_error("Psycle not available.");
  }
  lua_getfield(L, -1, "start");
  if (!lua_isnil(L, -1)) {
    int status = lua_pcall(L, 0, 0, 0);
    if (status) {         
      const char* msg = lua_tostring(L, -1); 
      ui::alert(msg);
      throw std::runtime_error(msg);       
    }
  }
}

void LuaControl::Free() {
  if (L) {
    invokelater_->Clear();
    lua_close(L);    
  }
  L = 0;
}

void LuaControl::PrepareState() {
  static const luaL_Reg methods[] = {    
    { NULL, NULL }
  };
  lua_newtable(L);
  luaL_setfuncs(L, methods, 0);
  LuaControl** ud = (LuaControl **)lua_newuserdata(L, sizeof(LuaProxy *));
  luaL_newmetatable(L, "psyhostmeta");
  lua_setmetatable(L, -2);
  *ud = this;
  lua_setfield(L, -2, "__self");
  lua_setglobal(L, "psycle");
  lua_getglobal(L, "psycle"); 
  lua_pushinteger(L, CHILDVIEWPORT);
  lua_setfield(L, -2, "CHILDVIEWPORT");
  lua_pushinteger(L, FRAMEVIEWPORT);
  lua_setfield(L, -2, "FRAMEVIEWPORT");
  lua_pushinteger(L, MDI);
  lua_setfield(L, -2, "MDI");
  lua_pushinteger(L, SDI);
  lua_setfield(L, -2, "SDI");
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

boost::shared_ptr<LuaPlugin> nullPtr;
struct NullDeleter {template<typename T> void operator()(T*) {} };
struct ReleaseImpDeleter {
  template<typename T> void operator()(T* frame) { 
    frame->release_imp(); 
  }
};

std::string LuaControl::install_script() const {
  std::string result;
  lua_getglobal(L, "psycle");
  if (lua_isnil(L, -1)) {
    throw std::runtime_error("Psycle not available.");
  }
  lua_getfield(L, -1, "install");
  if (!lua_isnil(L, -1)) {
    int status = lua_pcall(L, 0, 1, 0);    
    if (status) {         
      const char* msg = lua_tostring(L, -1); 
      ui::alert(msg);
      throw std::runtime_error(msg);       
    }
    result = luaL_checkstring(L, -1);
  }
  return result;
}

PluginInfo LuaControl::meta() const {
  PluginInfo result;
  lua_getglobal(L, "psycle");
  if (lua_isnil(L, -1)) {
    throw std::runtime_error("Psycle not available.");
  }
  lua_getfield(L, -1, "info");
  if (!lua_isnil(L, -1)) {
    int status = lua_pcall(L, 0, 1, 0);    
    if (status) {         
      const char* msg = lua_tostring(L, -1); 
      ui::alert(msg);
      throw std::runtime_error(msg);       
    }
    result = parse_info();    
  } else {
    throw std::runtime_error("no info found");
  }
  return result;
}

PluginInfo LuaControl::parse_info() const {
  PluginInfo result;
  result.type = MACH_LUA;
  result.mode = MACHMODE_FX;
  result.allow = true;
  result.identifier = 0;    
  if (lua_istable(L,-1)) {
    size_t len;
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      const char* key = luaL_checklstring(L, -2, &len);
      if (strcmp(key, "vendor") == 0) {
        const char* value = luaL_checklstring(L, -1, &len);
        result.vendor = std::string(value);
      } else
        if (strcmp(key, "name") == 0) {
          const char* value = luaL_checklstring(L, -1, &len);
          result.name = std::string(value);
        } else
          if (strcmp(key, "mode") == 0) {
            if (lua_isnumber(L, -1) == 0) {
              throw std::runtime_error("info mode not a number"); 
            }
            int value = luaL_checknumber(L, -1);
            switch (value) {
              case 0 : result.mode = MACHMODE_GENERATOR; break;
              case 3 : result.mode = MACHMODE_HOST; break;
              default: result.mode = MACHMODE_FX; break;
            }
          } else
          if (strcmp(key, "generator") == 0) {
            // deprecated, use mode instead
            int value = luaL_checknumber(L, -1);
            result.mode = (value==1) ? MACHMODE_GENERATOR : MACHMODE_FX;
          } else
            if (strcmp(key, "version") == 0) {
              const char* value = luaL_checklstring(L, -1, &len);
              result.version = value;
            } else
              if (strcmp(key, "api") == 0) {
                int value = luaL_checknumber(L, -1);
                result.APIversion = value;
              } else
                if (strcmp(key, "noteon") == 0) {
                  int value = luaL_checknumber(L, -1);
                  result.flags = value;
                }
    }
  }   
  std::ostringstream s;
  s << (result.mode == MACHMODE_GENERATOR ? "Lua instrument"
                                          : "Lua effect")
    << " by "
    << result.vendor;
  result.desc = s.str();
  return result;
}

void LuaStarter::PrepareState() {
  static const luaL_Reg methods[] = {    
    {"addmenu", addmenu},
    {"replacemenu", replacemenu},
    {"addextension", addextension},
    {NULL, NULL}
  };
  lua_newtable(L);
  luaL_setfuncs(L, methods, 0);
  LuaControl** ud = (LuaControl **)lua_newuserdata(L, sizeof(LuaProxy *));
  luaL_newmetatable(L, "psyhostmeta");
  lua_setmetatable(L, -2);
  *ud = this;
  lua_setfield(L, -2, "__self");
  lua_setglobal(L, "psycle");
  lua_getglobal(L, "psycle"); 
  lua_pushinteger(L, CHILDVIEWPORT);
  lua_setfield(L, -2, "CHILDVIEWPORT");
  lua_pushinteger(L, FRAMEVIEWPORT);
  lua_setfield(L, -2, "FRAMEVIEWPORT");
  lua_pushinteger(L, MDI);
  lua_setfield(L, -2, "MDI");
  lua_pushinteger(L, SDI);
  lua_setfield(L, -2, "SDI");
  lua_newtable(L); // table for canvasdata
  lua_setfield(L, -2, "userdata");
  lua_newtable(L); // table for canvasdata
  lua_newtable(L); // metatable
  lua_pushstring(L, "kv");
  lua_setfield(L, -2, "__mode");
  lua_setmetatable(L, -2);
  lua_setfield(L, -2, "weakuserdata");
  lua_pop(L, 1);
  LuaHelper::require<LuaPluginBind>(L, "psycle.extension");
}

int LuaStarter::addmenu(lua_State* L) {
  std::string menu_name(luaL_checkstring(L, 1));  
  lua_getfield(L, 2, "plugin");
  lua_getfield(L, 2, "label");    
  lua_getfield(L, 2, "viewport");
  lua_getfield(L, 2, "userinterface");

  LuaPluginPtr plugin = LuaHelper::test_sptr<LuaPlugin>(L, -4, LuaPluginBind::meta);
  std::string plugin_name = !plugin ? luaL_checkstring(L, -4) : "test";  
  Link link(plugin_name,
            luaL_checkstring(L, -3),
            luaL_checkinteger(L, -2),
            luaL_checkinteger(L, -1));   
   HostExtensions& host_extensions = *((CMainFrame*) ::AfxGetMainWnd())->m_wndView.host_extensions();
  if (plugin) {
    link.plugin = plugin;
    plugin->proxy().set_userinterface(link.user_interface());
	  plugin->Init();			    
    plugin->CanvasChanged.connect(boost::bind(&HostExtensions::OnPluginCanvasChanged, &host_extensions,  _1));
    plugin->ViewPortChanged.connect(boost::bind(&HostExtensions::OnPluginViewPortChanged, &host_extensions,  _1, _2));     
    if (link.user_interface() == MDI) {
      host_extensions.AddToWindowsMenu(link); 
    }
  }  
  if (menu_name == "view") {
    host_extensions.AddViewMenu(link);
  } else
  if (menu_name == "help") {
    host_extensions.AddHelpMenu(link);
  }
  return 0;
}

int LuaStarter::replacemenu(lua_State* L) {
  std::string menu_name(luaL_checkstring(L, 1));  
  int menu_pos(luaL_checkinteger(L, 2));
  lua_getfield(L, 3, "plugin");
  lua_getfield(L, 3, "label");    
  lua_getfield(L, 3, "viewport");
  lua_getfield(L, 3, "userinterface");

  LuaPluginPtr plugin = LuaHelper::test_sptr<LuaPlugin>(L, -4, LuaPluginBind::meta);
  std::string plugin_name = !plugin ? luaL_checkstring(L, -4) : "test";  
  Link link(plugin_name,
            luaL_checkstring(L, -3),
            luaL_checkinteger(L, -2),
            luaL_checkinteger(L, -1));   
   HostExtensions& host_extensions = *((CMainFrame*) ::AfxGetMainWnd())->m_wndView.host_extensions();
  if (plugin) {
    link.plugin = plugin;
    plugin->proxy().set_userinterface(link.user_interface());
	  plugin->Init();			    
    plugin->CanvasChanged.connect(boost::bind(&HostExtensions::OnPluginCanvasChanged, &host_extensions,  _1));
    plugin->ViewPortChanged.connect(boost::bind(&HostExtensions::OnPluginViewPortChanged, &host_extensions,  _1, _2));     
    if (link.user_interface() == MDI) {
      host_extensions.AddToWindowsMenu(link); 
    }
  }  
  if (menu_name == "view") {
    // host_extensions.AddViewMenu(link);
  } else
  if (menu_name == "help") {
    host_extensions.ReplaceHelpMenu(link, menu_pos);
  }
  return 0;
}

int LuaStarter::addextension(lua_State* L) {
  const char* extension_name = luaL_checkstring(L, 1);
  PluginCatcher* plug_catcher =
    static_cast<PluginCatcher*>(&Global::machineload());
  PluginInfo* info = plug_catcher->info(extension_name);
  if (info) {
    if (info->mode == MACHMODE_HOST) {          
     LuaPlugin* mac =new LuaPlugin(info->dllname, -1);     
     mac->proxy().set_userinterface(SDI);
	   mac->Init();			
     luaL_requiref(L, "psycle.extension", &LuaPluginBind::open, true);    
     LuaPluginPtr plugin = LuaHelper::new_shared_userdata<>(L, LuaPluginBind::meta, mac, lua_gettop(L));
     lua_remove(L, -2);     
     HostExtensions& host_extensions = *((CMainFrame*) ::AfxGetMainWnd())->m_wndView.host_extensions();
     host_extensions.Add(plugin);     
    }
  } else {
    ui::alert("Error in start.lua : Extension " + std::string(extension_name) + " not found.");
    lua_pushnil(L);
  }
  return 1;
}

// Class Proxy : export and import between psycle and lua
LuaProxy::LuaProxy(LuaPlugin* host, const std::string& dllname) : 
    host_(host),
    is_meta_cache_updated_(false),
    lua_mac_(0),
    user_interface_(SDI) {     
  Load(dllname);
}

LuaProxy::~LuaProxy() {}

int LuaProxy::invokelater(lua_State* L) {
  if lua_isnil(L, 1) {
    return luaL_error(L, "Argument is nil.");
  }
  boost::shared_ptr<LuaRun> run = LuaHelper::check_sptr<LuaRun>(L, 1, LuaRunBind::meta);  
  struct {
    LuaRun* run;
    void operator()() const
    {
      run->Run();
    }
   } f;   
  f.run = run.get();  
  LuaProxy* proxy = LuaGlobal::proxy(L);  
  if (!proxy) {
    return luaL_error(L, "invokelater: Proxy not found.");
  }
  proxy->invokelater_->Add(f);  
  return 0;
}

void LuaProxy::OnTimer() {      
  try {
    lock();    
    invokelater_->Invoke();              
    lua_gc(L, LUA_GCCOLLECT, 0);
    unlock();
  } catch(std::exception& e) {    
    std::string msg = std::string("LuaRun Errror.") + e.what();
    AfxMessageBox(msg.c_str());
    unlock();
    throw std::runtime_error(msg.c_str());
  }  
  try {
    LuaImport in(L, lua_mac_, this);
    if (in.open("ontimer")) {
      in.pcall(0);      
    }
  } catch(std::exception&) {
  }
}

void LuaProxy::OnFrameClose(ui::Frame&) {
  if (lua_mac_ && user_interface_ == MDI) {
    lua_mac_->doexit();
  }
}

void LuaProxy::PrepareState() {  
  LuaGlobal::proxy_map[L] = this;
  ExportCFunctions();
  // require c modules
  // config
  LuaHelper::require<LuaConfigBind>(L, "psycle.config");
  LuaHelper::require<LuaPluginInfoBind>(L, "psycle.plugininfo");
  LuaHelper::require<LuaPluginCatcherBind>(L, "psycle.plugincatcher");  
  // sound engine
  LuaHelper::require<LuaArrayBind>(L, "psycle.array");
  LuaHelper::require<LuaWaveDataBind>(L, "psycle.dsp.wavedata");  
  LuaHelper::require<LuaMachineBind>(L, "psycle.machine");
  LuaHelper::require<LuaMachinesBind>(L, "psycle.machines");
  LuaHelper::require<LuaWaveOscBind>(L, "psycle.osc");
  LuaHelper::require<LuaResamplerBind>(L, "psycle.dsp.resampler");
  LuaHelper::require<LuaDelayBind>(L, "psycle.delay");
  LuaHelper::require<LuaDspFilterBind>(L, "psycle.dsp.filter");
  LuaHelper::require<LuaEnvelopeBind>(L, "psycle.envelope");
  LuaHelper::require<LuaDspMathHelper>(L, "psycle.dsp.math");
  LuaHelper::require<LuaFileHelper>(L, "psycle.file");
  LuaHelper::require<LuaFileObserverBind>(L, "psycle.fileobserver");
  LuaHelper::require<LuaMidiHelper>(L, "psycle.midi");
  LuaHelper::require<LuaPlayerBind>(L, "psycle.player");
  LuaHelper::require<LuaPatternDataBind>(L, "psycle.pattern");
  // ui host interaction
  LuaHelper::require<LuaSequenceBarBind>(L, "psycle.sequencebar");
  LuaHelper::require<LuaActionListenerBind>(L, "psycle.ui.hostactionlistener");
  LuaHelper::require<LuaCmdDefBind>(L, "psycle.ui.cmddef");
  LuaHelper::require<LuaRunBind>(L, "psycle.run");
  lua_ui_requires(L);
  LuaHelper::require<LuaStockBind>(L, "psycle.stock");
#if !defined WINAMP_PLUGIN
  LuaHelper::require<LuaPlotterBind>(L, "psycle.plotter");
#endif //!defined WINAMP_PLUGIN
#if defined LUASOCKET_SUPPORT && !defined WINAMP_PLUGIN
  luaL_requiref(L, "socket.core", luaopen_socket_core, 1);
  lua_pop(L, 1);
  luaL_requiref(L, "mime", luaopen_mime_core, 1);
  lua_pop(L, 1);
#endif  //LUASOCKET_SUPPORT  
}

void LuaProxy::Reload() {
  try {      
    lock();
    GlobalTimer::instance().KillTimer();    
    host_->set_crashed(true);
    lua_State* old_state = L;
    lua_State* new_state = 0;
    try {
      L = LuaGlobal::load_script(host_->GetDllName());      
      PrepareState();
      Run();
      Init();      
      if (old_state) {
        LuaGlobal::proxy(old_state)->invokelater_->Clear();
        boost::weak_ptr<ui::MenuContainer> menu_bar = LuaGlobal::proxy(old_state)->menu_bar();
        if (!menu_bar.expired()) {
          menu_bar.lock()->root_node().lock()->erase_imps(menu_bar.lock()->imp());
          menu_bar.lock()->Invalidate();           
        }
        lua_close(old_state);
      }
      OnActivated(2);      
    } catch(std::exception &e) {
      if (new_state) {        
        lua_close(new_state);
      }
      L = old_state;
      std::string s = std::string("Reload Error, old script still running!\n") + e.what();        
      GlobalTimer::instance().StartTimer();
      unlock();
      throw std::exception(s.c_str());  
    }
    host_->Mute(false);
    host_->set_crashed(false);
    GlobalTimer::instance().StartTimer();
    unlock();
  } CATCH_WRAP_AND_RETHROW(host())
}

int LuaProxy::alert(lua_State* L) {    
  const char* msg = luaL_checkstring(L, 1);    
  AfxMessageBox(msg);
  return 0;
}


int LuaProxy::confirm(lua_State* L) {    
  const char* msg = luaL_checkstring(L, 1);    
  int result = AfxMessageBox(msg, MB_OK | MB_OKCANCEL | MB_TOPMOST);
  lua_pushboolean(L, result == IDOK);  
  return 1;  
}

int LuaProxy::terminal_output(lua_State* L) {
  int n = lua_gettop(L);  // number of arguments
  const char* out = 0;
  if (lua_isboolean(L, 1)) {
    int v = lua_toboolean(L, 1);
    if (v==1) out = "true"; else out = "false";
    ui::TerminalFrame::instance().output(out);
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
      ui::TerminalFrame::instance().output(s);
    }
  }
  return 0;
}

int LuaProxy::call_selmachine(lua_State* L) {    
  CNewMachine dlg(AfxGetMainWnd());
  dlg.DoModal();
  if (dlg.Outputmachine >= 0) {
    std::string filename = dlg.psOutputDll;
    boost::filesystem::path p(filename);
    lua_pushstring(L, p.stem().string().c_str());
    lua_pushstring(L, dlg.psOutputDll.c_str());
  } else {
    lua_pushnil(L);
  }   
  return 2;
}

int LuaProxy::set_machine(lua_State* L) {
  lua_getglobal(L, "psycle");
  lua_getfield(L, -1, "__self");
  LuaProxy* proxy = *(LuaProxy**)luaL_checkudata(L, -1, "psyhostmeta");
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_getfield(L, 1, "__self");
  LuaMachine** ud = (LuaMachine**) luaL_checkudata(L, -1, "psypluginmeta");
  (*ud)->set_mac(proxy->host_);
  proxy->lua_mac_ = *ud;
  proxy->lua_mac_->setproxy(proxy);    
  lua_pushvalue(L, 1);
  LuaHelper::register_weakuserdata(L, proxy->lua_mac());
  lua_setfield(L, 2, "proxy");
  // share samples
  (*ud)->build_buffer(proxy->host_->samplesV, 256);
  return 0;
}

int LuaProxy::set_menubar(lua_State* L) {
  boost::shared_ptr<LuaMenuBar> menu_bar = LuaHelper::check_sptr<LuaMenuBar>(L, 1, LuaMenuBarBind::meta);  
  lua_getglobal(L, "psycle");
  lua_getfield(L, -1, "__self");
  LuaProxy* proxy = *(LuaProxy**)luaL_checkudata(L, -1, "psyhostmeta");    
  proxy->menu_bar_ = menu_bar;
  return 0;
}

void LuaProxy::ExportCFunctions() {
  static const luaL_Reg methods[] = {
    {"invokelater", invokelater},
    {"output", terminal_output },
    {"alert", alert},
    {"confirm", confirm},
    {"setmachine", set_machine},
    {"setmenubar", set_menubar},    
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

  lua_pushinteger(L, CHILDVIEWPORT);
  lua_setfield(L, -2, "CHILDVIEWPORT");
  lua_pushinteger(L, FRAMEVIEWPORT);
  lua_setfield(L, -2, "FRAMEVIEWPORT");
  lua_pushinteger(L, MDI);
  lua_setfield(L, -2, "MDI");
  lua_pushinteger(L, SDI);
  lua_setfield(L, -2, "SDI");

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

bool LuaProxy::IsPsyclePlugin() const {   
  bool result = lua_mac_ != 0;  
  if (!result) {
    lua_getglobal(L, "psycle");
    if (lua_isnil(L, -1)) {
      lua_pop(L, 1);
    } else {
      lua_getfield(L, -1, "info");
      result = !lua_isnil(L, -1);      
      lua_pop(L, 2);
    }
  }  
  return result;
}

bool LuaProxy::HasDirectMetaAccess() const {
  bool result(false);
  lua_getglobal(L, "psycle");
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
  } else {
    lua_getfield(L, -1, "info");
    result = !lua_isnil(L, -1);      
    lua_pop(L, 2);
  }
  return result;
}

PluginInfo LuaProxy::meta() const {
  if (is_meta_cache_updated_) {
    return meta_cache_;
  }
  PluginInfo result;  
  try {
    result = cache_meta(LuaControl::meta());
  } catch (std::exception&) {
    try {        
      LuaImport in(L, lua_mac_, this);
      if (!in.open("info")) {
        throw std::runtime_error("no info found"); 
      }
      in.pcall(1);    
      result = cache_meta(parse_info());
    } CATCH_WRAP_AND_RETHROW(host())
  }
  return result;
}

void LuaProxy::Init() {
  try {
    LuaImport in(L, lua_mac_, this);
    if (in.open("init")) {      
      in.pcall(0);          
    } else {    
      const char* msg = "no init found";        
      throw std::runtime_error(msg);
    }    
  } catch(std::exception& e) {
    std::string msg = std::string("LuaProxy Init Errror.") + e.what();
    AfxMessageBox(msg.c_str());
    throw std::runtime_error(msg.c_str());
  }
}
  
void LuaProxy::call_execute() {
  try {
    LuaImport in(L, lua_mac_, this);
    if (in.open("onexecute")) {
      in.pcall(0);      
    }
  } CATCH_WRAP_AND_RETHROW(host())
}

void LuaProxy::OnCanvasChanged() { 
  host_->OnCanvasChanged();
}

void LuaProxy::OnActivated(int viewport) {
  try {
    LuaImport in(L, lua_mac_, this);
    if (in.open("onactivated")) {
      in << viewport << pcall(0);           
    }
  } CATCH_WRAP_AND_RETHROW(host())
}

void LuaProxy::OnDeactivated() {
  try {
    LuaImport in(L, lua_mac_, this);
    if (in.open("ondeactivated")) {
      in.pcall(0);      
    }
  } CATCH_WRAP_AND_RETHROW(host())
}

void LuaProxy::SequencerTick() {
  try {
    LuaImport in(L, lua_mac_, this);
    if (in.open("sequencertick")) {
      in.pcall(0);      
    }    
  } CATCH_WRAP_AND_RETHROW(host())
}

bool LuaProxy::DescribeValue(int param, char * txt) {
  try {
    if (host().crashed() || param < 0) {
      std::string par_display("Out of range or Crashed");
      std::sprintf(txt, "%s", par_display);
      return false;
    }
    if(param >= 0 && param < host().GetNumParams()) {
      try {
        std::string par_display = ParDisplay(param);
        std::string par_label = ParLabel(param);
        if (par_label == "")
          std::sprintf(txt, "%s", par_display.c_str());
        else {
          std::sprintf(txt, "%s(%s)", par_display.c_str(), par_label.c_str());
        }
        return true;
      } catch(std::exception &e) {
        e;
        std::string par_display("Out of range");
        std::sprintf(txt, "%s", par_display);
        return true;
      } //do nothing.
    }
    return false;
  } CATCH_WRAP_AND_RETHROW_WITH_FAKE_RETURN(host())
}

void LuaProxy::SeqTick(int channel, int note, int ins, int cmd, int val) {
  try {
    LuaImport in(L, lua_mac_, this);
    if (in.open("seqtick")) {
      in << channel << note << ins << cmd << val << pcall(0);   
    }
  } CATCH_WRAP_AND_RETHROW(host())
}
  
struct setcmd {
  void operator()(LuaImport& import) const {
    lua_State* L = import.L();        
    if (lastnote != notecommands::empty) {
      lua_pushinteger(L, lastnote);
    } else {
      lua_pushnil(L);
    }
    lua_pushinteger(L, inst);
    lua_pushinteger(L, cmd);
    lua_pushinteger(L, val);        
  }
  int lastnote, inst, cmd, val;
};

void LuaProxy::Command(int lastnote, int inst, int cmd, int val) {
  try {
    LuaImport in(L, lua_mac_, this);
    if (in.open("command")) {
      setcmd set = {lastnote, inst, cmd, val};
      in << set << pcall(0);      
    }
  } CATCH_WRAP_AND_RETHROW(host())
}
  
void LuaProxy::NoteOn(int note, int lastnote, int inst, int cmd, int val) {
  try {
    LuaImport in(L, lua_mac_, this);
    if (in.open("noteon")) {
      setcmd set = {lastnote, inst, cmd, val};
      in << note << set << pcall(0);      
    }
  } CATCH_WRAP_AND_RETHROW(host())
}

void LuaProxy::NoteOff(int note, int lastnote, int inst, int cmd, int val) {
  try {
    LuaImport in(L, lua_mac_, this);
    if (in.open("noteoff")) {
      setcmd set = {lastnote, inst, cmd, val};
      in << note << set << pcall(0);      
    }
  } CATCH_WRAP_AND_RETHROW(host())
}

std::string LuaProxy::call_help() {    
  try {
    LuaImport in(L, lua_mac_, this);
    if (in.open("help")) {  
      std::string str;
      in << pcall(1) >> str;
      return str;
    }       
  } CATCH_WRAP_AND_RETHROW(host())
  return "";
}

int LuaProxy::GetData(unsigned char **ptr, bool all) {
  try {
    LuaImport in(L, lua_mac_, this);
    if (in.open("data")) {      
      std::string str;
      in << all << pcall(1) >> str;
      *ptr = new unsigned char[str.size()];
      std::copy(str.begin(), str.end(), *ptr);
      return str.size();
    }      
    return 0;
  } CATCH_WRAP_AND_RETHROW_WITH_FAKE_RETURN(host()) 
}

uint32_t LuaProxy::GetDataSize() {
  try {
    std::string str;
    LuaImport in(L, lua_mac_, this);
    if (in.open("putdata")) {    
      in << pcall(1) >> str;
      return str.size();
    }
    return 0;
  } CATCH_WRAP_AND_RETHROW_WITH_FAKE_RETURN(host())
}

void LuaProxy::PutData(unsigned char* data, int size) {
  try {
    LuaImport in(L, lua_mac_, this);
    if (in.open("putdata")) {
      std::string s(reinterpret_cast<char const*>(data), size);    
      in << s << pcall(0);
    }
  } CATCH_WRAP_AND_RETHROW(host())
}

void LuaProxy::Work(int numSamples, int offset) {
  try {
    if (numSamples > 0) {      
      LuaImport in(L, lua_mac_, this);
      if (in.open("work")) {
        if (offset > 0) {
          lua_mac()->offset(offset);
        }
        lua_mac()->update_num_samples(numSamples);
        in << numSamples << pcall(0);      
        if (offset > 0) {
          lua_mac()->offset(-offset);
        }      
        lua_gc(L, LUA_GCSTEP, 5);            
      }
    }
  } CATCH_WRAP_AND_RETHROW(host())
}

void LuaProxy::Stop() {
  try {
    LuaImport in(L, lua_mac_, this);
    in.open("stop");
    in.pcall(0);
  } CATCH_WRAP_AND_RETHROW(host())
}

void LuaProxy::call_sr_changed(int rate) {
  try {
    LuaImport in(L, lua_mac_, this);
    in.open("onsrchanged"); 
    WaveOscTables::getInstance()->set_samplerate(rate);
    LuaDspFilterBind::setsamplerate(rate);
    LuaWaveOscBind::setsamplerate(rate);
    in << rate << pcall(0);
  } CATCH_WRAP_AND_RETHROW(host())
}

void LuaProxy::call_aftertweaked(int numparam) {
  LuaImport in(L, lua_mac_, this);
  in.open(); 
  in << getparam(numparam, "afternotify") << pcall(0);
}

// Parameter tweak range is [0..1]
void LuaProxy::ParameterTweak(int par, double val) {
  try {
    LuaImport in(L, lua_mac_, this);
    in.open(); 
    in << getparam(par, "setnorm") << val << pcall(0);
  } CATCH_WRAP_AND_RETHROW(host())
}

double LuaProxy::Val(int par) {
  try {
    double v(0);
    LuaImport in(L, lua_mac_, this);
    in.open(); 
    in << getparam(par, "norm") << pcall(1) >> v;
    return v;
  } CATCH_WRAP_AND_RETHROW_WITH_FAKE_RETURN(host())
}

void LuaProxy::Range(int par,int &minval, int &maxval) {
  try {
    int v(0);
    LuaImport in(L, lua_mac_, this);
    in.open(); 
    in << getparam(par, "range") << pcall(3) >> v >> v >> maxval;    
    minval = 0;
  } CATCH_WRAP_AND_RETHROW(host())
}

std::string LuaProxy::Name(int par) {
  try {
    std::string str;
    LuaImport in(L, lua_mac_, this);
    in.open(); 
    in << getparam(par, "name") << pcall(1) >> str;          
    return str;
  } CATCH_WRAP_AND_RETHROW(host())
  return "";
}

std::string LuaProxy::Id(int par) {
  try {
    std::string str;
    LuaImport in(L, lua_mac_, this);
    in.open(); 
    in << getparam(par, "id") << pcall(1) >> str;    
    return str;
  } CATCH_WRAP_AND_RETHROW(host())
  return "";
}

std::string LuaProxy::ParDisplay(int par) {    
  std::string str;
  LuaImport in(L, lua_mac_, this);
  in.open(); 
  in << getparam(par, "display") << pcall(1) >> str;    
  return str;    
}

std::string LuaProxy::ParLabel(int par) {    
  std::string str;
  LuaImport in(L, lua_mac_, this);
  in.open(); 
  in << getparam(par, "label") << pcall(1) >> str;    
  return str;    
}

int LuaProxy::Type(int par) {
  try {
    int v(0);
    LuaImport in(L, lua_mac_, this);
    in.open(); 
    in << getparam(par, "mpf") << pcall(1) >> v;    
    return v;
  } CATCH_WRAP_AND_RETHROW_WITH_FAKE_RETURN(host())
}

// call events
void LuaProxy::call_setprogram(int idx) {
  try {
    LuaImport in(L, lua_mac_, this);    
    if (in.open("setprogram")) {
      in << idx << pcall(0);
    }
  } CATCH_WRAP_AND_RETHROW(host())
}

// call events
int LuaProxy::get_curr_program() {
  try {
    int prog;
    LuaImport in(L, lua_mac_, this);    
    if (in.open("getprogram")) {
      in << pcall(1) >> prog;
    }
    return -1;
  } CATCH_WRAP_AND_RETHROW_WITH_FAKE_RETURN(host())
}

// call events
int LuaProxy::call_numprograms() {
    return lua_mac()->numprograms();
}

std::string LuaProxy::get_program_name(int bnkidx, int idx) {
  try {
    std::string str;
    LuaImport in(L, lua_mac_, this);    
    if (in.open("programname")) {
      in << bnkidx << idx << pcall(1) >> str;
    }    
    return str;
  } CATCH_WRAP_AND_RETHROW(host())
  return "";
}

bool LuaProxy::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
  bool res = true;
  try {
    LuaImport in(L, lua_mac_, this);    
    if (in.open("onkeydown")) {
      lua_newtable(L);
      int flags = 0;
      if (GetKeyState(VK_SHIFT) & 0x8000) {
          flags |= MK_SHIFT;
      }
      if (GetKeyState(VK_CONTROL) & 0x8000) {
        flags |= MK_CONTROL;
      }
      lua_pushboolean(L, GetKeyState(VK_SHIFT) & 0x8000);
      lua_setfield(L, -2, "shiftkey");
      lua_pushboolean(L, GetKeyState(VK_CONTROL) & 0x8000);
      lua_setfield(L, -2, "ctrlkey");      
      lua_pushinteger(L, nChar);
      lua_setfield(L, -2, "keycode");       
      in.pcall(1);
      in >> res;
    }
  } CATCH_WRAP_AND_RETHROW(host())
  return res;
}  

bool LuaProxy::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
  bool res = true;
  try {
    LuaImport in(L, lua_mac_, this);    
    if (in.open("onkeyup")) {
      lua_newtable(L);
      int flags = 0;
      if (GetKeyState(VK_SHIFT) & 0x8000) {
          flags |= MK_SHIFT;
      }
      if (GetKeyState(VK_CONTROL) & 0x8000) {
        flags |= MK_CONTROL;
      }
      lua_pushboolean(L, GetKeyState(VK_SHIFT) & 0x8000);
      lua_setfield(L, -2, "shiftkey");
      lua_pushboolean(L, GetKeyState(VK_CONTROL) & 0x8000);
      lua_setfield(L, -2, "ctrlkey");      
      lua_pushinteger(L, nChar);
      lua_setfield(L, -2, "keycode");       
      in.pcall(1);
      in >> res;
    }
  } CATCH_WRAP_AND_RETHROW(host())
  return res;
}  

void LuaProxy::OpenInFrame() {
  if (!frame_ && !canvas().expired()) {
    frame_.reset(new ui::Frame());
    frame_->set_viewport(canvas().lock());
    frame_->close.connect(boost::bind(&LuaProxy::OnFrameClose, this, _1));
    boost::shared_ptr<ui::FrameAligner> right_frame_aligner(new ui::FrameAligner(ALRIGHT));
    right_frame_aligner->SizeToScreen(0.4, 0.8);
    frame_->Show(right_frame_aligner);
  }
}

void LuaProxy::ToggleViewPort() {    
  if (!frame_) {
    OpenInFrame();
    if (host_) {
      host_->ViewPortChanged(*host_, FRAMEVIEWPORT);
    }
  } else {
    frame_.reset();
    if (host_) {
      host_->ViewPortChanged(*host_, CHILDVIEWPORT);
    }
  }
}

void LuaProxy::UpdateWindowsMenu() {
  HostExtensions& host_extensions = *((CMainFrame*) ::AfxGetMainWnd())->m_wndView.host_extensions();  
  host_extensions.ChangeWindowsMenuText(host_);
}

ui::Systems* LuaProxy::systems() { 
  if (!systems_.get()) {    
    systems_.reset(new Systems(new LuaSystems(L)));
  } 
  return systems_.get();
}


// End of Class Proxy

const char* LuaPluginBind::meta = "pluginmeta";

int  LuaPluginBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
    {"new", create},    
    {NULL, NULL}
  };
  return LuaHelper::open(L, meta, methods);
}

int LuaPluginBind::create(lua_State* L) {
  const char* dllname = luaL_checkstring(L, 2);
  LuaHelper::new_shared_userdata(L, meta, new LuaPlugin(dllname, AUTOID));
  return 1;
}

// Class HostExtensions : Container for HostExtensions

const int windows_menu_pos = 6;

void HostExtensions::StartScript() {    
  try {
    AutoInstall();
    LuaStarter starter;
    starter.Load(PsycleGlobal::configuration().GetAbsoluteLuaDir() + "\\" +
                 "start.lua");    
    starter.PrepareState();
    starter.Run();
  } catch (std::exception& e) {
    ui::alert(e.what());
  }
}

void HostExtensions::AutoInstall() {
  typedef std::vector<std::string> InstalledList;
  InstalledList installed = search_auto_install();
  PluginCatcher* plug_catcher = dynamic_cast<PluginCatcher*>(&Global::machineload()); 
	assert(plug_catcher);
  PluginInfoList list = plug_catcher->GetLuaExtensions();
	if (list.size() > 0) {            
	  PluginInfoList list = plug_catcher->GetLuaExtensions();
    std::ofstream outfile;
    std::string script_path = PsycleGlobal::configuration().GetAbsoluteLuaDir();
    std::string start_script_path = script_path + "\\start.lua";
    outfile.open(start_script_path.c_str(), std::ios_base::app);
	  PluginInfoList::iterator it = list.begin();	  
	  for (; it != list.end(); ++it) {
		  PluginInfo* info = *it;     		
      boost::filesystem::path p(info->dllname.c_str());            
	    InstalledList::iterator k = std::find(installed.begin(), installed.end(), p.filename().string());
      if (k == installed.end()) {         
         LuaControl mac;
         mac.Load(info->dllname.c_str());
         mac.PrepareState();
         mac.Run();
         std::string install_script = mac.install_script();                  
         //std::string install_script = luaL_checkstring(L, -1); // mac->install_script();                  
         outfile << std::endl << "-- @" << p.filename().string() << std::endl << install_script;         
      }
	  }         
  }
}

std::vector<std::string> HostExtensions::search_auto_install() {
  std::vector<std::string> result;
  std::string script_path = PsycleGlobal::configuration().GetAbsoluteLuaDir();
  std::string start_script_path = script_path + "\\start.lua";
  using namespace std;
  ifstream fileInput;  
  string line;
  char* search = "@"; // test variable to search in file
  // open file to search
  fileInput.open(start_script_path.c_str());
  if(fileInput.is_open()) {
    unsigned int curLine = 0;
    while(getline(fileInput, line)) { // I changed this, see below
      curLine++;
      if (line.find(search, 0) != string::npos) {
        cout << "found: " << search << "line: " << curLine << endl;
        std::string line_str(line.c_str());
        std::size_t pos = line_str.find("@");
        if (pos != std::string::npos) {
          result.push_back(line_str.substr(pos + 1));
        }       
      }
    }
    fileInput.close();
  }
  return result;
}

void HostExtensions::AddViewMenu(Link& link) {
  CMainFrame* pParentMain =(CMainFrame *)child_view_->pParentFrame;      
  CMenu* view_menu = pParentMain->GetMenu()->GetSubMenu(3);	
  int win32_menu_id = ID_DYNAMIC_MENUS_START + ui::MenuContainer::id_counter++;
  view_menu->InsertMenu(
      menu_pos_++,
      MF_STRING | MF_BYPOSITION,
      win32_menu_id,
      ui::mfc::Charset::utf8_to_win(link.label()).c_str());  
  menuItemIdMap_[win32_menu_id] = link;
}


CMenu* HostExtensions::FindSubMenu(CMenu* parent, const std::string& text) {
  CMainFrame* pParentMain =(CMainFrame *)child_view_->pParentFrame;      
  CMenu* main_menu = pParentMain->GetMenu();
  CMenu* result(0);
  for ( int i=0; i < main_menu->GetMenuItemCount(); ++i) {
    CString str;
    main_menu->GetMenuString(i, str, MF_BYPOSITION);
    std::string tmp(str.GetString());
    if (tmp.find(text) != std::string::npos) {
      result = main_menu->GetSubMenu(i);
      break;
    }
  }
  return result;
}


void HostExtensions::AddHelpMenu(Link& link) {  
  CMainFrame* pParentMain =(CMainFrame *)child_view_->pParentFrame;      
  CMenu* main_menu = pParentMain->GetMenu();
  CMenu* help_menu = FindSubMenu(main_menu, "Help");  
  if (help_menu) {    
    int win32_menu_id = ID_DYNAMIC_MENUS_START + ui::MenuContainer::id_counter++;
    help_menu->AppendMenu(      
      MF_STRING,
      win32_menu_id,
      ui::mfc::Charset::utf8_to_win(link.label()).c_str());  
    menuItemIdMap_[win32_menu_id] = link;
  }
}

void HostExtensions::ReplaceHelpMenu(Link& link, int pos) {
  CMainFrame* pParentMain =(CMainFrame *)child_view_->pParentFrame;      
  CMenu* main_menu = pParentMain->GetMenu();
  CMenu* help_menu = FindSubMenu(main_menu, "Help");
  if (help_menu) {    
    int win32_menu_id = ID_DYNAMIC_MENUS_START + ui::MenuContainer::id_counter++;    
    help_menu->RemoveMenu(pos, MF_BYPOSITION);
    help_menu->InsertMenu(pos, MF_BYPOSITION, win32_menu_id, ui::mfc::Charset::utf8_to_win(link.label()).c_str());    
    menuItemIdMap_[win32_menu_id] = link;
  }
}

void HostExtensions::InitWindowsMenu() {
  CMainFrame* pParentMain =(CMainFrame *)child_view_->pParentFrame;
  CMenu* main_menu = pParentMain->GetMenu();  
  windows_menu_ = ::CreateMenu();    
  InsertMenu(main_menu->GetSafeHmenu(), windows_menu_pos, MF_POPUP | MF_ENABLED, (UINT_PTR)windows_menu_, _T("Windows"));     
}

void HostExtensions::Load(CMenu* view_menu) {
   /*PluginCatcher* plug_catcher = dynamic_cast<PluginCatcher*>(&Global::machineload()); 
	 assert(plug_catcher);
   PluginInfoList list = plug_catcher->GetLuaExtensions();
	 if (list.size() > 0) {            
	    PluginInfoList list = plug_catcher->GetLuaExtensions();
	    PluginInfoList::iterator it = list.begin();
	    int pos = 8; bool has_ext = false;
	    for (; it != list.end(); ++it) {
		  PluginInfo* info = *it;     		  
	    int id = ID_DYNAMIC_MENUS_START+ui::MenuContainer::id_counter++;   
		  try {
			LuaPluginPtr mac(new LuaPlugin(info->dllname.c_str(), -1));
			mac->Init();
			ui::Canvas* user_view = 0;
			try {
			  user_view = mac->canvas().lock().get();
			} catch (std::exception&) {            
		  } 
		  //if (user_view) {
			   view_menu->InsertMenu(pos++, MF_STRING | MF_BYPOSITION, id, info->name.c_str());
		  //} else {            
			//   view_menu->AppendMenu(MF_STRING | MF_BYPOSITION, id, info->name.c_str());
		  //}
		    ///ui::MenuItem::id_counter++;
		    Add(mac); 
		    menuItemIdMap_[id] = mac.get();
		    // if (user_view) ui::MenuItem::id_counter++;          
		    has_ext = true;
		    mac->CanvasChanged.connect(bind(&HostExtensions::OnPluginCanvasChanged, this,  _1));
		  } catch (std::exception& e) {
		    ui::alert(e.what());		    
		  }                
	    } 
	    if (has_ext) {
		  view_menu->AppendMenu(MF_SEPARATOR, 0, "-");
		  int id = ID_DYNAMIC_MENUS_START + ui::MenuContainer::id_counter++;
		  view_menu->AppendMenu(MF_STRING | MF_BYPOSITION, id, "Reload Active Extension");
		  menuItemIdMap_[id] = NULL;
	    }
	  }*/
}

void HostExtensions::AddToWindowsMenu(Link& link) {
  const int id = ID_DYNAMIC_MENUS_START + ui::MenuContainer::id_counter++;
  std::string label;
  ::AppendMenu(windows_menu_,
               MF_STRING | MF_BYPOSITION,
               id,
               ui::mfc::Charset::utf8_to_win(menu_label(link)).c_str());
  menuItemIdMap_[id] = link;
}

std::string HostExtensions::menu_label(const Link& link) const {
  std::string result;
  if (!link.plugin.expired()) {  
    std::string filename_noext;
    filename_noext = boost::filesystem::path(link.dll_name()).stem().string();
    std::transform(filename_noext.begin(), filename_noext.end(), filename_noext.begin(), ::tolower);
    result = filename_noext + " - " + link.plugin.lock()->title();
  } else {
    result = link.label();
  }
  return result;
}

void HostExtensions::RemoveFromWindowsMenu(LuaPlugin* plugin) {
  MenuMap::iterator it = menuItemIdMap_.begin();
  for ( ; it != menuItemIdMap_.end(); ++it) {
    if (!it->second.plugin.expired()) {
      LuaPlugin* plug = it->second.plugin.lock().get();
      if (plug == plugin) {
        ::RemoveMenu(windows_menu_, it->first, MF_BYCOMMAND);
      }
    }
  }
}

void HostExtensions::OnDynamicMenuItems(UINT nID) {
  ui::mfc::MenuContainerImp* mbimp =  ui::mfc::MenuContainerImp::MenuContainerImpById(nID);
  if (mbimp != 0) {
     mbimp->WorkMenuItemEvent(nID);
     return;
  }
  /*if (menuItemIdMap_[nID]==NULL) {
   if (active_lua_) {          
     LuaPlugin* lp = active_lua_;
     try {                      
       lp->proxy().Reload();            
       child_view_->ChangeCanvas(lp->canvas().lock().get());
       boost::weak_ptr<ui::MenuContainer> menu_bar = active_lua_->proxy().menu_bar();
       if (!menu_bar.expired()) {
         ui::mfc::MenuContainerImp* menu_bar_imp = (ui::mfc::MenuContainerImp*) menu_bar.lock()->imp();
         menu_bar_imp->set_menu_window(::AfxGetMainWnd(), menu_bar.lock()->root_node().lock());
       }
     } catch (std::exception e) {
       ui::alert(e.what());
     }          
     child_view_->Invalidate(false);
   }
   return;  
   }*/
   MenuMap::iterator it = menuItemIdMap_.find(nID);
  int viewport = CHILDVIEWPORT;
  if (it != menuItemIdMap_.end()) {        
    Link link = it->second;
    boost::shared_ptr<LuaPlugin> plug;
    if (link.plugin.expired()) {
      std::string script_path = PsycleGlobal::configuration().GetAbsoluteLuaDir();
      try {
        link.plugin = plug = Execute(link);      
      } catch(std::exception&) {
        return;
      }
      if (link.user_interface() == SDI) {        
        menuItemIdMap_[nID] = link;
      }
      viewport = link.viewport();
    } else {
      plug = link.plugin.lock();
      ui::Canvas::WeakPtr user_view = plug->canvas();      
    }        
    if (plug->crashed()) return;
      if (active_lua_ != plug.get()) {
        ui::Canvas::WeakPtr user_view = plug->canvas();        
      if (!user_view.expired()) {            
        active_lua_ = plug.get();
        try {
          boost::weak_ptr<ui::MenuContainer> menu_bar = plug->proxy().menu_bar();
          if (!menu_bar.expired()) {
            ui::mfc::MenuContainerImp* menubar_imp = (ui::mfc::MenuContainerImp*) menu_bar.lock()->imp();
            menubar_imp->set_menu_window(::AfxGetMainWnd(), menu_bar.lock()->root_node().lock());
          }
          plug->ViewPortChanged(*plug.get(), viewport);
          plug->OnActivated(viewport);
        } catch (std::exception&) {               
          // AfxMessageBox(e.what());
        }
        child_view_->Invalidate(false);
      } else {  
        try {
          plug->OnExecute();
        } catch (std::exception&) {
          // LuaGlobal::onexception(plug->proxy().state());
          // AfxMessageBox(e.what());
        }  
      }               
    }
  }      
}

LuaPluginPtr HostExtensions::Execute(Link& link) {
  std::string script_path = PsycleGlobal::configuration().GetAbsoluteLuaDir();
  LuaPluginPtr mac(new LuaPlugin(script_path + "\\" + link.dll_name().c_str(), -1));
  mac->proxy().set_userinterface(link.user_interface());
	mac->Init();			
  Add(mac); 
  mac->CanvasChanged.connect(boost::bind(&HostExtensions::OnPluginCanvasChanged, this,  _1));
  mac->ViewPortChanged.connect(boost::bind(&HostExtensions::OnPluginViewPortChanged, this,  _1, _2));  
  link.plugin = mac;
  if (link.user_interface() == MDI) {
    AddToWindowsMenu(link); 
  }
  return mac;
}

void HostExtensions::OnPluginCanvasChanged(LuaPlugin& plugin) {     
  child_view_->ChangeCanvas(plugin.canvas().lock().get());
  CMainFrame* pParentMain =(CMainFrame *)child_view_->pParentFrame;      
  if (plugin.canvas().expired()) {        
    if (!child_view_->IsWindowVisible()) {              
      pParentMain->m_luaWndView->ShowWindow(SW_HIDE);
      child_view_->ShowWindow(SW_SHOW);
    }
  } else {
    if (child_view_->IsWindowVisible()) {        
      child_view_->ShowWindow(SW_HIDE);
      pParentMain->m_luaWndView->ShowWindow(SW_SHOW);            
    }
  }      
}

void HostExtensions::OnPluginViewPortChanged(LuaPlugin& plugin, int viewport) {
  CMainFrame* pParentMain =(CMainFrame *)child_view_->pParentFrame;      
  if (viewport == CHILDVIEWPORT) {
    child_view_->ChangeCanvas(plugin.canvas().lock().get());
    if (!child_view_->IsWindowVisible()) {      
      child_view_->ShowWindow(SW_HIDE);
      pParentMain->m_luaWndView->ShowWindow(SW_SHOW);
    }
  } else 
  if (viewport == FRAMEVIEWPORT) {
    if (!child_view_->IsWindowVisible()) {              
      pParentMain->m_luaWndView->ShowWindow(SW_HIDE);
      child_view_->ShowWindow(SW_SHOW);
    }
  }
}

void HostExtensions::HideActiveLua() {
  CMainFrame* pParentMain =(CMainFrame *)child_view_->pParentFrame;    
  pParentMain->m_luaWndView->ShowWindow(SW_HIDE);
  if (active_lua_) {
     active_lua_->OnDeactivated();
      HideActiveLuaMenu();
  }
  active_lua_ = 0;
}

void HostExtensions::HideActiveLuaMenu() {
  if (active_lua_) {
    boost::weak_ptr<ui::MenuContainer> menu_bar = active_lua_->proxy().menu_bar();
    if (!menu_bar.expired() && !menu_bar.lock()->root_node().expired()) {
      ui::Node::Ptr root = menu_bar.lock()->root_node().lock();
      root->erase_imps(menu_bar.lock()->imp());
      menu_bar.lock()->Invalidate();           
    }
  }
}

void HostExtensions::Free() {
  HostExtensions::List& plugs_ = extensions_;
  HostExtensions::List::iterator it = plugs_.begin();
  for ( ; it != plugs_.end(); ++it) {
    LuaPluginPtr ptr = *it;       
    ptr->Free();    
  }
}

HostExtensions::List HostExtensions::Get(const std::string& name) {
  HostExtensions::List list;
  HostExtensions::List& plugs_ = extensions_;
  HostExtensions::List::iterator it = plugs_.begin();
  for ( ; it != plugs_.end(); ++it) {
    LuaPluginPtr ptr = *it;       
    if (ptr->_editName == name) {
      list.push_back(ptr);
    }
  }
  return list;
}

LuaPluginPtr HostExtensions::Get(int idx) {
  HostExtensions::List list;     
  HostExtensions::List::iterator it = extensions_.begin();
  for (; it != extensions_.end(); ++it) {
    LuaPluginPtr ptr = *it;       
    if (ptr->_macIndex == idx) {
      return ptr;
    }
  }
  return nullPtr;
}

void HostExtensions::ChangeWindowsMenuText(LuaPlugin* plugin) {
  MenuMap::iterator it = menuItemIdMap_.begin();
  for ( ; it != menuItemIdMap_.end(); ++it) {
    if (!it->second.plugin.expired()) {
      LuaPlugin* plug = it->second.plugin.lock().get();
      if (plug == plugin) {    
        std::string label = ui::mfc::Charset::utf8_to_win(
                         menu_label(it->second)).c_str();    
        ::ModifyMenu(windows_menu_,
                     it->first,
                     MF_BYCOMMAND | MF_STRING,
                     it->first,
                     label.c_str()
                     );
      }
    }
  }
}
   
// Host
std::map<lua_State*, LuaProxy*> LuaGlobal::proxy_map;  

lua_State* LuaGlobal::load_script(const std::string& dllpath) {
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

PluginInfo LuaGlobal::LoadInfo(const std::string& dllpath) {  
  LuaPlugin plug(dllpath, 0, false);
  return plug.info();    
}

Machine* LuaGlobal::GetMachine(int idx) {
    Machine* mac = 0;
    if (idx < MAX_MACHINES) {
      mac = Global::song()._pMachine[idx];
    } else {      
      HostExtensions& host_extensions = *((CMainFrame*) ::AfxGetMainWnd())->m_wndView.host_extensions();
      mac = host_extensions.Get(idx).get();      
    }
    return mac;
}

std::vector<LuaPlugin*> LuaGlobal::GetAllLuas() {
  std::vector<LuaPlugin*> plugs;
  for (int i=0; i < MAX_MACHINES; ++i) {
    Machine* mac = Global::song()._pMachine[i];
    if (mac && mac->_type == MACH_LUA) {
      plugs.push_back((LuaPlugin*)mac);
    }
  }
  HostExtensions& host_extensions = *((CMainFrame*) ::AfxGetMainWnd())->m_wndView.host_extensions();
  for (HostExtensions::iterator it = host_extensions.begin(); it != host_extensions.end(); ++it) {
    plugs.push_back((*it).get());
  }
  return plugs;
}

namespace luaerrhandler {

int error_handler(lua_State* L) {  
  // first make sure that the error didn't occured in the plugineditor itself
  std::string edit_name = LuaGlobal::proxy(L)->host().GetEditName();
  if (edit_name == "Plugineditor") {
    const char* msg = lua_tostring(L, -1);
    AfxMessageBox(msg);
    return 1; // error in error handler
  }

  lua_getglobal(L, "debug");
  lua_getfield(L, -1, "traceback");
  lua_pushvalue(L, 1);  // pass error message 
  lua_pushinteger(L, 2);  // skip this function and traceback
  lua_call(L, 2, 1);
  HostExtensions& host_extensions = *((CMainFrame*) ::AfxGetMainWnd())->m_wndView.host_extensions();
  HostExtensions::List uilist = host_extensions.Get("Plugineditor");
  HostExtensions::iterator uit = uilist.begin();
  LuaPluginPtr editor;
  // try to find an open instance of plugineditor
  // editing the error plugin
  int macIdx = LuaGlobal::proxy(L)->host()._macIndex;    
  for ( ; uit != uilist.end(); ++uit) {
    LuaPluginPtr plug = *uit;
    LuaProxy& proxy = plug->proxy();
    int idx(-1);      
    LuaImport in(proxy.state(), proxy.lua_mac(), 0);
    if (in.open("editmachineindex")) {
      in << pcall(1) >> idx;
      if (macIdx == idx) {
        editor = plug;
        break;
      }
    }
  }
  if (!editor.get()) {    
    Link link("plugineditor.lua",
              "Plugin Editor",
              FRAMEVIEWPORT,
              MDI);
    try {
      editor = host_extensions.Execute(link);    
    } catch (std::exception&) {
      return 1;
    }
  }
  if (!editor.get()) {
    return 1; // uhps, plugin editor luascript missing ..
  }
    
  lua_State* LE = editor->proxy().state();  
  LuaImport in(LE, editor->proxy().lua_mac(), 0);
  try {
    if (in.open("onexecute")) {      
      lua_pushstring(LE, lua_tostring(L, -1));
      lua_pushnumber(LE, LuaGlobal::proxy(L)->host()._macIndex);

      std::string filename_noext;
      filename_noext = boost::filesystem::path(LuaGlobal::proxy(L)->host().GetDllName()).stem().string();
      std::transform(filename_noext.begin(), filename_noext.end(), filename_noext.begin(), ::tolower);
      PluginCatcher* plug_catcher =
        static_cast<PluginCatcher*>(&Global::machineload());
      PluginInfo* info = plug_catcher->info(filename_noext);
      if (info) {       
        LuaHelper::requirenew<LuaPluginInfoBind>(LE, "psycle.plugininfo", new PluginInfo(*info));
      } else {        
        lua_pop(LE, 4);
        throw std::runtime_error("No Plugininfo available.");
      }       
      int i = 1;
      int depth = 0;
      lua_Debug entry;
      lua_newtable(LE);
      while (lua_getstack(L, depth, &entry)) {
        int status = lua_getinfo(L, "nSl", &entry);
        assert(status);
        if (status == 0) {
          luaL_error(L, "lua getinfo error");
        }
        lua_newtable(LE);
        lua_pushinteger(LE, entry.linedefined);
        lua_setfield(LE, -2, "line");
        lua_pushstring(LE, entry.source);
        lua_setfield(LE, -2, "source");
        lua_pushstring(LE, entry.name ? entry.name : "");
        lua_setfield(LE, -2, "name");
        lua_rawseti(LE, -2, i);
        ++i;
        ++depth;
      }              
      in.pcall(0);          
      editor->ViewPortChanged(*editor.get(), FRAMEVIEWPORT);          
      editor->OnActivated(FRAMEVIEWPORT);
      editor->proxy().OpenInFrame();
      host_extensions.ChangeWindowsMenuText(editor.get());
      return 1;
    }
  } catch (std::exception& e ) {
    // error in plugineditor    
    AfxMessageBox((std::string("Error in error handler! Plugineditor failed!")+std::string(e.what())).c_str());    
  }  
  return 1;
}

} // namespace luaerrhandler
} // namespace host
} // namespace psycle