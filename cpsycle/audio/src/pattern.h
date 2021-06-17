/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PATTERN_H
#define psy_audio_PATTERN_H

/* local */
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

/* edit position in the pattern */
typedef struct {	
	uintptr_t track;
	psy_dsp_big_beat_t offset;
	psy_dsp_big_beat_t seqoffset;
	uintptr_t lpb;
	uintptr_t line;
	uintptr_t column;
	uintptr_t digit;
	uintptr_t patternid;
	uint8_t key;
} psy_audio_PatternCursor;

void psy_audio_patterncursor_init(psy_audio_PatternCursor*);

psy_audio_PatternCursor psy_audio_patterncursor_make(
	uintptr_t track, psy_dsp_big_beat_t offset);
psy_audio_PatternCursor psy_audio_patterncursor_make_all(
	uintptr_t track, psy_dsp_big_beat_t offset, uint8_t key);

INLINE psy_dsp_big_beat_t psy_audio_patterncursor_offset(
	const psy_audio_PatternCursor* self)
{
	return self->offset;
}

/* compares two pattern edit positions, if they are equal */
int psy_audio_patterncursor_equal(psy_audio_PatternCursor* lhs,
	psy_audio_PatternCursor* rhs);

typedef struct {
	psy_audio_PatternCursor topleft;
	psy_audio_PatternCursor bottomright;
	bool valid;
} psy_audio_PatternSelection;

void psy_audio_patternselection_init(psy_audio_PatternSelection*);
void psy_audio_patternselection_init_all(psy_audio_PatternSelection*,
	psy_audio_PatternCursor topleft, psy_audio_PatternCursor bottomright);
psy_audio_PatternSelection psy_audio_patternselection_make(
	psy_audio_PatternCursor topleft, psy_audio_PatternCursor bottomright);

INLINE bool psy_audio_patternselection_valid(const psy_audio_PatternSelection* self)
{
	return self->valid;
}

INLINE void psy_audio_patternselection_enable(psy_audio_PatternSelection* self)
{
	self->valid = TRUE;
}

INLINE void psy_audio_patternselection_disable(psy_audio_PatternSelection* self)
{
	self->valid = FALSE;
}

INLINE bool psy_audio_patternselection_test(psy_audio_PatternSelection* self,
	uintptr_t track, psy_dsp_big_beat_t offset)
{
	return psy_audio_patternselection_valid(self) &&
		track >= self->topleft.track &&
		track < self->bottomright.track&&
		offset >= self->topleft.offset + self->topleft.seqoffset &&
		offset < self->bottomright.offset + self->bottomright.seqoffset;
}

void psy_audio_patternselection_startdrag(psy_audio_PatternSelection*,
	psy_audio_PatternCursor dragselectionbase,
	psy_audio_PatternCursor cursor, double bpl);
void psy_audio_patternselection_drag(psy_audio_PatternSelection*,
	psy_audio_PatternCursor dragselectionbase,
	psy_audio_PatternCursor cursor, double bpl);

typedef bool (*psy_audio_fp_matches)(const uintptr_t test, const uintptr_t reference);
typedef uintptr_t (*psy_audio_fp_replacewith)(const uintptr_t current, const uintptr_t newval);

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

INLINE uintptr_t psy_audio_patternsearchreplacemode_replacewithempty(uintptr_t c,
	uintptr_t nv)
{
	return 0xff;
}

INLINE uintptr_t psy_audio_patternsearchreplacemode_replacewithcurrent(uintptr_t current,
	uintptr_t nv)
{
	return current;
}

INLINE uintptr_t psy_audio_patternsearchreplacemode_replacewithnewval(uintptr_t c,
	uintptr_t newval)
{
	return newval;
}

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
	uintptr_t timesig_nominator;
	uintptr_t timesig_denominator;
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
/*
** inserts an event by copy
** \return the pattern node containing the inserted event
*/
psy_audio_PatternNode* psy_audio_pattern_insert(psy_audio_Pattern*, psy_audio_PatternNode* prev, uintptr_t track,
	psy_dsp_big_beat_t offset, const psy_audio_PatternEvent*);
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
psy_audio_PatternNode* psy_audio_pattern_findnode_cursor(psy_audio_Pattern* pattern,
	psy_audio_PatternCursor cursor, psy_audio_PatternNode** prev);
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
psy_audio_PatternNode* psy_audio_pattern_next_track(psy_audio_Pattern*,
	psy_audio_PatternNode* node, uintptr_t track);
/* finds the prev pattern node on a track */
/* \return prev pattern node on a track or NULL */
psy_audio_PatternNode* psy_audio_pattern_prev_track(psy_audio_Pattern*,
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
void psy_audio_pattern_setevent(psy_audio_Pattern*, psy_audio_PatternNode*, const psy_audio_PatternEvent*);
/* gets the event or an empty event if node is 0 */
/* \return gets the event or an empty event if node is 0 */
psy_audio_PatternEvent psy_audio_pattern_event(psy_audio_Pattern*, psy_audio_PatternNode*);
/* gets the op count to determine changes */
INLINE uintptr_t psy_audio_pattern_opcount(const psy_audio_Pattern* self)
{
	assert(self);
	return self->opcount;
}
/* multiplies all entry offsets with the given factor */
void psy_audio_pattern_scale(psy_audio_Pattern*, float factor);
/* erases all entries of the block */
void psy_audio_pattern_blockremove(psy_audio_Pattern*,
	psy_audio_PatternCursor begin, psy_audio_PatternCursor end);
/* erases the pattern and copies the block from a source pattern */
void psy_audio_pattern_blockcopy(psy_audio_Pattern* self,
	psy_audio_Pattern* source, psy_audio_PatternSelection selection);
/* paste the block from a source pattern to the cursor position */
void psy_audio_pattern_blockpaste(psy_audio_Pattern* self,
	psy_audio_Pattern* source, psy_audio_PatternCursor destcursor,
	psy_dsp_big_beat_t bpl);
/* mix paste the block from a source pattern to the cursor position */
void psy_audio_pattern_blockmixpaste(psy_audio_Pattern* self,
	psy_audio_Pattern* source, psy_audio_PatternCursor destcursor,
	psy_dsp_big_beat_t bpl);
/* interpolates linear all entries of the block */
void psy_audio_pattern_blockinterpolatelinear(psy_audio_Pattern*,
	const psy_audio_PatternSelection*, psy_dsp_big_beat_t bpl);
void psy_audio_pattern_blockinterpolaterange(psy_audio_Pattern* self, psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, psy_dsp_big_beat_t bpl, intptr_t startval, intptr_t endval);
void psy_audio_pattern_blockinterpolaterangehermite(psy_audio_Pattern* self, psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, psy_dsp_big_beat_t bpl, intptr_t startval, intptr_t endval);
/* transposes all entries of the block with the offset */
void psy_audio_pattern_blocktranspose(psy_audio_Pattern*, psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, int offset);
/* swingfill */
void psy_audio_pattern_swingfill(psy_audio_Pattern*,
	const psy_audio_PatternSelection*, bool bTrackMode, psy_dsp_big_beat_t bpl,
	int tempo, int width, float variance, float phase, bool offset);
/* changes the machine column of all entries of the block with the offset */
void psy_audio_pattern_changemachine(psy_audio_Pattern*, psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, uintptr_t machine);
/* changes the instrument column of all entries of the block with the offset */
void psy_audio_pattern_changeinstrument(psy_audio_Pattern*, psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, uintptr_t machine);
/* sets the maximum number of songtracks */
/* used by the paste pattern, player uses songtracks of patterns */
void psy_audio_pattern_setmaxsongtracks(psy_audio_Pattern*, uintptr_t num);
/* returns the maximum number of songtracks */
/* used by the paste pattern, player uses songtracks of patterns */
uintptr_t psy_audio_pattern_maxsongtracks(const psy_audio_Pattern*);
/* searches the pattern */
psy_audio_PatternCursor psy_audio_pattern_searchinpattern(psy_audio_Pattern*,
	psy_audio_PatternSelection, psy_audio_PatternSearchReplaceMode);
/* number of lines a pattern will be created */
void psy_audio_pattern_setdefaultlines(uintptr_t numlines);
/* return number of lines a pattern will be created */
uintptr_t psy_audio_pattern_defaultlines(void);

/* psy_audio_PatternCursorNavigator */
typedef struct {
	psy_audio_PatternCursor* cursor;
	psy_audio_Pattern* pattern;	
	psy_dsp_big_beat_t bpl;
	bool wrap;
	uint8_t maxkeys;
} psy_audio_PatternCursorNavigator;

INLINE void psy_audio_patterncursornavigator_init(psy_audio_PatternCursorNavigator* self,
	psy_audio_PatternCursor* cursor, psy_audio_Pattern* pattern,
	psy_dsp_big_beat_t bpl, bool wrap, uint8_t maxkeys)
{
	self->cursor = cursor;
	self->pattern = pattern;
	self->bpl = bpl;
	self->wrap = wrap;
	self->maxkeys = maxkeys;
}

bool psy_audio_patterncursornavigator_advancelines(
	psy_audio_PatternCursorNavigator*, uintptr_t lines);
bool psy_audio_patterncursornavigator_prevlines(
	psy_audio_PatternCursorNavigator*, uintptr_t lines);
bool psy_audio_patterncursornavigator_advancekeys(
	psy_audio_PatternCursorNavigator*, uint8_t lines);
bool psy_audio_patterncursornavigator_prevkeys(
	psy_audio_PatternCursorNavigator*, uint8_t lines);
bool psy_audio_patterncursornavigator_prevtrack(
	psy_audio_PatternCursorNavigator*,
	uintptr_t numsongtracks);
bool psy_audio_patterncursornavigator_nexttrack(
	psy_audio_PatternCursorNavigator*,
	uintptr_t numsongtracks);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PATTERN_H */
