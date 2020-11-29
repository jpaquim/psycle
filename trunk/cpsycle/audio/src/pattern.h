// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PATTERN_H
#define psy_audio_PATTERN_H

#include "patternentry.h"

#include <signal.h>

#include "../../detail/stdint.h"
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

// edit position in the pattern
typedef struct {	
	uintptr_t track;
	psy_dsp_big_beat_t offset;
	uintptr_t line;
	uintptr_t column;
	uintptr_t digit;
	uintptr_t patternid;
	uint8_t key;
} psy_audio_PatternCursor;

void psy_audio_patterncursor_init(psy_audio_PatternCursor*);

/// compares two pattern edit positions, if they are equal
int psy_audio_patterncursor_equal(psy_audio_PatternCursor* lhs,
	psy_audio_PatternCursor* rhs);

typedef struct {
	psy_audio_PatternCursor topleft;
	psy_audio_PatternCursor bottomright;
} PatternSelection;

typedef struct psy_audio_Pattern {
	// public
	psy_Signal signal_namechanged;
	psy_Signal signal_lengthchanged;
	// incremented by each operation, the ui is using
	// this flag to synchronize its views
	uintptr_t opcount;
	// private
	psy_audio_PatternNode* events;
	psy_dsp_big_beat_t length;
	// used by the paste pattern, player uses songtracks of patterns
	uintptr_t maxsongtracks;
	char* name;
} psy_audio_Pattern;

/// initializes a pattern
void psy_audio_pattern_init(psy_audio_Pattern*);
/// frees all memory used
void psy_audio_pattern_dispose(psy_audio_Pattern*);
/// copies a pattern
void psy_audio_pattern_copy(psy_audio_Pattern* dst, psy_audio_Pattern* src);
/// allocates a pattern
///\return allocates a pattern
psy_audio_Pattern* psy_audio_pattern_alloc(void);
/// allocates and initializes a pattern
///\return allocates and initializes a pattern
psy_audio_Pattern* psy_audio_pattern_allocinit(void);
/// allocates a new pattern with a copy of all events
///\return allocates a new pattern with a copy of all events
psy_audio_Pattern* psy_audio_pattern_clone(psy_audio_Pattern*);
/// inserts an event by copy
///\return the pattern node containing the inserted event
psy_audio_PatternNode* psy_audio_pattern_insert(psy_audio_Pattern*, psy_audio_PatternNode* prev, uintptr_t track,
	psy_dsp_big_beat_t offset, const psy_audio_PatternEvent*);
/// removes a pattern node
void psy_audio_pattern_remove(psy_audio_Pattern*, psy_audio_PatternNode*);
/// finds the pattern node greater or equal than the offset
///\return the pattern node greater or equal than the offset
psy_audio_PatternNode* psy_audio_pattern_greaterequal(psy_audio_Pattern*, psy_dsp_big_beat_t offset);
psy_audio_PatternNode* psy_audio_pattern_greaterequal_track(psy_audio_Pattern*,
	uintptr_t track, psy_dsp_big_beat_t offset);
/// finds a pattern node
///\return the pattern node
psy_audio_PatternNode* psy_audio_pattern_findnode(psy_audio_Pattern* pattern, uintptr_t track,
	psy_dsp_big_beat_t offset, psy_dsp_big_beat_t bpl, psy_audio_PatternNode** prev);
/// gets the first pattern
///\return the first pattern node
INLINE psy_audio_PatternNode* psy_audio_pattern_begin(const psy_audio_Pattern* self)
{
	assert(self);
	return self->events;
}
/// finds the last pattern
///\return finds the last pattern node
psy_audio_PatternNode* psy_audio_pattern_last(psy_audio_Pattern*);
/// finds the next pattern node on a track
///\return next pattern node on a track or NULL
psy_audio_PatternNode* psy_audio_pattern_next_track(psy_audio_Pattern*,
	psy_audio_PatternNode* node, uintptr_t track);
/// finds the prev pattern node on a track
///\return prev pattern node on a track or NULL
psy_audio_PatternNode* psy_audio_pattern_prev_track(psy_audio_Pattern*,
	psy_audio_PatternNode* node, uintptr_t track);
/// sets the pattern description
void psy_audio_pattern_setname(psy_audio_Pattern*, const char*);
///\return pattern description
INLINE const char* psy_audio_pattern_name(const psy_audio_Pattern* self)
{
	assert(self);
	return self->name;
}
/// sets the pattern length
void psy_audio_pattern_setlength(psy_audio_Pattern*, psy_dsp_big_beat_t length);
/// return length of the pattern
INLINE psy_dsp_big_beat_t psy_audio_pattern_length(const psy_audio_Pattern* self)
{
	assert(self);
	return self->length;
}
/// tells if the pattern contains events
///\return tells if the pattern contains events
INLINE int psy_audio_pattern_empty(const psy_audio_Pattern* self)
{
	assert(self);
	return self->events == NULL;
}
/// sets the event or an empty event if event is 0
void psy_audio_pattern_setevent(psy_audio_Pattern*, psy_audio_PatternNode*, const psy_audio_PatternEvent*);
/// gets the event or an empty event if node is 0
///\return gets the event or an empty event if node is 0
psy_audio_PatternEvent psy_audio_pattern_event(psy_audio_Pattern*, psy_audio_PatternNode*);
/// gets the op count to determine changes
INLINE uintptr_t psy_audio_pattern_opcount(const psy_audio_Pattern* self)
{
	assert(self);
	return self->opcount;
}
/// multiplies all entry offsets with the given factor
void psy_audio_pattern_scale(psy_audio_Pattern*, float factor);
/// erases all entries of the block
void psy_audio_pattern_blockremove(psy_audio_Pattern*,
	psy_audio_PatternCursor begin, psy_audio_PatternCursor end);
/// interpolates linear all entries of the block
void psy_audio_pattern_blockinterpolatelinear(psy_audio_Pattern*, psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, psy_dsp_big_beat_t bpl);
void psy_audio_pattern_blockinterpolaterange(psy_audio_Pattern* self, psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, psy_dsp_big_beat_t bpl, intptr_t startval, intptr_t endval);
void psy_audio_pattern_blockinterpolaterangehermite(psy_audio_Pattern* self, psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, psy_dsp_big_beat_t bpl, intptr_t startval, intptr_t endval);
/// transposes all entries of the block with the offset
void psy_audio_pattern_blocktranspose(psy_audio_Pattern*, psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, int offset);
/// changes the machine column of all entries of the block with the offset
void psy_audio_pattern_changemachine(psy_audio_Pattern*, psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, int machine);
/// changes the instrument column of all entries of the block with the offset
void psy_audio_pattern_changeinstrument(psy_audio_Pattern*, psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, int machine);
/// sets the maximum number of songtracks 
/// used by the paste pattern, player uses songtracks of patterns
void psy_audio_pattern_setmaxsongtracks(psy_audio_Pattern*, uintptr_t num);
/// returns the maximum number of songtracks 
/// used by the paste pattern, player uses songtracks of patterns
uintptr_t psy_audio_pattern_maxsongtracks(const psy_audio_Pattern*);


typedef struct {
	psy_audio_PatternCursor* cursor;
	psy_audio_Pattern* pattern;	
	psy_dsp_big_beat_t bpl;
	bool wrap;
} psy_audio_PatternCursorNavigator;

INLINE void psy_audio_patterncursornavigator_init(psy_audio_PatternCursorNavigator* self,
	psy_audio_PatternCursor* cursor, psy_audio_Pattern* pattern,
	psy_dsp_big_beat_t bpl, bool wrap)
{
	self->cursor = cursor;
	self->pattern = pattern;
	self->bpl = bpl;
	self->wrap = wrap;
}

bool psy_audio_patterncursornavigator_advancelines(
	psy_audio_PatternCursorNavigator*, uintptr_t lines);
bool psy_audio_patterncursornavigator_prevlines(
	psy_audio_PatternCursorNavigator*, uintptr_t lines);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PATTERN_H */
