// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERN_H)
#define PATTERN_H

#include "event.h"
#include <list.h>

/// an event with additional position and track information
typedef struct {
	PatternEvent event;
	 /// position in beat unit
	float offset;
	 /// sound driver callback event position in beat unit
	float delta;
	/// the tracker channel
	unsigned int track; 
} PatternEntry;

/// a list of event entries ordered by position in beat unit
typedef List PatternNode;

typedef struct {
	PatternNode* events;
	float length;
	char* label;
	int opcount;
} Pattern;

/// initializes a pattern
void pattern_init(Pattern*);
/// frees all memory used
void pattern_dispose(Pattern*);
/// allocates a new pattern with a copy of all events
///\return allocates a new pattern with a copy of all events
Pattern* pattern_clone(Pattern*);
/// inserts an event by copy
///\return the pattern node containing the inserted event
PatternNode* pattern_insert(Pattern*, PatternNode* prev, int track,
	float offset, const PatternEvent*);
/// removes a pattern node
void pattern_remove(Pattern*, PatternNode*);
/// finds the pattern node greater or equal than the offset
///\return the pattern node greater or equal than the offset and its predeccessor
PatternNode* pattern_greaterequal(Pattern*, float offset, PatternNode** prev);
/// sets the pattern description
void pattern_setlabel(Pattern*, const char*);
/// sets the pattern length in beat unit
void pattern_setlength(Pattern*, float length);	
/// tells if the pattern contains events
///\return tells if the pattern contains events
int pattern_empty(Pattern*);

#endif

