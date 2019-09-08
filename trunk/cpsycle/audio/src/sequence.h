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
} SequenceIterator;

void sequenceiterator_inc(SequenceIterator*);
PatternNode* sequenceiterator_patternnode(SequenceIterator*);
SequenceEntry* sequenceiterator_entry(SequenceIterator*);
PatternEntry* sequenceiterator_patternentry(SequenceIterator*);
float sequenceiterator_offset(SequenceIterator*);

typedef List Track;

typedef struct {
	Track* entries;
	Patterns* patterns;
	SequenceIterator editpos;
	Signal signal_editposchanged;
} Sequence;

void sequence_init(Sequence*, Patterns*);
void sequence_dispose(Sequence*);
SequenceEntry* sequence_insert(Sequence*, SequenceIterator position, int pattern);
void sequence_remove(Sequence*, SequenceIterator position);
unsigned int sequence_size(Sequence*);
SequenceIterator sequence_at(Sequence*, unsigned int position);
SequenceIterator sequence_begin(Sequence* self, float offset);
SequenceIterator sequence_last(Sequence* self);
void sequence_seteditposition(Sequence* self, SequenceIterator position);
SequenceIterator sequence_editposition(Sequence* self);
#endif

