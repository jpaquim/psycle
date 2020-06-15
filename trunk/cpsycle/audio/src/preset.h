// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PRESET_H
#define psy_audio_PRESET_H

#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_Preset {
    char* name;
    psy_Table parameters;
    int32_t datasize;
    unsigned char* data;
} psy_audio_Preset;

void psy_audio_preset_init(psy_audio_Preset*);
void psy_audio_preset_dispose(psy_audio_Preset*);
psy_audio_Preset* psy_audio_preset_alloc(void);
psy_audio_Preset* psy_audio_preset_allocinit(void);
void psy_audio_preset_clear(psy_audio_Preset*);
void psy_audio_preset_setname(psy_audio_Preset*, const char* name);
const char* psy_audio_preset_name(psy_audio_Preset*);
void psy_audio_preset_setvalue(psy_audio_Preset*, uintptr_t param, intptr_t value);
intptr_t psy_audio_preset_value(psy_audio_Preset*, uintptr_t param);
void psy_audio_preset_setdatastruct(psy_audio_Preset* self,
    int num, const char* newname, int const* parameters, int size, void* newdata);
void psy_audio_preset_putdata(psy_audio_Preset*, int size, void* newdata);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PRESET_H */
