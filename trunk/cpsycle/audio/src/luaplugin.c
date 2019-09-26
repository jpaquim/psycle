// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "LuaPlugin.h"
#include "lauxlib.h"
#include "lualib.h"

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

static void work(LuaPlugin* self, BufferContext*);
static int hostevent(LuaPlugin* self, int const eventNr, int const val1, float const val2);
static void seqtick(LuaPlugin* self, int channel, const PatternEvent* event);
static void sequencertick(LuaPlugin* self);
static CMachineInfo* info(LuaPlugin* self);
static void parametertweak(LuaPlugin* self, int par, int val);
static int describevalue(LuaPlugin*, char* txt, int const param, int const value);
static int value(LuaPlugin*, int const param);
static void setvalue(LuaPlugin*, int const param, int const value);
static void dispose(LuaPlugin* self);
static int mode(LuaPlugin* self);
		
void luaplugin_init(LuaPlugin* self, MachineCallback callback, const char* path)
{
	machine_init(&self->machine, callback);
	self->L = luaL_newstate();   
	luaL_openlibs(self->L);	
}

void dispose(LuaPlugin* self)
{	
    lua_close(self->L);
	self->L = 0;
	machine_dispose(&self->machine);
}

CMachineInfo* luaplugin_psycle_test(const char* path)
{
	return 0;
}

void work(LuaPlugin* self, BufferContext* bc)
{
	
}

void seqtick(LuaPlugin* self, int channel, const PatternEvent* event)
{
	
}

int hostevent(LuaPlugin* self, int const eventNr, int const val1, float const val2)
{
	
}

void sequencertick(LuaPlugin* self)
{
	
}

CMachineInfo* info(LuaPlugin* self)
{

}

void parametertweak(LuaPlugin* self, int par, int val)
{

}

int describevalue(LuaPlugin* self, char* txt, int const param, int const value)
{ 
	return 0;
}

int value(LuaPlugin* self, int const param)
{

}

void setvalue(LuaPlugin* self, int const param, int const value)
{

}

int mode(LuaPlugin* self)
{
	if (self->machine.info(self) && (self->machine.info(self)->Flags == 3 )) {
		return MACHMODE_GENERATOR;
	} else {
		return MACHMODE_FX;
	}	
}
