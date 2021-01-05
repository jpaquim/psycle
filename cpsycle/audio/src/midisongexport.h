// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MIDISONGEXPORT_H
#define psy_audio_MIDISONGEXPORT_H

#include "pattern.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_SongFile;

typedef struct psy_audio_MidiSongExport {
	struct psy_audio_SongFile* songfile;	
} psy_audio_MidiSongExport;

void psy_audio_midisongexport_init(psy_audio_MidiSongExport*, struct psy_audio_SongFile*);
void psy_audio_midisongexport_dispose(psy_audio_MidiSongExport*);

int psy_audio_midisongexport_save(psy_audio_MidiSongExport*);

#ifdef __cplusplus
}
#endif


#endif /* psy_audio_MIDISONGEXPORT_H */
