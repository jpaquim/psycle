// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(FILE_H)
#define FILE_H

typedef struct {
	char* label;
	int flags;	
} fileitem;

void file_walk(const char* path, fileitem (*nextfile)(void*));
int file_exist(const char* path);


#endif