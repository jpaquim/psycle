/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(LUAUI_H)
#define LUAUI_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** PsyLuaUi
*/

typedef struct psy_LuaUi {
	int reserved;
} psy_LuaUi;

void psy_luaui_init(psy_LuaUi*);

#ifdef __cplusplus
}
#endif

#endif /* LUAUI_H */
