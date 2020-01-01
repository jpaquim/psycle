// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PRESETS_H)
#define PRESETS_H

#include "preset.h"
#include <list.h>

typedef struct {
	psy_List* container;
} psy_audio_Presets;

void presets_init(psy_audio_Presets*);
void presets_dispose(psy_audio_Presets*);
psy_audio_Presets* presets_alloc(void);
psy_audio_Presets* presets_allocinit(void);
void presets_append(psy_audio_Presets*, psy_audio_Preset*);

#endif