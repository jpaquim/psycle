// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQUENCE_H)
#define SEQUENCE_H

#include "patterns.h"
#include <list.h>
#include <signal.h>

typedef struct {
	uintptr_t pattern;
	psy_dsp_beat_t offset;
	int selplay;
	psy_List* node;
	int id;
} SequenceEntry;

void sequenceentry_init(SequenceEntry*, uintptr_t pattern, psy_dsp_beat_t offset);
SequenceEntry* sequenceentry_alloc(void);
SequenceEntry* sequenceentry_allocinit(uintptr_t pattern, psy_dsp_beat_t offset);

typedef struct {
	psy_List* entries;	
} SequenceTrack;

void sequencetrack_init(SequenceTrack*);
SequenceTrack* sequencetrack_alloc(void);
SequenceTrack* sequencetrack_allocinit(void);

typedef psy_List SequenceTrackNode;

typedef struct {
	psy_audio_Patterns* patterns;	
	SequenceTrackNode* tracknode;
	PatternNode* patternnode;	
} SequenceTrackIterator;

void sequencetrackiterator_inc(SequenceTrackIterator*);
void sequencetrackiterator_incentry(SequenceTrackIterator*);
void sequencetrackiterator_decentry(SequenceTrackIterator*);
PatternNode* sequencetrackiterator_patternnode(SequenceTrackIterator*);
SequenceEntry* sequencetrackiterator_entry(SequenceTrackIterator*);
psy_audio_PatternEntry* sequencetrackiterator_patternentry(SequenceTrackIterator*);
psy_dsp_beat_t sequencetrackiterator_offset(SequenceTrackIterator*);

typedef psy_List SequenceTracks;

typedef struct {
	SequenceTracks* track;
	SequenceTrackIterator trackposition;	
} SequencePosition;

void sequenceposition_init(SequencePosition*);
SequenceEntry* sequenceposition_entry(SequencePosition*);

typedef enum {
	SELECTIONMODE_SINGLE,
	SELECTIONMODE_MULTI
} SelectionMode;

typedef struct SequenceSelection {
	struct psy_audio_Sequence* sequence;	
	SequencePosition editposition;
	psy_Signal signal_editpositionchanged;
	psy_List* entries;
	SelectionMode selectionmode;
} SequenceSelection;

typedef struct psy_audio_Sequence {
	SequenceTracks* tracks;
	psy_audio_Patterns* patterns;
} psy_audio_Sequence;

void sequence_init(psy_audio_Sequence*, psy_audio_Patterns*);
void sequence_dispose(psy_audio_Sequence*);
SequenceTrackNode* sequence_insert(psy_audio_Sequence*, SequencePosition position, int pattern);
SequenceTrackNode* sequence_remove(psy_audio_Sequence*, SequencePosition position);
unsigned int sequence_size(psy_audio_Sequence*, psy_List* track);
SequencePosition sequence_at(psy_audio_Sequence*, unsigned int trackindex, unsigned int position);
SequenceTrackIterator sequence_begin(psy_audio_Sequence*, psy_List* track, psy_dsp_beat_t offset);
SequenceTrackIterator sequence_last(psy_audio_Sequence*, psy_List* track);
void sequence_clear(psy_audio_Sequence*);
psy_List* sequence_appendtrack(psy_audio_Sequence*, SequenceTrack*);
psy_List* sequence_removetrack(psy_audio_Sequence*, SequenceTracks*);
unsigned int sequence_sizetracks(psy_audio_Sequence*);
unsigned int sequence_maxtracksize(psy_audio_Sequence*);
int sequence_patternused(psy_audio_Sequence*, unsigned int patternslot);
void sequence_setpatternslot(psy_audio_Sequence*, SequencePosition, unsigned int slot);
psy_dsp_beat_t sequence_duration(psy_audio_Sequence*);
SequencePosition sequence_makeposition(psy_audio_Sequence*, SequenceTracks*, psy_List* entries);
SequencePosition sequence_positionfromid(psy_audio_Sequence*, int id);
void sequence_setplayselection(psy_audio_Sequence*, struct SequenceSelection*);
void sequence_clearplayselection(psy_audio_Sequence*);

void sequenceselection_init(SequenceSelection*, psy_audio_Sequence*);
void sequenceselection_seteditposition(SequenceSelection*, SequencePosition);
SequencePosition sequenceselection_editposition(SequenceSelection*);
void sequenceselection_setsequence(SequenceSelection*, psy_audio_Sequence*);



#endif

