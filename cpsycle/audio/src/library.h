// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_AUDIO_LIBRARY_H)
#define PSY_AUDIO_LIBRARY_H

typedef struct {
  void* module; // psycore (///\todo should not be void* on _WIN64)
  int err;
  char* path;
} psy_Library;

void psy_library_init(psy_Library*);
void psy_library_dispose(psy_Library*);
psy_Library* psy_library_alloc(void);
psy_Library* psy_library_allocinit(void);
void psy_library_deallocate(psy_Library*);

void psy_library_load(psy_Library*, const char* path);
void psy_library_unload(psy_Library*);
void* psy_library_functionpointer(psy_Library*, const char* name);
int psy_library_empty(psy_Library*);

#endif
