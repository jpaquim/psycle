// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SEQUENCE_H
#define psy_audio_SEQUENCE_H

// audio
#include "patterns.h"
// container
#include <list.h>
#include <signal.h>

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uintptr_t id;
	uintptr_t row;
	uintptr_t patternslot;
	psy_dsp_big_beat_t offset;
	int selplay;
	psy_List* node;	
} psy_audio_SequenceEntry;

void psy_audio_sequenceentry_init(psy_audio_SequenceEntry*, uintptr_t
	patternslot, psy_dsp_big_beat_t offset);
psy_audio_SequenceEntry* psy_audio_sequenceentry_alloc(void);
psy_audio_SequenceEntry* psy_audio_sequenceentry_allocinit(uintptr_t
	patternslot, psy_dsp_big_beat_t offset);

INLINE void psy_audio_sequenceentry_setpatternslot(psy_audio_SequenceEntry*
	self, uintptr_t slot)
{
	assert(self);

	self->patternslot = slot;
}

INLINE uintptr_t psy_audio_sequenceentry_patternslot(const
	psy_audio_SequenceEntry* self)
{
	assert(self);

	return self->patternslot;
}

INLINE psy_audio_Pattern* psy_audio_sequenceentry_pattern(const
	psy_audio_SequenceEntry* self, psy_audio_Patterns* patterns)
{
	assert(self);

	if (patterns) {
		return (psy_audio_Pattern*)psy_audio_patterns_at(patterns,
			self->patternslot);		
	}
	return NULL;
}

typedef psy_List SequenceTrackNode;

typedef struct {
	psy_List* entries;	
} psy_audio_SequenceTrack;

void psy_audio_sequencetrack_init(psy_audio_SequenceTrack*);

psy_audio_SequenceTrack* psy_audio_sequencetrack_alloc(void);
psy_audio_SequenceTrack* psy_audio_sequencetrack_allocinit(void);

psy_dsp_big_beat_t psy_audio_sequencetrack_duration(psy_audio_SequenceTrack*,
	psy_audio_Patterns* patterns);

typedef struct {
	psy_audio_Patterns* patterns;	
	SequenceTrackNode* tracknode;
	psy_audio_PatternNode* patternnode;	
} psy_audio_SequenceTrackIterator;

void psy_audio_sequencetrackiterator_inc(psy_audio_SequenceTrackIterator*);
void psy_audio_sequencetrackiterator_inc_entry(psy_audio_SequenceTrackIterator*);
void psy_audio_sequencetrackiterator_dec_entry(psy_audio_SequenceTrackIterator*);

INLINE psy_audio_PatternNode* psy_audio_sequencetrackiterator_patternnode(
	psy_audio_SequenceTrackIterator* self)
{
	return self->patternnode;
}

INLINE psy_audio_SequenceEntry* psy_audio_sequencetrackiterator_entry(
	psy_audio_SequenceTrackIterator* self)
{
	return (self->tracknode)
		? (psy_audio_SequenceEntry*)psy_list_entry(self->tracknode)
		: NULL;
}

INLINE psy_audio_PatternEntry* psy_audio_sequencetrackiterator_patternentry(
	psy_audio_SequenceTrackIterator* self)
{
	return (self->patternnode)
		? (psy_audio_PatternEntry*)(self->patternnode)->entry
		: NULL;
}

INLINE psy_dsp_big_beat_t psy_audio_sequencetrackiterator_offset(
	psy_audio_SequenceTrackIterator* self)
{
	return (psy_audio_sequencetrackiterator_patternentry(self))
		? psy_audio_sequencetrackiterator_entry(self)->offset +
		  psy_audio_sequencetrackiterator_patternentry(self)->offset
		: psy_audio_sequencetrackiterator_entry(self)->offset;
}

typedef psy_List psy_audio_SequenceTracks;

typedef struct {
	psy_audio_SequenceTracks* track;
	psy_audio_SequenceTrackIterator trackposition;	
} psy_audio_SequencePosition;

void psy_audio_sequenceposition_init(psy_audio_SequencePosition*);
psy_audio_SequenceEntry* psy_audio_sequenceposition_entry(
	psy_audio_SequencePosition*);

typedef enum {
	psy_audio_SEQUENCE_SELECTIONMODE_SINGLE,
	psy_audio_SEQUENCE_SELECTIONMODE_MULTI
} psy_audio_SequenceSelectionMode;

struct psy_audio_Sequence;

typedef struct psy_audio_SequenceSelection {
	struct psy_audio_Sequence* sequence;	
	psy_audio_SequencePosition editposition;
	psy_Signal signal_editpositionchanged;
	psy_List* entries;
	psy_audio_SequenceSelectionMode selectionmode;
} psy_audio_SequenceSelection;

void psy_audio_sequenceselection_init(psy_audio_SequenceSelection*,
	struct psy_audio_Sequence*);
void psy_audio_sequenceselection_dispose(psy_audio_SequenceSelection*);
void psy_audio_sequenceselection_seteditposition(psy_audio_SequenceSelection*,
	psy_audio_SequencePosition);
psy_audio_SequencePosition psy_audio_sequenceselection_editposition(
	psy_audio_SequenceSelection*);
void psy_audio_sequenceselection_setsequence(psy_audio_SequenceSelection*,
	struct psy_audio_Sequence*);

typedef struct psy_audio_Sequence {
	psy_audio_SequenceTracks* tracks;
	psy_audio_Patterns* patterns;
	psy_Signal sequencechanged;
} psy_audio_Sequence;

void psy_audio_sequence_init(psy_audio_Sequence*, psy_audio_Patterns*);
void psy_audio_sequence_dispose(psy_audio_Sequence*);
SequenceTrackNode* psy_audio_sequence_insert(psy_audio_Sequence*,
	psy_audio_SequencePosition, uintptr_t patternslot);
SequenceTrackNode* psy_audio_sequence_remove(psy_audio_Sequence*, psy_audio_SequencePosition);
uintptr_t psy_audio_sequence_size(psy_audio_Sequence*,
	psy_audio_SequenceTracks* track);
psy_audio_SequencePosition psy_audio_sequence_at(psy_audio_Sequence*, uintptr_t trackindex,
	uintptr_t position);
psy_audio_SequenceTrackIterator psy_audio_sequence_begin(psy_audio_Sequence*,
	psy_List* track, psy_dsp_big_beat_t position);
psy_audio_SequenceTrackIterator psy_audio_sequence_last(psy_audio_Sequence*,
	psy_List* track);
void psy_audio_sequence_clear(psy_audio_Sequence*);
psy_List* psy_audio_sequence_appendtrack(psy_audio_Sequence*, psy_audio_SequenceTrack*);
psy_List* psy_audio_sequence_removetrack(psy_audio_Sequence*, psy_audio_SequenceTracks*);
uintptr_t psy_audio_sequence_sizetracks(psy_audio_Sequence*);
uintptr_t psy_audio_sequence_maxtracksize(psy_audio_Sequence*);
bool psy_audio_sequence_patternused(psy_audio_Sequence*, uintptr_t patternslot);
void psy_audio_sequence_setpatternslot(psy_audio_Sequence*, psy_audio_SequencePosition,
	uintptr_t slot);
psy_dsp_big_beat_t psy_audio_sequence_duration(psy_audio_Sequence*);
psy_dsp_big_seconds_t psy_audio_sequence_calcdurationinms(psy_audio_Sequence*);
psy_audio_SequencePosition psy_audio_sequence_makeposition(psy_audio_Sequence*,
	psy_audio_SequenceTracks*, psy_List* entries);
psy_audio_SequencePosition psy_audio_sequence_positionfromid(psy_audio_Sequence*, int id);
void psy_audio_sequence_setplayselection(psy_audio_Sequence*,
	struct psy_audio_SequenceSelection*);
void psy_audio_sequence_clearplayselection(psy_audio_Sequence*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SEQUENCE_H */
