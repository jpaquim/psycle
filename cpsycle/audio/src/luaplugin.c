// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "luaplugin.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luaimport.h"
#include "array.h"
#include "luaarray.h"
#include "luaenvelope.h"
#include "luawaveosc.h"
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

static void generateaudio(psy_audio_LuaPlugin*, psy_audio_BufferContext*);
static void seqtick(psy_audio_LuaPlugin*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void sequencerlinetick(psy_audio_LuaPlugin*);
static psy_audio_MachineInfo* info(psy_audio_LuaPlugin*);
static uintptr_t numparameters(psy_audio_LuaPlugin*);
static unsigned int numparametercols(psy_audio_LuaPlugin*);
static int parameterlabel(psy_audio_LuaPlugin*, char* txt, uintptr_t param);
static int parametername(psy_audio_LuaPlugin*, char* txt, uintptr_t param);
static void parametertweak(psy_audio_LuaPlugin*, uintptr_t par, float val);
static int describevalue(psy_audio_LuaPlugin*, char* txt, uintptr_t param, int value);
static float parametervalue(psy_audio_LuaPlugin*, uintptr_t param);
static int parametertype(psy_audio_LuaPlugin*, uintptr_t param);
static void parameterrange(psy_audio_LuaPlugin*, uintptr_t param, int* minval,
	int* maxval);
static void dispose(psy_audio_LuaPlugin*);
static void reload(psy_audio_LuaPlugin*);
static int mode(psy_audio_LuaPlugin*);
static uintptr_t numinputs(psy_audio_LuaPlugin*);
static uintptr_t numoutputs(psy_audio_LuaPlugin*);
static void luamachine_newshareduserdata(lua_State*, const char* meta, void* ud, int self);
static void luaplugin_initmachine(psy_audio_LuaPlugin*);
static bool hasdirectmetaaccess(psy_audio_LuaPlugin*);
void psy_audio_luaplugin_getinfo(psy_audio_LuaPlugin*);
int psy_audio_plugin_luascript_exportcmodules(psy_audio_PsycleScript*);

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
static void getparam(psy_LuaImport*, int idx, const char* method);
static int luaL_orderednext(lua_State*);
static int luamachine_addparameters(lua_State*);
static int luamachine_setnumcols(lua_State*);
static int luamachine_setbuffer(lua_State*);

static MachineVtable vtable;
static int vtable_initialized = 0;

static const luaL_Reg psycle_methods[] = {
	{"setmachine", luascript_setmachine},
	{"output", luascript_terminal_output},
	{NULL, NULL}
};

static void vtable_init(psy_audio_LuaPlugin* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;
		vtable.generateaudio = (fp_machine_generateaudio) generateaudio;
		vtable.seqtick = (fp_machine_seqtick) seqtick;
		vtable.sequencerlinetick = (fp_machine_sequencerlinetick)
			sequencerlinetick;
		vtable.info = (fp_machine_info) info;
		vtable.parameterrange = (fp_machine_parameterrange) parameterrange;
		vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		vtable.numparameters = (fp_machine_numparameters) numparameters;
		vtable.parametername = (fp_machine_parametername) parametername;
		vtable.parameterlabel = (fp_machine_parameterlabel) parameterlabel;
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;		
		vtable.describevalue = (fp_machine_describevalue) describevalue;
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;
		vtable.parametertype = (fp_machine_parametertype) parametertype;
		vtable.dispose =(fp_machine_dispose) dispose;
		vtable.reload = (fp_machine_reload) reload;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable_initialized = 1;
	}
}
		
void psy_audio_luaplugin_init(psy_audio_LuaPlugin* self, MachineCallback callback,
	const char* path)
{
	int err = 0;	

	self->plugininfo = 0;
	custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	self->custommachine.machine.vtable = &vtable;
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
	self->lock = psy_audio_lock_allocinit();
	if (!self->lock) {
		return;
	}
	if (err = psyclescript_start(&self->script)) {
		return;
	}
	luaplugin_initmachine(self);
	self->plugininfo = machineinfo_allocinit();
	if (psyclescript_machineinfo(&self->script, self->plugininfo) != 0) {
		psy_audio_luaplugin_getinfo(self);
	}
	psy_audio_machine_seteditname(&self->custommachine.machine,
		self->plugininfo->ShortName);
}

void dispose(psy_audio_LuaPlugin* self)
{	
	if (self->plugininfo) {
		machineinfo_dispose(self->plugininfo);
		free(self->plugininfo);
		self->plugininfo = 0;
	}	
    psyclescript_dispose(&self->script);
	custommachine_dispose(&self->custommachine);
	psy_audio_lock_dispose(self->lock);
	free(self->lock);
}

void reload(psy_audio_LuaPlugin* self)
{	
	if (self->script.L && psyclescript_modulepath(&self->script)) {
		char path[4096];
		MachineCallback mcb;

		psy_snprintf(path, 4096, "%s", psyclescript_modulepath(&self->script));
		mcb = self->custommachine.machine.callback;		
		dispose(self);
		psy_audio_luaplugin_init(self, mcb, path);		
	}	
}

int psy_audio_plugin_luascript_test(const char* path, psy_audio_MachineInfo* machineinfo)
{		
	psy_audio_PsycleScript script;
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
	err = psyclescript_machineinfo(&script, machineinfo);
	if (err != 0) {
		psy_audio_LuaPlugin plugin;
		MachineCallback mcb;
		
		machinecallback_initempty(&mcb);
		psy_audio_luaplugin_init(&plugin, mcb, path);
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

int psy_audio_plugin_luascript_exportcmodules(psy_audio_PsycleScript* self)
{
	psyclescript_require(self, "psycle.machine", luamachine_open);
	psyclescript_require(self, "psycle.array",
		psy_audio_luabind_array_open);
	psyclescript_require(self, "psycle.envelope",
		psy_audio_luabind_envelope_open);
	psyclescript_require(self, "psycle.osc",
		psy_audio_luabind_waveosc_open);
	return 1;
}

void generateaudio(psy_audio_LuaPlugin* self, psy_audio_BufferContext* bc)
{
	if (bc->numsamples > 0) {				
		psy_LuaImport in;

		psy_audio_lock_enter(self->lock);
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
		psy_audio_lock_leave(self->lock);
    }	
}

void seqtick(psy_audio_LuaPlugin* self, uintptr_t channel,
	const psy_audio_PatternEvent* event)
{
	psy_LuaImport in;

	psy_audio_lock_enter(self->lock);
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
	psy_audio_lock_leave(self->lock);
}

void sequencerlinetick(psy_audio_LuaPlugin* self)
{	
	psy_LuaImport in;
	
	psy_audio_lock_enter(self->lock);
	psy_luaimport_init(&in, self->script.L, self->client);
	if (psy_luaimport_open(&in, "sequencertick")) {		
		psy_luaimport_pcall(&in, 0);		
    }
	psy_luaimport_dispose(&in);
	psy_audio_lock_leave(self->lock);
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

int parameterlabel(psy_audio_LuaPlugin* self, char* txt, uintptr_t param)
{
	int rv = FALSE;

	if (param >= 0 && param < self->client->numparameters_) {
		psy_LuaImport in;
		
		psy_audio_lock_enter(self->lock);
		psy_luaimport_init(&in, self->script.L, self->client);
		if (psy_luaimport_open(&in, 0)) {
			const char* str;
			int l;

			getparam(&in, param, "label");
			psy_luaimport_pcall(&in, 1);
			str = lua_tolstring(self->script.L, -1, &l);
			psy_snprintf(txt, 128, "%s", str);
			rv = TRUE;
		}
		psy_luaimport_dispose(&in);
		psy_audio_lock_leave(self->lock);
		return rv;
	}
	return rv;
}

int parametername(psy_audio_LuaPlugin* self, char* txt, uintptr_t param)
{
	int rv = FALSE;

	if (param >= 0 && param < self->client->numparameters_) {
		psy_LuaImport in;
		
		psy_audio_lock_enter(self->lock);
		psy_luaimport_init(&in, self->script.L, self->client);
		if (psy_luaimport_open(&in, 0)) {
			const char* str;
			int l;

			getparam(&in, param, "name");
			psy_luaimport_pcall(&in, 1);
			str = lua_tolstring(self->script.L, -1, &l);
			psy_snprintf(txt, 128, "%s", str);
			rv = TRUE;
		}
		psy_luaimport_dispose(&in);
		psy_audio_lock_leave(self->lock);
		return rv;
	}
	return rv;
}


void parametertweak(psy_audio_LuaPlugin* self, uintptr_t param, float val)
{
	// Parameter tweak range is [0..1]	
	psy_LuaImport in;	
	
	psy_audio_lock_enter(self->lock);
	psy_luaimport_init(&in, self->script.L, self->client);		
	if (psy_luaimport_open(&in, 0)) {
		getparam(&in, param, "setnorm");
		lua_pushnumber(self->script.L, val);
		psy_luaimport_pcall(&in, 0);
	}
	psy_luaimport_dispose(&in);
	psy_audio_lock_leave(self->lock);
}

int describevalue(psy_audio_LuaPlugin* self, char* txt, uintptr_t param,
	int value)
{ 	
	int rv = FALSE;

	if (param >= 0 && param < self->client->numparameters_) {
		psy_LuaImport in;
				
		psy_audio_lock_enter(self->lock);
		psy_luaimport_init(&in, self->script.L, self->client);
		if (psy_luaimport_open(&in, 0)) {
			const char* str;
			int l;

			getparam(&in, param, "display");			
			psy_luaimport_pcall(&in, 1);								
			str = lua_tolstring(self->script.L, -1, &l);
			psy_snprintf(txt, 128, "%s", str);
			rv = TRUE;
		}
		psy_luaimport_dispose(&in);
		psy_audio_lock_leave(self->lock);
		return rv;				
	}		
	return rv;
}

float parametervalue(psy_audio_LuaPlugin* self, uintptr_t param)
{
	psy_LuaImport in;
	lua_Number v;
	
	psy_audio_lock_enter(self->lock);
	psy_luaimport_init(&in, self->script.L, self->client);
	if (psy_luaimport_open(&in, 0)) {
		getparam(&in, param, "norm");
		psy_luaimport_pcall(&in, 1);
		v = luaL_checknumber(self->script.L, -1);		
	} else {
		v = 0;
	}
	psy_luaimport_dispose(&in);
	psy_audio_lock_leave(self->lock);
	return (float) v;
}

void parameterrange(psy_audio_LuaPlugin* self, uintptr_t param, int* minval,
	int* maxval)
{
	psy_LuaImport in;
	lua_Number v;
	
	psy_audio_lock_enter(self->lock);
	psy_luaimport_init(&in, self->script.L, self->client);
	if (psy_luaimport_open(&in, 0)) {
		getparam(&in, param, "range");
		psy_luaimport_pcall(&in, 2);		
		*minval = 0;
		v = luaL_checknumber(self->script.L, -1);
		*maxval = (int) v;
	} else {
		*minval = 0;
		*maxval = 65535;
	}
	psy_luaimport_dispose(&in);
	psy_audio_lock_leave(self->lock);
}

int parametertype(psy_audio_LuaPlugin* self, uintptr_t param)
{
	psy_LuaImport in;
	lua_Integer v;
	
	psy_audio_lock_enter(self->lock);
	psy_luaimport_init(&in, self->script.L, self->client);
	if (psy_luaimport_open(&in, 0)) {
		getparam(&in, param, "mpf");
		psy_luaimport_pcall(&in, 1);
		v = luaL_checkinteger(self->script.L, -1);
	} else {
		v = MPF_STATE;
	}
	psy_luaimport_dispose(&in);
	psy_audio_lock_leave(self->lock);
	return (int) v;	
}

int mode(psy_audio_LuaPlugin* self)
{
	return MACHMODE_FX;		
}

uintptr_t numinputs(psy_audio_LuaPlugin* self)
{
	if (info(self)) {
		return self->plugininfo->mode == MACHMODE_FX ? 2 : 0;
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
			LUA_QL("tostring") " must return a string to " LUA_QL("print"));
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
			mac = machines_at(psy_audio_machine_machines(
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
				machine = machinefactory_makemachine(machinefactory, MACH_PLUGIN,
					plug_name);
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
		machine_dispose(self->machine);
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
	luamachine_setnumparameters(self, lua_rawlen(L, 2));
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
	luamachine_setnumparameters(self, lua_rawlen(L, 3));
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
		int numsamples;

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
			for (c = 0, p = arrays; p != 0; p = p->next, ++c) {
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
			psy_audio_buffercontext_init(bc, 0, buffer, buffer, numsamples, 64, 0);
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
	MachineCallback callback;
		
	custommachine = (psy_audio_CustomMachine*) malloc(sizeof(psy_audio_CustomMachine));
	if (custommachine) {
		memset(&callback, 0, sizeof(MachineCallback));
		custommachine_init(custommachine, callback);
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
		size_t len;
		size_t i;
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

void getparam(psy_LuaImport* import, int idx, const char* method)
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
		lua_pushinteger(self->script.L,
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
			psyclescript_parse_machineinfo(&self->script, self->plugininfo);
		}
		else {
			machineinfo_dispose(self->plugininfo);
			free(self->plugininfo);
			self->plugininfo = 0;
		}
	}
	psy_luaimport_dispose(&in);
}
