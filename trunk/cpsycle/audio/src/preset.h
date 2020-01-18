// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PRESET_H
#define psy_audio_PRESET_H

#include <hashtbl.h>

typedef struct {
  char* name;
  psy_Table parameters;
} psy_audio_Preset;

void psy_audio_preset_init(psy_audio_Preset*);
void psy_audio_preset_dispose(psy_audio_Preset*);
psy_audio_Preset* psy_audio_preset_alloc(void);
psy_audio_Preset* psy_audio_preset_allocinit(void);

void psy_audio_preset_setname(psy_audio_Preset*, const char* name);
const char* psy_audio_preset_name(psy_audio_Preset*);
void psy_audio_preset_setvalue(psy_audio_Preset*, uintptr_t param, int value);
int psy_audio_preset_value(psy_audio_Preset*, uintptr_t param);

#endif /* psy_audio_PRESET_H */
