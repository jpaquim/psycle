/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http ://psycle.sourceforge.net
*/

#if !defined(SKINFILE_H)
#define SKINFILE_H

/* container */
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

void skin_locate_pattern_skins(psy_Property*, const char* path);
void skin_locate_machine_skins(psy_Property*, const char* path);

int skin_load(psy_Property* psv, const char* path);
int skin_load_machine(psy_Property* psm, const char* path);
int skin_load_pattern_header(psy_Property* psh, const char* path);
int skin_load_psc(psy_Property* psc, const char* path);

void skin_psh_values(const char* str, intptr_t maxcount, intptr_t* values);

#ifdef __cplusplus
}
#endif

#endif /* SKINFILE_H */
