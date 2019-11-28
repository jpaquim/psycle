// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "LuaPlugin.h"
#include "lauxlib.h"
#include "lualib.h"
#include <windows.h>

static void work(LuaPlugin*, BufferContext*);
static int hostevent(LuaPlugin*, int const eventNr, int const val1, float const val2);
static void seqtick(LuaPlugin*, int channel, const PatternEvent*);
static void sequencerlinetick(LuaPlugin*);
static MachineInfo* info(LuaPlugin*);
static void parametertweak(LuaPlugin*, int par, int val);
static int describevalue(LuaPlugin*, char* txt, int param, int value);
static int parametervalue(LuaPlugin*, int param);
static void dispose(LuaPlugin*);
static int mode(LuaPlugin*);

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(LuaPlugin* self)
{
	if (!vtable_initialized) {
		vtable = *self->machine.vtable;		
		vtable.hostevent = hostevent;
		vtable.seqtick = seqtick;
		vtable.sequencerlinetick = sequencerlinetick;
		vtable.info = info;		
		vtable.parametertweak = parametertweak;		
		vtable.describevalue = describevalue;
		vtable.parametervalue = parametervalue;
		vtable.dispose = dispose;
		vtable_initialized = 1;
	}
}
		
void luaplugin_init(LuaPlugin* self, MachineCallback callback, const char* path)
{
	int err;

	machine_init(&self->machine, callback);
	vtable_init(self);
	self->machine.vtable = &vtable;
	psyclescript_init(&self->script);
	if (err = psyclescript_load(&self->script, path)) {
		return;	
	}
	if (err = psyclescript_preparestate(&self->script)) {
		return;
	}
	if (err = psyclescript_run(&self->script)) {
		return;
	}
	if (err = psyclescript_start(&self->script)) {
		return;
	}
}

void dispose(LuaPlugin* self)
{	
    psyclescript_dispose(&self->script);
	machine_dispose(&self->machine);
}

MachineInfo* luaplugin_psycle_test(const char* path)
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
	return 0;	
}

void sequencerlinetick(LuaPlugin* self)
{
	
}

MachineInfo* info(LuaPlugin* self)
{
	return 0;
}

void parametertweak(LuaPlugin* self, int par, int val)
{

}

int describevalue(LuaPlugin* self, char* txt, int param, int value)
{ 
	return 0;
}

int parametervalue(LuaPlugin* self, int param)
{
	return 0;
}

int mode(LuaPlugin* self)
{
	return MACHMODE_FX;		
}
