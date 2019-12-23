// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(LUAIMPORT_H)
#define LUAIMPORT_H

#include "psyclescript.h"

enum {
	psy_audio_LUAIMPORT_ERR_NOT_OPEN = 1,
	psy_audio_LUAIMPORT_ERR_NO_PROXY = 2,
	psy_audio_LUAIMPORT_ERR_PCALL = 3	
};

typedef struct {	
	psy_audio_PsycleScript script;
	int begin_top_, num_ret_;  
	lua_State* L_;
	// const LockIF* lh_;
	int is_open_;  
	void* target_;
	int lasterr;
	const char* errmsg;
} psy_audio_LuaImport;

void luaimport_init(psy_audio_LuaImport*, lua_State*, void* target);
void luaimport_dispose(psy_audio_LuaImport*);

void luaimport_settarget(psy_audio_LuaImport*, void* target);
int luaimport_open(psy_audio_LuaImport*, const char* method);
void luaimport_pcall(psy_audio_LuaImport*, int numret);
void luaimport_close(psy_audio_LuaImport*);

#endif
