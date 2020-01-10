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
#include "custommachine.h"

#include <stdlib.h>
#include <string.h>

#include <windows.h>

static void generateaudio(psy_audio_LuaPlugin*, psy_audio_BufferContext*);
static void seqtick(psy_audio_LuaPlugin*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void sequencerlinetick(psy_audio_LuaPlugin*);
static psy_audio_MachineInfo* info(psy_audio_LuaPlugin*);
static void parametertweak(psy_audio_LuaPlugin*, uintptr_t par, int val);
static int describevalue(psy_audio_LuaPlugin*, char* txt, uintptr_t param, int value);
static int parametervalue(psy_audio_LuaPlugin*, uintptr_t param);
static void dispose(psy_audio_LuaPlugin*);
static int mode(psy_audio_LuaPlugin*);
static uintptr_t numinputs(psy_audio_LuaPlugin*);
static uintptr_t numoutputs(psy_audio_LuaPlugin*);

int luascript_setmachine(lua_State*);
int luascript_terminal_output(lua_State*);

static int luamachine_open(lua_State *L);
static const char* luamachine_meta = "psypluginmeta";
static int luamachine_create(lua_State* L);
static int luamachine_gc(lua_State* L);
static int luamachine_work(lua_State* L);
static int luamachine_channel(lua_State* L);

static const char* luaarraybind_meta = "array_meta";

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
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;		
		vtable.describevalue = (fp_machine_describevalue) describevalue;
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;
		vtable.dispose =(fp_machine_dispose) dispose;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable_initialized = 1;
	}
}
		
void luaplugin_init(psy_audio_LuaPlugin* self, MachineCallback callback,
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
	psyclescript_require(&self->script, "psycle.machine", luamachine_open);
	psyclescript_require(&self->script, "psycle.array",
		psy_audio_luabind_array_open);
	psyclescript_require(&self->script, "psycle.envelope",
		psy_audio_luabind_envelope_open);
	if (err = psyclescript_run(&self->script)) {
		return;
	}
	if (err = psyclescript_start(&self->script)) {
		return;
	}
	self->plugininfo = machineinfo_allocinit();
	psyclescript_machineinfo(&self->script, self->plugininfo);
	self->custommachine.machine.vtable->seteditname(
		&self->custommachine.machine, self->plugininfo->ShortName);
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
}

int plugin_luascript_test(const char* path, psy_audio_MachineInfo* machineinfo)
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
	psyclescript_require(&script, "psycle.machine", luamachine_open);
	err = psyclescript_run(&script);
	if (err) {
		return 0;
	}
	err = psyclescript_machineinfo(&script, machineinfo);
	if (err) {
		return 0;
	}	
	psyclescript_dispose(&script);
	return 1;
}

void generateaudio(psy_audio_LuaPlugin* self, psy_audio_BufferContext* bc)
{
	if (bc->numsamples > 0) {				
		psy_audio_LuaImport in;
		luaimport_init(&in, self->script.L, self->client);		
		if (luaimport_open(&in, "work")) {
			self->client->bc = bc;
			lua_pushinteger(self->script.L, bc->numsamples);
			luaimport_pcall(&in, 0);
			if (in.lasterr == 0) {
				lua_gc(self->script.L, LUA_GCSTEP, 5);
			} else {
				machine_output(&self->custommachine.machine, in.errmsg);	
			}
		}
		luaimport_dispose(&in);
    }	
}

void seqtick(psy_audio_LuaPlugin* self, uintptr_t channel,
	const psy_audio_PatternEvent* event)
{
	psy_audio_LuaImport in;
	luaimport_init(&in, self->script.L, self->client);
	if (luaimport_open(&in, "seqtick")) {
		lua_pushinteger(self->script.L, channel);
		lua_pushinteger(self->script.L, event->note);
		lua_pushinteger(self->script.L, event->inst);
		lua_pushinteger(self->script.L, event->cmd);
		lua_pushinteger(self->script.L, event->parameter);
		luaimport_pcall(&in, 0);
		if (in.lasterr == 0) {
			lua_gc(self->script.L, LUA_GCSTEP, 5);
		}
    }
	luaimport_dispose(&in);  
}

void sequencerlinetick(psy_audio_LuaPlugin* self)
{
	psy_audio_LuaImport in;
	luaimport_init(&in, self->script.L, self->client);
	if (luaimport_open(&in, "sequencertick")) {		
		luaimport_pcall(&in, 0);		
    }
	luaimport_dispose(&in);  
}

psy_audio_MachineInfo* info(psy_audio_LuaPlugin* self)
{
	return self->plugininfo;
}

void parametertweak(psy_audio_LuaPlugin* self, uintptr_t par, int val)
{

}

int describevalue(psy_audio_LuaPlugin* self, char* txt, uintptr_t param,
	int value)
{ 
	return 0;
}

int parametervalue(psy_audio_LuaPlugin* self, uintptr_t param)
{
	return 0;
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
	if (*ud) {
		proxy->client = *ud;
	}
	lua_pushvalue(L, 1);	
	psyclescript_register_weakuserdata(L, proxy->client);
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
		machine_output(&proxy->custommachine.machine, out);
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
			machine_output(&proxy->custommachine.machine, s);
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
		{NULL, NULL}
	  };
	psyclescript_open(L, luamachine_meta, methods, 
		luamachine_gc, 0);  
  return 1;
}

int luamachine_create(lua_State* L)
{	
	int n;
	int self = 1;
	psy_audio_CustomMachine* machine;
	psy_audio_LuaMachine** ud;
	MachineCallback callback;

	memset(&callback, 0, sizeof(MachineCallback));
	lua_pushvalue(L, self);
	n = lua_gettop(L);
	luaL_checktype(L, -1, LUA_TTABLE);  // self
	lua_newtable(L);  // new
	lua_pushvalue(L, self);
	lua_setmetatable(L, -2);
	lua_pushvalue(L, self);
	lua_setfield(L, self, "__index");		
	ud = (void*)lua_newuserdata(L, sizeof(psy_audio_LuaMachine*));	
	*ud = malloc(sizeof(psy_audio_LuaMachine));	
	machine = malloc(sizeof(psy_audio_CustomMachine));
	custommachine_init(machine, callback);
	(*ud)->machine = &machine->machine;
	(*ud)->bc = 0;
	luaL_getmetatable(L, luamachine_meta);
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "__self");
	lua_remove(L, n);
	psyclescript_register_weakuserdata(L, ud);
	return 1;
}

int luamachine_gc(lua_State* L)
{
	psy_audio_LuaMachine** ud = (psy_audio_LuaMachine**)
		luaL_checkudata(L, 1, luamachine_meta);	
	machine_dispose((*ud)->machine);
	free((*ud)->machine);
	free(*ud);
	return 0;
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
