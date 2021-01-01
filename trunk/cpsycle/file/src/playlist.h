// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(psy_PLAYLIST_H)
#define psy_PLAYLIST_H

// container
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_Playlist {
	psy_Property* recentsongs;
	psy_Property* recentfiles;
	char* path;
} psy_Playlist;

void psy_playlist_init(psy_Playlist*);
void psy_playlist_dispose(psy_Playlist*);

void psy_playlist_add(psy_Playlist*, const char* songfile);
void psy_playlist_clear(psy_Playlist*);
void psy_playlist_load(psy_Playlist*);
void psy_playlist_save(psy_Playlist*);
void psy_playlist_setpath(psy_Playlist*, const char* path);

#ifdef __cplusplus
}
#endif

#endif /* psy_PLAYLIST_H */
