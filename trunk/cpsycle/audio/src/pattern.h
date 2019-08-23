// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(PATTERN_H)
#define PATTERN_H

#include "event.h"
#include "list.h"

typedef struct 
{
	PatternEvent event;
	float offset;
	float delta;
	int track;
} PatternEntry;

typedef List PatternNode;

typedef struct {
	PatternNode* events;
	float length;
} Pattern;

void pattern_init(Pattern* self);
void pattern_free(Pattern* self);
void pattern_write(Pattern* self, int track, float offset, PatternEvent);
void pattern_remove(Pattern* self, int track, float offset);
PatternNode* pattern_greaterequal(Pattern* self, float offset);

#endif

