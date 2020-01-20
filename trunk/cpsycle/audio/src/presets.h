// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PRESETS_H
#define psy_audio_PRESETS_H

#include "preset.h"
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_List* container;
} psy_audio_Presets;

void psy_audio_presets_init(psy_audio_Presets*);
void psy_audio_presets_dispose(psy_audio_Presets*);
psy_audio_Presets* psy_audio_presets_alloc(void);
psy_audio_Presets* psy_audio_presets_allocinit(void);
void psy_audio_presets_append(psy_audio_Presets*, psy_audio_Preset*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PRESETS_H */
