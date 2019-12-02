// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PRESET_H)
#define PRESET_H

#include <hashtbl.h>

typedef struct {
  char* name;
  Table parameters;
} Preset;

void preset_init(Preset*);
void preset_dispose(Preset*);
Preset* preset_alloc(void);
Preset* preset_allocinit(void);

void preset_setname(Preset*, const char* name);
const char* preset_name(Preset*);
void preset_setvalue(Preset*, int numparam, int value);
int preset_value(Preset*, int numparam);

#endif
