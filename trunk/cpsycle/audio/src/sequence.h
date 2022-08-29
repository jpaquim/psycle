/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_SEQUENCE_H
#define psy_audio_SEQUENCE_H

/* local */
#include "sequenceentry.h"
#include "sequenceselection.h"
/* container */
#include <list.h>
#include <signal.h>
#include <command.h>

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_Sequence
**
** contains the playorder list of a song (multisequence)
*/

typedef psy_List psy_audio_SequenceEntryNode;

#define psy_audio_GLOBALTRACK 65535

/* psy_audio_Order */
typedef struct psy_audio_Order {
	psy_audio_OrderIndex index;
	psy_audio_SequenceEntry* entry;
} psy_audio_Order;

void psy_audio_order_init(psy_audio_Order*);
void psy_audio_order_dispose(psy_audio_Order*);

void psy_audio_order_setentry(psy_audio_Order*, const psy_audio_SequenceEntry*);

typedef struct psy_audio_SequencePaste {
	psy_List* nodes;
} psy_audio_SequencePaste;

void psy_audio_sequencepaste_init(psy_audio_SequencePaste*);
void psy_audio_sequencepaste_dispose(psy_audio_SequencePaste*);

void psy_audio_sequencepaste_clear(psy_audio_SequencePaste*);
void psy_audio_sequencepaste_copy(psy_audio_SequencePaste*,
	struct psy_audio_Sequence*, psy_audio_SequenceSelection*);

/*
** psy_audio_SequenceTrack
**
** A list of SequenceEntries sorted according to the playorder
*/
typedef struct psy_audio_SequenceTrack {
	psy_audio_SequenceEntryNode* nodes;
	psy_Table entries;
	char* name;
	double height;
} psy_audio_SequenceTrack;

void psy_audio_sequencetrack_init(psy_audio_SequenceTrack*);
void psy_audio_sequencetrack_dispose(psy_audio_SequenceTrack*);

psy_audio_SequenceTrack* psy_audio_sequencetrack_alloc(void);
psy_audio_SequenceTrack* psy_audio_sequencetrack_allocinit(void);
psy_audio_SequenceTrack* psy_audio_sequencetrack_clone(
	psy_audio_SequenceTrack* src);
void psy_audio_sequencetrack_deallocate(psy_audio_SequenceTrack*);

psy_dsp_big_beat_t psy_audio_sequencetrack_duration(
	const psy_audio_SequenceTrack*, const psy_audio_Patterns*);

void psy_audio_sequencetrack_set_name(psy_audio_SequenceTrack*,
	const char* name);

INLINE const char* psy_audio_sequencetrack_name(
	const psy_audio_SequenceTrack* self)
{
	assert(self);

	return self->name;
}

void psy_audio_sequencetrack_insert(psy_audio_SequenceTrack*,
	uintptr_t order, psy_audio_Patterns*, uintptr_t patidx);
uintptr_t psy_audio_sequencetrack_remove(psy_audio_SequenceTrack*,
	uintptr_t order);
psy_audio_SequenceEntry* psy_audio_sequencetrack_entry(psy_audio_SequenceTrack*,
	uintptr_t row);
psy_audio_SequenceEntryNode* psy_audio_sequencetrack_node_at_offset(
	psy_audio_SequenceTrack*, psy_dsp_big_beat_t offset);
uintptr_t psy_audio_sequencetrack_size(const psy_audio_SequenceTrack*);

/*
** psy_audio_SequenceTrackIterator
**
** Iterator the player uses to advance through the track and pattern
*/
typedef struct psy_audio_SequenceTrackIterator {
	psy_audio_Patterns* patterns;
	psy_audio_SequenceTrack* track;	
	psy_audio_SequenceEntryNode* sequencentrynode;
	psy_audio_PatternNode* patternnode;
	psy_audio_Pattern* pattern;
} psy_audio_SequenceTrackIterator;

void psy_audio_sequencetrackiterator_init(psy_audio_SequenceTrackIterator*);
void psy_audio_sequencetrackiterator_dispose(psy_audio_SequenceTrackIterator*);

psy_audio_SequenceTrackIterator* psy_audio_sequencetrackiterator_alloc(void);
psy_audio_SequenceTrackIterator* psy_audio_sequencetrackiterator_allocinit(void);

void psy_audio_sequencetrackiterator_inc(psy_audio_SequenceTrackIterator*);
void psy_audio_sequencetrackiterator_inc_entry(psy_audio_SequenceTrackIterator*);
void psy_audio_sequencetrackiterator_dec_entry(psy_audio_SequenceTrackIterator*);
bool psy_audio_sequencetrackiterator_has_next_entry(
	const psy_audio_SequenceTrackIterator*);

void psy_audio_sequencetrackiterator_set_pattern(psy_audio_SequenceTrackIterator*,
	psy_audio_Pattern*);

INLINE psy_audio_PatternNode* psy_audio_sequencetrackiterator_patternnode(
	psy_audio_SequenceTrackIterator* self)
{
	return self->patternnode;
}

INLINE psy_audio_SequenceEntry* psy_audio_sequencetrackiterator_entry(
	psy_audio_SequenceTrackIterator* self)
{
	return (self->sequencentrynode)
		? (psy_audio_SequenceEntry*)psy_list_entry(self->sequencentrynode)
		: NULL;
}

INLINE psy_audio_PatternEntry* psy_audio_sequencetrackiterator_patternentry(
	psy_audio_SequenceTrackIterator* self)
{
	return (self->patternnode)
		? (psy_audio_PatternEntry*)(self->patternnode)->entry
		: NULL;
}

INLINE uintptr_t psy_audio_sequencetrackiterator_patidx(
	psy_audio_SequenceTrackIterator* self)
{		
	psy_audio_SequenceEntry* entry;

	entry = psy_audio_sequencetrackiterator_entry(self);
	if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
		psy_audio_SequencePatternEntry* seqpatternentry;

		seqpatternentry = (psy_audio_SequencePatternEntry*)
			self->sequencentrynode->entry;
		return seqpatternentry->patternslot;
	}	
	return psy_INDEX_INVALID;
}

INLINE psy_dsp_big_beat_t psy_audio_sequencetrackiterator_seqoffset(
	psy_audio_SequenceTrackIterator* self)
{	
	psy_audio_SequenceEntry* seqentry;

	seqentry = psy_audio_sequencetrackiterator_entry(self);
	if (seqentry) {
		return psy_audio_sequenceentry_offset(seqentry);
	}
	return 0.0;
}

INLINE psy_dsp_big_beat_t psy_audio_sequencetrackiterator_offset(
	psy_audio_SequenceTrackIterator* self)
{			
	return psy_audio_sequencetrackiterator_seqoffset(self) +
		((self->patternnode)
			? psy_audio_sequencetrackiterator_patternentry(self)->offset
			: 0.0);
}

INLINE psy_dsp_big_beat_t psy_audio_sequencetrackiterator_entry_length(
	psy_audio_SequenceTrackIterator* self)
{	
	psy_audio_SequenceEntry* seqentry;

	seqentry = psy_audio_sequencetrackiterator_entry(self);
	if (seqentry) {
		return psy_audio_sequenceentry_length(seqentry);
	}
	return 0.0;
}

typedef psy_List psy_audio_SequenceTrackNode;

/*
** psy_audio_SequencePosition
**
**  defines the overall position in a song
*/
typedef struct {
	uintptr_t track_index;
	psy_audio_SequenceTrackIterator trackposition;	
} psy_audio_SequencePosition;

void psy_audio_sequenceposition_init(psy_audio_SequencePosition*);
void psy_audio_sequenceposition_dispose(psy_audio_SequencePosition*);

psy_audio_SequenceEntry* psy_audio_sequenceposition_entry(
	psy_audio_SequencePosition*);


struct psy_audio_Sequencer;

/* psy_audio_Sequence */
typedef struct psy_audio_Sequence {
	/* signals */	
	psy_Signal signal_insert;
	psy_Signal signal_remove;
	psy_Signal signal_reorder;
	psy_Signal signal_trackinsert;
	psy_Signal signal_trackremove;
	psy_Signal signal_trackswap;
	psy_Signal signal_trackreposition;
	psy_Signal signal_clear;
	psy_Signal signal_mutechanged;
	psy_Signal signal_solochanged;
	psy_Signal signal_cursorchanged;
	psy_Signal signal_tweak;
	/* internal */	
	psy_Table tracks;
	psy_audio_SequenceTrack globaltrack;
	psy_audio_Pattern globalpattern;
	psy_audio_TrackState trackstate;
	bool preventreposition;
	/* editposition */
	psy_audio_SequenceCursor cursor;
	psy_audio_SequenceCursor lastcursor;
	psy_audio_SequenceSelection selection;
	/* calcduration */
	struct psy_audio_Sequencer* sequencerduration;
	psy_dsp_big_seconds_t durationms;
	/* references */
	psy_audio_Patterns* patterns;
	psy_audio_Samples* samples;	
} psy_audio_Sequence;

void psy_audio_sequence_init(psy_audio_Sequence*, psy_audio_Patterns*,
	psy_audio_Samples*);
void psy_audio_sequence_dispose(psy_audio_Sequence*);
void psy_audio_sequence_copy(psy_audio_Sequence*, psy_audio_Sequence* other);

void psy_audio_sequence_clear(psy_audio_Sequence*);
/* sequenceentry methods */
void psy_audio_sequence_insert(psy_audio_Sequence*, psy_audio_OrderIndex,
	uintptr_t patidx);
void psy_audio_sequence_insert_sample(psy_audio_Sequence*,
	psy_audio_OrderIndex, psy_audio_SampleIndex);
void psy_audio_sequence_insert_marker(psy_audio_Sequence*,
	psy_audio_OrderIndex, const char* text);
void psy_audio_sequence_remove(psy_audio_Sequence*, psy_audio_OrderIndex);
void psy_audio_sequence_remove_selection(psy_audio_Sequence*,
	psy_audio_SequenceSelection*);
psy_audio_SequenceEntry* psy_audio_sequence_entry(psy_audio_Sequence*,
	psy_audio_OrderIndex);
const psy_audio_SequenceEntry* psy_audio_sequence_entry_const(const
	psy_audio_Sequence*, psy_audio_OrderIndex);
psy_audio_OrderIndex psy_audio_sequence_reorder(psy_audio_Sequence*,
	psy_audio_OrderIndex, psy_dsp_big_beat_t newposition);
void psy_audio_sequence_resetpatterns(psy_audio_Sequence*);
void psy_audio_sequence_fillpatterns(psy_audio_Sequence*);
void psy_audio_reposition(psy_audio_Sequence*);

void psy_audio_sequence_tweak(psy_audio_Sequence*);

uintptr_t psy_audio_sequence_order(const psy_audio_Sequence*,
	uintptr_t trackidx, psy_dsp_big_beat_t position);
psy_audio_Pattern* psy_audio_sequence_pattern(psy_audio_Sequence*,
	psy_audio_OrderIndex);	
void psy_audio_sequence_setpatternindex(psy_audio_Sequence*,
	psy_audio_OrderIndex, uintptr_t patidx);
uintptr_t psy_audio_sequence_patternindex(const psy_audio_Sequence*,
	psy_audio_OrderIndex);
bool psy_audio_sequence_patternused(psy_audio_Sequence*, uintptr_t patidx);
psy_audio_OrderIndex psy_audio_sequence_patternfirstused(psy_audio_Sequence*,
	uintptr_t patidx);
/* track methods */
void psy_audio_sequence_append_track(psy_audio_Sequence*,
	psy_audio_SequenceTrack*);
uintptr_t psy_audio_sequence_set_track(psy_audio_Sequence*,
	psy_audio_SequenceTrack*, uintptr_t index);
void psy_audio_sequence_remove_track(
	psy_audio_Sequence*, uintptr_t track_idx);
uintptr_t psy_audio_sequence_width(const psy_audio_Sequence*);
psy_audio_SequenceTrack* psy_audio_sequence_track_at(psy_audio_Sequence*,
	uintptr_t trackidx);
const psy_audio_SequenceTrack* psy_audio_sequence_track_at_const(const
	psy_audio_Sequence*, uintptr_t trackidx);
uintptr_t psy_audio_sequence_num_tracks(const psy_audio_Sequence*);
uintptr_t psy_audio_sequence_track_size(const psy_audio_Sequence*,
	uintptr_t trackindex);
uintptr_t psy_audio_sequence_maxtracksize(psy_audio_Sequence*);
void psy_audio_sequence_reposition_track(psy_audio_Sequence*,
	psy_audio_SequenceTrack*);
psy_dsp_big_beat_t psy_audio_sequence_offset(const psy_audio_Sequence*,
	psy_audio_OrderIndex);
void psy_audio_sequence_swaptracks(psy_audio_Sequence*,
	uintptr_t src, uintptr_t dst);
/* play selection */
void psy_audio_sequence_setplayselection(psy_audio_Sequence*,
	struct psy_audio_SequenceSelection*);
void psy_audio_sequence_clearplayselection(psy_audio_Sequence*);
/* methods for the sequencer */
void psy_audio_sequence_begin(psy_audio_Sequence*, uintptr_t track_idx,
	psy_dsp_big_beat_t position, psy_audio_SequenceTrackIterator* rv);
void psy_audio_sequence_at(psy_audio_Sequence*, uintptr_t trackindex,
	uintptr_t position, psy_audio_SequencePosition* rv);
void psy_audio_sequence_make_position(psy_audio_Sequence*,
	uintptr_t track_idx, psy_List* entries,
	psy_audio_SequencePosition* rv);
psy_audio_SequenceEntryNode* psy_audio_sequence_node(psy_audio_Sequence*,
	psy_audio_OrderIndex, psy_audio_SequenceTrack** rv);
/* calcduration */
psy_dsp_big_beat_t psy_audio_sequence_duration(const psy_audio_Sequence*);
void psy_audio_sequence_startcalcdurationinms(psy_audio_Sequence*);
psy_dsp_big_seconds_t psy_audio_sequence_endcalcdurationinmsresult(
	psy_audio_Sequence*);
bool psy_audio_sequence_calcdurationinms(psy_audio_Sequence*);
/* trackstate */
void psy_audio_sequence_activatesolotrack(psy_audio_Sequence*,
	uintptr_t trackindex);
void psy_audio_sequence_deactivatesolotrack(psy_audio_Sequence*);
void psy_audio_sequence_mutetrack(psy_audio_Sequence*,
	uintptr_t trackindex);
void psy_audio_sequence_unmutetrack(psy_audio_Sequence*,
	uintptr_t trackindex);
int psy_audio_sequence_istrackmuted(const psy_audio_Sequence*,
	uintptr_t trackindex);
bool psy_audio_sequence_istracksoloed(const psy_audio_Sequence*,
	uintptr_t trackindex);
/* gui stuff */
double psy_audio_sequence_trackheight(const psy_audio_Sequence*,
	uintptr_t trackindex);
void psy_audio_sequence_settrackheight(psy_audio_Sequence*,
	uintptr_t trackindex, double height);
void psy_audio_sequence_set_cursor(psy_audio_Sequence*,
	psy_audio_SequenceCursor);
double psy_audio_sequence_seqoffset(const psy_audio_Sequence*,
	psy_audio_OrderIndex);
void psy_audio_sequence_dec_seqpos(psy_audio_Sequence*);
void psy_audio_sequence_inc_seqpos(psy_audio_Sequence*);

INLINE psy_audio_Patterns* psy_audio_sequence_patterns(
	psy_audio_Sequence* self)
{
	return self->patterns;
}

INLINE const psy_audio_Patterns* psy_audio_sequence_patterns_const(
	const psy_audio_Sequence* self)
{
	return self->patterns;
}

INLINE psy_audio_SequenceCursor psy_audio_sequence_cursor(
	const psy_audio_Sequence* self)
{
	return self->cursor;
}

INLINE bool psy_audio_sequence_lpb_changed(const psy_audio_Sequence* self)
{
	assert(self);

	return (psy_audio_sequencecursor_lpb(&self->cursor) !=
		psy_audio_sequencecursor_lpb(&self->lastcursor));
}

void psy_audio_sequence_blockremove(psy_audio_Sequence*,
	psy_audio_BlockSelection selection);
void psy_audio_sequence_blocktranspose(psy_audio_Sequence*,
	psy_audio_BlockSelection selection, intptr_t offset);
void psy_audio_sequence_blockcopypattern(psy_audio_Sequence*,
	psy_audio_BlockSelection selection,
	psy_audio_Pattern* dest);
void psy_audio_sequence_blockpastepattern(psy_audio_Sequence*,
	psy_audio_BlockSelection selection,
	psy_audio_Pattern* source);

void psy_audio_sequence_block_traverse(psy_audio_Sequence*,
	psy_audio_BlockSelection selection, psy_Command*);
void psy_audio_sequence_block_traverse_lines(psy_audio_Sequence*,
	psy_audio_BlockSelection selection, psy_Command*);

psy_audio_PatternEvent psy_audio_sequence_pattern_event_at_cursor(
	const psy_audio_Sequence*, psy_audio_SequenceCursor);

const psy_audio_Pattern* psy_audio_sequence_pattern_const(
	const psy_audio_Sequence*, psy_audio_OrderIndex);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SEQUENCE_H */
