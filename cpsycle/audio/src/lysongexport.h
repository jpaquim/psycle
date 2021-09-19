/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_LYEXPORT_H
#define psy_audio_LYEXPORT_H

#include "song.h"
#include "songio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Lilypond song export */

struct psy_audio_SongFile;

typedef struct psy_audio_LySongExport {	
	psy_audio_SongFile* songfile;
	PsyFile* fp;
	psy_audio_Song* song;
	bool bemol;
} psy_audio_LySongExport;

void psy_audio_lysongexport_init(psy_audio_LySongExport*,
	struct psy_audio_SongFile*);
void psy_audio_lysongexport_dispose(psy_audio_LySongExport*);

int psy_audio_lysongexport_save(psy_audio_LySongExport*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_LYEXPORT_H */
