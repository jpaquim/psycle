// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net


#if !defined(SKINFILE_H)
#define SKINFILE_H

#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

int skin_load(psy_Property*, const char* path);
int skin_loadpsh(psy_Property*, const char* path);
void skin_psh_values(const char* str, int maxcount, int* values);

#ifdef __cplusplus
}
#endif

#endif /* SKINFILE_H */
