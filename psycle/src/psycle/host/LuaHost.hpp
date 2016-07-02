// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "plugininfo.hpp"
#include "LuaArray.hpp"
#include "LuaInternals.hpp"
#include "LockIF.hpp"

struct lua_State;
struct luaL_Reg;

namespace psycle {
namespace host {

namespace ui { 
  class Commands; 
  namespace canvas {
    class Canvas;
    class TerminalFrame;
  }
  class MenuContainer;
}
    
class LuaPlugin;

typedef boost::shared_ptr<LuaPlugin> LuaPluginPtr;
extern boost::shared_ptr<LuaPlugin> nullPtr;

class LuaPlugin;

class LuaControl : public LockIF {
 public:
  LuaControl();
  virtual ~LuaControl();
  
  void Load(const std::string& filename);
  virtual void PrepareState();
  void Run();
  void Free();  

  
  lua_State* state() const { return L; }

  // LockIF Implementation
  void lock() const { ::EnterCriticalSection(&cs); }
  void unlock() const { ::LeaveCriticalSection(&cs); }
  
  protected:
   lua_State* L;
   std::auto_ptr<ui::Commands> invokelater;   
  private:   
   mutable CRITICAL_SECTION cs;   
};

class LuaProxy : public LuaControl {
public:
	LuaProxy(LuaPlugin* plug, const std::string& dllname);
	~LuaProxy();

  // Host accessors
  LuaPlugin& host() { return *host_; }
  LuaPlugin& host() const { return *host_; }  
  LuaMachine* lua_mac() { return lua_mac_; };

  const PluginInfo& info() const;
	
  // Script Control	
	void Init();
  void Reload();
  void PrepareState();

  boost::weak_ptr<ui::canvas::Canvas> canvas() { 
    return lua_mac_->canvas();
  }

  // Plugin calls
  void SequencerTick();
  void ParameterTweak(int par, double val);
  void Work(int numsamples, int offset=0);
  void Stop();
  void OnTimer();
  void PutData(unsigned char* data, int size);
  int GetData(unsigned char **ptr, bool all);
	uint32_t GetDataSize();
  void Command(int lastnote, int inst, int cmd, int val);
  void NoteOn(int note, int lastnote, int inst, int cmd, int val);
	void NoteOff(int note, int lastnote, int inst, int cmd, int val);
  bool DescribeValue(int parameter, char * txt);
  void SeqTick(int channel, int note, int ins, int cmd, int val);	
  double Val(int par);
  std::string Id(int par);
	std::string Name(int par);		
	void Range(int par,int &minval, int &maxval);
	int Type(int par);				
  bool OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);  
  bool OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
  void call_execute();
	void call_sr_changed(int rate);
	void call_aftertweaked(int idx);	  
	std::string call_help();
	    
  MachineUiType::Value ui_type() const { return lua_mac_->ui_type(); }
  void call_setprogram(int idx);
  int call_numprograms();
  int get_curr_program();
  std::string get_program_name(int bnkidx, int idx);
  MachinePresetType::Value prsmode() const { return lua_mac_->prsmode(); }  
  int num_cols() const { return lua_mac_->numcols(); }
	int num_parameter() const { return lua_mac_->numparams(); }
        
  void OnCanvasChanged();
  void OnActivated();
  void OnDeactivated();
  
  boost::weak_ptr<ui::MenuContainer> menu_bar() { return menu_bar_; }

private:
  void export_c_funcs();
  static int invoke_later(lua_State* L);
	// script callbacks
  static int set_parameter(lua_State* L);
  static int alert(lua_State* L);
  static int confirm(lua_State* L);  
  static int terminal_output(lua_State* L);
  static int call_selmachine(lua_State* L);
  static int set_machine(lua_State* L);  
  static int set_menubar(lua_State* L);
  std::string ParDisplay(int par);
  std::string ParLabel(int par);
    
  mutable bool info_update_;
	mutable PluginInfo info_;
	LuaPlugin *host_;
	LuaMachine* lua_mac_;	
  mutable CRITICAL_SECTION cs;  
  boost::weak_ptr<ui::MenuContainer> menu_bar_;
  static boost::shared_ptr<ui::canvas::TerminalFrame> terminal_frame_;
};

// Container for LuaUiExtensions
class LuaUiExtentions {     
 public:
  typedef std::list<LuaPluginPtr> List;
  typedef boost::shared_ptr<LuaUiExtentions> Ptr;

  LuaUiExtentions() {}
  ~LuaUiExtentions() {}
  static LuaUiExtentions::Ptr instance();

  void Free();

  typedef LuaUiExtentions::List::iterator iterator;
  virtual iterator begin() { return uiluaplugins_.begin(); }
  virtual iterator end() { return uiluaplugins_.end(); }
  virtual bool empty() const { return true; }

  void Add(const LuaPluginPtr& ptr) { uiluaplugins_.push_back(ptr); }
  void Remove(const LuaPluginPtr& ptr) { uiluaplugins_.remove(ptr); }
  LuaUiExtentions::List Get(const std::string& name);
  LuaPluginPtr Get(int idx);  
  
 private:
  LuaUiExtentions::List uiluaplugins_;
};

struct LuaGlobal {
   static std::map<lua_State*, LuaProxy*> proxy_map;
   static LuaProxy* proxy(lua_State* L) {
     std::map<lua_State*, LuaProxy*>::iterator it = proxy_map.find(L);
     return it != proxy_map.end() ? it->second : 0;
   }   
   static lua_State* load_script(const std::string& dllpath);
   static PluginInfo LoadInfo(const std::string& dllpath);         
   static Machine* GetMachine(int idx);
   static class LuaPlugin* GetLuaPlugin(int idx) {
      Machine* mac = GetMachine(idx);
      assert(mac);
      if (mac->_type == MACH_LUA) {
        return (LuaPlugin*) mac;
      }
      return 0;
   }
   static std::vector<LuaPlugin*> GetAllLuas();   
};

 
}  // namespace host
}  // namespace psycle