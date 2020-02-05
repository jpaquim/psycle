// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PATTERN_H
#define psy_audio_PATTERN_H

#include "patternentry.h"

#include "../../detail/stdint.h"
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

// edit position in the pattern
typedef struct {	
	unsigned int track;
	psy_dsp_big_beat_t offset;
	unsigned int line;
	unsigned int column;
	unsigned int digit;
	uintptr_t pattern;
} PatternEditPosition;

void patterneditposition_init(PatternEditPosition*);

/// compares two pattern edit positions, if they are equal
int patterneditposition_equal(PatternEditPosition* lhs,
	PatternEditPosition* rhs);

typedef struct psy_audio_Pattern {
	PatternNode* events;
	psy_dsp_beat_t length;
	// used by the paste pattern, player uses songtracks of patterns
	uintptr_t maxsongtracks;
	char* label;
	// incremented by each operation, the ui is using
	// this flag to synchronize its views
	unsigned int opcount;
} psy_audio_Pattern;

/// initializes a pattern
void pattern_init(psy_audio_Pattern*);
/// frees all memory used
void pattern_dispose(psy_audio_Pattern*);
/// copies a pattern
void pattern_copy(psy_audio_Pattern* dst, psy_audio_Pattern* src);
/// allocates a pattern
///\return allocates a pattern
psy_audio_Pattern* pattern_alloc(void);
/// allocates and initializes a pattern
///\return allocates and initializes a pattern
psy_audio_Pattern* pattern_allocinit(void);
/// allocates a new pattern with a copy of all events
///\return allocates a new pattern with a copy of all events
psy_audio_Pattern* pattern_clone(psy_audio_Pattern*);
/// inserts an event by copy
///\return the pattern node containing the inserted event
PatternNode* pattern_insert(psy_audio_Pattern*, PatternNode* prev, int track,
	psy_dsp_beat_t offset, const psy_audio_PatternEvent*);
/// removes a pattern node
void pattern_remove(psy_audio_Pattern*, PatternNode*);
/// finds the pattern node greater or equal than the offset
///\return the pattern node greater or equal than the offset
PatternNode* pattern_greaterequal(psy_audio_Pattern*, psy_dsp_beat_t offset);
/// finds a pattern node
///\return the pattern node
PatternNode* pattern_findnode(psy_audio_Pattern* pattern, unsigned int track,
	float offset, psy_dsp_beat_t bpl, PatternNode** prev);
/// finds the last pattern
///\return finds the last pattern node
PatternNode* pattern_last(psy_audio_Pattern*);
/// sets the pattern description
void pattern_setlabel(psy_audio_Pattern*, const char*);
/// sets the pattern length
void pattern_setlength(psy_audio_Pattern*, psy_dsp_beat_t length);
/// return length of the pattern
psy_dsp_beat_t pattern_length(psy_audio_Pattern*);
/// tells if the pattern contains events
///\return tells if the pattern contains events
int pattern_empty(psy_audio_Pattern*);
/// sets the event or an empty event if event is 0
void pattern_setevent(psy_audio_Pattern*, PatternNode*, const psy_audio_PatternEvent*);
/// gets the event or an empty event if node is 0
///\return gets the event or an empty event if node is 0
psy_audio_PatternEvent pattern_event(psy_audio_Pattern*, PatternNode*);
/// gets the op count to determine changes
unsigned int pattern_opcount(psy_audio_Pattern*);
/// multiplies all entry offsets with the given factor
void pattern_scale(psy_audio_Pattern*, float factor);
/// erases all entries of the block
void pattern_blockremove(psy_audio_Pattern*, PatternEditPosition begin, 
	PatternEditPosition end);
/// interpolates linear all entries of the block
void pattern_blockinterpolatelinear(psy_audio_Pattern*, PatternEditPosition begin,
	PatternEditPosition end, psy_dsp_beat_t bpl);
/// transposes all entries of the block with the offset
void pattern_blocktranspose(psy_audio_Pattern*, PatternEditPosition begin, 
	PatternEditPosition end, int offset);
/// changes the machine column of all entries of the block with the offset
void pattern_changemachine(psy_audio_Pattern*, PatternEditPosition begin, 
	PatternEditPosition end, int machine);
/// changes the instrument column of all entries of the block with the offset
void pattern_changeinstrument(psy_audio_Pattern*, PatternEditPosition begin, 
	PatternEditPosition end, int machine);
/// sets the maximum number of songtracks 
/// used by the paste pattern, player uses songtracks of patterns
void pattern_setmaxsongtracks(psy_audio_Pattern*, uintptr_t num);
/// returns the maximum number of songtracks 
/// used by the paste pattern, player uses songtracks of patterns
uintptr_t pattern_maxsongtracks(psy_audio_Pattern*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PATTERN_H */
