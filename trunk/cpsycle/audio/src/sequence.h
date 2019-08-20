// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(SEQUENCE_H)
#define SEQUENCE_H

#include "patterns.h"
#include <list.h>

typedef struct {
	int pattern;
	float offset;	
} SequenceEntry;

typedef struct {
	PatternNode* (*next)(void*);
	void (*unget)(void*);
	List* sequence;
	PatternNode* pattern;
	List* prevsequence;
	PatternNode* prevpattern;
} SequencePtr;

typedef struct {
	List* entries;
	Patterns* patterns;
	SequencePtr pos;
	SequencePtr prev;
} Sequence;

void sequence_init(Sequence*, Patterns*);
void sequence_dispose(Sequence*);
void sequence_insert(Sequence*, float offset, int pattern);
SequenceEntry* sequence_append(Sequence*, int pattern);
unsigned int sequence_size(Sequence*);
SequenceEntry* sequence_at(Sequence*, unsigned int position);
void sequence_seek(Sequence*, float pos);
SequencePtr sequence_begin(Sequence* self, float offset);
#endif

