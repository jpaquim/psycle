// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include <psycle/host/detail/project.hpp>

#include "LuaInternals.hpp"

#include "LuaHelper.hpp"
#include "LuaHost.hpp"
#include "LuaPlugin.hpp"
#include "Player.hpp"
#include "Registry.hpp"
#include "WinIniFile.hpp"
#include "Configuration.hpp"
#include "PsycleConfig.hpp"

#if !defined WINAMP_PLUGIN
#include "PlotterDlg.hpp"
#endif //!defined WINAMP_PLUGIN

#include "plugincatcher.hpp"
#include "Song.hpp"

#include <boost/filesystem.hpp>
#include <psycle/helpers/resampler.hpp>
#include <universalis/os/terminal.hpp>
#include "Mainfrm.hpp"

#include <lua.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include "PsycleConfig.hpp"

namespace psycle { namespace host {
  /////////////////////////////////////////////////////////////////////////////
  // LuaConfig+Bind
  /////////////////////////////////////////////////////////////////////////////

  LuaConfig::LuaConfig() : store_(PsycleGlobal::conf().CreateStore()) { }

  LuaConfig::LuaConfig(const std::string& group) 
    : store_(PsycleGlobal::conf().CreateStore()){     
     OpenGroup(group);
  }
  
  void LuaConfig::OpenGroup(const std::string& group) {
    // Do not open group if loading version 1.8.6
    if(!store_->GetVersion().empty()) {
		  store_->OpenGroup(group);
	  }
  }

  void LuaConfig::CloseGroup() {
    // Do not open group if loading version 1.8.6
    if(!store_->GetVersion().empty()) {
		  store_->CloseGroup();
	  }
  }

  const char* LuaConfigBind::meta = "psyconfigmeta";

  int LuaConfigBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"get", get},
      {"opengroup", opengroup},
      {"closegroup", closegroup},
      {"groups", groups},
      {"keys", keys},
      {"luapath", plugindir},
      {NULL, NULL}
    };
    LuaHelper::open(L, meta, methods,  gc);
    return 1;
  }

  int LuaConfigBind::create(lua_State *L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1 and n!=2) {
      return luaL_error(L, "Got %d arguments expected 2 (self or self and group)", n);
    }
    LuaConfig* cfg = (n==1) ?  new LuaConfig() : new LuaConfig(luaL_checkstring(L, 2));
    LuaHelper::new_userdata<LuaConfig>(L, meta, cfg);
    return 1;
  }

  int LuaConfigBind::opengroup(lua_State *L) {
    LuaHelper::callstrictstr<LuaConfig>(L, meta, &LuaConfig::OpenGroup);
    return LuaHelper::chaining(L);
  }

  int LuaConfigBind::closegroup(lua_State *L) {
    LuaHelper::call<LuaConfig>(L, meta, &LuaConfig::CloseGroup);
    return LuaHelper::chaining(L);
  }

  int LuaConfigBind::get(lua_State *L) {
    int err = LuaHelper::check_argnum(L, 2, "self, key");
    if (err!=0) return err;
    LuaConfig* cfg = LuaHelper::check<LuaConfig>(L, 1, meta);
    ConfigStorage* store = cfg->store();
    if (store) {
      const char* key = luaL_checkstring(L, 2);
      COLORREF color;
      store->Read(key, color);
      LuaHelper::push_argb(L, color);
    } else {
      return 0;
    }
    return 1;
  }

  int LuaConfigBind::groups(lua_State *L) {
    LuaConfig* cfg = LuaHelper::check<LuaConfig>(L, 1, meta);
    typedef std::list<std::string> storelist;
    storelist l = cfg->store()->GetGroups();
    lua_newtable(L);
    int i=1;
    for (storelist::iterator it = l.begin(); it!=l.end(); ++it, ++i) {
      const char* name = (*it).c_str();
      lua_pushstring(L, name);
      lua_rawseti(L, -2, i);
    }
    return 1;
  }

  int LuaConfigBind::keys(lua_State* L) {
    LuaConfig* cfg = LuaHelper::check<LuaConfig>(L, 1, meta);
    typedef std::list<std::string> storelist;
    storelist l = cfg->store()->GetKeys();
    lua_newtable(L);
    int i=1;
    for (storelist::iterator it = l.begin(); it!=l.end(); ++it, ++i) {
      const char* name = (*it).c_str();
      lua_pushstring(L, name);
      lua_rawseti(L, -2, i);
    }
    return 1;
  }

  int LuaConfigBind::plugindir(lua_State* L) {
    lua_pushstring(L, PsycleGlobal::configuration().GetAbsoluteLuaDir().c_str());
    return 1;
  }

  int LuaConfigBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<LuaConfig>(L, meta);
  }

  /////////////////////////////////////////////////////////////////////////////
  // LuaMachine+Bind
  /////////////////////////////////////////////////////////////////////////////

  LuaMachine::~LuaMachine() {
    if (mac_!=0 && !shared_) {
      delete mac_;
    }
  }

  void LuaMachine::lock() const {
    if (proxy_) {
      proxy_->lock();
    }
  }

  void LuaMachine::unlock() const {
    if (proxy_) {
      proxy_->unlock();
    }
  }

  void LuaMachine::load(const char* name) {
    PluginCatcher* plug_catcher =
      static_cast<PluginCatcher*>(&Global::machineload());
    PluginInfo* info = plug_catcher->info(name);
    if (info) {
      Song& song =  Global::song();
      mac_ = song.CreateMachine(info->type, info->dllname.c_str(), 1024, 0);
      mac_->Init();
      build_buffer(mac_->samplesV, 256);
      shared_ = false;
      num_cols_ = mac_->GetNumCols();
    } else {
      mac_ = 0;
      throw std::runtime_error("plugin not found error");
    }
  }

  void LuaMachine::work(int samples) {
    update_num_samples(samples);
    mac_->GenerateAudio(samples, false);
  }

  void LuaMachine::build_buffer(std::vector<float*>& buf, int num) {
    sampleV_.clear();
    std::vector<float*>::iterator it = buf.begin();
    for ( ; it != buf.end(); ++it) {
      sampleV_.push_back(PSArray(*it, num));
    }
  }

  void LuaMachine::set_buffer(std::vector<float*>& buf) {
    mac_->change_buffer(buf);
    build_buffer(mac_->samplesV, 256);
  }

  void LuaMachine::update_num_samples(int num) {
    psybuffer::iterator it = sampleV_.begin();
    for ( ; it != sampleV_.end(); ++it) {
      (*it).set_len(num);
    }
  }

  void LuaMachine::offset(int offset) {
    psybuffer::iterator it = sampleV_.begin();
    for ( ; it != sampleV_.end(); ++it) {
      (*it).offset(offset);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // LuaMachineBind
  /////////////////////////////////////////////////////////////////////////////

  const char* LuaMachineBind::meta = "psypluginmeta";

  int LuaMachineBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"work", work},
      {"tick", tick},
      {"channel", channel},
      {"setnumchannels", set_numchannels},
      {"numchannels", numchannels},
      {"setnumprograms", set_numprograms},
      {"numprograms", numprograms},
      {"resize", resize},
      {"setbuffer", setbuffer},
      {"addparameters", add_parameters},
      {"setparameters", set_parameters},
      {"setmenus", set_menus},
      {"setnumcols", set_numcols},
      {"numcols", numcols},
      {"shownativegui", show_native_gui},
      {"showcustomgui", show_custom_gui},
      {"showchildviewgui", show_childview_gui},
      {"parambyid", getparam},
      {"setpresetmode", setpresetmode},
      {"addhostlistener", addhostlistener},
      {NULL, NULL}
    };
    LuaHelper::open(L, meta, methods,  gc);
    // define constants
    lua_pushnumber(L, 0);
    lua_setfield(L, -2, "FX");
    lua_pushnumber(L, 1);
    lua_setfield(L, -2, "GENERATOR");
    lua_pushnumber(L, 3);
    lua_setfield(L, -2, "HOSTUI");
    lua_pushnumber(L, 3);
    lua_setfield(L, -2, "PRSNATIVE");
    lua_pushnumber(L, 1);
    lua_setfield(L, -2, "PRSCHUNK");    
    return 1;
  }

  int LuaMachineBind::create(lua_State* L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n==1) {
      LuaHelper::new_userdata<LuaMachine>(L, meta, new LuaMachine());
      lua_newtable(L);
      lua_setfield(L, -2, "params");
      return 1;
    }
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 1 (pluginname)", n);
    }
    LuaMachine* udata = LuaHelper::new_userdata<LuaMachine>(L, meta,
      new LuaMachine());
    if (lua_isnumber(L, 2)) {
      int idx = luaL_checknumber(L, 2);
      if (idx < 0 || idx > MAX_VIRTUALINSTS ||
        Global::song()._pMachine[idx] == NULL) {
          luaL_error(L, "no machine at index %d", n);
      }
      Machine* mac = Global::song()._pMachine[idx];
      udata->set_mac(mac);
    } else {
      try {
        size_t len;
        const char* plug_name = luaL_checklstring(L, 2, &len);
        udata->load(plug_name);
      } catch (std::exception &e) {
        e; luaL_error(L, "plugin not found error");
      }
    }
    lua_createtable(L, udata->mac()->GetNumParams(), 0);
    for (int idx = 0; idx < udata->mac()->GetNumParams(); ++idx) {
      lua_getglobal(L, "require");
      lua_pushstring(L, "parameter");
      lua_pcall(L, 1, 1, 0);
      lua_getfield(L, -1, "new");
      lua_pushvalue(L, -2);
      lua_pushstring(L, "");
      lua_pushstring(L, "");
      lua_pushnumber(L, 0);
      lua_pushnumber(L, 1);
      lua_pushnumber(L, 10);
      lua_pushnumber(L, 1);
      lua_pushnumber(L, 2); // mpf state
      std::string id;
      udata->mac()->GetParamId(idx, id);
      lua_pushstring(L, id.c_str());
      lua_pcall(L, 9, 1, 0);
      lua_pushcclosure(L, setnorm, 0);
      lua_setfield(L, -2, "setnorm");
      lua_pushcclosure(L, name, 0);
      lua_setfield(L, -2, "name");
      lua_pushcclosure(L, display, 0);
      lua_setfield(L, -2, "display");
      lua_pushcclosure(L, getnorm, 0);
      lua_setfield(L, -2, "norm");
      lua_pushcclosure(L, getrange, 0);
      lua_setfield(L, -2, "range");
      lua_pushvalue(L,3);
      lua_setfield(L, -2, "plugin");
      lua_pushnumber(L, idx);
      lua_setfield(L, -2, "idx");
      lua_rawseti(L, 4, idx+1);
    }
    lua_pushvalue(L, 4);
    lua_setfield(L, 3, "params");
    lua_pushvalue(L, 3);
    return 1;
  }

   int LuaMachineBind::show_native_gui(lua_State* L) {
    LuaMachine* plug = LuaHelper::check<LuaMachine>(L, 1, meta);
    plug->set_ui_type(MachineUiType::NATIVE);
    return 0;
  }

  int LuaMachineBind::show_custom_gui(lua_State* L) {
    LuaMachine* plug = LuaHelper::check<LuaMachine>(L, 1, meta);
    plug->set_ui_type(MachineUiType::CUSTOMWND);
    return 0;
  }

  int LuaMachineBind::show_childview_gui(lua_State* L) {
    LuaMachine* plug = LuaHelper::check<LuaMachine>(L, 1, meta);
    plug->set_ui_type(MachineUiType::CHILDVIEW);
    return 0;
  }

  int LuaMachineBind::tick(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 7) {
      LuaMachine* plug = LuaHelper::check<LuaMachine>(L, 1, meta);
      int track = luaL_checknumber(L, 2);
      int note = luaL_checknumber(L, 3);
      int inst = luaL_checknumber(L, 4);
      int mach = luaL_checknumber(L, 5);
      int cmd = luaL_checknumber(L, 6);
      int param = luaL_checknumber(L, 7);
      PatternEntry data(note, inst, mach, cmd, param);
      plug->mac()->Tick(track, &data);
    } else {
      luaL_error(L, "Got %d arguments expected 7 (self)", n);
    }
    return 0;
  }

  int LuaMachineBind::setnorm(lua_State* L) {
    double newval = luaL_checknumber(L, 2);
    lua_pushvalue(L, 1);
    lua_getfield(L, -1, "idx");
    int idx = luaL_checknumber(L, -1);
    lua_pop(L,1);
    lua_getfield(L, -1, "plugin");
    lua_getfield(L, -1, "__self");
    LuaMachine** ud = (LuaMachine**) luaL_checkudata(L, -1, meta);
    int minval; int maxval;
    (*ud)->mac()->GetParamRange(idx, minval, maxval);
    int quantization = (maxval-minval);
    (*ud)->mac()->SetParameter(idx, newval*quantization);
    return 0;
  }

  int LuaMachineBind::name(lua_State* L) {
    lua_getfield(L, -1, "idx");
    int idx = luaL_checknumber(L, -1);
    lua_pop(L,1);
    lua_getfield(L, -1, "plugin");
    lua_getfield(L, -1, "__self");
    LuaMachine** ud = (LuaMachine**) luaL_checkudata(L, -1, meta);
    char buf[128];
    (*ud)->mac()->GetParamName(idx, buf);
    lua_pushstring(L, buf);
    return 1;
  }

  int LuaMachineBind::display(lua_State* L) {
    lua_getfield(L, -1, "idx");
    int idx = luaL_checknumber(L, -1);
    lua_pop(L,1);
    lua_getfield(L, -1, "plugin");
    lua_getfield(L, -1, "__self");
    LuaMachine** ud = (LuaMachine**) luaL_checkudata(L, -1, meta);
    char buf[128];
    (*ud)->mac()->GetParamValue(idx, buf);
    lua_pushstring(L, buf);
    return 1;
  }

  int LuaMachineBind::getnorm(lua_State* L) {
    lua_getfield(L, -1, "idx");
    int idx = luaL_checknumber(L, -1);
    lua_pop(L,1);
    lua_getfield(L, -1, "plugin");
    lua_getfield(L, -1, "__self");
    LuaMachine** ud = (LuaMachine**) luaL_checkudata(L, -1, meta);
    int minval; int maxval;
    (*ud)->mac()->GetParamRange(idx, minval, maxval);
    int quantization = (maxval-minval);
    double val = (*ud)->mac()->GetParamValue(idx)-minval;
    val = val  / quantization;
    lua_pushnumber(L, val);
    return 1;
  }

  int LuaMachineBind::getrange(lua_State* L) {
    lua_getfield(L, -1, "idx");
    int idx = luaL_checknumber(L, -1);
    lua_pop(L,1);
    lua_getfield(L, -1, "plugin");
    lua_getfield(L, -1, "__self");
    LuaMachine** ud = (LuaMachine**) luaL_checkudata(L, -1, meta);
    int minval; int maxval;
    (*ud)->mac()->GetParamRange(idx, minval, maxval);
    lua_pushnumber(L, 0);
    lua_pushnumber(L, 1);
    int steps = maxval - minval;
    lua_pushnumber(L, steps);
    return 3;
  }

  int LuaMachineBind::work(lua_State* L) {
    return LuaHelper::callstrict1<LuaMachine, int>(L, meta, &LuaMachine::work);
  }

  int LuaMachineBind::resize(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
      LuaMachine* plug = LuaHelper::check<LuaMachine>(L, 1, meta);
      int size = luaL_checknumber(L, 2);
      plug->update_num_samples(size);
    }  else {
      luaL_error(L, "Got %d arguments expected 2 (self, size)", n);
    }
    return 0;
  }

  int LuaMachineBind::channel(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
      LuaMachine* plug = LuaHelper::check<LuaMachine>(L, 1, meta);
      int idx = luaL_checknumber(L, 2);
      PSArray ** udata = (PSArray **)lua_newuserdata(L, sizeof(PSArray *));
      PSArray* a = plug->channel(idx);
      *udata = new PSArray(a->data(), a->len());
      luaL_setmetatable(L, LuaArrayBind::meta);
    }  else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    return 1;
  }

  int LuaMachineBind::getparam(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
      LuaHelper::check<LuaMachine>(L, 1, meta);
      std::string search = luaL_checkstring(L, 2);
      lua_getfield(L, -2, "params");
      size_t len = lua_rawlen(L, -1);
      for (size_t i = 1; i <= len; ++i) {
        lua_rawgeti(L, 3, i);
        lua_getfield(L, -1, "id");
        lua_pushvalue(L, -2);
        lua_pcall(L, 1, 1, 0);
        std::string id(luaL_checkstring(L, -1));
        if (id==search) {
           lua_pop(L, 1);
           return 1;
        }
        lua_pop(L, 1);
      }
      lua_pushnil(L);
      return 1;
    }  else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    return 0;
  }

  int LuaMachineBind::set_numchannels(lua_State* L) {
    LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, meta);
    int num = luaL_checknumber(L, 2);
    plugin->mac()->InitializeSamplesVector(num);
    plugin->build_buffer(plugin->mac()->samplesV, 256);
    return 0;
  }

  int LuaMachineBind::numcols(lua_State* L) {
    return LuaHelper::getnumber<LuaMachine, int>(L, meta, &LuaMachine::numcols);
  }

  int LuaMachineBind::set_numcols(lua_State* L) {
    LuaHelper::callstrict1<LuaMachine, int>(L, meta, &LuaMachine::set_numcols);
    return LuaHelper::chaining(L);
  }

  int LuaMachineBind::numchannels(lua_State* L) {
    return LuaHelper::getnumber<LuaMachine, int>(L, meta, &LuaMachine::numchannels);
  }

  int LuaMachineBind::set_numprograms(lua_State* L) {
    LuaHelper::callstrict1<LuaMachine, int>(L, meta, &LuaMachine::set_numprograms);
    return LuaHelper::chaining(L);
  }

  int LuaMachineBind::numprograms(lua_State* L) {
    return LuaHelper::getnumber<LuaMachine, int>(L, meta, &LuaMachine::numprograms);
  }

  int LuaMachineBind::gc(lua_State* L) {
    return LuaHelper::delete_userdata<LuaMachine>(L, meta);
  }

  int LuaMachineBind::setbuffer(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
      LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, meta);
      luaL_checktype(L, 2, LUA_TTABLE);
      lua_pushvalue(L, 2);
      std::vector<float*> sampleV;
      for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
        PSArray* v = *(PSArray **)luaL_checkudata(L, -1, LuaArrayBind::meta);
        sampleV.push_back(v->data());
      }
      plugin->set_buffer(sampleV);
    } else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    return 0;
  }

  int LuaMachineBind::add_parameters(lua_State* L) {
    LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, meta);
    lua_getfield(L, 1, "params");
    lua_pushvalue(L, 2);
    luaL_checktype(L, 2, LUA_TTABLE);
    // t:opairs()
    lua_getfield(L, 2, "opairs");
    lua_pushvalue(L, -2);
    lua_call(L, 1, 2);
    size_t len;
    // iter, self (t), nil
    for(lua_pushnil(L); LuaHelper::luaL_orderednext(L);)
    {
      luaL_checklstring(L, -2, &len);
      lua_pushvalue(L, -2);
      lua_setfield(L, -2, "id_");
      lua_rawseti(L, 3, lua_rawlen(L, 3)+1); // params[#params+1] = newparam
    }
    plugin->set_numparams(lua_rawlen(L, 3));
    return 0;
  }

  int LuaMachineBind::set_menus(lua_State* L) {
    lua_setfield(L, 1, "__menus");
    return 0;
  }

  int LuaMachineBind::set_parameters(lua_State* L) {
    LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, meta);
    plugin->set_numparams(lua_rawlen(L, 2));
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_setfield(L, -2, "params");
    return 0;
  }

  int LuaMachineBind::setpresetmode(lua_State* L) {
    LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, meta);
    MachinePresetType::Value mode = (MachinePresetType::Value) ((int) luaL_checknumber(L, 2));
    plugin->setprsmode(mode);
    return 0;
  }

  int LuaMachineBind::addhostlistener(lua_State* L) {
    LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, meta);
    LuaActionListener* listener = LuaHelper::check<LuaActionListener>(L, 2, LuaActionListenerBind::meta);
    listener->setmac(plugin);
    PsycleGlobal::actionHandler().AddListener(listener);
    return 0;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // PlayerBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaPlayerBind::meta = "psyplayermeta";

  int LuaPlayerBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"samplerate", samplerate},
      {"tpb", tpb},
      {"spt", spt},
      {"rline", rline},
      {"line", line},
      {"playing", playing},
      {NULL, NULL}
    };
    return LuaHelper::open(L, meta, methods);
  }

  int LuaPlayerBind::create(lua_State* L) {
    LuaHelper::new_userdata<Player>(L, meta, &Global::player());
    return 1;
  }

  int LuaPlayerBind::samplerate(lua_State* L) {
    return LuaHelper::getnumber<Player, int>(L, meta, &Player::SampleRate);
  }

  int LuaPlayerBind::tpb(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    Player* p = LuaHelper::check<Player>(L, 1, meta);
    lua_pushnumber(L, p->lpb);
    return 1;
  }

  int LuaPlayerBind::rline(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    Player* p = LuaHelper::check<Player>(L, 1, meta);
    double row = (p->SamplesPerRow()-p->_samplesRemaining)/(double)p->SamplesPerRow() + p->_lineCounter;
    lua_pushnumber(L, row);
    return 1;
  }

   int LuaPlayerBind::spt(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    Player* p = LuaHelper::check<Player>(L, 1, meta);
    lua_pushnumber(L, p->SamplesPerTick());
    return 1;
  }

  int LuaPlayerBind::line(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    Player* p = LuaHelper::check<Player>(L, 1, meta);
    lua_pushnumber(L, p->_lineCounter);
    return 1;
  }

  int LuaPlayerBind::playing(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    Player* p = LuaHelper::check<Player>(L, 1, meta);
    lua_pushboolean(L, p->_playing);
    return 1;
  }

  const char* LuaSequenceBarBind::meta = "psysequencebarmeta";

  int LuaSequenceBarBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"currpattern", currpattern},
      {NULL, NULL}
    };
    return LuaHelper::open(L, meta, methods);
  }

  int LuaSequenceBarBind::create(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    CMainFrame* main = (CMainFrame*) AfxGetMainWnd();
    CChildView* view = &main->m_wndView;
    LuaHelper::new_userdata<CChildView>(L, meta, view);
    return 1;
  }

  int LuaSequenceBarBind::currpattern(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    CChildView* view = LuaHelper::check<CChildView>(L, 1, meta);
    int pos = Global::song().playOrder[view->editPosition];
    lua_pushnumber(L, pos);
    return 1;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaPatternData+Bind
  ///////////////////////////////////////////////////////////////////////////////

   int LuaPatternData::numlines(int ps) const {
     return Global::song().patternLines[ps];
   }

   int LuaPatternData::numtracks() const {
     return song_->SONGTRACKS;
   }

  const char* LuaPatternDataBind::meta = "psypatterndatameta";

  int LuaPatternDataBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"setevent", insertevent},
      {"eventat", eventat},
      {"track", track},      
      {"pattern", create},      
      {"numtracks", numtracks},      
      {"numlines", numlines},
      {NULL, NULL}
    };
    return LuaHelper::open(L, meta, methods);
  }

  int LuaPatternDataBind::createevent(lua_State* L, LuaPatternEvent& ev) {
    lua_createtable(L, 0, 3);
    lua_pushnumber(L, ev.pos);
    lua_setfield(L, -2, "pos");
    lua_pushnumber(L, ev.val);
    lua_setfield(L, -2, "val");
    lua_pushnumber(L, ev.len);
    lua_setfield(L, -2, "len");
    return 1;
  }

  int LuaPatternDataBind::create(lua_State* L) {
    int err = LuaHelper::check_argnum(L, 1, "self");
    if (err!=0) return err;
    LuaPatternData* pattern = new LuaPatternData(L, Global::song().ppPatternData, &Global::song());
    LuaHelper::new_userdata<LuaPatternData>(L, meta, pattern);
    return 1;
  }

  int LuaPatternDataBind::numtracks(lua_State* L) {
    return LuaHelper::getnumber<LuaPatternData>(L, meta, &LuaPatternData::numtracks);
  }

  int LuaPatternDataBind::numlines(lua_State* L) {
    return LuaHelper::getnumber1<LuaPatternData, int, int>(L, meta, &LuaPatternData::numlines);
  }

  int LuaPatternDataBind::eventat(lua_State* L) {
    LuaPatternData* pattern = LuaHelper::check<LuaPatternData>(L, 1, meta);
    int ps = luaL_checknumber(L, 2);
    int trk = luaL_checknumber(L, 3);
    int pos = luaL_checknumber(L, 4);
    unsigned char* e = pattern->ptrackline(ps, trk, pos);
    LuaPatternEvent ev(static_cast<int>(*e), 1, pos);
    return createevent(L, ev);
  }

  int LuaPatternDataBind::pattern(lua_State* L) {
    // LuaPatternData* pattern = LuaHelper::check<LuaPatternData>(L, 1, meta);
    lua_newtable(L);
    //const int len = pattern->numtracks();
    /*for (int i=1; i <= len; ++i) {
      lua_pushvalue(L, 1);
      lua_pushnumber
      track(L);
      lua_rawseti(L, i
    }*/
    return 1;
  }

  int LuaPatternDataBind::track(lua_State* L) {
    LuaPatternData* pattern = LuaHelper::check<LuaPatternData>(L, 1, meta);
    int ps = luaL_checknumber(L, 2);
    int trk = luaL_checknumber(L, 3);
    PatternEntry entry;    
    std::vector<LuaPatternEvent> events;
    LuaPatternEvent* last = 0;
    int lastpos = 0;
    int pos = 0;
    for (size_t i = 0; i < pattern->numlines(trk); ++i, ++pos) {
      unsigned char* e = pattern->ptrackline(ps, trk, i);
      int note = static_cast<int>(*e);
      if (note == notecommands::empty) continue;
      if (last) {
         int len = i - lastpos;
         last->len = len;
      }
      lastpos = i;
      if (note == notecommands::release) {
        last = 0;
      } else {
        LuaPatternEvent ev(note, 1, pos);
        events.push_back(ev);
        last = &events.back();
      }
    }
    if (events.size()!=0 && events.back().val!=notecommands::release) {
      events.back().len = pattern->numlines(ps) - events.back().pos;
    }
    lua_createtable(L, events.size(), 0);
    int i=1;
    std::vector<LuaPatternEvent>::iterator it = events.begin();
    for (; it != events.end(); ++it, ++i) {
      LuaPatternEvent& ev = *it;
      createevent(L, ev);
      lua_rawseti(L, -2, i);
    }
    return 1;
  }

  int LuaPatternDataBind::insertevent(lua_State* L) {
    LuaPatternData* pattern = LuaHelper::check<LuaPatternData>(L, 1, meta);
    int ps = luaL_checknumber(L, 2);
    int trk = luaL_checknumber(L, 3);
    int pos = luaL_checknumber(L, 4);
    int note = luaL_checknumber(L, 5);
    unsigned char* e = pattern->ptrackline(ps, trk, pos);
    *e = note;
    return 0;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // PlotterBind
  ///////////////////////////////////////////////////////////////////////////////
#if !defined WINAMP_PLUGIN
  int LuaPlotterBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"stem", stem },
      { NULL, NULL }
    };
    return LuaHelper::open(L, "psyplottermeta", methods,  gc);
  }

  int LuaPlotterBind::create(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n);
    }
    LuaHelper::new_userdata<CPlotterDlg>(L, "psyplottermeta",
      new CPlotterDlg(AfxGetMainWnd()));
    return 1;
  }

  int LuaPlotterBind::gc(lua_State* L) {
    CPlotterDlg* ud = *(CPlotterDlg**) luaL_checkudata(L, 1, "psyplottermeta");
    // todo this is all a mess. Dunno how to multithread mfc...
    // Maybe write a pipe in the winmain thread ...
    AfxGetMainWnd()->SendMessage(0x0501, (WPARAM)ud, 0);
    /*if (ud) {
    // ud->DestroyWindow();
    }*/
    return 0;
  }

  int LuaPlotterBind::stem(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
      CPlotterDlg* plotter = LuaHelper::check<CPlotterDlg>(L, 1,
        "psyplottermeta");
      plotter->ShowWindow(SW_SHOW);
      PSArray* x = *(PSArray **)luaL_checkudata(L, -1, LuaArrayBind::meta);
      plotter->set_data(x->data(), x->len());
      plotter->UpdateWindow();
    } else {
      luaL_error(L, "Got %d arguments expected 2 (self, array)", n);
    }
    return 0;
  }
#endif // !defined WINAMP_PLUGIN
  ///////////////////////////////////////////////////////////////////////////////
  // Delay
  ///////////////////////////////////////////////////////////////////////////////
  void PSDelay::work(PSArray& x, PSArray& y) {
    int n = x.len();
    int k = mem.len();
    if (n>k) {
      y.copyfrom(x, k);
      y.copyfrom(mem, 0);
      int o = n-k;
      x.offset(o);
      mem.copyfrom(x, 0);
      x.offset(-o);
    } else
    if (n==k) {
      y.copyfrom(mem);
      mem.copyfrom(x);
    } else {
      y.copyfrom(mem, 0);
      PSArray tmp(k, 0);
      mem.offset(n);
      tmp.copyfrom(mem, 0);
      mem.offset(-n);
      tmp.copyfrom(x, k-n);
      mem.copyfrom(tmp);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  // DelayBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaDelayBind::meta = "psydelaymeta";

  int  LuaDelayBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"work", work},
      {"tostring", tostring},
      {NULL, NULL}
    };
    return LuaHelper::open(L, meta, methods, gc, tostring);
  }

  int LuaDelayBind::create(lua_State *L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, k)", n);
    }
    int k = luaL_checknumber (L, 2);
    luaL_argcheck(L, k >= 0, 2, "negative index not allowed");
    LuaHelper::new_userdata<PSDelay>(L, meta, new PSDelay(k));
    return 1;
  }

  int LuaDelayBind::work(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
      PSDelay* delay = LuaHelper::check<PSDelay>(L, 1, meta);
      PSArray* x = *(PSArray **)luaL_checkudata(L, 2, LuaArrayBind::meta);
      PSArray** y = (PSArray **)lua_newuserdata(L, sizeof(PSArray *));
      *y = new PSArray(x->len(), 0);
      luaL_setmetatable(L, LuaArrayBind::meta);
      delay->work(*x, **y);
    }  else {
      luaL_error(L, "Got %d arguments expected 2 (self, arrayinput)", n);
    }
    return 1;
  }

  int LuaDelayBind::gc (lua_State *L) {
    PSDelay* ptr = *(PSDelay **)luaL_checkudata(L, 1, meta);
	  delete ptr;
    return 0;
  }

  int LuaDelayBind::tostring(lua_State *L) {
    LuaHelper::check_tostring<PSDelay>(L, meta);
    std::stringstream r;
    r << "delay";
    lua_pushfstring(L, r.str().c_str());
    return 1;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // WaveOscTables (Singleton/Lazy Creation)
  ///////////////////////////////////////////////////////////////////////////////

  WaveOscTables::WaveOscTables() {
    srand((unsigned)time(NULL));
    set_samplerate(Global::player().SampleRate()); // Lazy Creation
  }

  void WaveOscTables::saw(float* data, int num, int maxharmonic)  {
    double gain = 0.5 / 0.777;
    for  (int h = 1; h <= maxharmonic; ++h) {
      double amplitude = gain / h;
      double to_angle = 2* psycle::helpers::math::pi / num * h;
      for (int i = 0; i < num; ++i) {
        data[i] += ::sin(pow(1.0,h+1)*i*to_angle)*amplitude;
      }
    }
  }

  void WaveOscTables::sqr(float* data, int num, int maxharmonic)  {
    double gain = 0.5 / 0.777;
    for  (int h = 1; h <= maxharmonic; h=h+2) {
      double amplitude = gain / h;
      double to_angle = 2* psycle::helpers::math::pi / num * h;
      for (int i = 0; i < num; ++i) {
        data[i] += ::sin(i*to_angle)*amplitude;
      }
    }
  }

  void WaveOscTables::tri(float* data, int num, int maxharmonic)  {
    // double gain = 0.5 / 0.777;
    for (int h = 1; h <= maxharmonic; h=h+2) {
      double to_angle = 2*psycle::helpers::math::pi/num*h;
      for (int i = 0; i < num; ++i) {
        data[i] += pow(-1.0,(h-1)/2.0)/(h*h)*::sin(i*to_angle);
      }
    }
  }

  void WaveOscTables::sin(float* data, int num, int maxharmonic)  {
    double to_angle = 2*psycle::helpers::math::pi/num;
    for (int i = 0; i < num; ++i) {
      data[i] = ::sin(i*to_angle);
    }
  }

  void WaveOscTables::ConstructWave(double fh,
    XMInstrument::WaveData<float>& wave,
    void (*func)(float*, int, int),
    int sr) {
      double f = 261.6255653005986346778499935233; // C4
      int num = (int) (sr/f + 0.5);
      int hmax = (int) (sr/2/fh);
      wave.AllocWaveData(num, false);
      wave.WaveSampleRate(sr);
      wave.WaveLoopStart(0);
      wave.WaveLoopEnd(num);
      wave.WaveLoopType(XMInstrument::WaveData<float>::LoopType::NORMAL);
      //Warning!! do not use dsp::Clear. Memory is not aligned in wavedata.
      for (int i = 0; i < num; ++i) wave.pWaveDataL()[i]=0;
      func(wave.pWaveDataL(), num, hmax);
  }

  void WaveOscTables::set_samplerate(int sr) {
    if (sin_tbl.size() != 0) {
      cleartbl(sin_tbl);
      cleartbl(tri_tbl);
      cleartbl(sqr_tbl);
      cleartbl(saw_tbl);
      cleartbl(rnd_tbl);
    }
    double f_lo = 440*std::pow(2.0, (0-notecommands::middleA)/12.0);
    for (int i = 0; i < 10; ++i) {
      double f_hi = 2 * f_lo;
      if (i==0) {
        f_lo = 0;
      }
      XMInstrument::WaveData<float>* w;
      w =  new XMInstrument::WaveData<float>();
      ConstructWave(f_hi, *w, &sqr, sr);
      sqr_tbl[range<double>(f_lo, f_hi)] = w;
      w =  new XMInstrument::WaveData<float>();
      ConstructWave(f_hi, *w, &saw, sr);
      saw_tbl[range<double>(f_lo, f_hi)] = w;
      w =  new XMInstrument::WaveData<float>();
      ConstructWave(f_hi, *w, &sin, sr);
      sin_tbl[range<double>(f_lo, f_hi)] = w;
      w =  new XMInstrument::WaveData<float>();
      ConstructWave(f_hi, *w, &tri, sr);
      tri_tbl[range<double>(f_lo, f_hi)] = w;
      f_lo = f_hi;
    }
  }

  void WaveOscTables::cleartbl(WaveList<float>::Type& tbl) {
    WaveList<float>::Type::iterator it;
    for (it = tbl.begin(); it != tbl.end(); ++it) {
      delete it->second;
    }
    tbl.clear();
  }

  ///////////////////////////////////////////////////////////////////////////////
  // WaveOsc (delegator to resampler)
  ///////////////////////////////////////////////////////////////////////////////

  WaveOsc::WaveOsc(WaveOscTables::Shape shape) : shape_(shape) {
    if (shape != WaveOscTables::PWM) {
      WaveList<float>::Type tbl = WaveOscTables::getInstance()->tbl(shape);
      resampler_.reset(new ResamplerWrap<float, 1>(tbl));
      resampler_->set_frequency(263);
    } else {
      // pwm subtract two saw's
      resampler_.reset(new PWMWrap<float, 1>());
      resampler_->set_frequency(263);
    }
  }

  void WaveOsc::set_shape(WaveOscTables::Shape shape) {
    if (shape_ != shape) {
      if (shape != WaveOscTables::PWM) {
        WaveList<float>::Type tbl = WaveOscTables::getInstance()->tbl(shape);
        if (shape_ == WaveOscTables::PWM) {
          ResamplerWrap<float, 1>* rnew = new ResamplerWrap<float, 1>(tbl);
          rnew->set_gain(resampler_->gain());
          rnew->set_frequency(resampler_->frequency());
          rnew->setphase(resampler_->phase());
          if (resampler_->Playing()) {
            rnew->Start(resampler_->phase());
          }
          resampler_.reset(rnew);          
        } else {
          resampler_->SetData(tbl);
        }
      } else {
        PWMWrap<float, 1>* rnew = new PWMWrap<float,1>();
        rnew->set_gain(resampler_->gain());
        rnew->set_frequency(resampler_->frequency());
        if (resampler_->Playing()) {
          rnew->Start(resampler_->phase());
        }
        resampler_.reset(rnew);
      }
      shape_ = shape;
    }
  }

  void WaveOsc::work(int num, float* data, SingleWorkInterface* master) {
      if (shape() != WaveOscTables::RND) {
        resampler_->work(num, data, 0, master);
      } else {
        // todo fm, am, pw
        for (int i = 0; i < num; ++i) {
          float r = ((float) rand() / (RAND_MAX))*2-1;
          *data++ = r;
        }
      }
  }

  ///////////////////////////////////////////////////////////////////////////////
  // WaveOscBind
  ///////////////////////////////////////////////////////////////////////////////
  const char* LuaWaveOscBind::meta = "psyoscmeta";

  std::map<WaveOsc*, WaveOsc*> LuaWaveOscBind::oscs; // osc map for samplerates
  // changes

  int LuaWaveOscBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"frequency", get_base_frequency},
      {"setfrequency", set_base_frequency},
      {"setgain", set_gain},
      {"gain", gain},
      {"setpw", setpw},
      {"pw", pw},
      {"work", work},
      {"stop", stop},
      {"start", start},
      {"tostring", tostring},
      {"isplaying", isplaying},
      {"setshape", set_shape},
      {"shape", shape},
      {"setquality", set_quality},
      {"quality", set_quality},
      {"setsync", set_sync},
      {"setsyncfadeout", set_sync_fadeout},
      {"phase", phase},
      {"setphase", setphase},
      {"setpm", setpm},
      {"setam", setam},
      {"setfm", setfm},
      {"setpwm", setpwm},
      { NULL, NULL}
    };
    LuaHelper::open(L, meta, methods, gc, tostring);
    static const char* const e[] = {"SIN", "SAW", "SQR", "TRI", "PWM", "RND"};
    LuaHelper::buildenum(L, e, sizeof(e)/sizeof(e[0]), 1);    
    static const char* const f[] = 
       {"ZEROHOLD", "LINEAR", "SPLINE", "SINC", "SOXR"};
    LuaHelper::buildenum(L, f, sizeof(f)/sizeof(f[0]), 1);
    return 1;
  }

  int LuaWaveOscBind::create(lua_State *L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2 and n!=3) {
      return luaL_error(L,
        "Got %d arguments expected 2[,3] (self, shape [, f])",
        n);
    }
    int type = luaL_checknumber (L, 2);
    // luaL_argcheck(L, f >= 0, 2, "negative frequency is not allowed");
    WaveOsc* osc = LuaHelper::new_userdata<WaveOsc>(L, meta,
      new WaveOsc((WaveOscTables::Shape)type));
    oscs[osc] = osc;
    if (n==3) {
      double f = luaL_checknumber (L, 3);
      osc->set_frequency(f);
    }
    return 1;
  }

  int LuaWaveOscBind::phase(lua_State* L) {
    return LuaHelper::getnumber<WaveOsc, double>(L, meta, &WaveOsc::phase);
  }

  int LuaWaveOscBind::setphase(lua_State* L) {
    return LuaHelper::callstrict1<WaveOsc, double>(L, meta, &WaveOsc::setphase);
  }

  void LuaWaveOscBind::setsamplerate(double sr) {
    std::map<WaveOsc*, WaveOsc*>::iterator it = oscs.begin();
    for ( ; it != oscs.end(); ++it) {
      it->second->set_shape(it->second->shape()); // reset wave data
    }
  }

  int LuaWaveOscBind::set_gain(lua_State* L) {
    return LuaHelper::callstrict1<WaveOsc, float>(L, meta, &WaveOsc::set_gain);
  }

  int LuaWaveOscBind::gain(lua_State* L) {
    return LuaHelper::getnumber<WaveOsc, float>(L, meta, &WaveOsc::gain);
  }

  int LuaWaveOscBind::setpw(lua_State* L) {
    return LuaHelper::callstrict1<WaveOsc, float>(L, meta, &WaveOsc::setpw);
  }

  int LuaWaveOscBind::pw(lua_State* L) {
    return LuaHelper::getnumber<WaveOsc, float>(L, meta, &WaveOsc::pw);
  }

  int LuaWaveOscBind::set_shape(lua_State* L) {
    return LuaHelper::callstrict1<WaveOsc, double>(L, meta, &WaveOsc::set_shape);
  }

  int LuaWaveOscBind::shape(lua_State* L) {
    return LuaHelper::getnumber<WaveOsc, WaveOscTables::Shape>(L, meta, &WaveOsc::shape);
  }

  int LuaWaveOscBind::stop(lua_State* L) {
    return LuaHelper::callopt1<WaveOsc, double>(L, meta, &WaveOsc::Stop, 0);
  }

  int LuaWaveOscBind::start(lua_State* L) {
    return LuaHelper::callopt1<WaveOsc, double>(L, meta, &WaveOsc::Start, 0);
  }

  int LuaWaveOscBind::isplaying(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n);
    }
    WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, meta);
    lua_pushboolean(L, osc->IsPlaying());
    return 1;
  }

  int LuaWaveOscBind::get_base_frequency(lua_State* L) {
    return LuaHelper::getnumber<WaveOsc, float>(L, meta, &WaveOsc::base_frequency);
  }

  int LuaWaveOscBind::set_base_frequency(lua_State* L) {
    return LuaHelper::callstrict1<WaveOsc, float>(L, meta, &WaveOsc::set_frequency);
  }

  int LuaWaveOscBind::setpm(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
      WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, meta);
      if (lua_isnil(L, 2)) {
        osc->setpm(0);
      } else {
        PSArray* v = *(PSArray **)luaL_checkudata(L, 2, LuaArrayBind::meta);
        osc->setpm(v->data());
      }
    } else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    return LuaHelper::chaining(L);
  }

  int LuaWaveOscBind::setfm(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
      WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, meta);
      if (lua_isnil(L, 2)) {
        osc->setfm(0);
      } else {
        PSArray* v = *(PSArray **)luaL_checkudata(L, 2, LuaArrayBind::meta);
        osc->setfm(v->data());
      }
    } else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    return LuaHelper::chaining(L);
  }

  int LuaWaveOscBind::setam(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
      WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, meta);
      if (lua_isnil(L, 2)) {
        osc->setam(0);
      } else {
        PSArray* v = *(PSArray **)luaL_checkudata(L, 2, LuaArrayBind::meta);
        osc->setam(v->data());
      }
    } else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    return LuaHelper::chaining(L);
  }

  int LuaWaveOscBind::setpwm(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
      WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, meta);
      if (lua_isnil(L, 2)) {
        osc->setam(0);
      } else {
        PSArray* v = *(PSArray **)luaL_checkudata(L, 2, LuaArrayBind::meta);
        osc->setpwm(v->data());
      }
    } else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    return LuaHelper::chaining(L);
  }

  int LuaWaveOscBind::work(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2 or n==3 or n==4 or n==5) {
      WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, meta);
      PSArray* data = *(PSArray **)luaL_checkudata(L, 2, LuaArrayBind::meta);
      float* fm = 0;
      float* env = 0;
      // float* pwm = 0;
      if (n>2 && (!lua_isnil(L,3))) {
        PSArray*arr = *(PSArray **)luaL_checkudata(L, 3, LuaArrayBind::meta);
        fm = arr->data();
      }
      if (n>3 && (!lua_isnil(L,4))) {
        PSArray* arr = *(PSArray **)luaL_checkudata(L, 4, LuaArrayBind::meta);
        env = arr->data();
      }
      // check for master
      std::auto_ptr<LuaSingleWorker> master;
      lua_getfield(L, 1, "sync");
      if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
      } else {
        lua_pushvalue(L, 1);
        master.reset(new LuaSingleWorker(L));
      }
      osc->work(data->len(), data->data(), master.get());
      lua_pushnumber(L,  data->len()); // return processed samples      
    }  else {
      luaL_error(L, "Got %d arguments expected 2 or 3 (self, num, fm)", n);
    }
    return 1;
  }

  int LuaWaveOscBind::gc(lua_State *L) {
    WaveOsc* ptr = *(WaveOsc **)luaL_checkudata(L, 1, meta);
    std::map<WaveOsc*, WaveOsc*>::iterator it = oscs.find(ptr);
    assert(it != oscs.end());
    oscs.erase(it);
    delete ptr;
    return 0;
  }

  int LuaWaveOscBind::tostring(lua_State *L) {
    WaveOsc* ptr = LuaHelper::check_tostring<WaveOsc>(L, meta);
    std::stringstream r;
    r << "osc " << "f=" << ptr->base_frequency();
    lua_pushfstring(L, r.str().c_str());
    return 1;
  }

  int LuaWaveOscBind::set_quality(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, frequency)", n);
    }
    WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, meta);
    int quality = luaL_checknumber(L, 2)-1;
    osc->set_quality((psycle::helpers::dsp::resampler::quality::type)quality);
    return 0;
  }

  int LuaWaveOscBind::quality(lua_State* L) {
    return LuaHelper::getnumber<WaveOsc, psycle::helpers::dsp::resampler::quality::type>
      (L, meta, &WaveOsc::quality);
  }

  int LuaWaveOscBind::set_sync(lua_State* L) {
    LuaHelper::check<WaveOsc>(L, 1, meta);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_setfield(L, -2, "sync");
    lua_getglobal(L, "require");
    lua_pushstring(L, "psycle.array");
    lua_pcall(L, 1, 1, 0);
    lua_getfield(L, -1, "new");
    lua_pushnumber(L, 256);
    lua_pcall(L, 1, 1, 0);
    lua_setfield(L, 1, "syncarray");
    return 0;
  }

  int LuaWaveOscBind::set_sync_fadeout(lua_State* L) {
    WaveOsc* plugin = LuaHelper::check<WaveOsc>(L, 1, meta);
    int fadeout = luaL_checknumber (L, 2);
    plugin->resampler()->set_sync_fadeout_size(fadeout);
    return 0;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // DspMathHelperBind
  ///////////////////////////////////////////////////////////////////////////////
  int LuaDspMathHelper::open(lua_State *L) {
    static const luaL_Reg funcs[] = {
      {"notetofreq", notetofreq},
      {"freqtonote", freqtonote},
      {NULL, NULL}
    };
    luaL_newlib(L, funcs);
    return 1;
  }

  int LuaDspMathHelper::notetofreq(lua_State* L) {
    double note = luaL_checknumber(L, 1);
    int n = lua_gettop(L);
    int base = notecommands::middleA;
    if (n==2) {
      base = luaL_checknumber(L, 2);
    }
    lua_pushnumber(L, 440*std::pow(2.0, (note-base)/12.0));
    return 1;
  }

  int LuaDspMathHelper::freqtonote(lua_State* L) {
    double f = luaL_checknumber(L, 1);
    int n = lua_gettop(L);
    int base = notecommands::middleA;
    if (n==2) {
      base = luaL_checknumber(L, 2);
    }
    double note = 12*std::log10(f/440.0)/std::log10(2.0)+base;
    lua_pushnumber(L, note);
    return 1;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // MidiHelperBind
  ///////////////////////////////////////////////////////////////////////////////
  int LuaMidiHelper::open(lua_State *L) {
    static const luaL_Reg funcs[] = {
      {"notename", notename},
      {"gmpercussionnames", gmpercussionnames},
      {"combine", combine},
      {NULL, NULL}
    };
    luaL_newlib(L, funcs);
    return 1;
  }

  int LuaMidiHelper::notename(lua_State* L) {
    const char* notenames[]=
        {"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-", "C-"};
    int note = luaL_checknumber(L, 1);    
    std::stringstream str;
    if (note >= 0) {
      str << notenames[note%12] << note/12;
    } else {      
      str << notenames[12 + note%12] << "(" << note/12-1 << ")";
    }    
    lua_pushstring(L, str.str().c_str());
    return 1;
  }

  int LuaMidiHelper::gmpercussionnames(lua_State* L) {
    const char* names[]=
      {"Acoustic Bass Drum","Bass Drum 1","Side Stick","Acoustic Snare",
       "Hand Clap","Electric Snare","Low Floor Tom","Closed Hi Hat",
       "High Floor Tom","Pedal Hi-Hat","Low Tom","Open Hi-Hat",
       "Low-Mid Tom", "Hi Mid Tom", "Crash Cymbal 1", "High Tom",
       "Ride Cymbal 1", "Chinese Cymbal", "Ride Bell", "Tambourine",
       "Splash Cymbal", "Cowbell", "Crash Cymbal 2", "Vibraslap",
       "Ride Cymbal 2", "Hi Bongo", "Low Bongo", "Mute Hi Conga",
       "Open Hi Conga", "Low Bongo", "Mute Hi Conga", "Open Hi Conga",
       "Low Conga", "High Timbale", "Low Timbale", "High Agogo", "Low Agogo",
       "Cabasa", "Maracas", "Short Whistle", "Long Whistle", "Short Guiro",
       "Long Guiro", "Claves", "Hi Wood Block", "Low Wood Block",
       "Mute Cuica", "Open Cuica", "Mute Triangle", "Open Triangle"
      };
    lua_createtable(L, 127, 0);
    for (int i = 1; i<128; ++i) {
      lua_pushstring(L, i>34 && i<82 ? names[i-35] : "");
      lua_rawseti(L, -2, i);
    }
    return 1;
  }

  int LuaMidiHelper::combine(lua_State* L) {
    std::uint8_t lsb = luaL_checknumber(L, 1);
    std::uint8_t msb = luaL_checknumber(L, 2);
    std::uint16_t val = (msb << 7) | lsb;
    lua_pushnumber(L, val);
    return 1;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // DspFilterBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaDspFilterBind::meta = "psydspfiltermeta";

  std::map<LuaDspFilterBind::Filter*, LuaDspFilterBind::Filter*>
    LuaDspFilterBind::filters;   // store map for samplerate change

  int LuaDspFilterBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"work", work},
      {"setcutoff", setcutoff},
      {"cutoff", getcutoff},
      {"setresonance", setresonance},
      {"resonance", getresonance},
      {"settype", setfiltertype},
      {"tostring", tostring},
      { NULL, NULL }
    };
    LuaHelper::open(L, meta, methods, gc, tostring);    
    static const char* const e[] = 
      {"LOWPASS", "HIGHPASS", "BANDPASS", "BANDREJECT", "NONE", "ITLOWPASS",
       "MPTLOWPASS", "MPTHIGHPASS", "LOWPASS12E", "HIGHPASS12E", "BANDPASS12E",
       "BANDREJECT12E"};        
    LuaHelper::buildenum(L, e, sizeof(e)/sizeof(e[0]));
    return 1;
  }

  int LuaDspFilterBind::create(lua_State *L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, f)", n);
    }
    int type = luaL_checknumber (L, 2);
    Filter* ud = new Filter();
    ud->Init(Global::player().SampleRate());
    ud->Type(static_cast<psycle::helpers::dsp::FilterType>(type));
    LuaHelper::new_userdata<Filter>(L, meta, ud);
    filters[ud] = ud;
    return 1;
  }

  void LuaDspFilterBind::setsamplerate(double sr) {
    std::map<Filter*, Filter*>::iterator it = filters.begin();
    for ( ; it != filters.end(); ++it) {
      it->second->SampleRate(sr);
    }
  }

  int LuaDspFilterBind::work(lua_State* L) {
    int n = lua_gettop(L);
    if (n ==2 or n==3 or n==4) {
      float* vcfc = 0;
      float* vcfr = 0;
      Filter* filter = LuaHelper::check<Filter>(L, 1,  meta);
      PSArray* x_input = *(PSArray **)luaL_checkudata(L, 2, LuaArrayBind::meta);
      if (n>2) {
        PSArray* arr = *(PSArray **)luaL_checkudata(L, 3, LuaArrayBind::meta);
        vcfc = arr->data();
      }
      if (n>3) {
        PSArray* arr = *(PSArray **)luaL_checkudata(L, 3, LuaArrayBind::meta);
        vcfr = arr->data();
      }
      int num = x_input->len();
      float* data = x_input->data();
      for (int i=0; i < num; ++i) {
        if (vcfc) {
          filter->Cutoff(std::min(std::max(0.0f, vcfc[i]), 127.0f));
        }
        if (vcfr) {
          filter->Ressonance(std::min(std::max(0.0f, vcfr[i]), 127.0f));
        }
        data[i] = filter->Work(data[i]);
      }
      lua_pushvalue(L, 2);
    }  else {
      luaL_error(L,
        "Got %d arguments expected 2 or 3(self, array filter input [, voltage control])", n);
    }
    return 1;
  }

  int LuaDspFilterBind::setcutoff(lua_State* L) {
    return LuaHelper::callstrict1<Filter, int>(L, meta, &Filter::Cutoff);
  }

  int LuaDspFilterBind::getcutoff(lua_State* L) {
    return LuaHelper::getnumber<Filter, int>(L, meta, &Filter::Cutoff);
  }

  int LuaDspFilterBind::setresonance(lua_State* L) {
    return LuaHelper::callstrict1<Filter, int>(L, meta, &Filter::Ressonance);
  }

  int LuaDspFilterBind::getresonance(lua_State* L) {
    return LuaHelper::getnumber<Filter, int>(L, meta, &Filter::Ressonance);
  }

  int LuaDspFilterBind::setfiltertype(lua_State* L) {
    int n = lua_gettop(L);
    if (n ==2) {
      Filter* filter = LuaHelper::check<Filter>(L, 1, meta);
      int filtertype = luaL_checknumber(L, -1);
      filter->Type(static_cast<psycle::helpers::dsp::FilterType>(filtertype));
    }  else {
      luaL_error(L, "Got %d arguments expected 2(self, filtertype)", n);
    }
    return 0;
  }

  int LuaDspFilterBind::gc (lua_State *L) {
    Filter* ptr = *(Filter **)luaL_checkudata(L, 1, meta);
    std::map<Filter*, Filter*>::iterator it = filters.find(ptr);
    assert(it != filters.end());
    filters.erase(it);
    delete ptr;
    return 0;
  }

  int LuaDspFilterBind::tostring(lua_State *L) {
    Filter* ptr = LuaHelper::check_tostring<Filter>(L, meta);
    std::stringstream r;
    r << "filter " << "c=" << ptr->Cutoff();
    lua_pushfstring(L, r.str().c_str());
    return 1;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // WaveDataBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaWaveDataBind::meta = "psywavedatameta";

  int LuaWaveDataBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"copy", copy},
      {"setwavesamplerate", set_wave_sample_rate},
      {"setwavetune", set_wave_tune},
      {"setwavefinetune", set_wave_fine_tune},
      {"setloop", set_loop},
      {"copytobank", set_bank},
      { NULL, NULL }
    };
    LuaHelper::open(L, meta, methods, gc);    
    static const char* const e[] = {"DO_NOT", "NORMAL", "BIDI"};      
    LuaHelper::buildenum(L, e, sizeof(e)/sizeof(e[0]));    
    return 1;
  }

  int LuaWaveDataBind::create(lua_State *L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 2 (self)", n);
    }
    LuaHelper::new_userdata<>(L, meta, new XMInstrument::WaveData<float>);
    return 1;
  }

  int LuaWaveDataBind::copy(lua_State *L) {
    int n = lua_gettop(L);
    if (n!=2 and n!=3) {
      return luaL_error(L,
        "Got %d arguments expected 2 (self, array, array)", n);
    }
    XMInstrument::WaveData<float>* wave =
      LuaHelper::check<XMInstrument::WaveData<float>>(L, 1, meta);
    PSArray* la = *(PSArray **)luaL_checkudata(L, 2, LuaArrayBind::meta);
    PSArray* ra = 0;
    bool is_stereo = (n==3 );
    if (is_stereo) {
      ra = *(PSArray **)luaL_checkudata(L, 3, LuaArrayBind::meta);
    }
    wave->AllocWaveData(la->len(), is_stereo);
    float* l = wave->pWaveDataL();
    float* r = wave->pWaveDataR();
    for (int i = 0; i < la->len(); ++i) {
      l[i] = la->get_val(i);
      if (is_stereo) {
        r[i] = ra->get_val(i);
      }
    }
    return 0;
  }

  int LuaWaveDataBind::set_bank(lua_State *L) {
    int n = lua_gettop(L);
    if (n!=2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    int index = luaL_checknumber(L, 2);
    XMInstrument::WaveData<float>* wave
      = LuaHelper::check<XMInstrument::WaveData<float>>(L, 1, meta);
    XMInstrument::WaveData<std::int16_t> wave16;
    int len = wave->WaveLength();
    wave16.AllocWaveData(wave->WaveLength(), wave->IsWaveStereo());
    wave16.WaveName("test");
    std::int16_t* ldest = wave16.pWaveDataL();
    std::int16_t* rdest = wave16.pWaveDataR();
    for (int i = 0; i < len; ++i) {
      ldest[i] = wave->pWaveDataL()[i]*32767;
      if (wave->IsWaveStereo()) {
        rdest[i] = wave->pWaveDataL()[i]*32767;
      }
    }
    wave16.WaveLoopStart(wave->WaveLoopStart());
    wave16.WaveLoopEnd(wave->WaveLoopEnd());
    wave16.WaveLoopType(wave->WaveLoopType());
    wave16.WaveTune(wave->WaveTune());
    wave16.WaveFineTune(wave->WaveFineTune());
    wave16.WaveSampleRate(wave->WaveSampleRate());
    wave16.WaveName("test");
    SampleList& list = Global::song().samples;
    list.SetSample(wave16, index);
    return 0;
  }

  int LuaWaveDataBind::set_wave_sample_rate(lua_State *L) {
    int n = lua_gettop(L);
    if (n!=2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, samplerate)", n);
    }
    XMInstrument::WaveData<float>* wave =
      LuaHelper::check<XMInstrument::WaveData<float> >(L, 1, meta);
    int rate = luaL_checknumber(L, 2);
    wave->WaveSampleRate(rate);
    return 0;
  }

  int LuaWaveDataBind::set_wave_tune(lua_State *L) {
    int n = lua_gettop(L);
    if (n!=2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, tune)", n);
    }
    XMInstrument::WaveData<float>* wave =
      LuaHelper::check<XMInstrument::WaveData<float> >(L, 1, meta);
    int tune = luaL_checknumber(L, 2);
    wave->WaveTune(tune);
    return 0;
  }

  int LuaWaveDataBind::set_wave_fine_tune(lua_State *L) {
    int n = lua_gettop(L);
    if (n!=2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, finetune)", n);
    }
    XMInstrument::WaveData<float>* wave =
      LuaHelper::check<XMInstrument::WaveData<float> >(L, 1, meta);
    int tune = luaL_checknumber(L, 2);
    wave->WaveFineTune(tune);
    return 0;
  }

  int LuaWaveDataBind::set_loop(lua_State *L) {
    int n = lua_gettop(L);
    if (n!=3 and n!=4) {
      return luaL_error(L,
        "Got %d arguments expected 3 (self, start, end [, looptype])", n);
    }
    XMInstrument::WaveData<float>* wave =
      LuaHelper::check<XMInstrument::WaveData<float> >(L, 1, meta);
    int loop_start = luaL_checknumber(L, 2);
    int loop_end = luaL_checknumber(L, 3);
    if (n==4) {
      int loop_type = luaL_checknumber(L, 4);
      wave->WaveLoopType(
        static_cast<const XMInstrument::WaveData<float>::LoopType::Type>(loop_type));
    } else {
      wave->WaveLoopType(XMInstrument::WaveData<>::LoopType::NORMAL);
    }
    wave->WaveLoopStart(loop_start);
    wave->WaveLoopEnd(loop_end);
    return 0;
  }

  int LuaWaveDataBind::gc (lua_State *L) {
    return LuaHelper::delete_userdata<XMInstrument::WaveData<float>>(L, meta);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // PWM Wrap
  ///////////////////////////////////////////////////////////////////////////////

  template <class T, int VOL>
  PWMWrap<T, VOL>::PWMWrap() :
  ResamplerWrap<T, VOL>(WaveOscTables::getInstance()->tbl(WaveOscTables::SAW)),
    pw_(0.5), pwm_(0) {
      resampler.quality(helpers::dsp::resampler::quality::linear);
      set_frequency(f_);
      wavectrl.Playing(false);
      wavectrl2.Playing(false);
      wave_it = waves_.find(range<double>(f_));
      last_wave = wave_it != waves_.end() ? wave_it->second : 0;
      wave_it2 = waves_.find(range<double>(f_));
      last_wave2 = wave_it2 != waves_.end() ? wave_it->second : 0;
  }

  template <class T, int VOL>
  void PWMWrap<T, VOL>::Start(double phase) {
    wave_it = waves_.find(range<double>(f_));
    last_wave = wave_it != waves_.end() ? wave_it->second : 0;
    if (wave_it != waves_.end()) {
      wavectrl.Init(last_wave, 0, resampler);
      wavectrl.Position2(phase*last_wave->WaveLength());
      double sr = Global::player().SampleRate();
      basef = 440 *std::pow(2.0,
        (notecommands::middleC-last_wave->WaveTune()-
        last_wave->WaveFineTune()/100.0-notecommands::middleA)/12.0)*
        sr/last_wave->WaveSampleRate();
      speed_ = f_/basef;
      wavectrl.Speed(resampler, speed_);
      wavectrl.Playing(true);
    }
    // phase2
    wave_it2 = waves_.find(range<double>(f_));
    last_wave2 = wave_it2 != waves_.end() ? wave_it->second : 0;
    if (wave_it2 != waves_.end()) {
      wavectrl2.Init(last_wave2, 0, resampler);
      double p2 = (phase+pw_);
      while (p2 >=1) {p2--;}
      while (p2<0) {p2++;}
      wavectrl2.Position2(p2*last_wave2->WaveLength());
      double sr = Global::player().SampleRate();
      basef = 440 *std::pow(2.0,
        (notecommands::middleC-last_wave2->WaveTune()-
        last_wave2->WaveFineTune()/100.0-notecommands::middleA)/12.0)*
        sr/last_wave2->WaveSampleRate();
      speed_ = f_/basef;
      wavectrl2.Speed(resampler, speed_);
      wavectrl2.Playing(true);
    }
    last_sgn = 1;
    oldtrigger = 0;
    dostop_ = false;
  }

  template <class T, int VOL>
  void PWMWrap<T, VOL>::set_frequency(double f) {
    f_ = f;
    speed_ = f_/basef;
    wavectrl.Speed(resampler, speed_);
    wavectrl2.Speed(resampler, speed_);
  }

  // hardsync work
  template <class T, int VOL>
  int PWMWrap<T, VOL>::work_trigger(int numSamples,
    float* pSamplesL,
    float* pSamplesR,
    float* fm,
    float* env,
    float* phase,
    SingleWorkInterface* master) {
      if (numSamples==0) return 0;
      int num = 0;
      if (oldtrigger) {
        for (int i=0; i < fsize; ++i) oldtrigger[i] = oldtrigger[n+i];
      }
      n = numSamples + ((oldtrigger) ? 0 : fsize);
      PSArray* arr = master->work(n, fsize);
      float* trigger = arr->data();
      float* samples = (oldtrigger) ? trigger : trigger+fsize;
      while (numSamples) {
        if (startfadeout) {
          int i = std::min(fsize-fadeout_sc, numSamples);
          if (i > 0) {
            num += work_samples(i, pSamplesL, pSamplesR, fm, env, phase);
            for (int k = 0; k < i; ++k) {
              float fader =  (fsize-fadeout_sc) / (float)fsize;
              if (pSamplesL) pSamplesL[0] *= fader;
              if (pSamplesR) pSamplesR[0] *= fader;
              numSamples--;
              samples++;
              if (pSamplesL) pSamplesL++;
              if (pSamplesR) pSamplesR++;
              if (fadeout_sc == (fsize-1)) {
                wavectrl.Position(0);
                wavectrl2.Position(pw_*last_wave2->WaveLength());
                startfadeout = false;
                fadeout_sc = 0;
                break;
              }
              ++fadeout_sc;
            }
          } else {
            wavectrl.Position(0);
            wavectrl2.Position(pw_*last_wave2->WaveLength());
            startfadeout = false;
            fadeout_sc = 0;
          }
        } else {
          int i = next_trigger(numSamples+fsize, samples)-fsize;
          if (i>=0) {
            num += work_samples(i, pSamplesL, pSamplesR, fm, env, phase);
            if (i != numSamples) {
              startfadeout = true;
              samples += i;
            }
            if (pSamplesL) pSamplesL += i;
            if (pSamplesR) pSamplesR += i;
            numSamples -= i;
          } else {
            int k = std::min(i+fsize, numSamples);
            num += work_samples(k, pSamplesL, pSamplesR, fm, env, phase);
            samples += i+fsize;
            if (pSamplesL) pSamplesL += k;
            if (pSamplesR) pSamplesR += k;
            numSamples -= k;
            if (numSamples < 0 ) {
              numSamples = 0;
            }
          }
        }
      }
      assert(numSamples==0);
      // adjust the master array to do slave/osc mix
      arr->margin(oldtrigger ? 0 : fsize, n);
      oldtrigger = trigger;
      return num;
  }

  template <class T, int VOL>
  int PWMWrap<T, VOL>::work_samples(int numSamples,
    float* pSamplesL,
    float* pSamplesR,
    float* fm,
    float* env,
    float* ph) {
      if (!wavectrl.Playing())
        return 0;
      int num = numSamples;
      float left_output = 0.0f;
      float right_output = 0.0f;
      float left_output2 = 0.0f;
      float right_output2 = 0.0f;
      double f = f_;
      while(numSamples) {
        XMSampler::WaveDataController<T>::WorkFunction pWork;
        int pos = wavectrl.Position();
        if (wave_it != waves_.end() && speed_ > 0) {
          int step = 1;
          if (ph==NULL) {
            step = numSamples;
          }
          for (int z = 0; z < num; z+=step) {
          if(ph!=0) {
              double phase = wavectrl.Position2() / last_wave->WaveLength();
              phase += (*ph++);
              while (phase >=1) {--phase;}
              while (phase<0) {++phase;}
              wavectrl.Position2(phase*last_wave->WaveLength());
              double p2 = (phase+pw_);
              while (p2 >=1) {p2--;}
              while (p2<0) {p2++;}
              wavectrl2.Position2(p2*last_wave2->WaveLength());
          }
           int nextsamples = std::min(wavectrl.PreWork(
              ph!=0 ? 1 : numSamples, &pWork, false),
              ph!=0 ? 1 : numSamples);
          numSamples-=nextsamples;
          while (nextsamples) {
            if (pwm_!=0) {
              double phase = wavectrl.Position2() / last_wave->WaveLength();
              double pw = pw_+(*pwm_++)*0.5;
              pw = std::min(1.0, std::max(0.0, pw));
              double p2 = (phase+pw);
              while (p2 >=1) {p2--;}
              while (p2<0) {p2++;}
              wavectrl2.Position2(p2*last_wave2->WaveLength());
            }
            wavectrl2.PreWork(1, &pWork, false);
            pWork(wavectrl2, &left_output2, &right_output2);
            wavectrl2.PostWork();
            ++pos;
            if (dostop_) {
              double phase = pos / (double) last_wave->WaveLength();
              while (phase >= 1) {
                phase -= 1;
              }
              if (abs(stopphase_ - phase) <= speed_/ (double) last_wave->WaveLength()) {
                wavectrl.NoteOff();
                wavectrl.Playing(false);
                dostop_ = false;
              }
            }
            if (fm != 0) {
              f = (f_+*(fm++));
              speed_ = f/basef;
              wavectrl.Speed(resampler, speed_);
            }
            pWork(wavectrl, &left_output, &right_output);
            if (!wavectrl.IsStereo()) {
              right_output = left_output;
            }
            if (env != 0) {
              float v = (*env++);
              left_output *= v;
              right_output *= v;
              left_output2 *= v;
              right_output2 *= v;
            }
            if (pSamplesL) *pSamplesL++ += (left_output2-left_output)*adjust_vol*gain_;
            if (pSamplesR) *pSamplesR++ += (right_output2-right_output)*adjust_vol*gain_;
            nextsamples--;
          }
          wavectrl.PostWork();
          }
        } else {
          numSamples--;
          if (wave_it == waves_.end()) {
            check_wave(f);
          } else
            if (fm != 0) {
              f = (f_+*(fm++));
              speed_ = f/basef;
              wavectrl.Speed(resampler, speed_);
            }
        }
        if (!wavectrl.Playing()) {
          return num - numSamples;
        }
        check_wave(f);
      }
      return num;
  }

  template <class T, int VOL>
  void PWMWrap<T, VOL>::check_wave(double f) {
    if (wave_it == waves_.end()) {
      wave_it = waves_.find(range<double>(f_));
    } else {
      while (wave_it != waves_.begin() && wave_it->first.min() > f) {
        wave_it--;
      }
      while (wave_it != waves_.end() && wave_it->first.max() < f) {
        wave_it++;
      }
    }
    if (wave_it != waves_.end()) {
      const XMInstrument::WaveData<T>* wave = wave_it->second;
      if (last_wave!=wave) {
        int oldpos = wavectrl.Position();
        wavectrl.Init(wave, 0, resampler);
        basef = 440 *std::pow(2.0,
          (notecommands::middleC-wave->WaveTune()-
          wave->WaveFineTune()/100-notecommands::middleA)/12.0)*
          Global::player().SampleRate() / wave->WaveSampleRate();
        wavectrl.Speed(resampler, f/basef);
        wavectrl.Playing(true);
        double phase = oldpos / (double) last_wave->WaveLength();
        int newpos = phase * wave->WaveLength();
        wavectrl.Position(newpos);
        last_wave = wave;
      }
    } else {
      last_wave = 0;
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Resampler Wrap
  ///////////////////////////////////////////////////////////////////////////////
  template <class T, int VOL>
  void ResamplerWrap<T, VOL>::Start(double phase) {
    wave_it = waves_.find(range<double>(f_));
    last_wave = wave_it != waves_.end() ? wave_it->second : 0;
    if (wave_it != waves_.end()) {
      wavectrl.Init(last_wave, 0, resampler);
      wavectrl.Position(phase*last_wave->WaveLength());
      double sr = Global::player().SampleRate();
      double l = last_wave->WaveSampleRate();
      basef = 440*std::pow(2.0,
        (notecommands::middleC-last_wave->WaveTune()-
        last_wave->WaveFineTune()/100.0-notecommands::middleA)/12.0)*sr/l;
      speed_ = f_/basef;
      wavectrl.Speed(resampler, speed_);
      wavectrl.Playing(true);
    }
    last_sgn = 1;
    oldtrigger = 0;
    dostop_ = false;
    lph_ = 0;
    lpos_ = 0;
  }

  template <class T, int VOL>
  void ResamplerWrap<T, VOL>::set_frequency(double f) {
    f_ = f;
    speed_ = f_/basef;
    wavectrl.Speed(resampler, speed_);
  }

  // compute hardsync trigger points
  template <class T, int VOL>
  int ResamplerWrap<T, VOL>::next_trigger(int numSamples, float* trigger) {
    int res = numSamples;
    int tmp = 1;
    for (int i = 0; i < numSamples; ++i) {
      // check conversion from negative to positive
      if (i == numSamples-fsize-1) {
        tmp = last_sgn;
      }
      int sgn = (trigger[i] >= 0) ? 1 : -1;
      if (last_sgn < sgn) {
        last_sgn = sgn;
        res = i;
        break;
      }
      last_sgn = sgn;
    }
    if (res == numSamples) {
      last_sgn = tmp;
    }
    return res;
  }

  template <class T, int VOL>
  int ResamplerWrap<T, VOL>::work(int num, float* left, float* right,
      SingleWorkInterface* master) {
    if (!master) {
      return work_samples(num, left, right, fm_, am_, pm_);
    } else {
      return work_trigger(num, left, right, fm_, am_, pm_, master);
    }
  }

  // hardsync work
  template <class T, int VOL>
  int ResamplerWrap<T, VOL>::work_trigger(int numSamples,
    float* pSamplesL,
    float* pSamplesR,
    float* fm,
    float* env,
    float* phase,
    SingleWorkInterface* master) {
      if (numSamples==0) return 0;
      int num = 0;
      if (oldtrigger) {
        for (int i=0; i < fsize; ++i) oldtrigger[i] = oldtrigger[n+i];
      }
      n = numSamples + ((oldtrigger) ? 0 : fsize);
      PSArray* arr = master->work(n, fsize);
      float* trigger = arr->data();
      float* samples = (oldtrigger) ? trigger : trigger+fsize;
      while (numSamples) {
        if (startfadeout) {
          int i = std::min(fsize-fadeout_sc, numSamples);
          if (i > 0) {
            num += work_samples(i, pSamplesL, pSamplesR, fm, env, phase);
            for (int k = 0; k < i; ++k) {
              float fader =  (fsize-fadeout_sc) / (float)fsize;
              if (pSamplesL) pSamplesL[0] *= fader;
              if (pSamplesR) pSamplesR[0] *= fader;
              numSamples--;
              samples++;
              if (pSamplesL) pSamplesL++;
              if (pSamplesR) pSamplesR++;
              if (fadeout_sc == (fsize-1)) {
                wavectrl.Position(0);
                startfadeout = false;
                fadeout_sc = 0;
                break;
              }
              ++fadeout_sc;
            }
          } else {
            wavectrl.Position(0);
            startfadeout = false;
            fadeout_sc = 0;
          }
        } else {
          int i = next_trigger(numSamples+fsize, samples)-fsize;
          if (i>=0) {
            num += work_samples(i, pSamplesL, pSamplesR, fm, env, phase);
            if (i != numSamples) {
              startfadeout = true;
              samples += i;
            }
            if (pSamplesL) pSamplesL += i;
            if (pSamplesR) pSamplesR += i;
            numSamples -= i;
          } else {
            int k = std::min(i+fsize, numSamples);
            num += work_samples(k, pSamplesL, pSamplesR, fm, env, phase);
            samples += i+fsize;
            if (pSamplesL) pSamplesL += k;
            if (pSamplesR) pSamplesR += k;
            numSamples -= k;
            if (numSamples < 0 ) {
              numSamples = 0;
            }
          }
        }
      }
      assert(numSamples==0);
      // adjust the master array to do slave/osc mix
      arr->margin(oldtrigger ? 0 : fsize, n);
      oldtrigger = trigger;
      return num;
  }

  template <class T, int VOL>
  int ResamplerWrap<T, VOL>::work_samples(int numSamples,
    float* pSamplesL,
    float* pSamplesR,
    float* fm,
    float* env,
    float* ph) {
      if (!wavectrl.Playing())
        return 0;
      int num = numSamples;
      float left_output = 0.0f;
      float right_output = 0.0f;
      double f = f_;

      while(numSamples) {
        XMSampler::WaveDataController<T>::WorkFunction pWork;
        int pos = wavectrl.Position();
        if (wave_it != waves_.end() && speed_ > 0) {
          int step = 1;
          if (ph==NULL) {
            step = numSamples;
          }
          for (int z = 0; z < num; z+=step) {
            if(ph!=0) {
              double phase = wavectrl.Position2() / last_wave->WaveLength();
              phase += (*ph++);
              while (phase >=1) {--phase;}
              while (phase<0) {++phase;}
              wavectrl.Position2(phase*last_wave->WaveLength());
            }
            int nextsamples = std::min(wavectrl.PreWork(
              ph!=0 ? 1 : numSamples, &pWork, false),
              ph!=0 ? 1 : numSamples);
            numSamples-=nextsamples;
            while (nextsamples) {
              ++pos;
              if (dostop_) {
                double phase = pos / (double) last_wave->WaveLength();
                while (phase >= 1) {
                  phase -= 1;
                }
                if (abs(stopphase_ - phase) <= speed_/ (double) last_wave->WaveLength()) {
                  wavectrl.NoteOff();
                  wavectrl.Playing(false);
                  dostop_ = false;
                }
              }
              pWork(wavectrl, &left_output, &right_output);
              if (fm != 0) {
                f = (f_+*(fm++));
                speed_ = f/basef;
                wavectrl.Speed(resampler, speed_);
              }
              if (env != 0) {
                float v = (*env++);
                left_output *= v;
                right_output *= v;
              }
              if (!wavectrl.IsStereo()) {
                right_output = left_output;
              }
              if (pSamplesL) *pSamplesL++ += left_output*adjust_vol*gain_;
              if (pSamplesR) *pSamplesR++ += right_output*adjust_vol*gain_;
              nextsamples--;
            }
            wavectrl.PostWork();
          }
        } else {
          numSamples--;
          if (wave_it == waves_.end()) {
            check_wave(f);
          } else
            if (fm != 0) {
              f = (f_+*(fm++));
              speed_ = f/basef;
              wavectrl.Speed(resampler, speed_);
            }
        }
        if (!wavectrl.Playing()) {
          return num - numSamples;
        }
        check_wave(f);
      }
      return num;
  }

  template <class T, int VOL>
  void ResamplerWrap<T, VOL>::check_wave(double f) {
    if (wave_it == waves_.end()) {
      wave_it = waves_.find(range<double>(f_));
    } else {
      while (wave_it != waves_.begin() && wave_it->first.min() > f) {
        wave_it--;
      }
      while (wave_it != waves_.end() && wave_it->first.max() < f) {
        wave_it++;
      }
    }
    if (wave_it != waves_.end()) {
      const XMInstrument::WaveData<T>* wave = wave_it->second;
      if (last_wave!=wave) {
        int oldpos = wavectrl.Position();
        wavectrl.Init(wave, 0, resampler);
        basef = 440 *std::pow(2.0,
          (notecommands::middleC-wave->WaveTune()-wave->WaveFineTune()/100-
          notecommands::middleA)/12.0) *
          Global::player().SampleRate() / wave->WaveSampleRate();
        wavectrl.Speed(resampler, f/basef);
        wavectrl.Playing(true);
        double phase = oldpos / (double) last_wave->WaveLength();
        int newpos = phase * wave->WaveLength();
        wavectrl.Position(newpos);
        last_wave = wave;
      }
    } else {
      last_wave = 0;
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Resampler Bind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaResamplerBind::meta = "psyresamplermeta";

  PSArray* LuaSingleWorker::work(int numSamples, int val) {
    lua_getfield(L, -1, "syncarray");
    PSArray* arr = *(PSArray **)luaL_checkudata(L, -1, LuaArrayBind::meta);
    arr->resize(numSamples+val);
    arr->margin(val, val+numSamples);
    arr->fillzero();
    lua_getfield(L, -2, "sync");
    lua_getfield(L, -1, "work");
    lua_pushvalue(L, -2);
    lua_pushvalue(L, -4);
    int status = lua_pcall(L, 2, 1 ,0);
    if (status) {
      const char* msg =lua_tostring(L, -1);
      std::ostringstream s; s << "Failed: " << msg << std::endl;
      luaL_error(L, "single work error", 0);
    }
    // int num = luaL_checknumber(L, -1);
    lua_pop(L, 3);
    arr->clearmargin();
    return arr;
  }

  int LuaResamplerBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"newwavetable", createwavetable},
      {"work", work},
      {"noteoff", noteoff},
      {"setfrequency", set_frequency},
      {"frequency", frequency},
      {"start", start},
      {"stop", stop },
      {"phase", phase},
      {"setphase", setphase},
      {"isplaying", isplaying},
      {"setwavedata", set_wave_data},
      {"setquality", set_quality},
      {"quality", set_quality},
      {"setsync", set_sync},
      {"setsyncfadeout", set_sync_fadeout},
      {"setpm", setpm},
      {"setam", setam},
      {"setfm", setfm},
      {NULL, NULL}
    };
    LuaHelper::open(L, meta, methods, gc);
    lua_pushnumber(L, 1);
    lua_setfield(L, -2, "ZEROHOLD");
    lua_pushnumber(L, 2);
    lua_setfield(L, -2, "LINEAR");
    lua_pushnumber(L, 3);
    lua_setfield(L, -2, "SINC");
    return 1;
  }

  int LuaResamplerBind::createwavetable(lua_State *L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, wavetable)", n);
    }
    LuaHelper::new_userdata<>(L, meta,
      new ResamplerWrap<float, 1>(check_wavelist(L)));
    return 1;
  }

  int LuaResamplerBind::create(lua_State *L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, wave)", n);
    }
    RWInterface* rwrap = 0;
    XMInstrument::WaveData<float>* wave =
      LuaHelper::check<XMInstrument::WaveData<float> >(L, 4,
      LuaWaveDataBind::meta);
    WaveList<float>::Type waves;
    waves[range<double>(0, 96000)] = wave;
    rwrap = new ResamplerWrap<float, 1>(waves);
    LuaHelper::new_userdata<>(L, meta, rwrap);
    lua_pushnil(L);
    lua_setfield(L, -2, "sync");
    return 1;
  }

  WaveList<float>::Type LuaResamplerBind::check_wavelist(lua_State* L) {
    WaveList<float>::Type waves;
    if (lua_istable(L, 2)) {
      size_t len = lua_rawlen(L, 2);
      for (size_t i = 1; i <= len; ++i) {
        lua_rawgeti(L, 2, i); // get triple {Wave, flo, fhi}
        lua_rawgeti(L, 3, 1); // GetWaveData
        XMInstrument::WaveData<float>* wave =
          LuaHelper::check<XMInstrument::WaveData<float> >(L, 4,
          LuaWaveDataBind::meta);
        lua_pop(L,1);
        lua_rawgeti(L, 3, 2); // GetFreqLo
        double lo = luaL_checknumber(L, 4);
        lua_pop(L,1);
        lua_rawgeti(L, 3, 3); // GetFreqHi
        double hi = luaL_checknumber(L, 4);
        lua_pop(L,1);
        lua_pop(L,1);
        waves[range<double>( lo, hi )] = wave;
      }
    }
    return waves;
  }

  int LuaResamplerBind::set_wave_data(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, wavedata)", n);
    }
    RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, meta);
    WaveList<float>::Type waves = check_wavelist(L);
    rwrap->SetData(waves);
    return 0;
  }

  int LuaResamplerBind::set_sync(lua_State* L) {
    LuaHelper::check<RWInterface>(L, 1, meta);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_setfield(L, -2, "sync");
    lua_getglobal(L, "require");
    lua_pushstring(L, "psycle.array");
    lua_pcall(L, 1, 1, 0);
    lua_getfield(L, -1, "new");
    lua_pushnumber(L, 256);
    lua_pcall(L, 1, 1, 0);
    lua_setfield(L, 1, "syncarray");
    return 0;
  }

  int LuaResamplerBind::set_sync_fadeout(lua_State* L) {
    return LuaHelper::callstrict1<RWInterface, int>(L, meta, &RWInterface::set_sync_fadeout_size);
  }

  int LuaResamplerBind::work(lua_State* L) {
    int n = lua_gettop(L);
    if (n!=3 and n != 4 and n!=5 and n!=6) {
      return luaL_error(L, "Got %d arguments expected 2 (self, left, right [,fm])", n);
    }
    RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, meta);
    if (!rwrap->Playing()) {
      lua_pushnumber(L, 0);
      return 1;
    }
    PSArray* l = *(PSArray **)luaL_checkudata(L, 2, LuaArrayBind::meta);
    PSArray* r = *(PSArray **)luaL_checkudata(L, 3, LuaArrayBind::meta);

    float* fm = 0;
    float* env = 0;
    // frequence modulation -- optional
    if (n>3 && (!lua_isnil(L, 4))) {
      PSArray*arr = *(PSArray **)luaL_checkudata(L, 4, LuaArrayBind::meta);
      fm = arr->data();
    }
    // volume envelope -- optional
    if (n>4 && !lua_isnil(L, 5)) {
      PSArray* arr = *(PSArray **)luaL_checkudata(L, 5, LuaArrayBind::meta);
      env = arr->data();
    }
    // check for master
    std::auto_ptr<LuaSingleWorker> master;
    lua_getfield(L, 1, "sync");
    if (lua_isnil(L, -1)) {
      lua_pop(L, 1);
    } else {
      lua_pushvalue(L, 1);
      master.reset(new LuaSingleWorker(L));
    }
    int processed = rwrap->work(l->len(), l->data(), r->data(), master.get());
    lua_pushnumber(L, processed); // return total number of resampled samples    
    return 1;
  }

  int LuaResamplerBind::isplaying(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n);
    }
    RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, meta);
    lua_pushboolean(L, rwrap->Playing());
    return 1;
  }

  int LuaResamplerBind::phase(lua_State* L) {
    return LuaHelper::getnumber<RWInterface, double>(L, meta, &RWInterface::phase);
  }

  int LuaResamplerBind::setphase(lua_State* L) {
    return LuaHelper::callstrict1<RWInterface, double>(L, meta, &RWInterface::setphase);
  }

  int LuaResamplerBind::noteoff(lua_State* L) {
    LuaHelper::call<RWInterface>(L, meta, &RWInterface::NoteOff);
    return 0;
  }

  int LuaResamplerBind::start(lua_State* L) {
    return LuaHelper::callopt1<RWInterface, double>(L, meta, &RWInterface::Start, 0);
  }

  int LuaResamplerBind::stop(lua_State* L) {
    return LuaHelper::callopt1<RWInterface, double>(L, meta, &RWInterface::Stop, 0);
  }

  int LuaResamplerBind::set_frequency(lua_State* L) {
    return LuaHelper::callstrict1<RWInterface, double>(L, meta, &RWInterface::set_frequency);
  }

  int LuaResamplerBind::frequency(lua_State* L) {
    return LuaHelper::getnumber<RWInterface, double>(L, meta, &RWInterface::frequency);
  }

  int LuaResamplerBind::set_quality(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 2) {
      return luaL_error(L, "Got %d arguments expected 2 (self, frequency)", n);
    }
    RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, meta);
    int quality = luaL_checknumber(L, 2);
    rwrap->set_quality(
      (psycle::helpers::dsp::resampler::quality::type)(quality-1));
    return 0;
  }

  int LuaResamplerBind::quality(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 1) {
      return luaL_error(L, "Got %d arguments expected 1 (self)", n);
    }
    RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, meta);
    lua_pushnumber(L, (int)(rwrap->quality()+1));
    return 1;
  }

  int LuaResamplerBind::gc (lua_State *L) {
    return LuaHelper::delete_userdata<RWInterface>(L, meta);
  }

  int LuaResamplerBind::setpm(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
      RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, meta);
      if (lua_isnil(L, 2)) {
        rwrap->setpm(0);
      } else {
        PSArray* v = *(PSArray **)luaL_checkudata(L, 2, LuaArrayBind::meta);
        rwrap->setpm(v->data());
      }
    } else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    return LuaHelper::chaining(L);
  }

  int LuaResamplerBind::setfm(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
      RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, meta);
      if (lua_isnil(L, 2)) {
        rwrap->setfm(0);
      } else {
        PSArray* v = *(PSArray **)luaL_checkudata(L, 2, LuaArrayBind::meta);
        rwrap->setfm(v->data());
      }
    } else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    return LuaHelper::chaining(L);
  }

  int LuaResamplerBind::setam(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
      RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, meta);
      if (lua_isnil(L, 2)) {
        rwrap->setam(0);
      } else {
        PSArray* v = *(PSArray **)luaL_checkudata(L, 2, LuaArrayBind::meta);
        rwrap->setam(v->data());
      }
    } else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    return LuaHelper::chaining(L);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Envelope
  ///////////////////////////////////////////////////////////////////////////////
  void LEnvelope::work(int num) {
    out_.resize(num);
    if (!is_playing() || (stage_ == sus_ && !susdone_)) {
      out_.fill(lv_);
      return;
    }
    float* data = out_.data();
    for (int k = 0; k < num; ++k) {
      if (nexttime_ == sc_) {
        ++stage_;
        if (stage_ > peaks_.size()-1 or (!susdone_ && stage_ == sus_)) {
          lv_ = peaks_[stage_-1];
          m_ = 0;
          for (int j = k; j < num; ++j) out_.set_val(j, lv_);
          break;
        }
        calcstage(peaks_[stage_-1], peaks_[stage_], times_[stage_],
          types_[stage_]);
      }
      if (types_[stage_]==0) {
        *(data++) = lv_;
        lv_ += m_;
      } else {
        // *(data++) = (up_) ? peaks_[stage_]-lv_ : lv_;
        *(data++) = lv_;
        lv_ *= m_;
      }
      ++sc_;
    }
  }

  void LEnvelope::release() {
    if (is_playing()) {
      // int old = stage_;
      susdone_ = true;
      if (stage_ != sus_) {
        stage_ = peaks_.size()-1;
      }
      //calcstage((up_) ?  peaks_[old]-lv_
      //	            : lv_, peaks_[stage_], times_[stage_], types_[stage_]);
      calcstage(lv_, peaks_[stage_], times_[stage_], types_[stage_]);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  // LuaEnvelopeBind
  ///////////////////////////////////////////////////////////////////////////////

  const char* LuaEnvelopeBind::meta = "psyenvelopemeta";

  int LuaEnvelopeBind::open(lua_State *L) {
    static const luaL_Reg methods[] = {
      {"new", create},
      {"work", work },
      {"release", release},
      {"isplaying", isplaying},
      {"start", start },
      {"setpeak", setpeak},
      {"setstartvalue", setstartvalue},
      {"lastvalue", lastvalue},
      {"peak", peak},
      {"settime", setstagetime},
      {"time", time},
      {"tostring", tostring},
      {NULL, NULL}
    };
    LuaHelper::open(L, meta, methods, gc, tostring);
    lua_pushnumber(L, 0);
    lua_setfield(L, -2, "LIN");
    lua_pushnumber(L, 1);
    lua_setfield(L, -2, "EXP");
    return 1;
  }

  int LuaEnvelopeBind::create(lua_State *L) {
    int n = lua_gettop(L);  // Number of arguments
    if (n != 3) {
      return luaL_error(L,
        "Got %d arguments expected 3 (self, points, sustainpos", n);
    }
    std::vector<double> times;
    std::vector<double> peaks;
    std::vector<int> types;
    if (lua_istable(L, 2)) {
      size_t len = lua_rawlen(L, 2);
      for (size_t i = 1; i <= len; ++i) {
        lua_rawgeti(L, 2, i); // get point {time, peak}
        size_t argnum = lua_rawlen(L, 4);
        lua_rawgeti(L, 4, 1); // get time
        double t = luaL_checknumber(L, 5);
        times.push_back(t);
        lua_pop(L,1);
        lua_rawgeti(L, 4, 2); // get peak
        peaks.push_back(luaL_checknumber(L, 5));
        lua_pop(L,1);
        if (argnum == 3) {
          lua_rawgeti(L, 4, 3); // get type
          if (!lua_isnil(L, 5)) {
            types.push_back(luaL_checknumber(L, 5));
          }
          lua_pop(L,1);
        } else types.push_back(0);
        lua_pop(L,1);
      }
    }
    int suspos = luaL_checknumber(L, 3)-1;
    double startpeak = 0;
    if (n==4) {
      startpeak = luaL_checknumber(L, 4);
    }
    LuaHelper::new_userdata<LEnvelope>(L, meta,
      new LEnvelope(times,
      peaks,
      types,
      suspos,
      startpeak,
      Global::player().SampleRate()
      ));
    return 1;
  }

  int LuaEnvelopeBind::work(lua_State* L) {
    int n = lua_gettop(L);
    if (n ==2) {
      LEnvelope* env = LuaHelper::check<LEnvelope>(L, 1, meta);
      int num = luaL_checknumber(L, 2);
      env->work(num);
      PSArray ** rv = (PSArray **)lua_newuserdata(L, sizeof(PSArray *));
      luaL_setmetatable(L, LuaArrayBind::meta);
      *rv = new PSArray(env->out().data(), num);
    }  else {
      luaL_error(L, "Got %d arguments expected 2 (self, num)", n);
    }
    return 1;
  }

  int LuaEnvelopeBind::isplaying(lua_State *L) {
    LEnvelope* env = LuaHelper::check<LEnvelope>(L, 1, meta);
    lua_pushboolean(L, env->is_playing());
    return 1;
  }

  int LuaEnvelopeBind::release(lua_State *L) {
    LuaHelper::call(L, meta, &LEnvelope::release);
    return 0;
  }

  int LuaEnvelopeBind::start(lua_State *L) {
    LuaHelper::call(L, meta, &LEnvelope::start);
    return 0;
  }

  int LuaEnvelopeBind::setpeak(lua_State* L) {
    LuaHelper::callstrict2<LEnvelope>(L, meta, &LEnvelope::setstagepeak, true);
    return LuaHelper::chaining(L);
  }

  int LuaEnvelopeBind::peak(lua_State* L) {
    int n = lua_gettop(L);
    if (n ==2) {
      LEnvelope* env = LuaHelper::check<LEnvelope>(L, 1, meta);
      int idx = luaL_checknumber(L, 2)-1;
      lua_pushnumber(L, env->peak(idx));
    } else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    return 1;
  }

  int LuaEnvelopeBind::time(lua_State* L) {
    int n = lua_gettop(L);
    if (n ==2) {
      LEnvelope* env = LuaHelper::check<LEnvelope>(L, 1, meta);
      int idx = luaL_checknumber(L, 2)-1;
      lua_pushnumber(L, env->time(idx));
    } else {
      luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    return 1;
  }

  int LuaEnvelopeBind::setstagetime(lua_State* L) {
    LuaHelper::callstrict2<LEnvelope>(L, meta, &LEnvelope::setstagetime, true);
    return LuaHelper::chaining(L);
  }

  int LuaEnvelopeBind::setstartvalue(lua_State* L) {
    LuaHelper::callstrict1<LEnvelope, float>(L, meta, &LEnvelope::setstartvalue);
    return LuaHelper::chaining(L);
  }

  int LuaEnvelopeBind::lastvalue(lua_State *L) {
    return LuaHelper::getnumber<LEnvelope, float>(L, meta, &LEnvelope::lastvalue);
  }

  int LuaEnvelopeBind::gc(lua_State *L) {
    return LuaHelper::delete_userdata<LEnvelope>(L, meta);
  }

  int LuaEnvelopeBind::tostring(lua_State *L) {
    LuaHelper::check_tostring<LEnvelope>(L, meta);
    std::stringstream r;
    r << "env";
    lua_pushfstring(L, r.str().c_str());
    return 1;
  }
} // namespace
} // namespace