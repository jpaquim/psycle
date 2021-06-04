/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PSY3SAVER_H
#define psy_audio_PSY3SAVER_H

#include "pattern.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_SongFile;
struct PsyFile;
struct psy_audio_Song;
struct psy_audio_Instrument;

typedef struct psy_audio_PSY3Saver {
	/* references */
	struct psy_audio_SongFile* songfile;
	struct PsyFile* fp;
	struct psy_audio_Song* song;	
} psy_audio_PSY3Saver;

void psy_audio_psy3saver_init(psy_audio_PSY3Saver*, struct psy_audio_SongFile*);
void psy_audio_psy3saver_dispose(psy_audio_PSY3Saver*);

int psy_audio_psy3saver_save(psy_audio_PSY3Saver*);

/* save psycle instrument(.psins) */
int psy_audio_psy3saver_saveinstrument(psy_audio_PSY3Saver*,
	struct psy_audio_Instrument*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PSY3SAVER_H */
