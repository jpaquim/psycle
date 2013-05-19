#pragma once
#include <psycle/host/detail/project.hpp>
#include "plugininfo.hpp"
#include "LuaArray.hpp"

struct lua_State;

namespace psycle { namespace host {

class LuaPlugin;

class LuaProxy {
public:    
    static const std::string meta_name;
	static const std::string userdata_name;

	LuaProxy(LuaPlugin* plug, lua_State* state);
	~LuaProxy();

	float get_parameter(int numparam);
	const char* get_parameter_name(int numparam);
	const char* get_parameter_display(int numparam);
	const char* get_parameter_label(int numparam);
	
	void run_call_init(std::vector<float*>& sample_buf);
	void call_seqtick(int /*channel*/, int /*note*/, int /*ins*/, int /*cmd*/, int /*val*/);
	void call_work(int num) throw (...);
    void call_parameter(int numparameter, float val);
	void call_stop();

	const PluginInfo& info() const { return info_; }

	int num_parameter() const { return num_parameter_; }
	void free_state();
	void set_state(lua_State* state);

	void lock() const;
    void unlock() const;
private:
	static int set_machine_info(lua_State* L);
	static int set_parameter(lua_State* L);
	static int message(lua_State* L);
	void export_c_funcs();

	// some call helper 
	int GetRawParameter(const char* field, int index);
	const char* GetString();

	PluginInfo info_;
	int num_parameter_;
	LuaPlugin *plug_;
	lua_State* L;
    LuaArrayBind array_bind_;
	mutable CRITICAL_SECTION cs;
};


struct LuaHost {
   static lua_State* load_script(const char * sName);
   static LuaPlugin* LoadPlugin(const char * sName, int macIdx);	
};

}
}
