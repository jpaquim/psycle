// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PRESETIO_H)
#define PRESETIO_H

#include "presets.h"

void presetsio_load(const char* path, psy_audio_Presets*);
void presetsio_save(const char* path, psy_audio_Presets*);

#endif
