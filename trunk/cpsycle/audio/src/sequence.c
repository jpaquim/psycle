// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequence.h"
#include "sequencer.h" // calculate duration

#include <stdlib.h>
#include <stdlib.h>

#include "../../detail/portable.h"

static int sequenceentryid = 1;

// psy_audio_SequenceSelection
// prototypes
static void sequenceselection_addeditposition(psy_audio_SequenceSelection*);
static psy_audio_PatternNode* SequenceTrackIterator_next(psy_audio_SequenceTrackIterator*);
static void SequenceTrackIterator_unget(psy_audio_SequenceTrackIterator*);
static psy_audio_SequenceTrackIterator sequence_makeiterator(psy_audio_Sequence*, psy_List* entries);
// implementation
void psy_audio_sequenceselection_init(psy_audio_SequenceSelection* self, psy_audio_Sequence* sequence)
{
	self->sequence = sequence;
	self->entries = 0;
	self->selectionmode = psy_audio_SEQUENCE_SELECTIONMODE_SINGLE;
	self->editposition = psy_audio_sequence_makeposition(self->sequence,
		self->sequence->tracks, 
		self->sequence->tracks
			? ((psy_audio_SequenceTrack*) self->sequence->tracks->entry)->entries
			: 0);
	sequenceselection_addeditposition(self);
	psy_signal_init(&self->signal_editpositionchanged);
}

void psy_audio_sequenceselection_dispose(psy_audio_SequenceSelection* self)
{
	psy_signal_dispose(&self->signal_editpositionchanged);
	psy_list_free(self->entries);
}

void sequenceselection_addeditposition(psy_audio_SequenceSelection* self)
{						
	if (self->editposition.trackposition.sequencentrynode) {
		psy_list_append(&self->entries, 
			self->editposition.trackposition.sequencentrynode->entry);
	}
	psy_audio_sequence_setplayselection(self->sequence, self);
}

void psy_audio_sequenceselection_seteditposition(psy_audio_SequenceSelection* self,
	psy_audio_SequencePosition position)
{
	psy_List* p;
	int append = 1;

	if (self->selectionmode == psy_audio_SEQUENCE_SELECTIONMODE_SINGLE) {
		psy_list_free(self->entries);
		self->entries = 0;
	} else
	if ((p = psy_list_findentry(self->entries, 
			position.trackposition.sequencentrynode->entry)) != 0) {
		psy_list_remove(&self->entries, p);
		append = 0;
	}
	self->editposition = position;
	if (append) {
		if (self->editposition.trackposition.sequencentrynode) {
			psy_list_append(&self->entries,
			self->editposition.trackposition.sequencentrynode->entry);
		}
	} else {		
		psy_audio_SequenceTrack* track;
		psy_List* p = psy_list_last(self->entries);
			
		if (p) {
			track = (psy_audio_SequenceTrack*)position.tracknode->entry;
			p = psy_list_findentry(track->entries, p->entry);			
			self->editposition = psy_audio_sequence_makeposition(self->sequence,
				position.tracknode,
				p);
		}
	}
// 	signal_emit(&self->signal_editpositionchanged, self, 0);
}

psy_audio_SequencePosition psy_audio_sequenceselection_editposition(psy_audio_SequenceSelection* self)
{
	return self->editposition;
}

void psy_audio_sequenceselection_setsequence(psy_audio_SequenceSelection* self,
	psy_audio_Sequence* sequence)
{
	self->sequence = sequence;
	self->editposition = psy_audio_sequence_makeposition(self->sequence,
		self->sequence->tracks, 
		self->sequence->tracks
			? ((psy_audio_SequenceTrack*) self->sequence->tracks->entry)->entries
			: 0);
	free(self->entries);
	self->entries = 0;
	sequenceselection_addeditposition(self);	
}

psy_dsp_big_beat_t psy_audio_sequenceselection_editposition_offset(
	const psy_audio_SequenceSelection* self)
{
	psy_audio_SequenceEntry* entry;

	entry = psy_audio_sequenceposition_entry(
		&((psy_audio_SequenceSelection*)self)->editposition);
	return (entry)
		? entry->offset
		: (psy_dsp_big_beat_t)0.0;
}

uintptr_t psy_audio_sequenceselection_editposition_entry_id(
	const psy_audio_SequenceSelection* self)
{
	uintptr_t rv;
	assert(self);
	
	if (self->editposition.trackposition.sequencentrynode) {
		psy_audio_SequenceEntry* entry;

		entry = (psy_audio_SequenceEntry*)
			self->editposition.trackposition.sequencentrynode->entry;
		rv = entry->id;
	} else {
		rv = UINTPTR_MAX;
	}
	return rv;
}


// psy_audio_SequenceTrack
void psy_audio_sequencetrack_init(psy_audio_SequenceTrack* self)
{
	self->entries = NULL;
	self->name = psy_strdup("seq");	
}

void psy_audio_sequencetrack_dispose(psy_audio_SequenceTrack* self)
{
	psy_list_deallocate(&self->entries, (psy_fp_disposefunc)NULL);
}


psy_audio_SequenceTrack* psy_audio_sequencetrack_alloc(void)
{
	return malloc(sizeof(psy_audio_SequenceTrack));
}

psy_audio_SequenceTrack* psy_audio_sequencetrack_allocinit(void)
{
	psy_audio_SequenceTrack* rv;

	rv = psy_audio_sequencetrack_alloc();
	if (rv) {
		psy_audio_sequencetrack_init(rv);
	}
	return rv;
}

psy_dsp_big_beat_t psy_audio_sequencetrack_duration(psy_audio_SequenceTrack* self,
	psy_audio_Patterns* patterns)
{
	psy_dsp_big_beat_t rv;
	psy_List* p;

	assert(self);

	rv = 0.0;
	p = self->entries;
	if (p) {
		psy_audio_Pattern* pattern;
		psy_audio_SequenceEntry* entry;

		p = p->tail;
		entry = (psy_audio_SequenceEntry*)p->entry;
		pattern = psy_audio_patterns_at(patterns, entry->patternslot);
		if (pattern) {
			if (rv < entry->offset + pattern->length) {
				rv = entry->offset + pattern->length;
			}
		}
	}
	return rv;
}

void psy_audio_sequencetrack_setname(psy_audio_SequenceTrack* self, const char* name)
{
	assert(self);

	self->name = psy_strreset(&self->name, name);
}


// psy_audio_SequenceEntry
void psy_audio_sequenceentry_init(psy_audio_SequenceEntry* self, uintptr_t patternslot,
	psy_dsp_big_beat_t offset)
{
	self->patternslot = patternslot;
	self->offset = offset;
	self->repositionoffset = 0.0;
	self->selplay = 0;
	self->id = sequenceentryid++;
	self->row = 0;
}

psy_audio_SequenceEntry* psy_audio_sequenceentry_alloc(void)
{
	return (psy_audio_SequenceEntry*) malloc(sizeof(psy_audio_SequenceEntry));
}

psy_audio_SequenceEntry* psy_audio_sequenceentry_allocinit(uintptr_t pattern, psy_dsp_big_beat_t offset)
{
	psy_audio_SequenceEntry* rv;

	rv = psy_audio_sequenceentry_alloc();
	if (rv) {
		psy_audio_sequenceentry_init(rv, pattern, offset);
	}
	return rv;
}

// psy_audio_SequencePosition
void psy_audio_sequenceposition_init(psy_audio_SequencePosition* self)
{
	assert(self);

	self->tracknode = NULL;
	self->trackposition.patternnode = NULL;
	self->trackposition.sequencentrynode = NULL;
}

psy_audio_SequenceEntry* psy_audio_sequenceposition_entry(psy_audio_SequencePosition* self)
{
	psy_audio_SequenceEntry* rv = 0;

	assert(self);

	if (self->trackposition.sequencentrynode) {
		rv = (psy_audio_SequenceEntry*)self->trackposition.sequencentrynode->entry;
	}
	return rv;
}

// Sequence
static void sequence_onpatternlengthchanged(psy_audio_Sequence* self,
	psy_audio_Pattern* sender);
static void sequence_onpatternnamechanged(psy_audio_Sequence* self,
	psy_audio_Pattern* sender);

void psy_audio_sequence_init(psy_audio_Sequence* self, psy_audio_Patterns* patterns)
{
	self->tracks = 0;
	self->patterns = patterns;
	psy_audio_trackstate_init(&self->trackstate);
	psy_signal_init(&self->sequencechanged);
	self->lastchange = psy_audio_SEQUENCE_CHANGE_NONE;
	self->lastchangedtrack = UINTPTR_MAX;
}

void psy_audio_sequence_dispose(psy_audio_Sequence* self)
{
	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		psy_audio_sequencetrack_dispose);
	psy_audio_trackstate_dispose(&self->trackstate);
	psy_signal_dispose(&self->sequencechanged);
}

psy_audio_SequenceEntryNode* psy_audio_sequence_insert(psy_audio_Sequence* self, psy_audio_SequencePosition position,
	uintptr_t patternslot)
{		
	psy_List* rv = 0;
	if (position.tracknode) {
		psy_audio_SequenceEntry* entry;
		psy_audio_SequenceTrack* track;

		entry = psy_audio_sequenceentry_allocinit(patternslot, (psy_dsp_big_beat_t)0.f);
		track = (psy_audio_SequenceTrack*)position.tracknode->entry;
		if (track->entries) {		
			if (position.trackposition.sequencentrynode) {
				rv = psy_list_insert(&track->entries,
					position.trackposition.sequencentrynode, entry);
				psy_audio_sequence_reposition_track(self, track);				
				psy_signal_emit(&self->sequencechanged, self, 0);
			} 
		} else {		
			rv = track->entries = psy_list_create(entry);		
		}
		if (self->patterns) {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_patterns_at(self->patterns, patternslot);
			if (pattern) {
				psy_signal_connect(&pattern->signal_lengthchanged,
					self, sequence_onpatternlengthchanged);
				psy_signal_connect(&pattern->signal_namechanged,
					self, sequence_onpatternnamechanged);
			}
		}
	}
	return rv;
}

psy_audio_SequenceEntryNode* psy_audio_sequence_remove(psy_audio_Sequence* self,
	psy_audio_SequencePosition position)
{
	assert(self);

	if (position.tracknode) {
		psy_audio_SequenceEntryNode* rv;
		psy_audio_SequenceTrack* track;			

		rv = NULL;
		track = (psy_audio_SequenceTrack*)position.tracknode->entry;
		if (position.trackposition.sequencentrynode) {
			psy_audio_SequenceEntry* sequenceentry;
			uintptr_t patternslot;

			sequenceentry = (psy_audio_SequenceEntry*)
				position.trackposition.sequencentrynode->entry;
			patternslot = sequenceentry->patternslot;

			rv = psy_list_remove(&track->entries, 
				position.trackposition.sequencentrynode);
			if (track->entries != NULL) {				
				psy_audio_sequence_reposition_track(self, track);
				psy_signal_emit(&self->sequencechanged, self, 0);
			}
			if (self->patterns && !psy_audio_sequence_patternused(self,
					patternslot)) {
				psy_audio_Pattern* pattern;

				pattern = psy_audio_patterns_at(self->patterns, patternslot);
				if (pattern) {
					psy_signal_disconnect(&pattern->signal_lengthchanged,
						self, sequence_onpatternlengthchanged);
					psy_signal_disconnect(&pattern->signal_namechanged,
						self, sequence_onpatternnamechanged);
				}
			}
		}
	}
	return NULL;
}

psy_audio_SequenceEntryNode* psy_audio_sequence_reorder_entry(psy_audio_Sequence* self,	
	psy_audio_SequencePosition position, psy_dsp_big_beat_t newposition)
{
	psy_audio_SequenceEntryNode* rv;
	psy_audio_SequenceTrack* track;
	psy_audio_SequenceEntry* seqentry;	
	psy_dsp_big_beat_t curroffset;
	psy_List* p;
	uintptr_t row;
	bool append;

	assert(self);
	assert(position.trackposition.sequencentrynode);
	assert(position.tracknode);

	track = (psy_audio_SequenceTrack*)position.tracknode->entry;
	seqentry = position.trackposition.sequencentrynode->entry;
	psy_list_remove(&track->entries,
		position.trackposition.sequencentrynode);	
	psy_audio_sequence_reposition_track(self, track);
	append = TRUE;
	for (p = track->entries, row = 0, curroffset = 0.0; p != NULL;
		psy_list_next(&p), ++row) {
		psy_audio_Pattern* pattern;
		psy_audio_SequenceEntry* sequenceentry;

		sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
		pattern = psy_audio_patterns_at(self->patterns,
			psy_audio_sequenceentry_patternslot(sequenceentry));
		curroffset = sequenceentry->offset;
		if (newposition < curroffset + pattern->length) {
			append = FALSE;
			if (p->prev) {
				p = p->prev;
			}
			break;
		}
	}	
	if (!append) {
		rv = psy_list_insert(&track->entries, p, seqentry);
	} else {
		rv = psy_list_append(&track->entries, seqentry);
	}
	psy_audio_sequence_reposition_track(self, track);
	psy_signal_emit(&self->sequencechanged, self, 0);
	return rv;
}

void psy_audio_sequence_clear(psy_audio_Sequence* self)
{
	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		psy_audio_sequencetrack_dispose);
}

psy_audio_SequenceTrackIterator psy_audio_sequence_last(psy_audio_Sequence* self,
	psy_List* tracknode)
{
	psy_audio_SequenceTrackIterator p;
	psy_audio_SequenceTrack* track;

	if (tracknode == 0) {
		return psy_audio_sequence_begin(self, tracknode, 0);
	}
	 
	track = (psy_audio_SequenceTrack*) tracknode->entry;
	if (track->entries == 0) {
		return psy_audio_sequence_begin(self, tracknode, 0);
	}	
	p.patternnode = 0;		
	p.sequencentrynode = track->entries->tail;
	if (p.sequencentrynode) {
		psy_audio_Pattern* pattern;
		psy_audio_SequenceEntry* entry = (psy_audio_SequenceEntry*) track->entries->tail->entry;
		pattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
		if (pattern) {
			p.patternnode = pattern->events;
		} else {
			p.patternnode = 0;
		}
	}
	return p;
}

void psy_audio_sequence_reposition_track(psy_audio_Sequence* self, psy_audio_SequenceTrack* track)
{
	psy_dsp_big_beat_t curroffset;	
	psy_List* p;
	uintptr_t row;
			
	for (p = track->entries, row = 0, curroffset = 0.0; p != NULL;
			psy_list_next(&p), ++row) {
		psy_audio_Pattern* pattern;
		psy_audio_SequenceEntry* sequenceentry;
		
		sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
		sequenceentry->row = row;
		pattern = psy_audio_patterns_at(self->patterns,
			psy_audio_sequenceentry_patternslot(sequenceentry));
		if (pattern) {
			sequenceentry->offset = curroffset + sequenceentry->repositionoffset;
			curroffset = sequenceentry->offset + pattern->length;
		} else {
			sequenceentry->offset = curroffset + sequenceentry->repositionoffset;
		}		
	}
	self->lastchange = psy_audio_SEQUENCE_CHANGE_REPOSITION;
	self->lastchangedtrack = psy_list_entry_index(self->tracks, track);
}

void psy_audio_reposition(psy_audio_Sequence* self)
{
	psy_audio_SequenceTrackNode* t;

	for (t = self->tracks; t != NULL; psy_list_next(&t)) {
		psy_audio_sequence_reposition_track(self, (psy_audio_SequenceTrack*)(t->entry));
	}
}

uintptr_t psy_audio_sequence_size(psy_audio_Sequence* self,
	psy_audio_SequenceTrackNode* tracknode)
{		
	if (tracknode) {
		psy_audio_SequenceTrack* track;		
		
		track = (psy_audio_SequenceTrack*)psy_list_entry(tracknode);
		return psy_list_size(track->entries);
	}
	return 0;
}

psy_audio_SequencePosition psy_audio_sequence_at(psy_audio_Sequence* self,
	uintptr_t trackindex, uintptr_t position)
{
	psy_audio_SequencePosition rv;	
	psy_List* ptr;	
	uintptr_t c = 0;
	psy_audio_SequenceTrackNode* ptracks;
	psy_audio_SequenceTrack* track;

	psy_audio_sequenceposition_init(&rv);	
	rv.trackposition.patterns = self->patterns;
	ptracks = self->tracks;
	while (ptracks) {
		if (c == trackindex) {
			break;
		}
		ptracks = ptracks->next;
		++c;
	}

	if (ptracks) {
		track = (psy_audio_SequenceTrack*)(ptracks->entry);
	} else {
		track = 0;
	}
	
	rv.tracknode = ptracks;
	if (rv.tracknode) {
		ptr = track->entries;
		c = 0;
		while (ptr) {
			if (c == position) {			
				rv = psy_audio_sequence_makeposition(self, ptracks, ptr);
				break;
			}
			++c;
			ptr = ptr->next;
		}
	}
	return rv;
}

psy_List* sequenceentry_at_offset(psy_audio_Sequence* self,
	psy_audio_SequenceTrackNode* tracknode, psy_dsp_big_beat_t offset)
{
	psy_dsp_big_beat_t curroffset = 0.0f;	
	psy_List* p = 0;

	if (tracknode) {		
		psy_audio_SequenceTrack* track;

		track = (psy_audio_SequenceTrack*)tracknode->entry;
		p = track->entries;	
		while (p) {
			psy_audio_Pattern* pattern;
			psy_audio_SequenceEntry* entry = (psy_audio_SequenceEntry*) p->entry;
			pattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
			if (pattern) {
				if (offset >= curroffset && offset < curroffset + pattern->length) {
					break;
				}
				curroffset += pattern->length;
			}
			psy_list_next(&p);
		}
	}
	return p;
}

psy_audio_SequenceTrackIterator psy_audio_sequence_begin(psy_audio_Sequence* self, psy_List* track,
	psy_dsp_big_beat_t pos)
{		
	psy_audio_SequenceTrackIterator rv;		
	psy_audio_SequenceEntry* entry;	

	rv.patterns = self->patterns;
	rv.sequencentrynode = sequenceentry_at_offset(self, track, pos);
	if (rv.sequencentrynode) {
		psy_audio_Pattern* pattern;

		entry = (psy_audio_SequenceEntry*) rv.sequencentrynode->entry;
		pattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
		rv.patternnode = psy_audio_pattern_greaterequal(pattern, pos - entry->offset);
	} else {
		rv.patternnode = 0;
	}
	return rv;	
}

void psy_audio_sequencetrackiterator_inc(psy_audio_SequenceTrackIterator* self)
{	
	if (self->patternnode) {		
		self->patternnode = self->patternnode->next;
		if (self->patternnode == NULL) {
			if (self->sequencentrynode->next) {
				psy_audio_SequenceEntry* entry;
				psy_audio_Pattern* pattern;
				self->sequencentrynode = self->sequencentrynode->next;
				entry = (psy_audio_SequenceEntry*) self->sequencentrynode->entry;
				pattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
				if (pattern) {
					self->patternnode = pattern->events;
				} else {
					self->patternnode = NULL;
				}
			}
		}
	}
}

void psy_audio_sequencetrackiterator_inc_entry(psy_audio_SequenceTrackIterator* self)
{	
	if (self->sequencentrynode) {
		self->sequencentrynode = self->sequencentrynode->next;
		if (self->sequencentrynode) {
			psy_audio_SequenceEntry* entry;
			psy_audio_Pattern* pattern;

			entry = (psy_audio_SequenceEntry*) self->sequencentrynode->entry;
			pattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
			if (pattern) {
				self->patternnode = pattern->events;
			} else {
				self->patternnode = 0;
			}
		} else {
			self->patternnode = 0;
		}
	}
}

void psy_audio_sequencetrackiterator_dec_entry(psy_audio_SequenceTrackIterator* self)
{	
	if (self->sequencentrynode) {
		self->sequencentrynode = self->sequencentrynode->prev;
		if (self->sequencentrynode) {
			psy_audio_SequenceEntry* entry;
			psy_audio_Pattern* pattern;

			entry = (psy_audio_SequenceEntry*) self->sequencentrynode->entry;
			pattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
			self->patternnode = pattern->events;
		} else {
			self->patternnode = 0;
		}
	}
}

psy_audio_SequenceTrackIterator sequence_makeiterator(psy_audio_Sequence* self, psy_List* entries)
{
	psy_audio_SequenceTrackIterator rv;
	psy_audio_Pattern* pPattern  = 0;	
	
	rv.patterns = self->patterns;
	rv.sequencentrynode = entries;
	if (entries) {
		psy_audio_SequenceEntry* entry = (psy_audio_SequenceEntry*) entries->entry;
		pPattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
		if (pPattern) {
			rv.patternnode = pPattern->events;
		} else {
			rv.patternnode = 0;
		}
	}		
	return rv;
}

psy_audio_SequencePosition psy_audio_sequence_makeposition(psy_audio_Sequence* self,
	psy_audio_SequenceTrackNode* track, psy_List* entries)
{
	psy_audio_SequencePosition rv;

	rv.trackposition = sequence_makeiterator(self, entries);
	rv.tracknode = track;
	return rv;
}

psy_audio_SequenceTrackNode* psy_audio_sequence_appendtrack(psy_audio_Sequence*
	self, psy_audio_SequenceTrack* track)
{	
	psy_List* rv;

	rv = psy_list_append(&self->tracks, track);
	psy_signal_emit(&self->sequencechanged, self, 0);
	return rv;
}

psy_audio_SequenceTrackNode* psy_audio_sequence_removetrack(psy_audio_Sequence*
	self, psy_audio_SequenceTrackNode* tracknode)
{	
	psy_List* rv;

	rv = psy_list_remove(&self->tracks, tracknode);
	psy_signal_emit(&self->sequencechanged, self, 0);
	return rv;
}

psy_audio_SequenceTrack* psy_audio_sequence_track_at(psy_audio_Sequence* self,
	uintptr_t index)
{
	psy_audio_SequenceTrackNode* t;

	assert(self);

	t = psy_list_at(self->tracks, index);
	if (t) {
		return (psy_audio_SequenceTrack*)psy_list_entry(t);
	}
	return NULL;
}

const psy_audio_SequenceTrack* psy_audio_sequence_track_at_const(const
	psy_audio_Sequence* self, uintptr_t index)
{
	assert(self);

	return psy_audio_sequence_track_at((psy_audio_Sequence*)self, index);
}

uintptr_t psy_audio_sequence_sizetracks(psy_audio_Sequence* self)
{
	uintptr_t c = 0;	
	psy_audio_SequenceTrackNode* p;
	
	for (p = self->tracks; p != NULL; psy_list_next(&p), ++c);
	return c;
}

bool psy_audio_sequence_patternused(psy_audio_Sequence* self, uintptr_t patternslot)
{
	int rv = FALSE;
	psy_audio_SequenceTrackNode* t;

	assert(self);

	t = self->tracks;
	while (t) {
		psy_audio_SequenceTrack* track;
		psy_audio_SequenceEntryNode* p;

		track = (psy_audio_SequenceTrack*)t->entry;
		p = track->entries;
		while (p) {
			psy_audio_SequenceEntry* sequenceentry;

			sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (psy_audio_sequenceentry_patternslot(sequenceentry) ==
					patternslot) {
				rv = TRUE;
				break;
			}
			psy_list_next(&p);
		}
		psy_list_next(&t);
	}
	return rv;
}

psy_audio_SequencePosition psy_audio_sequence_patternfirstused(psy_audio_Sequence* self,
	uintptr_t patternslot)
{
	psy_audio_SequencePosition rv;
	psy_audio_SequenceTrackNode* t;

	assert(self);
	rv.tracknode = NULL;
	rv.trackposition.patternnode = NULL;
	rv.trackposition.patterns = self->patterns;
	rv.trackposition.sequencentrynode = NULL;

	t = self->tracks;
	while (t) {
		psy_audio_SequenceTrack* track;
		psy_audio_SequenceEntryNode* p;

		track = (psy_audio_SequenceTrack*)t->entry;
		p = track->entries;
		while (p) {
			psy_audio_SequenceEntry* sequenceentry;

			sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (psy_audio_sequenceentry_patternslot(sequenceentry) ==
					patternslot) {
				psy_audio_Pattern* pattern;

				pattern = psy_audio_patterns_at(self->patterns, patternslot);
				rv.tracknode = t;
				rv.trackposition.sequencentrynode = p;
				rv.trackposition.patternnode = pattern->events;
				break;
			}
			psy_list_next(&p);
		}
		psy_list_next(&t);
	}
	return rv;
}

void psy_audio_sequence_setpatternslot(psy_audio_Sequence* self,
	psy_audio_SequencePosition position, uintptr_t slot)
{
	psy_audio_SequenceEntry* sequenceentry;

	assert(self);

	sequenceentry = psy_audio_sequenceposition_entry(&position);
	if (sequenceentry) {
		psy_audio_Pattern* pattern;		
		
		pattern = psy_audio_patterns_at(self->patterns, slot);
		if (pattern == NULL) {			
			pattern = psy_audio_pattern_allocinit();
			// todo set correct default lines
			psy_audio_patterns_insert(self->patterns, slot, pattern);		
		}
		psy_audio_sequenceentry_setpatternslot(sequenceentry, slot);
	}
}

psy_dsp_big_beat_t psy_audio_sequence_duration(psy_audio_Sequence* self)
{	
	psy_audio_SequenceTrackNode* t;
	psy_dsp_big_beat_t rv;

	assert(self);

	rv = 0.0;
	t = self->tracks;	
	for (t = self->tracks; t != NULL; psy_list_next(&t)) {
		psy_audio_SequenceTrack* track;
		psy_dsp_big_beat_t trackduration;

		track = (psy_audio_SequenceTrack*)t->entry;
		trackduration = psy_audio_sequencetrack_duration(track,
			self->patterns);		
		if (rv < trackduration) {
			rv = trackduration;			
		}
	}
	return rv;
}

uintptr_t psy_audio_sequence_maxtracksize(psy_audio_Sequence* self)
{
	uintptr_t rv = 0;
	psy_audio_SequenceTrackNode* t;
	
	assert(self);

	for (t = self->tracks; t != NULL; psy_list_next(&t)) {
		uintptr_t maxtracksize;

		maxtracksize = psy_audio_sequence_size(self, t);
		if (maxtracksize > rv) {
			rv = maxtracksize;
		}		
	}
	return rv;
}

void psy_audio_sequence_setplayselection(psy_audio_Sequence* self, psy_audio_SequenceSelection* selection)
{
	psy_List* p;

	psy_audio_sequence_clearplayselection(self);
	for (p = selection->entries; p != NULL; psy_list_next(&p)) {
		psy_audio_SequenceEntry* entry;

		entry = (psy_audio_SequenceEntry*)p->entry;
		entry->selplay = 1;
	}
}

void psy_audio_sequence_clearplayselection(psy_audio_Sequence* self)
{
	psy_audio_SequenceTrackNode* t;
	
	for (t = self->tracks; t != 0; t = t->next) {
		psy_audio_SequenceTrack* track;
		psy_List* p;

		track = t->entry;
		for (p = track->entries; p != NULL; psy_list_next(&p)) {
			psy_audio_SequenceEntry* entry;

			entry = (psy_audio_SequenceEntry*) p->entry;
			entry->selplay = 0;
		}
	}
}

psy_dsp_big_seconds_t psy_audio_sequence_calcdurationinms(psy_audio_Sequence* self)
{
	psy_dsp_big_seconds_t rv;
	psy_audio_Sequencer sequencer;	
	uintptr_t maxamount;
	uintptr_t amount;
	uintptr_t numsamplex;
	
	psy_audio_sequencer_init(&sequencer, self, NULL);
	psy_audio_sequencer_stoploop(&sequencer);
	psy_audio_sequencer_start(&sequencer);
	sequencer.calcduration = TRUE;
	while (psy_audio_sequencer_playing(&sequencer)) {
		numsamplex = psy_audio_MAX_STREAM_SIZE;
		maxamount = numsamplex;		
		do {
			amount = maxamount;
			if (amount > numsamplex) {
				amount = numsamplex;
			}
			if (sequencer.linetickcount <=
				psy_audio_sequencer_frametooffset(&sequencer, amount)) {
				if (sequencer.linetickcount > 0) {
					uintptr_t pre;

					pre = psy_audio_sequencer_frames(&sequencer,
						sequencer.linetickcount);
					if (pre) {
						pre--;
						if (pre) {
							psy_audio_sequencer_frametick(&sequencer, pre);
							numsamplex -= pre;
							amount -= pre;
							sequencer.linetickcount -=
								psy_audio_sequencer_frametooffset(
									&sequencer, pre);
						}
					}
				}					
				psy_audio_sequencer_onnewline(&sequencer);
			}			
			if (amount > 0) {
				psy_audio_sequencer_frametick(&sequencer, amount);
				numsamplex -= amount;
				sequencer.linetickcount -=
					psy_audio_sequencer_frametooffset(&sequencer, amount);				
			}
		} while (numsamplex > 0);
	}
	rv = psy_audio_sequencer_currplaytime(&sequencer);
	psy_audio_sequencer_dispose(&sequencer);	
	return rv;
}

void sequence_onpatternlengthchanged(psy_audio_Sequence* self,
	psy_audio_Pattern* sender)
{
	psy_audio_reposition(self);
	psy_signal_emit(&self->sequencechanged, self, 0);
}

void sequence_onpatternnamechanged(psy_audio_Sequence* self,
	psy_audio_Pattern* sender)
{
	psy_signal_emit(&self->sequencechanged, self, 0);
}

// TrackState
void psy_audio_sequence_activatesolotrack(psy_audio_Sequence* self,
	uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_activatesolotrack(&self->trackstate, track);
}

void psy_audio_sequence_deactivatesolotrack(psy_audio_Sequence* self)
{
	assert(self);

	psy_audio_trackstate_deactivatesolotrack(&self->trackstate);
}

void psy_audio_sequence_mutetrack(psy_audio_Sequence* self, uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_mutetrack(&self->trackstate, track);
}

void psy_audio_sequence_unmutetrack(psy_audio_Sequence* self, uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_unmutetrack(&self->trackstate, track);
}

int psy_audio_sequence_istrackmuted(const psy_audio_Sequence* self, uintptr_t track)
{
	assert(self);

	return psy_audio_trackstate_istrackmuted(&self->trackstate, track);
}

int psy_audio_sequence_istracksoloed(const psy_audio_Sequence* self, uintptr_t track)
{
	assert(self);

	return psy_audio_trackstate_istracksoloed(&self->trackstate, track);
}


// simplified methods, refactoring


void psy_audio_newsequenceselection_init(psy_audio_NewSequenceSelection* self)
{	
	self->editposition = psy_audio_sequenceorderindex_make(0, 0);
	self->entries = NULL;
	self->selectionmode = psy_audio_SEQUENCE_SELECTIONMODE_SINGLE;
}

void psy_audio_newsequenceselection_dispose(psy_audio_NewSequenceSelection* self)
{
	psy_list_deallocate(&self->entries, NULL);
}

void psy_audio_newsequenceselection_seteditposition(psy_audio_NewSequenceSelection* self,
	psy_audio_SequenceOrderIndex index)
{
	self->editposition = index;
}

bool psy_audio_newsequenceselection_exists(const psy_audio_NewSequenceSelection* self,
	psy_audio_SequenceOrderIndex index)
{
	psy_List* p;

	p = NULL;
	while (p) {
		psy_audio_SequenceOrderIndex* currindex;

		currindex = (psy_audio_SequenceOrderIndex*)psy_list_entry(p);
		if (currindex->order == index.order && currindex->track == index.track) {
			break;
		}
		psy_list_next(&p);
	}
	return p != NULL;
}

void psy_audio_sequence_insert_order(psy_audio_Sequence* self,
	psy_audio_SequenceOrderIndex index, uintptr_t patternslot)
{
	psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at(self, index.track);
	if (track) {
		psy_List* p;
		psy_audio_SequenceEntry* entry;

		p = psy_list_at(track->entries, index.order);			
		entry = psy_audio_sequenceentry_allocinit(patternslot,
			(psy_dsp_big_beat_t)0.0);
		psy_list_insert(&track->entries, p, entry);
		psy_audio_sequence_reposition_track(self, track);
		if (self->patterns) {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_patterns_at(self->patterns, patternslot);
			if (pattern) {
				psy_signal_connect(&pattern->signal_lengthchanged,
					self, sequence_onpatternlengthchanged);
				psy_signal_connect(&pattern->signal_namechanged,
					self, sequence_onpatternnamechanged);
			}
		}
		psy_signal_emit(&self->sequencechanged, self, 0);
	}
}

void psy_audio_sequence_remove_order(psy_audio_Sequence* self,
	psy_audio_SequenceOrderIndex index)
{
	psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at(self, index.track);
	if (track) {
		psy_List* p;

		p = psy_list_at(track->entries, index.order);
		if (p) {
			psy_audio_SequenceEntry* entry;
			uintptr_t patidx;

			entry = (psy_audio_SequenceEntry*)p->entry;
			patidx = entry->patternslot;
			psy_list_remove(&track->entries, p);
			free(entry);
			entry = NULL;
			psy_audio_sequence_reposition_track(self, track);			
			if (self->patterns &&
					!psy_audio_sequence_patternused(self, patidx)) {
				psy_audio_Pattern* pattern;

				pattern = psy_audio_patterns_at(self->patterns, patidx);
				if (pattern) {
					psy_signal_disconnect_context(&pattern->signal_lengthchanged, self);
					psy_signal_disconnect_context(&pattern->signal_namechanged, self);						
				}
			}
			psy_signal_emit(&self->sequencechanged, self, 0);
		}
	}
}

uintptr_t psy_audio_sequence_order(psy_audio_Sequence* self,
	uintptr_t trackidx, psy_dsp_big_beat_t position)
{
	psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at(self, trackidx);
	if (track) {
		psy_dsp_big_beat_t curroffset;
		psy_List* p;
		uintptr_t row;		
		
		for (p = track->entries, row = 0, curroffset = 0.0; p != NULL;
				psy_list_next(&p), ++row) {
			psy_audio_Pattern* pattern;
			psy_audio_SequenceEntry* sequenceentry;

			sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			pattern = psy_audio_patterns_at(self->patterns,
				psy_audio_sequenceentry_patternslot(sequenceentry));
			curroffset = sequenceentry->offset;
			if (position < curroffset + pattern->length) {				
				break;
			}
		}
		return row;
	}
	return UINTPTR_MAX;
}

psy_audio_SequenceEntry* psy_audio_sequence_entry(psy_audio_Sequence* self,
	psy_audio_SequenceOrderIndex index)
{
	psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at(self, index.track);
	if (track) {
		psy_List* p;

		p = psy_list_at(track->entries, index.order);
		if (p) {
			psy_audio_SequenceEntry* entry;			

			entry = (psy_audio_SequenceEntry*)p->entry;
			return entry;
		}
	}
	return NULL;
}

const psy_audio_SequenceEntry* psy_audio_sequence_entry_const(const
	psy_audio_Sequence* self, psy_audio_SequenceOrderIndex index)
{
	return psy_audio_sequence_entry((psy_audio_Sequence*)self, index);
}

psy_audio_Pattern* psy_audio_sequence_pattern(psy_audio_Sequence* self,
	psy_audio_SequenceOrderIndex index)
{
	psy_audio_SequenceEntry* entry;

	assert(self);

	entry = psy_audio_sequence_entry(self, index);
	if (entry) {
		return psy_audio_patterns_at(self->patterns,
			psy_audio_sequenceentry_patternslot(entry));
	}
	return NULL;
}

uintptr_t psy_audio_sequence_track_size(const psy_audio_Sequence* self,
	uintptr_t trackindex)
{
	const psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at((psy_audio_Sequence*)self, trackindex);
	if (track) {		
		return psy_list_size(track->entries);
	}
	return 0;
}

void psy_audio_sequence_setpatternindex(psy_audio_Sequence* self,
	psy_audio_SequenceOrderIndex index, uintptr_t patidx)
{
	psy_audio_SequenceEntry* entry;

	assert(self);

	entry = psy_audio_sequence_entry(self, index);
	if (entry) {
		psy_audio_Pattern* pattern;

		pattern = psy_audio_patterns_at(self->patterns, patidx);
		if (pattern == NULL) {
			pattern = psy_audio_pattern_allocinit();
			// todo set correct default lines
			psy_audio_patterns_insert(self->patterns, patidx, pattern);
		}
		psy_audio_sequenceentry_setpatternslot(entry, patidx);
	}
}

uintptr_t psy_audio_sequence_patternindex(const psy_audio_Sequence* self,
	psy_audio_SequenceOrderIndex index)
{
	const psy_audio_SequenceEntry* entry;

	assert(self);

	entry = psy_audio_sequence_entry_const(self, index);
	if (entry) {
		return entry->patternslot;		
	}
	return UINTPTR_MAX;
}

psy_dsp_big_beat_t psy_audio_sequence_offset(const psy_audio_Sequence* self,
	psy_audio_SequenceOrderIndex index)
{
	const psy_audio_SequenceEntry* entry;

	assert(self);

	entry = psy_audio_sequence_entry_const(self, index);
	if (entry) {
		return entry->offset;
	}
	return (psy_dsp_big_beat_t)0.0;
}
