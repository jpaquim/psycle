// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PRESETIO_H
#define psy_audio_PRESETIO_H

#include "presets.h"

#ifdef __cplusplus
extern "C" {
#endif

#define psy_audio_PRESETIO_OK 0
#define psy_audio_PRESETIO_ERROR_OPEN 1
#define psy_audio_PRESETIO_ERROR_READ 2
#define psy_audio_PRESETIO_ERROR_UPTODATE 3
#define psy_audio_PRESETIO_ERROR_NEWVERSION 4
#define psy_audio_PRESETIO_ERROR_WRITEOPEN 5
#define psy_audio_PRESETIO_ERROR_WRITE 6


int psy_audio_presetsio_load(const char* path, psy_audio_Presets*,
	uintptr_t numparameters, uintptr_t datasizestruct, const char* pluginroot);
int psy_audio_presetsio_save(const char* path, psy_audio_Presets*);
const char* psy_audio_presetsio_statusstr(int status);

int psy_audio_presetio_savefxp(const char* path, psy_audio_Preset*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PRESETIO_H */
