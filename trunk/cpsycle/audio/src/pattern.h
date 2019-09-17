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
	char* label;
} Pattern;

void pattern_init(Pattern*);
void pattern_free(Pattern*);
Pattern* pattern_clone(Pattern*);
PatternEntry* pattern_write(Pattern*, int track, float offset, PatternEvent);
PatternNode* pattern_insert(Pattern*, PatternNode* prev, int track, float offset, PatternEvent*);
void pattern_remove(Pattern*, PatternNode*);
PatternNode* pattern_greaterequal(Pattern*, float offset, PatternNode** prev);
void pattern_setlabel(Pattern*, const char*);
void pattern_setlength(Pattern*, float length);

#endif

