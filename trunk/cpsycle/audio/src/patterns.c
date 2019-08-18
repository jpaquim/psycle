// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "patterns.h"
#include <stdlib.h>

static int OnEnumPattern(Patterns* self, unsigned int slot, Pattern* Pattern);

void patterns_init(Patterns* self)
{
	InitIntHashTable(&self->slots, 256);
}

void patterns_dispose(Patterns* self)
{	
	patterns_enumerate(self, self, OnEnumPattern);
	DisposeIntHashTable(&self->slots);
}

int OnEnumPattern(Patterns* self, unsigned int slot, Pattern* pattern)
{
	//pattern_dispose( ->dispose(pattern);
	free(pattern);
	return 1;
}

void patterns_insert(Patterns* self, unsigned int slot, Pattern* pattern)
{
	InsertIntHashTable(&self->slots, slot, pattern);
}

int patterns_append(Patterns* self, Pattern* pattern)
{
	int slot = 0;
	
	while (SearchIntHashTable(&self->slots, slot)) {
		++slot;
	}
	InsertIntHashTable(&self->slots, slot, pattern);	
	return slot;
}

Pattern* patterns_at(Patterns* self, unsigned int slot)
{
	return SearchIntHashTable(&self->slots, slot);
}

void patterns_enumerate(Patterns* self, void* context, unsigned int (*enumproc)(void*, unsigned int, Pattern*))
{
	Pattern* pattern;
	int slot;
	for (slot = 0; slot < 256; ++slot) {
		pattern = (Pattern*) SearchIntHashTable(&self->slots, slot);
		if (pattern) {
			if (!enumproc(context, slot, pattern)) {
				break;
			}
		}
	}
}
