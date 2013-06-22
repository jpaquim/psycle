#pragma once
#include <psycle/host/detail/project.hpp>
#include "plugininfo.hpp"
#include "LuaArray.hpp"

struct lua_State;
struct luaL_Reg;

namespace universalis { namespace os {
	struct terminal;
}}

namespace psycle { namespace host {

class LuaPlugin;

class PSLuaPluginImport {
public:
    static const std::string metaname;

	PSLuaPluginImport() {}
	~PSLuaPluginImport() {}

	void load(const char* name);
	void work(int samples);
	
	Machine* mac() { return mac_; }

	PSArray* channel(int idx) { return &sampleV_[idx]; }
	void update_num_samples(int num);
	void build_buffer(std::vector<float*>& buf, int num);
	void set_buffer(std::vector<float*>& buf);
private:
		
	Machine* mac_;
	psybuffer sampleV_;
};

class LuaProxy {
public:    
    static const std::string meta_name;
	static const std::string userdata_name;

	LuaProxy(LuaPlugin* plug, lua_State* state);
	~LuaProxy();

	double get_parameter_value(int numparam);
	std::string get_parameter_name(int numparam);
	std::string get_parameter_display(int numparam);
	std::string get_parameter_label(int numparam);
	void get_parameter_range(int numparam,int &minval, int &maxval);
	
	void call_run(std::vector<float*>& sample_buf);
	void call_init();
	PluginInfo call_info();
	void call_seqtick(int /*channel*/, int /*note*/, int /*ins*/, int /*cmd*/, int /*val*/);
	void call_work(int num) throw (...);
    void call_parameter(int numparameter, double val);
	void call_stop();

	const PluginInfo& info() const { return info_; }

	int num_parameter() const { return num_parameter_; }
	void free_state();
	void set_state(lua_State* state);

	void lock() const;
    void unlock() const;

	void reload();

private:
	static int set_parameters(lua_State* L);
	static int set_parameter(lua_State* L);
	static int message(lua_State* L);

	static int plotter_new(lua_State* L);
	static int plotter_gc(lua_State* L);
	static int plotter_stem(lua_State* L);

	static int plugin_new(lua_State* L);
	static int plugin_work(lua_State* L);
	static int plugin_channel(lua_State* L);
	static int plugin_resize(lua_State* L);
	static int plugin_param(lua_State* L);
	static int plugin_numparams(lua_State* L);
	static int plugin_paramdisplay(lua_State* L);
	static int plugin_setparam(lua_State* L);
	static int plugin_paramname(lua_State* L);
	static int plugin_gc(lua_State* L);	
	static int plugin_setbuffer(lua_State* L);

	static int terminal_output(lua_State* L);

	static int dummy(lua_State*L) {return 0;}

	void export_c_funcs();
	// some call helper 
	int GetRawParameter(const char* field, int index);
	void register_userdata(const luaL_Reg* funcs,
	                       const luaL_Reg* methods,
	                       const char* metaname,
						   const char* userdataname);	
	const char* GetString();
	void push_proxy();

	PluginInfo info_;
	int num_parameter_;
	LuaPlugin *plug_;
	lua_State* L;
    LuaArrayBind array_bind_;
	mutable CRITICAL_SECTION cs;
	static universalis::os::terminal * terminal;	
};


struct LuaHost {
   static lua_State* load_script(const char * sName);
   static LuaPlugin* LoadPlugin(const char * sName, int macIdx);
   static PluginInfo LoadInfo(const char * sName);
};

}
}
