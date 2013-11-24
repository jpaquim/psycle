#pragma once
#include <psycle/host/detail/project.hpp>
#include "plugininfo.hpp"
#include "LuaArray.hpp"
#include "LuaInternals.hpp"

struct lua_State;
struct luaL_Reg;

namespace universalis { namespace os {
	class terminal;
}}

namespace psycle { namespace host {

class LuaPlugin;

class LuaProxy {
public:    
	LuaProxy(LuaPlugin* plug, lua_State* state);
	~LuaProxy();

    const PluginInfo& info() const { return info_; }
	int num_cols() const { return plugimport_->numcols(); }
	int num_parameter() const { return plugimport_->numparams(); }	
	double get_parameter_value(int numparam);
	std::string get_parameter_id(int numparam);
	std::string get_parameter_name(int numparam);
	std::string get_parameter_display(int numparam);
	std::string get_parameter_label(int numparam);
	void get_parameter_range(int numparam,int &minval, int &maxval);
	int get_parameter_type(int numparam);
	// calls from proxy to script
	void call_run();
	void call_init();
	PluginInfo call_info();
	void call_seqtick(int /*channel*/, int /*note*/, int /*ins*/, int /*cmd*/, int /*val*/);
	void call_newline();
	void call_work(int num) throw (...);
    void call_parameter(int numparameter, double val);
	void call_stop();
	void call_sr_changed(int rate);
	std::string call_help();
	void free_state();
	void set_state(lua_State* state);
	void reload();
	// buffers
	void update_num_samples(int num) { plugimport_->update_num_samples(num); }

private:
	void export_c_funcs();
	// script callbacks
	static int set_parameter(lua_State* L);
	static int message(lua_State* L);
	static int terminal_output(lua_State* L);
	static int set_machine(lua_State* L);

	void get_method_strict(lua_State* L, const char* method);
	bool get_method_optional(lua_State* L, const char* method);
	bool get_param(lua_State* L, int index, const char* method);

	// mutex
	void lock() const;
    void unlock() const;
	std::string GetString();
	PluginInfo info_;
	LuaPlugin *plug_;
	LuaMachine* plugimport_;
	lua_State* L;
	mutable CRITICAL_SECTION cs;
	static universalis::os::terminal * terminal;
};

struct LuaHost {
   static lua_State* load_script(const std::string& dllpath);
   static LuaPlugin* LoadPlugin(const std::string& dllpath, int macIdx);
   static PluginInfo LoadInfo(const std::string& dllpath);
};

}
}
