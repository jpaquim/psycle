/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_LUABIND_RESAMPLER_H
#define psy_audio_LUABIND_RESAMPLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>

#include "../../detail/psydef.h"

typedef struct LuaResampler {
	int rsvd;
} LuaResampler;

void luaresampler_init(LuaResampler*);
void luaresampler_dispose(LuaResampler*);

INLINE bool luaresampler_playing(const LuaResampler* self)
{
	return FALSE;
}

int luaresampler_work(LuaResampler*, int numSamples, float* pSamplesL,
	float* pSamplesR, void* master);

int psy_audio_luabind_resampler_open(lua_State*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_LUABIND_RESAMPLER_H */
