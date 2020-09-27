// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PSY2_H
#define psy_audio_PSY2_H

#ifdef __cplusplus
extern "C" {
#endif

// PSY2 file load
// This loads the psy2 file format used in older psycle
// versions, where many songs were written in.
// File save in PSY2 was dropped with the new PSY3 format.
// Later psycle versions are still able to load and convert it to PSY3.
// loader based on
// Psycle Version : 1.66
// Document Version : 1.1.2
// Date : 1 - Dec - 2002
// svn/psycle/doc/file-format.description.doc

struct psy_audio_SongFile;

void psy_audio_psy2_load(struct psy_audio_SongFile*);
// no psy2 save implemented

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PSY2_H */
