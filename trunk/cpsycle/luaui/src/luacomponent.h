/* 
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2007-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_LUAUI_COMPONENT_H
#define psy_audio_LUAUI_COMPONENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>

int psy_luaui_component_open(lua_State* L);

extern const char* luacomponentbind_meta;

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_LUAUI_COMPONENT_H */
