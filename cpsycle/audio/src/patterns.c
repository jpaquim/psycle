// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patterns.h"
#include <stdlib.h>

static int OnEnumFreePattern(Patterns* self, unsigned int slot, Pattern* Pattern);

void patterns_init(Patterns* self)
{
	table_init(&self->slots);
	self->songtracks = 16;
	self->sharetracknames = 0;
}

void patterns_dispose(Patterns* self)
{	
	patterns_enumerate(self, self, OnEnumFreePattern);
	table_dispose(&self->slots);
}

void patterns_clear(Patterns* self)
{
	patterns_enumerate(self, self, OnEnumFreePattern);
	table_dispose(&self->slots);
	table_init(&self->slots);	
}

int OnEnumFreePattern(Patterns* self, unsigned int slot, Pattern* pattern)
{	
	pattern_dispose(pattern);
	free(pattern);
	return 1;
}

void patterns_insert(Patterns* self, unsigned int slot, Pattern* pattern)
{
	table_insert(&self->slots, slot, pattern);
}

int patterns_append(Patterns* self, Pattern* pattern)
{
	int slot = 0;
	
	while (table_at(&self->slots, slot)) {
		++slot;
	}
	table_insert(&self->slots, slot, pattern);	
	return slot;
}

Pattern* patterns_at(Patterns* self, unsigned int slot)
{
	return table_at(&self->slots, slot);
}

void patterns_enumerate(Patterns* self, void* context, int (*enumproc)(void*, unsigned int, Pattern*))
{
	Pattern* pattern;
	int slot;
	for (slot = 0; slot < 256; ++slot) {
		pattern = (Pattern*) table_at(&self->slots, slot);
		if (pattern) {
			if (!enumproc(context, slot, pattern)) {
				break;
			}
		}
	}
}

void patterns_erase(Patterns* self, unsigned int slot)
{

	table_remove(&self->slots, slot);
}

void patterns_remove(Patterns* self, unsigned int slot)
{
	Pattern* pattern;
	
	pattern = (Pattern*) table_at(&self->slots, slot);
	table_remove(&self->slots, slot);
	pattern_dispose(pattern);
	free(pattern);	
}

size_t patterns_size(Patterns* self)
{
	return self->slots.count;
}
