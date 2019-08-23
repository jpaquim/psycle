// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(SEQUENCE_H)
#define SEQUENCE_H

#include "patterns.h"
#include <list.h>
#include <signal.h>

typedef struct {
	int pattern;
	float offset;	
} SequenceEntry;

typedef struct {
	Patterns* patterns;	
	List* sequence;
	PatternNode* patternnode;		
} SequencePtr;

void sequenceptr_inc(SequencePtr*);
PatternNode* sequenceptr_patternnode(SequencePtr*);
SequenceEntry* sequenceptr_entry(SequencePtr*);

typedef List Track;

typedef struct {
	Track* entries;
	Patterns* patterns;
	SequencePtr editpos;
	Signal signal_editposchanged;
} Sequence;

void sequence_init(Sequence*, Patterns*);
void sequence_dispose(Sequence*);
SequenceEntry* sequence_insert(Sequence*, SequencePtr position, int pattern);
void sequence_remove(Sequence*, SequencePtr position);
unsigned int sequence_size(Sequence*);
SequencePtr sequence_at(Sequence*, unsigned int position);
SequencePtr sequence_begin(Sequence* self, float offset);
SequencePtr sequence_last(Sequence* self);
void sequence_seteditposition(Sequence* self, SequencePtr position);
SequencePtr sequence_editposition(Sequence* self);
#endif

