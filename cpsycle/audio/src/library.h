// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(LIBRARY)
#define LIBRARY

typedef struct {
  void* module; // psycore (///\todo should not be void* on _WIN64)
  int err;
  char* path;
} Library;

void library_init(Library*);
void library_load(Library*, const char* path);
void* library_functionpointer(Library*, const char* name);
void library_dispose(Library*);

#endif