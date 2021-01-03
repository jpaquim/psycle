// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PSY3SAVER_H
#define psy_audio_PSY3SAVER_H

#include "pattern.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_SongFile;

typedef struct psy_audio_PSY3Saver {
	struct psy_audio_SongFile* songfile;	
} psy_audio_PSY3Saver;

void psy_audio_psy3saver_init(psy_audio_PSY3Saver*, struct psy_audio_SongFile*);
int psy_audio_psy3saver_save(psy_audio_PSY3Saver*);

#ifdef __cplusplus
}
#endif


#endif /* psy_audio_PSY3SAVER_H */
