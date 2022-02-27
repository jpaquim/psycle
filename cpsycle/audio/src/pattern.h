/*
/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PATTERN_H
#define psy_audio_PATTERN_H

/* local */
#include "blockselection.h"
#include "sequencecursor.h"
#include "patternentry.h"
/* container */
#include <signal.h>
/* platform */
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_Pattern
**
** A pattern is a small section of your song which is placed in
** sequence (sequence.h) with other patterns (patterns.h).
*/

#define psy_audio_GLOBALPATTERN INT32_MAX - 1
#define psy_audio_GLOBALPATTERN_TIMESIGTRACK 0
#define psy_audio_GLOBALPATTERN_LOOPTRACK 1

typedef bool (*psy_audio_fp_matches)(const uintptr_t test,
	const uintptr_t reference);
typedef uintptr_t (*psy_audio_fp_replacewith)(const uintptr_t current,
	const uintptr_t newval);

typedef struct psy_audio_SwingFill {
	int tempo;
	int width;
	float variance;
	float phase;
	bool offset;
} psy_audio_SwingFill;

typedef struct psy_audio_PatternSearchReplaceMode
{	
	psy_audio_fp_matches notematcher;
	psy_audio_fp_matches instmatcher;
	psy_audio_fp_matches machmatcher;
	psy_audio_fp_replacewith notereplacer;
	psy_audio_fp_replacewith instreplacer;
	psy_audio_fp_replacewith machreplacer;
	psy_audio_fp_replacewith tweakreplacer;
	uintptr_t notereference;
	uintptr_t instreference;
	uintptr_t machreference;
	uintptr_t notereplace;
	uintptr_t instreplace;
	uintptr_t machreplace;
	uintptr_t tweakreplace;
} psy_audio_PatternSearchReplaceMode;

INLINE bool psy_audio_patternsearchreplacemode_matchesall(uintptr_t t, uintptr_t r)
{
	return TRUE;
}

INLINE bool psy_audio_patternsearchreplacemode_matchesempty(uintptr_t test,
	uintptr_t r)
{
	return test == 0xff;
}

INLINE bool psy_audio_patternsearchreplacemode_matchesnonempty(uintptr_t test,
	uintptr_t r)
{
	return test != 0xff;
}

INLINE bool psy_audio_patternsearchreplacemode_matchesequal(uintptr_t test,
	uintptr_t reference)
{
	return test == reference;
}

INLINE uintptr_t psy_audio_patternsearchreplacemode_replacewithempty(
	uintptr_t c, uintptr_t nv)
{
	return 0xff;
}

INLINE uintptr_t psy_audio_patternsearchreplacemode_replacewithcurrent(
	uintptr_t current, uintptr_t nv)
{
	return current;
}

INLINE uintptr_t psy_audio_patternsearchreplacemode_replacewithnewval(
	uintptr_t c, uintptr_t newval)
{
	return newval;
}

/* psy_audio_Loop */
typedef struct psy_audio_Loop {
	psy_audio_PatternNode* start;
	psy_audio_PatternNode* end;
} psy_audio_Loop;

/* psy_audio_TimeSig */
typedef struct psy_audio_TimeSig {
	/* 0 = not set */
	uintptr_t numerator;
	/* 0 = not set */
	uintptr_t denominator;
} psy_audio_TimeSig;

void psy_audio_timesig_init(psy_audio_TimeSig*);
void psy_audio_timesig_init_all(psy_audio_TimeSig*,
	uintptr_t numerator, uintptr_t denominator);

/* psy_audio_Pattern */

struct psy_audio_SequenceTrackIterator;

typedef struct psy_audio_Pattern {
	/* public */
	psy_Signal signal_namechanged;
	psy_Signal signal_lengthchanged;
	/*
	** incremented by each operation, the ui is using
	** this flag to synchronize its views
	*/
	uintptr_t opcount;
	/* internal */
	psy_audio_PatternNode* events;
	psy_dsp_big_beat_t length;
	/* used by the paste pattern, player uses songtracks of patterns */
	uintptr_t maxsongtracks;
	char* name;
	psy_audio_TimeSig timesig;	
	/* references */
	psy_List* seqiterators;
} psy_audio_Pattern;

/* initializes a pattern */
void psy_audio_pattern_init(psy_audio_Pattern*);
/* frees all memory used */
void psy_audio_pattern_dispose(psy_audio_Pattern*);
/* copies a pattern */
void psy_audio_pattern_copy(psy_audio_Pattern* dst, psy_audio_Pattern* src);
/*
** allocates a pattern
** \return allocates a pattern
*/
psy_audio_Pattern* psy_audio_pattern_alloc(void);
/*
** allocates and initializes a pattern
** \return allocates and initializes a pattern
*/
psy_audio_Pattern* psy_audio_pattern_allocinit(void);
/*
** allocates a new pattern with a copy of all events
** \return allocates a new pattern with a copy of all events
*/
psy_audio_Pattern* psy_audio_pattern_clone(psy_audio_Pattern*);

void psy_audio_pattern_clear(psy_audio_Pattern*);
/*
** inserts an event by copy
** \return the pattern node containing the inserted event
*/
psy_audio_PatternNode* psy_audio_pattern_insert(psy_audio_Pattern*,
	psy_audio_PatternNode* prev, uintptr_t track, psy_dsp_big_beat_t offset,
	const psy_audio_PatternEvent*);
psy_audio_PatternNode* psy_audio_pattern_insert(psy_audio_Pattern*,
	psy_audio_PatternNode* prev, uintptr_t track, psy_dsp_big_beat_t offset,
	const psy_audio_PatternEvent*);
/* removes a pattern node */
void psy_audio_pattern_remove(psy_audio_Pattern*, psy_audio_PatternNode*);
/* finds the pattern node greater or equal than the offset */
/* \return the pattern node greater or equal than the offset */
psy_audio_PatternNode* psy_audio_pattern_greaterequal(psy_audio_Pattern*, psy_dsp_big_beat_t offset);
psy_audio_PatternNode* psy_audio_pattern_greaterequal_track(psy_audio_Pattern*,
	uintptr_t track, psy_dsp_big_beat_t offset);
/* finds a pattern node */
/* \return the pattern node */
psy_audio_PatternNode* psy_audio_pattern_findnode(psy_audio_Pattern* pattern, uintptr_t track,
	psy_dsp_big_beat_t offset, psy_dsp_big_beat_t bpl, psy_audio_PatternNode** prev);
psy_audio_PatternNode* psy_audio_pattern_findnode_cursor(psy_audio_Pattern*,
	psy_audio_SequenceCursor, psy_audio_PatternNode** prev);

INLINE const psy_audio_PatternNode* psy_audio_pattern_findnode_cursor_const(
	const psy_audio_Pattern* self, psy_audio_SequenceCursor cursor,
	psy_audio_PatternNode** prev)
{
	return psy_audio_pattern_findnode_cursor((psy_audio_Pattern*)self, cursor,
		prev);
}
/* gets the first pattern */
/* \return the first pattern node */
INLINE psy_audio_PatternNode* psy_audio_pattern_begin(const psy_audio_Pattern* self)
{
	assert(self);
	return self->events;
}
/* finds the last pattern */
/* \return finds the last pattern node */
psy_audio_PatternNode* psy_audio_pattern_last(psy_audio_Pattern*);
/* finds the next pattern node on a track */
/* \return next pattern node on a track or NULL */
psy_audio_PatternNode* psy_audio_patternnode_next_track(
	psy_audio_PatternNode* node, uintptr_t track);
/* finds the prev pattern node on a track */
/* \return prev pattern node on a track or NULL */
psy_audio_PatternNode* psy_audio_patternnode_prev_track(
	psy_audio_PatternNode* node, uintptr_t track);
/* test if track is used */
/* \return TRUE if track used else FALSE */
bool psy_audio_pattern_track_used(const psy_audio_Pattern*, uintptr_t track);
/* sets the pattern description */
void psy_audio_pattern_setname(psy_audio_Pattern*, const char*);
/* \return pattern description */
INLINE const char* psy_audio_pattern_name(const psy_audio_Pattern* self)
{
	assert(self);
	return self->name;
}
/* sets the pattern length */
void psy_audio_pattern_setlength(psy_audio_Pattern*, psy_dsp_big_beat_t length);
/* \return length of the pattern */
INLINE psy_dsp_big_beat_t psy_audio_pattern_length(const psy_audio_Pattern* self)
{	
	assert(self);

	return self->length;
}
/* tells if the pattern contains events */
/* \return tells if the pattern contains events */
INLINE int psy_audio_pattern_empty(const psy_audio_Pattern* self)
{
	assert(self);

	return self->events == NULL;
}
/* sets the event or an empty event if event is 0 */
void psy_audio_pattern_setevent(psy_audio_Pattern*, psy_audio_PatternNode*,
	const psy_audio_PatternEvent*, uintptr_t index);
/* gets the event or an empty event if node is 0 */
/* \return gets the event or an empty event if node is 0 */
psy_audio_PatternEvent psy_audio_pattern_event(psy_audio_Pattern*,
	psy_audio_PatternNode*, uintptr_t index);
/* \set the event at the cursor position */
psy_audio_PatternNode* psy_audio_pattern_set_event_at_cursor(
	psy_audio_Pattern*, psy_audio_SequenceCursor,
	const psy_audio_PatternEvent*);
/* \return gets the event or an empty event at the cursor position */
psy_audio_PatternEvent psy_audio_pattern_event_at_cursor(
	const psy_audio_Pattern*, psy_audio_SequenceCursor);

/* gets the op count to determine changes */
INLINE uintptr_t psy_audio_pattern_opcount(const psy_audio_Pattern* self)
{
	assert(self);
	return self->opcount;
}
/* multiplies all entry offsets with the given factor */
void psy_audio_pattern_scale(psy_audio_Pattern*, float factor);
/* erases the pattern and copies the block from a source pattern */
void psy_audio_pattern_blockcopy(psy_audio_Pattern* self,
	psy_audio_Pattern* source, psy_audio_BlockSelection selection);
/* paste the block from a source pattern to the cursor position */
void psy_audio_pattern_blockpaste(psy_audio_Pattern* self,
	psy_audio_Pattern* source, psy_audio_SequenceCursor destcursor,
	psy_dsp_big_beat_t bpl);
/* mix paste the block from a source pattern to the cursor position */
void psy_audio_pattern_blockmixpaste(psy_audio_Pattern* self,
	psy_audio_Pattern* source, psy_audio_SequenceCursor destcursor,
	psy_dsp_big_beat_t bpl);
/* interpolates linear all entries of the block */
void psy_audio_pattern_blockinterpolatelinear(psy_audio_Pattern*,
	const psy_audio_BlockSelection*);
void psy_audio_pattern_blockinterpolaterange(psy_audio_Pattern* self, psy_audio_SequenceCursor begin,
	psy_audio_SequenceCursor end, psy_dsp_big_beat_t bpl, intptr_t startval, intptr_t endval);
void psy_audio_pattern_blockinterpolaterangehermite(psy_audio_Pattern* self, psy_audio_SequenceCursor begin,
	psy_audio_SequenceCursor end, psy_dsp_big_beat_t bpl, intptr_t startval, intptr_t endval);
/* transposes all entries of the block with the offset */
void psy_audio_pattern_blocktranspose(psy_audio_Pattern*, psy_audio_SequenceCursor begin,
	psy_audio_SequenceCursor end, intptr_t offset);
/* swingfill */
void psy_audio_pattern_swingfill(psy_audio_Pattern*,
	const psy_audio_BlockSelection*, bool bTrackMode, psy_dsp_big_beat_t bpl,
	psy_audio_SwingFill);
/* changes the machine column of all entries of the block with the offset */
void psy_audio_pattern_changemachine(psy_audio_Pattern*, psy_audio_SequenceCursor begin,
	psy_audio_SequenceCursor end, uintptr_t machine);
/* changes the instrument column of all entries of the block with the offset */
void psy_audio_pattern_changeinstrument(psy_audio_Pattern*, psy_audio_SequenceCursor begin,
	psy_audio_SequenceCursor end, uintptr_t machine);
/* sets the maximum number of songtracks */
/* used by the paste pattern, player uses songtracks of patterns */
void psy_audio_pattern_setmaxsongtracks(psy_audio_Pattern*, uintptr_t num);
/* returns the maximum number of songtracks */
/* used by the paste pattern, player uses songtracks of patterns */
uintptr_t psy_audio_pattern_maxsongtracks(const psy_audio_Pattern*);
/* searches the pattern */
psy_audio_SequenceCursor psy_audio_pattern_searchinpattern(psy_audio_Pattern*,
	psy_audio_BlockSelection, psy_audio_PatternSearchReplaceMode);
/* number of lines a pattern will be created */
void psy_audio_pattern_setdefaultlines(uintptr_t numlines);
/* return number of lines a pattern will be created */
uintptr_t psy_audio_pattern_defaultlines(void);
/* return loop */
psy_audio_Loop psy_audio_pattern_loop_at(psy_audio_Pattern*, uintptr_t index,
	uintptr_t track);
/* return loop index */
uintptr_t psy_audio_pattern_loop_index(psy_audio_Pattern*, psy_audio_PatternNode*,
	uintptr_t track);
/* return loop */
psy_audio_PatternNode* psy_audio_pattern_timesig_at(psy_audio_Pattern*, uintptr_t index,
	uintptr_t track);
/* return loop index */
uintptr_t psy_audio_pattern_timesig_index(psy_audio_Pattern*, psy_audio_PatternNode*,
	uintptr_t track);
void psy_audio_pattern_add_seqiterator(psy_audio_Pattern*,	
	struct psy_audio_SequenceTrackIterator*);
void psy_audio_pattern_remove_seqiterator(psy_audio_Pattern*,
	struct psy_audio_SequenceTrackIterator*);

/* psy_audio_SequenceCursorNavigator */
typedef struct {
	psy_audio_SequenceCursor* cursor;
	psy_audio_Pattern* pattern;	
	psy_dsp_big_beat_t bpl;
	bool wrap;
	uint8_t maxkeys;
} psy_audio_SequenceCursorNavigator;

INLINE void psy_audio_patterncursornavigator_init(psy_audio_SequenceCursorNavigator* self,
	psy_audio_SequenceCursor* cursor, psy_audio_Pattern* pattern,
	psy_dsp_big_beat_t bpl, bool wrap, uint8_t maxkeys)
{
	self->cursor = cursor;
	self->pattern = pattern;
	self->bpl = bpl;
	self->wrap = wrap;
	self->maxkeys = maxkeys;
}

bool psy_audio_patterncursornavigator_advancekeys(
	psy_audio_SequenceCursorNavigator*, uint8_t lines);
bool psy_audio_patterncursornavigator_prevkeys(
	psy_audio_SequenceCursorNavigator*, uint8_t lines);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PATTERN_H */
