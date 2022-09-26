/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PRESET_H
#define psy_audio_PRESET_H

#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_Preset {
    char* name;
    psy_Table parameters;
    uintptr_t datasize;
    unsigned char* data;
    bool isfloat;
    int32_t magic;
    int32_t id;
    int32_t version;
} psy_audio_Preset;

void psy_audio_preset_init(psy_audio_Preset*);
void psy_audio_preset_dispose(psy_audio_Preset*);

psy_audio_Preset* psy_audio_preset_alloc(void);
psy_audio_Preset* psy_audio_preset_allocinit(void);

void psy_audio_preset_clear(psy_audio_Preset*);
void psy_audio_preset_setname(psy_audio_Preset*, const char* name);
const char* psy_audio_preset_name(psy_audio_Preset*);
void psy_audio_preset_setvalue(psy_audio_Preset*, uintptr_t param,
	intptr_t value);
intptr_t psy_audio_preset_value(psy_audio_Preset*, uintptr_t param);
void psy_audio_preset_setdatastruct(psy_audio_Preset* self,
    uintptr_t num, const char* newname, int const* parameters, uintptr_t size,
	void* newdata);
void psy_audio_preset_putdata(psy_audio_Preset*, uintptr_t size,
	void* newdata);

INLINE uintptr_t psy_audio_preset_numparameters(psy_audio_Preset* self)
{
    return psy_table_size(&self->parameters);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PRESET_H */
