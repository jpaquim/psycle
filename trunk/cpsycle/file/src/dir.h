// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DIR_H)
#define DIR_H

void dir_enum(void* self, const char* path, const char* wildcard, int flag, void (*enumproc)(void*, const char* path, int flag));
char* workdir(char* buffer);
const char* pathenv(void);
void setpathenv(const char* path);
void insertpathenv(const char* path);
void extract_path(const char* path, char* prefix, char* name, char* ext);

#endif
