// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PRESET_H)
#define PRESET_H

#include <hashtbl.h>

typedef struct {
  char* name;
  psy_Table parameters;
} psy_audio_Preset;

void preset_init(psy_audio_Preset*);
void preset_dispose(psy_audio_Preset*);
psy_audio_Preset* preset_alloc(void);
psy_audio_Preset* preset_allocinit(void);

void preset_setname(psy_audio_Preset*, const char* name);
const char* preset_name(psy_audio_Preset*);
void preset_setvalue(psy_audio_Preset*, int numparam, int value);
int preset_value(psy_audio_Preset*, int numparam);

#endif
