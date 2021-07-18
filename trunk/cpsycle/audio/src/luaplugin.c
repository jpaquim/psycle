// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "luaplugin.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luaimport.h"
#include "array.h"
#include "luaarray.h"
#include "luaenvelope.h"
#include "luafilter.h"
#include "luadspmath.h"
#include "luawaveosc.h"
#include "luamidinotes.h"
#include "songio.h"
#include "custommachine.h"
#include "plugin_interface.h"
#include "exclusivelock.h"
#include "lock.h"
#include "machines.h"
#include "machinefactory.h"

#include <list.h>

#include <stdlib.h>
#include <string.h>
#include "../../detail/portable.h"

static void getparam(psy_LuaImport* import, uintptr_t idx, const char* method);
static const char* luaplugin_id(psy_audio_LuaPlugin* self, int index);

// Parameter
static int luapluginparam_name(psy_audio_LuaPluginMachineParam*, char* text);
static int luapluginparam_label(psy_audio_LuaPluginMachineParam*, char* text);
static void luapluginparam_tweak(psy_audio_LuaPluginMachineParam*, float val);
static int luapluginparam_describe(psy_audio_LuaPluginMachineParam*, char* text);
static float luapluginparam_normvalue(psy_audio_LuaPluginMachineParam*);
static void luapluginparam_range(psy_audio_LuaPluginMachineParam*, intptr_t* minval,
	intptr_t* maxval);
static int luapluginparam_type(psy_audio_LuaPluginMachineParam*);

static MachineParamVtable luapluginparam_vtable;
static int luapluginparam_vtable_initialized = 0;

static void luapluginparam_vtable_init(psy_audio_LuaPluginMachineParam* self)
{
	if (!luapluginparam_vtable_initialized) {
		luapluginparam_vtable = *(self->custommachineparam.machineparam.vtable);
		luapluginparam_vtable.name = (fp_machineparam_name)luapluginparam_name;
		luapluginparam_vtable.label = (fp_machineparam_label)luapluginparam_label;
		luapluginparam_vtable.tweak = (fp_machineparam_tweak)luapluginparam_tweak;
		luapluginparam_vtable.normvalue = (fp_machineparam_normvalue)luapluginparam_normvalue;
		luapluginparam_vtable.range = (fp_machineparam_range)luapluginparam_range;
		luapluginparam_vtable.type = (fp_machineparam_type)luapluginparam_type;
		luapluginparam_vtable.describe = (fp_machineparam_describe)luapluginparam_describe;
	}
}

void psy_audio_luapluginmachineparam_init(psy_audio_LuaPluginMachineParam* self,
	struct psy_audio_LuaPlugin* plugin,
	uintptr_t index)
{
	psy_audio_custommachineparam_init(&self->custommachineparam,
		"",
		"",
		MPF_STATE,
		0,
		0xFFFF);
	luapluginparam_vtable_init(self);
	self->custommachineparam.machineparam.vtable = &luapluginparam_vtable;
	self->plugin = plugin;
	self->index = index;
}

void psy_audio_luapluginmachineparam_dispose(psy_audio_LuaPluginMachineParam* self)
{
	psy_audio_custommachineparam_dispose(&self->custommachineparam);
}

psy_audio_LuaPluginMachineParam* psy_audio_luapluginmachineparam_alloc(void)
{
	return (psy_audio_LuaPluginMachineParam*)malloc(sizeof(psy_audio_LuaPluginMachineParam));
}

psy_audio_LuaPluginMachineParam* psy_audio_luapluginmachineparam_allocinit(struct psy_audio_LuaPlugin* plugin,
	uintptr_t index)
{
	psy_audio_LuaPluginMachineParam* rv;

	rv = psy_audio_luapluginmachineparam_alloc();
	if (rv) {
		psy_audio_luapluginmachineparam_init(rv, plugin, index);
	}
	return rv;
}

int luapluginparam_label(psy_audio_LuaPluginMachineParam* self, char* text)
{
	int rv = FALSE;

	if (self->index >= 0 && self->index < self->plugin->client->numparameters_) {
		psy_LuaImport in;

		psy_lock_enter(self->plugin->lock);
		psy_luaimport_init(&in, self->plugin->script.L, self->plugin->client);
		if (psy_luaimport_open(&in, 0)) {
			const char* str;
			size_t l = 0;

			getparam(&in, self->index, "label");
			psy_luaimport_pcall(&in, 1);
			str = lua_tolstring(self->plugin->script.L, -1, &l);
			psy_snprintf(text, 128, "%s", str);
			rv = TRUE;
		}
		psy_luaimport_dispose(&in);
		psy_lock_leave(self->plugin->lock);
		return rv;
	}
	return rv;
}

int luapluginparam_name(psy_audio_LuaPluginMachineParam* self, char* text)
{
	int rv = FALSE;

	if (self->index >= 0 && self->index < self->plugin->client->numparameters_) {
		psy_LuaImport in;

		psy_lock_enter(self->plugin->lock);
		psy_luaimport_init(&in, self->plugin->script.L, self->plugin->client);
		if (psy_luaimport_open(&in, 0)) {
			const char* str;
			size_t l;

			getparam(&in, self->index, "name");
			psy_luaimport_pcall(&in, 1);
			str = lua_tolstring(self->plugin->script.L, -1, &l);
			psy_snprintf(text, 128, "%s", str);
			rv = TRUE;
		}
		psy_luaimport_dispose(&in);
		psy_lock_leave(self->plugin->lock);
		return rv;
	}
	return rv;
}

void luapluginparam_tweak(psy_audio_LuaPluginMachineParam* self, float value)
{
	// Parameter tweak range is [0..1]	
	psy_LuaImport in;

	psy_lock_enter(self->plugin->lock);
	psy_luaimport_init(&in, self->plugin->script.L, self->plugin->client);
	if (psy_luaimport_open(&in, 0)) {
		getparam(&in, self->index, "setnorm");
		lua_pushnumber(self->plugin->script.L, value);
		psy_luaimport_pcall(&in, 0);
	}
	psy_luaimport_dispose(&in);
	psy_lock_leave(self->plugin->lock);
}

int luapluginparam_describe(psy_audio_LuaPluginMachineParam* self, char* text)
{
	int rv = FALSE;

	if (self->index >= 0 && self->index < self->plugin->client->numparameters_) {
		psy_LuaImport in;

		psy_lock_enter(self->plugin->lock);
		psy_luaimport_init(&in, self->plugin->script.L, self->plugin->client);
		if (psy_luaimport_open(&in, 0)) {
			const char* str;
			size_t l;

			getparam(&in, self->index, "display");
			psy_luaimport_pcall(&in, 1);
			str = lua_tolstring(self->plugin->script.L, -1, &l);
			psy_snprintf(text, 128, "%s", str);
			rv = TRUE;
		}
		psy_luaimport_dispose(&in);
		psy_lock_leave(self->plugin->lock);
		return rv;
	}
	return rv;
}

float luapluginparam_normvalue(psy_audio_LuaPluginMachineParam* self)
{
	psy_LuaImport in;
	lua_Number v;

	psy_lock_enter(self->plugin->lock);
	psy_luaimport_init(&in, self->plugin->script.L, self->plugin->client);
	if (psy_luaimport_open(&in, 0)) {
		getparam(&in, self->index, "norm");
		psy_luaimport_pcall(&in, 1);
		v = luaL_checknumber(self->plugin->script.L, -1);
	} else {
		v = 0;
	}
	psy_luaimport_dispose(&in);
	psy_lock_leave(self->plugin->lock);
	return (float)v;
}

void luapluginparam_range(psy_audio_LuaPluginMachineParam* self, intptr_t* minval,
	intptr_t* maxval)
{
	psy_LuaImport in;
	lua_Number v;

	psy_lock_enter(self->plugin->lock);
	psy_luaimport_init(&in, self->plugin->script.L, self->plugin->client);
	if (psy_luaimport_open(&in, 0)) {
		getparam(&in, self->index, "range");
		psy_luaimport_pcall(&in, 3);
		v = luaL_checknumber(self->plugin->script.L, -3);
		v = luaL_checknumber(self->plugin->script.L, -2);
		*minval = 0;
		v = luaL_checknumber(self->plugin->script.L, -1);
		*maxval = (int)v;
	} else {
		*minval = 0;
		*maxval = 65535;
	}
	psy_luaimport_dispose(&in);
	psy_lock_leave(self->plugin->lock);
}

int luapluginparam_type(psy_audio_LuaPluginMachineParam* self)
{
	psy_LuaImport in;
	lua_Integer v;

	psy_lock_enter(self->plugin->lock);
	psy_luaimport_init(&in, self->plugin->script.L, self->plugin->client);
	if (psy_luaimport_open(&in, 0)) {
		getparam(&in, self->index, "mpf");
		psy_luaimport_pcall(&in, 1);
		v = luaL_checkinteger(self->plugin->script.L, -1);
	} else {
		v = MPF_STATE;
	}
	psy_luaimport_dispose(&in);
	psy_lock_leave(self->plugin->lock);
	return (int)v;
}

static int luaplugin_parse_machineinfo(psy_PsycleScript* self, psy_audio_MachineInfo* rv);

static int luaplugin_machineinfo(psy_PsycleScript* self, psy_audio_MachineInfo* rv)
{
	int err = 0;
	lua_getglobal(self->L, "psycle");
	if (lua_isnil(self->L, -1)) {
		err = 1; // throw std::runtime_error("Psycle not available.");
		return err;
	}
	lua_getfield(self->L, -1, "info");
	if (!lua_isnil(self->L, -1)) {
		int status = lua_pcall(self->L, 0, 1, 0);    
		if (status) {         
			const char* msg = lua_tostring(self->L, -1); 
			// ui::error(msg);
			err = 1; // throw std::runtime_error(msg);
			return err;
		}		
		luaplugin_parse_machineinfo(self, rv);
	} else {
		err = 1;
		// throw std::runtime_error("no info found");
	}
	return err;
}

int luaplugin_parse_machineinfo(psy_PsycleScript* self, psy_audio_MachineInfo* rv)
{  
	char* name = 0;
	char* vendor = 0;	
	int mode = psy_audio_MACHMODE_FX;
	int version = 0;
	int apiversion = 0;
	int noteon = 0;
	int err = 0;
	
	if (lua_istable(self->L, -1)) {
		size_t len;
		for (lua_pushnil(self->L); lua_next(self->L, -2);
				lua_pop(self->L, 1)) {
			const char* key = luaL_checklstring(self->L, -2, &len);
			if (strcmp(key, "vendor") == 0) {
				const char* value = luaL_checklstring(self->L, -1, &len);
				if (value) {
					vendor = strdup(value);
				}        
			} else
			if (strcmp(key, "name") == 0) {
				const char* value = luaL_checklstring(self->L, -1, &len);
				if (value) {
					name = strdup(value);
				}          
			} else
			if (strcmp(key, "mode") == 0) {
				int64_t value;

				if (lua_isnumber(self->L, -1) == 0) {
					err = 1;
					break;
					// throw std::runtime_error("info mode not a number"); 
				}			
				value = luaL_checkinteger(self->L, -1);
				switch (value) {
					case 0:
						mode = psy_audio_MACHMODE_GENERATOR;
					break;
					case 3:
						// mode = MACHMODE_HOST;
						mode = psy_audio_MACHMODE_FX;
					break;
					default:
						mode = psy_audio_MACHMODE_FX;
					break;
				}
			} else
			if (strcmp(key, "generator") == 0) {
				// deprecated, use mode instead
				int64_t value = luaL_checkinteger(self->L, -1);
				mode = (value == 1) ? psy_audio_MACHMODE_GENERATOR : psy_audio_MACHMODE_FX;
			} else
			if (strcmp(key, "version") == 0) {
				version = (int) luaL_checkinteger(self->L, -1);
			} else
			if (strcmp(key, "api") == 0) {
				apiversion = (int) luaL_checkinteger(self->L, -1);			
			} else
			if (strcmp(key, "noteon") == 0) {
				noteon = (int) luaL_checkinteger(self->L, -1);
				rv->flags = noteon;
			}
		}
	}	     
	// std::ostringstream s;
	// s << (result.mode == psy_audio_MACHMODE_GENERATOR ? "Lua instrument"
	//                                          : "Lua effect")
	//    << " by "
	//    << result.vendor;
	// result.desc = s.str();
	machineinfo_set(rv, vendor, "", 0, mode, name, name, (short) apiversion, 
		(short) version, psy_audio_LUA, "", 0, "", "Lua", "");
	free(name);
	free(vendor);
	return err;
}


/* prototypes */
static void generateaudio(psy_audio_LuaPlugin*, psy_audio_BufferContext*);
static void seqtick(psy_audio_LuaPlugin*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void newline(psy_audio_LuaPlugin*);
static void stop(psy_audio_LuaPlugin*);
static psy_audio_MachineInfo* info(psy_audio_LuaPlugin*);
static uintptr_t numparameters(psy_audio_LuaPlugin*);
static unsigned int numparametercols(psy_audio_LuaPlugin*);
static void dispose(psy_audio_LuaPlugin*);
static void reload(psy_audio_LuaPlugin*);
static int mode(psy_audio_LuaPlugin*);
static uintptr_t numinputs(psy_audio_LuaPlugin*);
static uintptr_t numoutputs(psy_audio_LuaPlugin*);
static void luamachine_newshareduserdata(lua_State*, const char* meta, void* ud, int self);
static void luaplugin_initmachine(psy_audio_LuaPlugin*);
static bool hasdirectmetaaccess(psy_audio_LuaPlugin*);
void psy_audio_luaplugin_getinfo(psy_audio_LuaPlugin*);
int psy_audio_plugin_luascript_exportcmodules(psy_PsycleScript*);
static int loadspecific(psy_audio_LuaPlugin*, psy_audio_SongFile*,
	uintptr_t slot);
static int savespecific(psy_audio_LuaPlugin*, psy_audio_SongFile*,
	uintptr_t slot);
static psy_dsp_amp_range_t amprange(psy_audio_LuaPlugin* self)
{
	return PSY_DSP_AMP_RANGE_VST;
}

int luascript_setmachine(lua_State*);
int luascript_terminal_output(lua_State*);

static const char* luamachine_meta = "psypluginmeta";

static int luamachine_open(lua_State*);
static int luamachine_create(lua_State*);
static int luamachine_gc(lua_State*);
static int luamachine_work(lua_State*);
static int luamachine_channel(lua_State*);
static int luamachine_getparam(lua_State*);
static int luamachine_setparameters(lua_State*);
static int luaL_orderednext(lua_State*);
static int luamachine_addparameters(lua_State*);
static int luamachine_setnumcols(lua_State*);
static int luamachine_setbuffer(lua_State*);

static psy_audio_MachineParam* parameter(psy_audio_LuaPlugin*, uintptr_t param);

static MachineVtable vtable;
static bool vtable_initialized = FALSE;

static const luaL_Reg psycle_methods[] = {
	{"setmachine", luascript_setmachine},
	{"output", luascript_terminal_output},
	{NULL, NULL}
};

static void vtable_init(psy_audio_LuaPlugin* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;
		vtable.amprange = (fp_machine_amprange)amprange;
		vtable.generateaudio = (fp_machine_generateaudio)generateaudio;
		vtable.seqtick = (fp_machine_seqtick)seqtick;
		vtable.newline = (fp_machine_newline)
			newline;
		vtable.stop = (fp_machine_stop)
			stop;
		vtable.info = (fp_machine_info)info;
		vtable.parameter = (fp_machine_parameter)parameter;
		vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		vtable.numparameters = (fp_machine_numparameters)numparameters;
		vtable.dispose =(fp_machine_dispose)dispose;
		vtable.reload = (fp_machine_reload)reload;
		vtable.numinputs = (fp_machine_numinputs)numinputs;
		vtable.numoutputs = (fp_machine_numoutputs)numoutputs;
		vtable.loadspecific = (fp_machine_loadspecific)loadspecific;
		vtable.savespecific = (fp_machine_savespecific)savespecific;
		vtable_initialized = TRUE;
	}
}
		
void psy_audio_luaplugin_init(psy_audio_LuaPlugin* self, psy_audio_MachineCallback* callback,
	const char* path)
{
	int err = 0;	

	self->plugininfo = 0;
	self->usenoteon = FALSE;
	psy_audio_custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	psy_audio_luaplugin_base(self)->vtable = &vtable;
	psy_audio_luapluginmachineparam_init(&self->parameter, self, UINTPTR_MAX);
	psyclescript_init(&self->script);
	if (err = psyclescript_load(&self->script, path)) {
		return;	
	}
	if (err = psyclescript_preparestate(&self->script, psycle_methods,
		self)) {
		return;
	}
	psy_audio_plugin_luascript_exportcmodules(&self->script);
	if (err = psyclescript_run(&self->script)) {
		return;
	}
	self->lock = psy_lock_allocinit();
	if (!self->lock) {
		return;
	}
	if (err = psyclescript_start(&self->script)) {
		return;
	}
	self->plugininfo = machineinfo_allocinit();
	if (luaplugin_machineinfo(&self->script, self->plugininfo) != 0) {
		psy_audio_luaplugin_getinfo(self);
	}
	free(self->plugininfo->modulepath);
	self->plugininfo->modulepath = strdup(path);
	self->usenoteon = self->plugininfo->flags;
	luaplugin_initmachine(self);	
	psy_audio_machine_seteditname(psy_audio_luaplugin_base(self),
		self->plugininfo->shortname);
}

void dispose(psy_audio_LuaPlugin* self)
{	
	if (self->plugininfo) {
		machineinfo_dispose(self->plugininfo);
		free(self->plugininfo);
		self->plugininfo = 0;
	}	
    psyclescript_dispose(&self->script);
	psy_audio_custommachine_dispose(&self->custommachine);
	psy_lock_dispose(self->lock);
	free(self->lock);
	psy_audio_luapluginmachineparam_dispose(&self->parameter);
}

void reload(psy_audio_LuaPlugin* self)
{	
	if (!psyclescript_empty(&self->script)) {
		char path[4096];
		psy_audio_MachineCallback* mcb;

		psy_snprintf(path, 4096, "%s", psyclescript_modulepath(&self->script));
		mcb = self->custommachine.machine.callback;		
		dispose(self);
		psy_audio_luaplugin_init(self, mcb, path);		
	}	
}

int psy_audio_plugin_luascript_test(const char* path, psy_audio_MachineInfo* machineinfo)
{		
	psy_PsycleScript script;
	int err = 0;	
	
	psyclescript_init(&script);
	err = psyclescript_load(&script, path);
	if (err) {
		return 0;
	}
	err = psyclescript_preparestate(&script, psycle_methods, 0);
	if (err) {
		return 0;
	}	
	psy_audio_plugin_luascript_exportcmodules(&script);
	err = psyclescript_run(&script);
	if (err)
	{
		return 0;
	}
	err = luaplugin_machineinfo(&script, machineinfo);
	if (err != 0) {
		psy_audio_LuaPlugin plugin;
		
		psy_audio_luaplugin_init(&plugin, NULL, path);
		if (psy_audio_machine_info(psy_audio_luaplugin_base(&plugin))) {
			machineinfo_copy(machineinfo,
				psy_audio_machine_info(psy_audio_luaplugin_base(&plugin)));
			err = 0;
		} else {
			psy_audio_machine_dispose(psy_audio_luaplugin_base(&plugin));
			return 0;
		}
		psy_audio_machine_dispose(psy_audio_luaplugin_base(&plugin));		
	}
	if (err) {
		return 0;
	}	
	psyclescript_dispose(&script);
	return 1;
}

int psy_audio_plugin_luascript_exportcmodules(psy_PsycleScript* self)
{
	psyclescript_require(self, "psycle.machine", luamachine_open);
	psyclescript_require(self, "psycle.array",
		psy_audio_luabind_array_open);
	psyclescript_require(self, "psycle.envelope",
		psy_audio_luabind_envelope_open);
	psyclescript_require(self, "psycle.dsp.filter",
		psy_audio_luabind_filter_open);
	psyclescript_require(self, "psycle.dsp.math",
		psy_audio_luabind_dspmath_open);
	psyclescript_require(self, "psycle.midi",
		psy_audio_luabind_midinotes_open);
	psyclescript_require(self, "psycle.osc",
		psy_audio_luabind_waveosc_open);
	return 1;
}

void generateaudio(psy_audio_LuaPlugin* self, psy_audio_BufferContext* bc)
{
	if (bc->numsamples > 0) {				
		psy_LuaImport in;

		psy_lock_enter(self->lock);
		psy_luaimport_init(&in, self->script.L, self->client);		
		if (psy_luaimport_open(&in, "work")) {
			self->client->bcshared_ = TRUE;
			self->client->bc = bc;
			lua_pushinteger(self->script.L, bc->numsamples);
			psy_luaimport_pcall(&in, 0);
			if (in.lasterr == 0) {
				lua_gc(self->script.L, LUA_GCSTEP, 5);
			} else {
				psy_audio_machine_output(psy_audio_luaplugin_base(self),
					in.errmsg);
				self->custommachine.machine.err = in.lasterr;
			}
		}
		psy_luaimport_dispose(&in);
		psy_lock_leave(self->lock);
    }	
}

void seqtick(psy_audio_LuaPlugin* self, uintptr_t channel,
	const psy_audio_PatternEvent* event)
{
	psy_LuaImport in;

	psy_lock_enter(self->lock);
	psy_luaimport_init(&in, self->script.L, self->client);
	if (psy_luaimport_open(&in, "seqtick")) {
		lua_pushinteger(self->script.L, channel);
		lua_pushinteger(self->script.L, event->note);
		lua_pushinteger(self->script.L, event->inst);
		lua_pushinteger(self->script.L, event->cmd);
		lua_pushinteger(self->script.L, event->parameter);
		psy_luaimport_pcall(&in, 0);
		if (in.lasterr == 0) {
			lua_gc(self->script.L, LUA_GCSTEP, 5);
		}
    }
	psy_luaimport_dispose(&in);
	psy_lock_leave(self->lock);
}

void newline(psy_audio_LuaPlugin* self)
{	
	psy_LuaImport in;
	
	psy_lock_enter(self->lock);
	psy_luaimport_init(&in, self->script.L, self->client);
	if (psy_luaimport_open(&in, "sequencertick")) {		
		psy_luaimport_pcall(&in, 0);		
    }
	psy_luaimport_dispose(&in);
	psy_lock_leave(self->lock);
}

void stop(psy_audio_LuaPlugin* self)
{
	psy_LuaImport in;

	psy_lock_enter(self->lock);
	psy_luaimport_init(&in, self->script.L, self->client);
	if (psy_luaimport_open(&in, "stop")) {
		psy_luaimport_pcall(&in, 0);
	}
	psy_luaimport_dispose(&in);
	psy_lock_leave(self->lock);
}

psy_audio_MachineInfo* info(psy_audio_LuaPlugin* self)
{
	return self->plugininfo;
}

unsigned int numparametercols(psy_audio_LuaPlugin* self)
{
	return self->client ? self->client->numcols_ : 0;
}

uintptr_t numparameters(psy_audio_LuaPlugin* self)
{
	return self->client ? self->client->numparameters_ : 0;
}


int mode(psy_audio_LuaPlugin* self)
{
	return psy_audio_MACHMODE_FX;		
}

uintptr_t numinputs(psy_audio_LuaPlugin* self)
{
	if (info(self)) {
		return self->plugininfo->mode == psy_audio_MACHMODE_FX ? 2 : 0;
	} else {
		return 0;
	}
}

uintptr_t numoutputs(psy_audio_LuaPlugin* self)
{
	if (info(self)) {
		return 2;
	} else {
		return 0;
	}
}

// global psycle methods
int luascript_setmachine(lua_State* L)
{
	psy_audio_LuaPlugin* proxy;
	psy_audio_LuaMachine** ud;

	lua_getglobal(L, "psycle");
	lua_getfield(L, -1, "__self");
	proxy = *(psy_audio_LuaPlugin**)luaL_checkudata(L, -1, "psyhostmeta");
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_getfield(L, 1, "__self");	
	ud = (psy_audio_LuaMachine**) luaL_checkudata(L, -1, "psypluginmeta");	
	if (*ud && proxy) {
		proxy->client = *ud;
	}
	lua_pushvalue(L, 1);
	if (proxy) {
		psyclescript_register_weakuserdata(L, proxy->client);
	}
	lua_setfield(L, 2, "proxy");
	return 0;
}

int luascript_terminal_output(lua_State* L)
{	
	psy_audio_LuaPlugin* proxy;
	int n = lua_gettop(L);  // number of arguments 
	
	lua_getglobal(L, "psycle");	
	lua_getfield(L, -1, "__self");
	proxy = *(psy_audio_LuaPlugin**)luaL_checkudata(L, -1, "psyhostmeta");
	lua_pop(L, 1);
	lua_pop(L, 1);
	if (lua_isboolean(L, 1)) {
		const char* out = 0;
		int v = lua_toboolean(L, 1);
		if (v==1) out = "true"; else out = "false";		
		psy_audio_machine_output(&proxy->custommachine.machine, out);
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
			"tostring" " must return a string to " "print");
			lua_pop(L, 1);  /* pop result */
			psy_audio_machine_output(&proxy->custommachine.machine, s);
		}
	}
	return 0;
}

// lua machine bind
int luamachine_open(lua_State *L)
{
	static const luaL_Reg methods[] = {
		{"new", luamachine_create},
		{"channel", luamachine_channel},
		{"setparameters", luamachine_setparameters},
		{"addparameters", luamachine_addparameters},
		{"setnumcols", luamachine_setnumcols},
		{"machinework", luamachine_work},
		{"setbuffer", luamachine_setbuffer},
		{NULL, NULL}
	  };
	psyclescript_open(L, luamachine_meta, methods, 
		luamachine_gc, 0);  
  return 1;
}

void luamachine_newshareduserdata(lua_State* L, const char* meta, void* ud, int self)
{
	int n;
	psy_audio_LuaMachine** p;

	lua_pushvalue(L, self);
	n = lua_gettop(L);
	lua_newtable(L);  // new
	lua_pushvalue(L, self);
	lua_setmetatable(L, -2);
	lua_pushvalue(L, self);
	lua_setfield(L, self, "__index");		
	p = (void*)lua_newuserdata(L, sizeof(psy_audio_LuaMachine*));	
	*p = ud;
	luaL_getmetatable(L, luamachine_meta);
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "__self");
	lua_remove(L, n);
	psyclescript_register_weakuserdata(L, p);
}

int luamachine_create(lua_State* L)
{	
	int n;

	n = lua_gettop(L);
	if (n == 1) {
		psy_audio_LuaMachine* luamachine;

		luamachine = malloc(sizeof(psy_audio_LuaMachine));
		psy_audio_luamachine_init(luamachine);		
		luamachine_newshareduserdata(L, luamachine_meta, luamachine, 1);
		lua_newtable(L);
		lua_setfield(L, -2, "params");
	} else
	if (n == 2) {
		psy_audio_LuaPlugin* host = 0;
		psy_audio_LuaMachine* ud = 0;
		psy_audio_Machine* mac = 0;
		if (lua_isnumber(L, 2)) {
			int idx = (int) luaL_checkinteger(L, 2);
			if (idx < 0) {
				return luaL_error(L, "negative index not allowed");
			}
			host = (psy_audio_LuaPlugin*)psyclescript_host(L);
			if (!host) {
				return luaL_error(L, "host not found");
			}
			mac = psy_audio_machines_at(psy_audio_machine_machines(
				psy_audio_luaplugin_base(host)), idx);
			if (mac) {
				ud = malloc(sizeof(psy_audio_LuaMachine));
				psy_audio_luamachine_init_shared(ud, mac);
				luamachine_newshareduserdata(L, luamachine_meta, ud, 1);
				lua_newtable(L);
				lua_setfield(L, -2, "params");
			}
		} else {
			size_t len;
			const char* plug_name = luaL_checklstring(L, 2, &len);
			psy_audio_LuaMachine* ud = 0;
			psy_audio_Machine* machine;
			psy_audio_LuaPlugin* host = 0;
			psy_audio_MachineFactory* machinefactory;

			host = (psy_audio_LuaPlugin*)psyclescript_host(L);
			if (!host) {
				return luaL_error(L, "host not found");
			}
			machinefactory = psy_audio_machine_machinefactory(psy_audio_luaplugin_base(host));
			if (machinefactory) {
				machine = psy_audio_machinefactory_makemachine(machinefactory, psy_audio_PLUGIN,
					plug_name, psy_INDEX_INVALID);
				if (machine) {
					ud = malloc(sizeof(psy_audio_LuaMachine));
					psy_audio_luamachine_init_machine(ud, machine);
					luamachine_newshareduserdata(L, luamachine_meta, ud, 1);
					lua_newtable(L);
					lua_setfield(L, -2, "params");
				} else {
					luaL_error(L, "plugin not found error");
				}
			} 
		}
	}
	return 1;
}

void psy_audio_luamachine_dispose(psy_audio_LuaMachine* self)
{
	if (self->machine && !luamachine_shared(self)) {
		machine_base_dispose(self->machine);
		free(self->machine);
		self->machine = 0;
	}
	if (self->bcshared_ == FALSE) {
		if (self->bc && self->bc->output) {
			psy_audio_buffer_dispose(self->bc->output);
		}
		free(self->bc);
	}
}

int luamachine_gc(lua_State* L)
{
	psy_audio_LuaMachine** ud = (psy_audio_LuaMachine**)
		luaL_checkudata(L, 1, luamachine_meta);
	psy_audio_luamachine_dispose(*ud);
	free(*ud);
	return 0;
}

int luamachine_setparameters(lua_State* L)
{
	psy_audio_LuaMachine* self;

	self = psyclescript_checkself(L, 1, luamachine_meta);
	luamachine_setnumparameters(self, (uintptr_t)lua_rawlen(L, 2));
	lua_pushvalue(L, 1);
	lua_pushvalue(L, 2);
	lua_setfield(L, -2, "params");
	lua_pushvalue(L, 1);
	return 1;
}

int luamachine_work(lua_State* L)
{
	psy_audio_LuaMachine* self;
	uintptr_t numsamples;

	self = psyclescript_checkself(L, 1, luamachine_meta);
	numsamples = (uintptr_t) luaL_checkinteger(L, 2);
	if (self->bc) {
		self->bc->numsamples = numsamples;
		psy_audio_machine_work(self->machine, self->bc);
	}
	return 0;
}

int luamachine_addparameters(lua_State* L)
{
	psy_audio_LuaMachine* self;
	size_t len;
	
	self = psyclescript_checkself(L, 1, luamachine_meta);		
	lua_getfield(L, 1, "params");
	lua_pushvalue(L, 2);
	luaL_checktype(L, 2, LUA_TTABLE);
	// t:opairs()
	lua_getfield(L, 2, "opairs");
	lua_pushvalue(L, -2);
	lua_call(L, 1, 2);
	// iter, self (t), nil
	for (lua_pushnil(L); luaL_orderednext(L);)
	{
		luaL_checklstring(L, -2, &len);
		lua_pushvalue(L, -2);
		lua_setfield(L, -2, "id_");
		lua_rawseti(L, 3, lua_rawlen(L, 3) + 1); // params[#params+1] = newparam
	}
	luamachine_setnumparameters(self, (uintptr_t)lua_rawlen(L, 3));
	lua_pushvalue(L, 1);
	return 1;
}

int luamachine_setnumcols(lua_State* L)
{
	psy_audio_LuaMachine* self;

	self = psyclescript_checkself(L, 1, luamachine_meta);
	self->numcols_ = (int) luaL_checkinteger(L, 2);
	lua_pushvalue(L, 1);
	return 1;
}

int luamachine_setbuffer(lua_State* L)
{	
	int n = lua_gettop(L);		
	if (n == 2) {
		psy_audio_LuaMachine* self;
		psy_audio_Buffer* buffer;
		psy_audio_BufferContext* bc;
		psy_List* arrays = 0;
		psy_List* p;
		int c;
		uintptr_t numsamples;

		self = psyclescript_checkself(L, 1, luamachine_meta);
		luaL_checktype(L, 2, LUA_TTABLE);
		lua_pushvalue(L, 2);
			
		for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
			psy_audio_Array* v = *(psy_audio_Array**)luaL_checkudata(L, -1, luaarraybind_meta);
			numsamples = psy_audio_array_len(v);
			psy_list_append(&arrays, v);
		}
		if (arrays) {
			buffer = psy_audio_buffer_allocinit(psy_list_size(arrays));
			for (c = 0, p = arrays; p != NULL; psy_list_next(&p), ++c) {
				psy_audio_Array* v;

				v = (psy_audio_Array*)p->entry;
				buffer->samples[c] = psy_audio_array_data(v);
			}
			if (self->bc && self->bcshared_ == FALSE) {
				if (self->bc->output) {
					psy_audio_buffer_dispose(self->bc->output);
				}
				free(self->bc);
			}
			bc = (psy_audio_BufferContext*)malloc(sizeof(psy_audio_BufferContext));
			psy_audio_buffercontext_init(bc, 0, buffer, buffer, numsamples, 64);
			self->bc = bc;
			self->bcshared_ = FALSE;
			psy_list_free(arrays);
		}
	} else {
		luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
	}
	lua_pushvalue(L, 1);
	return 1;
}

// c iterator for orderedtable
static int luaL_orderednext(lua_State* L)
{
	luaL_checkany(L, -1);                 // previous key
	luaL_checktype(L, -2, LUA_TTABLE);    // self
	luaL_checktype(L, -3, LUA_TFUNCTION); // iterator
	lua_pop(L, 1);                        // pop the key since
										  // opair doesn't use it
	// iter(self)
	lua_pushvalue(L, -2);
	lua_pushvalue(L, -2);
	lua_call(L, 1, 2);

	if (lua_isnil(L, -2)) {
		lua_pop(L, 2);
		return 0;
	}
	return 2;
}

int luamachine_channel(lua_State* L)
{
	int n;
	
	n = lua_gettop(L);			
	if (n == 2) {		
		psy_audio_Array ** udata;
		intptr_t idx;

		psy_audio_LuaMachine* self;				
		self = psyclescript_checkself(L, 1, luamachine_meta);
		if (self) {
			idx = (intptr_t) luaL_checkinteger(L, 2);
			udata = (psy_audio_Array **)lua_newuserdata(L, sizeof(psy_audio_Array *));
			*udata = malloc(sizeof(psy_audio_Array));
			psy_audio_array_init_shared(*udata, self->bc->output->samples[idx],
				self->bc->numsamples);
			luaL_setmetatable(L, luaarraybind_meta);
		} else {
			luaL_error(L, "no self");
			return 0;
		}
	} else {
		luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
		return 0;
	}	
	return 1;
}

void psy_audio_luamachine_init(psy_audio_LuaMachine* self)
{	
	psy_audio_CustomMachine* custommachine;
		
	custommachine = (psy_audio_CustomMachine*) malloc(sizeof(psy_audio_CustomMachine));
	if (custommachine) {		
		psy_audio_custommachine_init(custommachine, NULL);
		self->machine = &custommachine->machine;
	} else {
		self->machine = 0;
	}
	self->bc = 0;
	self->numparameters_ = 0;
	self->numcols_ = 0;
	self->numprograms_ = 0;
	self->shared_ = FALSE;
	self->bcshared_ = FALSE;
}

void psy_audio_luamachine_init_shared(psy_audio_LuaMachine* self,
	psy_audio_Machine* machine)
{
	self->machine = machine;
	self->bc = 0;
	self->numparameters_ = 0;
	self->numcols_ = 0;
	self->numprograms_ = 0;
	self->shared_ = TRUE;
	self->bcshared_ = FALSE;
}

void psy_audio_luamachine_init_machine(psy_audio_LuaMachine* self,
	psy_audio_Machine* machine)
{
	self->machine = machine;
	self->bc = 0;
	self->numparameters_ = 0;
	self->numcols_ = 0;
	self->numprograms_ = 0;
	self->shared_ = FALSE;
	self->bcshared_ = FALSE;
}

int luamachine_getparam(lua_State* L)
{
	int n;
	
	n = lua_gettop(L);
	if (n == 2) {
		psy_audio_LuaMachine* self;
		lua_Unsigned len;
		lua_Unsigned i;
		const char* search;
		const char* id;

		self = psyclescript_checkself(L, 1, luamachine_meta);
		search = luaL_checkstring(L, 2);
		lua_getfield(L, -2, "params");
		len = lua_rawlen(L, -1);
		for (i = 1; i <= len; ++i) {
			lua_rawgeti(L, 3, i);
			lua_getfield(L, -1, "id");
			lua_pushvalue(L, -2);
			lua_pcall(L, 1, 1, 0);
			id = luaL_checkstring(L, -1);
			if (strcmp(id, search) == 0) {
				lua_pop(L, 1);
				return 1;
			}
			lua_pop(L, 1);
		}
		lua_pushnil(L);
		return 1;
	} else {
		luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
	}
	return 0;
}

void getparam(psy_LuaImport* import, uintptr_t idx, const char* method)
{   
	lua_State* L = import->L_;     
	if (idx < 0) {       
	//   throw std::runtime_error("index less then zero");
	}    
	psy_luaimport_findweakuserdata(import);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
	// throw std::runtime_error("no proxy found");
	}
	lua_getfield(L, -1, "params");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 2);       
	    // throw std::runtime_error("params not found");
	}    
	lua_rawgeti(L, -1, idx + 1);    
	lua_getfield(L, -1, method);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 4);
	// throw std::runtime_error("no param found");
	}    
	lua_replace(L, -4);
	lua_replace(L, -2);                     
 }

void luaplugin_initmachine(psy_audio_LuaPlugin* self)
{
	//try {
	psy_LuaImport in;

	psy_luaimport_init(&in, self->script.L, self->client);
	if (psy_luaimport_open(&in, "init")) {
		lua_pushinteger(self->script.L, (lua_Integer)
			psy_audio_machine_samplerate(psy_audio_luaplugin_base(self)));
		psy_luaimport_pcall(&in, 0);
		if (in.lasterr == 0) {
			lua_gc(self->script.L, LUA_GCSTEP, 5);
		} else {
			psy_audio_machine_output(psy_audio_luaplugin_base(self), in.errmsg);
		}
	} else {
		psy_audio_machine_output(psy_audio_luaplugin_base(self), "no init found");
	}
	psy_luaimport_dispose(&in);	
	//}
	//catch (std::exception & e) {
	//	std::string msg = std::string("LuaProxy Init Errror.") + e.what();
	//	ui::alert(msg);
	//	throw std::runtime_error(msg.c_str());
	// }
}

bool hasdirectmetaaccess(psy_audio_LuaPlugin* self)
{	
	bool rv = FALSE;

	lua_getglobal(self->script.L, "psycle");
	if (lua_isnil(self->script.L, -1)) {
		lua_pop(self->script.L, 1);
	}
	else {
		lua_getfield(self->script.L, -1, "info");
		rv = !lua_isnil(self->script.L, -1);
		lua_pop(self->script.L, 2);
	}
	return rv;	
}

// old script
void psy_audio_luaplugin_getinfo(psy_audio_LuaPlugin* self)
{
	psy_LuaImport in;

	psy_luaimport_init(&in, self->script.L, self->client);
	if (psy_luaimport_open(&in, "info")) {
		psy_luaimport_pcall(&in, 1);
		if (in.lasterr == 0) {
			luaplugin_parse_machineinfo(&self->script, self->plugininfo);
		}
		else {
			machineinfo_dispose(self->plugininfo);
			free(self->plugininfo);
			self->plugininfo = 0;
		}
	}
	psy_luaimport_dispose(&in);
}

psy_audio_MachineParam* parameter(psy_audio_LuaPlugin* self, uintptr_t param)
{
	self->parameter.index = param;
	return &self->parameter.custommachineparam.machineparam;
}

int loadspecific(psy_audio_LuaPlugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	uint32_t numparams;
	uintptr_t num;
	uint32_t i;
	psy_Table ids;
	psy_Table vals;
	int status;

	if (status = psyfile_read(songfile->file, &size, sizeof(size))) {
		return status;
	}
	if (status = psyfile_read(songfile->file, &numparams, sizeof(numparams))) {
		return status;
	}
	//Read vals and names to do SetParameter.
	//It is done this way to allow parameters to change without disrupting the loader.
	psy_table_init(&ids);
	psy_table_init(&vals);
	psy_lock_enter(self->lock);
	for (i = 0; i < numparams; i++) {
		int temp;
		if (status = psyfile_read(songfile->file, &temp, sizeof(temp))) {
			psy_table_dispose(&vals);
			psy_table_dispose(&ids);
			return status;
		}
		psy_table_insert(&vals, psy_table_size(&vals), (void*)(intptr_t)temp);
	}
	for (i = 0; i < numparams; i++) {
		char id[1024];

		if (status = psyfile_readstring(songfile->file, id, 1024)) {
			return status;
		}
		psy_table_insert_strhash(&ids, id, (void*)(uintptr_t)i);
	}
	num = self->client->numparameters_;
	for (i = 0; i < num; ++i) {		
		const char* id = luaplugin_id(self, i);		
		if (psy_table_at_strhash(&ids, id)) {
			int idx = (int)(intptr_t)psy_table_at_strhash(&ids, id);
			psy_audio_MachineParam* param;

			param = psy_audio_machine_tweakparameter(&self->custommachine.machine, i);
			if (param) {
				intptr_t minval;
				intptr_t maxval;
				float value;
				int temp;

				temp = (int)(intptr_t)psy_table_at(&vals, idx);
				psy_audio_machineparam_range(param, &minval, &maxval);
				value = ((maxval - minval) != 0)
					? (temp) / (float)(maxval - minval)
					: 0.f;
				psy_audio_machineparam_tweak(param, value);
			}
		}
	}
	psy_table_dispose(&vals);
	psy_table_dispose(&ids);
	psy_lock_leave(self->lock);
	if (psyfile_skip(songfile->file, size - sizeof(numparams) - (numparams * sizeof(uint32_t))) == -1) {
		return PSY_ERRFILE;
	}
	return PSY_OK;
}

int savespecific(psy_audio_LuaPlugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	int status;

	//if (proxy_.prsmode() == MachinePresetType::NATIVE) {
		uint32_t count = (uint32_t)self->client->numparameters_;
		uint32_t size2 = 0;
		//unsigned char* pData = 0;
		//try
		//{
			//size2 = proxy().GetData(&pData, false);
		//}
		//catch (const std::exception&)
		//{
			// data won't be saved
		//}
		uint32_t size = size2 + sizeof(count) + sizeof(int) * count;
		uint32_t i;

		assert(self);

		for (i = 0; i < count; i++) {
			const char* id = luaplugin_id(self, i);
			size += (uint32_t)psy_strlen(id) + 1;
		}
		if (status = psyfile_write(songfile->file, &size, sizeof(size))) {
			return status;
		}
		if (status = psyfile_write(songfile->file, &count, sizeof(count))) {
			return status;
		}
		for (i = 0; i < count; i++)
		{
			psy_audio_MachineParam* param;

			param = psy_audio_machine_tweakparameter(&self->custommachine.machine, i);
			if (param) {
				int32_t temp;

				temp = (int32_t) psy_audio_machineparam_scaledvalue(param);
				psyfile_write(songfile->file, &temp, sizeof(temp));
			}
		}
		// ids
		for (i = 0; i < count; i++) {
			const char* id = luaplugin_id(self, i);

			if (status = psyfile_writestring(songfile->file, id)) {
				return status;
			}
		}
		if (status = psyfile_write(songfile->file, &size2, sizeof(size2))) {
			return status;
		}
		if (size2)
		{
			// psyfile_writes(songfile->file, pData, size2); // Number of parameters
			// zapArray(pData);
		}			 
/*else {
		try {
			UINT count(GetNumParams());
			unsigned char _program = 0;
			UINT size(sizeof _program + sizeof count);
			UINT chunksize(0);
			unsigned char* pData(0);
			bool b = proxy_.prsmode() == MachinePresetType::CHUNK;
			if (b)
			{
				count = 0;
				chunksize = proxy().GetData(&pData, true);
				size += chunksize;
			} else
			{
				size += sizeof(float) * count;
			}
			pFile->Write(&size, sizeof size);
			// _program = static_cast<unsigned char>(GetProgram());
			pFile->Write(&_program, sizeof _program);
			pFile->Write(&count, sizeof count);

			if (b)
			{
				pFile->Write(pData, chunksize);
			} else
			{
				for (UINT i(0); i < count; ++i)
				{
					float temp = proxy_.Val(i);
					pFile->Write(&temp, sizeof temp);
				}
			}
		}
		catch (...) {
		}
	}*/
	return PSY_OK;
}

const char* luaplugin_id(psy_audio_LuaPlugin* self, int index)
{
	const char* rv = NULL;
	psy_LuaImport in;

	psy_luaimport_init(&in, self->script.L, self->client);
	if (psy_luaimport_open(&in, 0)) {
		const char* str;
		size_t l = 0;

		getparam(&in, index, "id");
		psy_luaimport_pcall(&in, 1);
		str = lua_tolstring(self->script.L, -1, &l);
		rv = str;
	}
	psy_luaimport_dispose(&in);	
	return rv;
}
