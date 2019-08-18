// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(PATTERNS_H)
#define PATTERNS_H

#include <hashtbl.h>
#include <list.h>
#include "pattern.h"

typedef struct {	
	IntHashTable slots;
} Patterns;

void patterns_init(Patterns*);
void patterns_dispose(Patterns*);
void patterns_insert(Patterns*, unsigned int slot, Pattern*);
int patterns_append(Patterns*, Pattern*);
Pattern* patterns_at(Patterns*, unsigned int slot);
void patterns_enumerate(Patterns*, void* context, int (*enumproc)(void*, unsigned int, Pattern*));
#endif
