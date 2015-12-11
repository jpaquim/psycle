// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "plugininfo.hpp"
#include "LuaArray.hpp"
#include "LuaInternals.hpp"
#include "LuaGui.hpp"

struct lua_State;
struct luaL_Reg;

namespace universalis { namespace os {
	class terminal;
}}

// namespace psycle { namespace host { namespace ui { namespace canvas { struct Event; }}}}

namespace psycle {
namespace host {
    
class LuaPlugin;

typedef boost::shared_ptr<LuaPlugin> LuaPluginPtr;
extern boost::shared_ptr<LuaPlugin> nullPtr;

  //controlling function header
static UINT StartThread (LPVOID param);

//structure for passing to the controlling function
typedef struct THREADSTRUCT
{
    class TDlg*    _this;
        //you can add here other parameters you might be interested on
} THREADSTRUCT;

class LuaPlugin;

class LuaProxy : public LockIF {
public:
	LuaProxy(LuaPlugin* plug, const std::string& dllname);
	~LuaProxy();

  const PluginInfo& info() const;
	int num_cols() const { return lua_mac_->numcols(); }
	int num_parameter() const { return lua_mac_->numparams(); }
					
	void Run();
	void Init();
  void Reload();
  void Free();

  LuaCanvas::WeakPtr canvas() { return lua_mac_->canvas(); }

  void SequencerTick();
  void ParameterTweak(int par, double val);
  void Work(int numsamples, int offset=0);
  void Stop();
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
				
  void call_execute();
	void call_sr_changed(int rate);
	void call_aftertweaked(int idx);
	void call_menu(UINT id);  
	std::string call_help();	
	void set_state(lua_State* state);
  lua_State* state() const { return L; }	
  void update_menu(void* menu);
  ui::MenuBar* get_menu(ui::Menu* menu);
  MachineUiType::Value ui_type() const { return lua_mac_->ui_type(); }
  void call_setprogram(int idx);
  int call_numprograms();
  int get_curr_program();
  std::string get_program_name(int bnkidx, int idx);
  MachinePresetType::Value prsmode() const { return lua_mac_->prsmode(); }
  void lock() const { ::EnterCriticalSection(&cs); }
  void unlock() const { ::LeaveCriticalSection(&cs); }

  LuaPlugin& host() { return *host_; }
  LuaPlugin& host() const { return *host_; }
  LuaMachine* lua_mac() { return lua_mac_; };
      
private:
	void export_c_funcs();
	// script callbacks
	static int set_parameter(lua_State* L);
	static int message(lua_State* L);
	static int terminal_output(lua_State* L);
	static int call_filedialog(lua_State* L);
  static int call_selmachine(lua_State* L);
	static int set_machine(lua_State* L);  
  std::string ParDisplay(int par);
  std::string ParLabel(int par);
  
  mutable bool info_update_;
	mutable PluginInfo info_;
	LuaPlugin *host_;
	LuaMachine* lua_mac_;
	lua_State* L;
	static universalis::os::terminal * terminal;  
  mutable CRITICAL_SECTION cs;  
};

typedef std::list<LuaPluginPtr> LuaUiList;
typedef boost::shared_ptr<class LuaUiExtentions> LuaUiExtentionsPtr;

class LuaUiExtentions {     
 public:
   LuaUiExtentions() { }
   ~LuaUiExtentions() { }
   static LuaUiExtentionsPtr instance();   
   void Add(const LuaPluginPtr& ptr) { uiluaplugins.push_back(ptr); }
   void Remove(const LuaPluginPtr& ptr) { uiluaplugins.remove(ptr); }
   LuaUiList Get(const std::string& name);
   LuaPluginPtr Get(int idx);   
 private:
   LuaUiList uiluaplugins;
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
      if (mac->_type == MACH_LUA) {
        return (LuaPlugin*) mac;
      }
      return 0;
   }   
};

 
}  // namespace host
}  // namespace psycle