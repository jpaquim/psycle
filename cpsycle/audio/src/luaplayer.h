/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_LUABIND_PLAYER_H
#define psy_audio_LUABIND_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>

struct psy_audio_Player;

int psy_audio_luabind_player_open(lua_State*);

void psy_audio_luabind_setplayer(struct psy_audio_Player*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_LUABIND_PLAYER_H */
