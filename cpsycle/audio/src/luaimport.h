// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_LUAIMPORT_H
#define psy_audio_LUAIMPORT_H

#include "psyclescript.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	psy_LUAIMPORT_ERR_NOT_OPEN = 1,
	psy_LUAIMPORT_ERR_NO_PROXY = 2,
	psy_LUAIMPORT_ERR_PCALL = 3	
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
} psy_LuaImport;

void psy_luaimport_init(psy_LuaImport*, lua_State*, void* target);
void psy_luaimport_dispose(psy_LuaImport*);

void psy_luaimport_settarget(psy_LuaImport*, void* target);
int psy_luaimport_open(psy_LuaImport*, const char* method);
void psy_luaimport_pcall(psy_LuaImport*, int numret);
void psy_luaimport_close(psy_LuaImport*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_LUAIMPORT_H */
