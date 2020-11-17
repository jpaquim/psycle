// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PSY3LOADER_H
#define psy_audio_PSY3LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_SongFile;

typedef struct psy_audio_PSY3Loader {
	struct psy_audio_SongFile* songfile;
} psy_audio_PSY3Loader;

void psy_audio_psy3loader_init(psy_audio_PSY3Loader*,
	struct psy_audio_SongFile*);
int psy_audio_psy3loader_load(psy_audio_PSY3Loader*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PSY3LOADER_H */
