// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(DIR_H)
#define DIR_H

void psy_dir_enumerate(void* self, const char* path, const char* wildcard, int flag, void (*enumproc)(void*, const char* path, int flag));
void psy_dir_enumerate_recursive(void* self, const char* path, const char* wildcard, int flag, void (*enumproc)(void*, const char* path, int flag));
void psy_dir_findfile(const char* searchpath, const char* wildcard, char* filepath);
char* workdir(char* buffer);
const char* pathenv(void);
void setpathenv(const char* path);
void insertpathenv(const char* path);
void psy_dir_extract_path(const char* path, char* prefix, char* name, char* ext);

#endif

