// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERN_H)
#define PATTERN_H

#include "patternevent.h"
#include <list.h>
#include "../../detail/stdint.h"

// edit position in the pattern
typedef struct {	
	unsigned int track;
	big_beat_t offset;
	unsigned int line;
	unsigned int subline;
	unsigned int totallines;
	unsigned int col;
	uintptr_t pattern;
} PatternEditPosition;

/// an event with additional position and track information
typedef struct {
	PatternEvent event;
	/// position in beat unit
	beat_t offset;
	/// sound driver callback event position
	beat_t delta;
	/// current sequencer bpm
	beat_t bpm;
	/// the tracker channel
	unsigned int track; 
} PatternEntry;

PatternEntry* patternentry_clone(PatternEntry*);

/// a list of event entries ordered by position in beat unit
typedef List PatternNode;

typedef struct {
	PatternNode* events;	
	beat_t length;
	// used by the paste pattern, player uses songtracks of patterns
	uintptr_t maxsongtracks;
	char* label;
	// incremented by each operation, the ui is using
	// this flag to synchronize its views
	unsigned int opcount;
} Pattern;

/// initializes a pattern
void pattern_init(Pattern*);
/// frees all memory used
void pattern_dispose(Pattern*);
/// allocates a pattern
///\return allocates a pattern
Pattern* pattern_alloc(void);
/// allocates and initializes a pattern
///\return allocates and initializes a pattern
Pattern* pattern_allocinit(void);
/// allocates a new pattern with a copy of all events
///\return allocates a new pattern with a copy of all events
Pattern* pattern_clone(Pattern*);
/// inserts an event by copy
///\return the pattern node containing the inserted event
PatternNode* pattern_insert(Pattern*, PatternNode* prev, int track,
	beat_t offset, const PatternEvent*);
/// removes a pattern node
void pattern_remove(Pattern*, PatternNode*);
/// finds the pattern node greater or equal than the offset
///\return the pattern node greater or equal than the offset
PatternNode* pattern_greaterequal(Pattern*, beat_t offset);
/// finds a pattern node
///\return the pattern node
PatternNode* pattern_findnode(Pattern* pattern, unsigned int track,
	float offset, unsigned int subline, beat_t bpl, PatternNode** prev);
/// finds the last pattern
///\return finds the last pattern node
PatternNode* pattern_last(Pattern*);
/// sets the pattern description
void pattern_setlabel(Pattern*, const char*);
/// sets the pattern length
void pattern_setlength(Pattern*, beat_t length);	
/// tells if the pattern contains events
///\return tells if the pattern contains events
int pattern_empty(Pattern*);
/// sets the event or an empty event if event is 0
void pattern_setevent(Pattern*, PatternNode*, const PatternEvent*);
/// gets the event or an empty event if node is 0
///\return gets the event or an empty event if node is 0
PatternEvent pattern_event(Pattern*, PatternNode*);
/// gets the op count to determine changes
unsigned int pattern_opcount(Pattern*);
/// multiplies all entry offsets with the given factor
void pattern_scale(Pattern*, float factor);
/// erases all entries of the block
void pattern_blockremove(Pattern*, PatternEditPosition begin, 
	PatternEditPosition end);
/// transposes all entries of the block with the offset
void pattern_blocktranspose(Pattern*, PatternEditPosition begin, 
	PatternEditPosition end, int offset);
/// changes the machine column of all entries of the block with the offset
void pattern_changemachine(Pattern*, PatternEditPosition begin, 
	PatternEditPosition end, int machine);
/// changes the instrument column of all entries of the block with the offset
void pattern_changeinstrument(Pattern*, PatternEditPosition begin, 
	PatternEditPosition end, int machine);
/// sets the maximum number of songtracks 
/// used by the paste pattern, player uses songtracks of patterns
void pattern_setmaxsongtracks(Pattern*, uintptr_t num);
/// returns the maximum number of songtracks 
/// used by the paste pattern, player uses songtracks of patterns
uintptr_t pattern_maxsongtracks(Pattern*);

#endif
