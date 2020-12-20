// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_DIR_H
#define psy_DIR_H

#include "../../detail/psydef.h"

#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

struct FileSearch;

typedef struct psy_Path {
	char* path;
	char* prefix;
	char* name;
	char* ext;
	char* filename;
} psy_Path;

void psy_path_init(psy_Path*, const char* path);
void psy_path_dispose(psy_Path*);
void psy_path_setpath(psy_Path*, const char* path);
void psy_path_setprefix(psy_Path*, const char* prefix);
void psy_path_setname(psy_Path*, const char* name);
void psy_path_setext(psy_Path*, const char* name);

INLINE const char* psy_path_full(const psy_Path* self)
{
	return self->path;
}

INLINE const char* psy_path_name(const psy_Path* self)
{
	return self->name;
}

INLINE const char* psy_path_prefix(const psy_Path* self)
{
	return self->prefix;
}

INLINE const char* psy_path_ext(const psy_Path* self)
{
	return self->ext;
}

INLINE const char* psy_path_filename(const psy_Path* self)
{
	return self->filename;
}

bool psy_path_hasprefix(psy_Path*);
bool psy_path_hasext(psy_Path*);
bool psy_path_hasname(psy_Path*);

typedef int (*psy_fp_findfile)(void* context, const char* path, int flag);

void psy_dir_enumerate(void* self, const char* path, const char* wildcard, int flag,
	psy_fp_findfile enumproc);
void psy_dir_enumerate_recursive(void* self, const char* path, const char* wildcard, int flag,
	psy_fp_findfile enumproc);
void psy_dir_findfile(const char* searchpath, const char* wildcard, char* filepath);
const char* psy_dir_config(void);
const char* psy_dir_home(void);
char* workdir(char* buffer);
const char* pathenv(void);
void setpathenv(const char* path);
void insertpathenv(const char* path);
void psy_dir_extract_path(const char* path, char* prefix, char* name, char* ext);
void psy_mkdir(const char* path);
bool psy_direxists(const char* path);
bool psy_filereadable(const char* fname);
psy_List* psy_drives(void);
psy_List* psy_directories(const char* path);

#ifdef __cplusplus
}
#endif

#endif /* psy_DIR_H */

