// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SONGIO_H)
#define SONGIO_H

#include <properties.h>
#include "song.h"
#include <fileio.h>

/// song load and save
typedef struct SongFile {
	struct Song* song;
	struct PsyFile* file;
	Properties* workspaceproperties;
	int err;
} SongFile;

/// loads a song
///\return ui properties (coordinates, ...)
void songfile_load(struct SongFile*, const char* path);
/// saves a song
void songfile_save(struct SongFile*, const char* path);

#endif
