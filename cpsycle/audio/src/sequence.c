// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequence.h"
#include <stdlib.h>

static void sequenceselection_addeditposition(SequenceSelection*);
static PatternNode* SequenceTrackIterator_next(SequenceTrackIterator*);
static void SequenceTrackIterator_unget(SequenceTrackIterator*);
static void sequence_reposition(Sequence* self, SequenceTrack*);
static SequenceTrackIterator sequence_makeiterator(Sequence*, List* entries);

void sequenceselection_init(SequenceSelection* self, Sequence* sequence)
{
	self->sequence = sequence;
	self->entries = 0;
	self->selectionmode = SELECTIONMODE_SINGLE;
	self->editposition = sequence_makeposition(self->sequence,
		self->sequence->tracks, 
		self->sequence->tracks
			? ((SequenceTrack*) self->sequence->tracks->entry)->entries
			: 0);
	sequenceselection_addeditposition(self);
	signal_init(&self->signal_editpositionchanged);
}

void sequenceselection_addeditposition(SequenceSelection* self)
{						
	if (self->editposition.trackposition.tracknode) {				
		list_append(&self->entries, 
			self->editposition.trackposition.tracknode->entry);	
	}
	sequence_setplayselection(self->sequence, self);
}


void sequenceselection_dispose(SequenceSelection* self)
{
	signal_dispose(&self->signal_editpositionchanged);
}

void sequenceselection_seteditposition(SequenceSelection* self,
	SequencePosition position)
{
	List* p;
	int append = 1;

	if (self->selectionmode == SELECTIONMODE_SINGLE) {
		list_free(self->entries);
		self->entries = 0;
	} else
	if ((p = list_findentry(self->entries, 
			position.trackposition.tracknode->entry)) != 0) {
		list_remove(&self->entries, p);
		append = 0;
	}
	self->editposition = position;
	if (append) {
		if (self->editposition.trackposition.tracknode) {
			list_append(&self->entries,
			self->editposition.trackposition.tracknode->entry);
		}
	} else {		
		SequenceTrack* track;
		List* p = list_last(self->entries);
			
		if (p) {
			track = (SequenceTrack*)position.track->entry;
			p = list_findentry(track->entries, p->entry);			
			self->editposition = sequence_makeposition(self->sequence,
				position.track,
				p);
		}
	}
// 	signal_emit(&self->signal_editpositionchanged, self, 0);
}

SequencePosition sequenceselection_editposition(SequenceSelection* self)
{
	return self->editposition;
}

void sequenceselection_setsequence(SequenceSelection* self,
	Sequence* sequence)
{
	self->sequence = sequence;
	self->editposition = sequence_makeposition(self->sequence,
		self->sequence->tracks, 
		self->sequence->tracks
			? ((SequenceTrack*) self->sequence->tracks->entry)->entries
			: 0);
	free(self->entries);
	self->entries = 0;
	sequenceselection_addeditposition(self);	
}

void sequencetrack_init(SequenceTrack* self)
{
	self->entries = 0;
}

SequenceTrack* sequencetrack_alloc(void)
{
	return malloc(sizeof(SequenceTrack));
}

SequenceTrack* sequencetrack_allocinit(void)
{
	SequenceTrack* rv;

	rv = sequencetrack_alloc();
	if (rv) {
		sequencetrack_init(rv);
	}
	return rv;
}

void sequenceentry_init(SequenceEntry* self, uintptr_t pattern, beat_t offset)
{
	self->pattern = pattern;
	self->offset = offset;
	self->selplay = 0;
}

SequenceEntry* sequenceentry_alloc(void)
{
	return (SequenceEntry*) malloc(sizeof(SequenceEntry));
}

SequenceEntry* sequenceentry_allocinit(uintptr_t pattern, beat_t offset)
{
	SequenceEntry* rv;

	rv = sequenceentry_alloc();
	if (rv) {
		sequenceentry_init(rv, pattern, offset);
		rv->node = 0;
	}
	return rv;
}

void sequenceposition_init(SequencePosition* self)
{
	self->track = 0;
	self->trackposition.patternnode = 0;
	self->trackposition.tracknode = 0;
}

SequenceEntry* sequenceposition_entry(SequencePosition* position)
{
	SequenceEntry* rv = 0;

	if (position->trackposition.tracknode) {				
		rv = (SequenceEntry*)position->trackposition.tracknode->entry;
	}
	return rv;
}

void sequencetrack_dispose(SequenceTrack* self)
{
	List* p;
	List* next;

	p = self->entries;
	while (p) {
		next = p->next;
		free(p->entry);
		p = next;
	}
	list_free(self->entries);
	self->entries = 0;
}

void sequence_init(Sequence* self, Patterns* patterns)
{
	self->tracks = 0;
	self->patterns = patterns;	
}

void sequence_dispose(Sequence* self)
{
	SequenceTracks* p;
	List* next;

	p = self->tracks;
	while (p) {
		next = p->next;
		sequencetrack_dispose((SequenceTrack*)p->entry);
		free(p->entry);
		p = next;
	}
	list_free(self->tracks);	
}


SequenceTrackNode* sequence_insert(Sequence* self, SequencePosition position,
	int pattern)
{		
	List* rv = 0;
	SequenceEntry* entry;
	SequenceTrack* track;

	entry = sequenceentry_allocinit(pattern, (beat_t) 0.f);	
	track = (SequenceTrack*) position.track->entry;
	if (track->entries) {		
		if (position.trackposition.tracknode) {			
			rv = list_insert(&track->entries, position.trackposition.tracknode,
				entry);			
			sequence_reposition(self, track);						
		} 
	} else {		
		rv = track->entries = list_create(entry);		
	}	
	entry->node = rv;
	return rv;
}

SequenceTrackNode* sequence_remove(Sequence* self, SequencePosition position)
{					
	SequenceTrackNode* rv = 0;
	if (position.track) {
		SequenceTrack* track;
		SequenceEntry* entry;		

		track = (SequenceTrack*)position.track->entry;		
		if (position.trackposition.tracknode) {
			entry = (SequenceEntry*)position.trackposition.tracknode->entry;
			rv = list_remove(&track->entries, 
				position.trackposition.tracknode);
			if (track->entries != 0) {
				SequencePosition newposition;
				
				newposition.track = position.track;
				sequence_reposition(self, track);					
			}
		}
	}
	return rv;
}

void sequence_clear(Sequence* self)
{
	SequenceTracks* p;	
	
	for (p = self->tracks; p != 0; p = p->next) {		
		sequencetrack_dispose((SequenceTrack*)p->entry);
		free(p->entry);
	}
	list_free(self->tracks);
	self->tracks = 0;	
}

SequenceTrackIterator sequence_last(Sequence* self, List* tracknode)
{
	SequenceTrackIterator p;
	SequenceTrack* track;

	if (tracknode == 0) {
		return sequence_begin(self, tracknode, 0);
	}
	 
	track = (SequenceTrack*) tracknode->entry;
	if (track->entries == 0) {
		return sequence_begin(self, tracknode, 0);
	}
	p.patternnode = 0;		
	p.tracknode = track->entries->tail;
	if (p.tracknode) {
		Pattern* pattern;
		SequenceEntry* entry = (SequenceEntry*) track->entries->tail->entry;
		pattern = patterns_at(self->patterns, entry->pattern);
		if (pattern) {
			p.patternnode = pattern->events;
		} else {
			p.patternnode = 0;
		}
	}
	return p;
}

void sequence_reposition(Sequence* self, SequenceTrack* track)
{
	beat_t curroffset = 0.0f;	
	List* p;	
			
	for (p = track->entries; p != 0; p = p->next) {
		Pattern* pattern;
		SequenceEntry* entry = (SequenceEntry*) p->entry;
		pattern = patterns_at(self->patterns, entry->pattern);
		if (pattern) {
			entry->offset = curroffset;
			curroffset += pattern->length;
		} else {
			entry->offset = curroffset;
		}		
	}
}

unsigned int sequence_size(Sequence* self, List* tracknode)
{	
	unsigned int rv = 0;

	if (tracknode) {
		SequenceTrack* track;
		List* p;

		track = (SequenceTrack*)(tracknode->entry);
		for (p = track->entries; p != 0; p = p->next, ++rv);
	}
	return rv;
}

SequencePosition sequence_at(Sequence* self, unsigned int trackindex,
	unsigned int position)
{
	SequencePosition rv;	
	List* ptr;	
	unsigned int c = 0;	
	SequenceTracks* ptracks;
	SequenceTrack* track;

	sequenceposition_init(&rv);	
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
		track = (SequenceTrack*)(ptracks->entry);
	} else {
		track = 0;
	}
	
	rv.track = ptracks;
	if (rv.track) {
		ptr = track->entries;
		c = 0;
		while (ptr) {
			if (c == position) {			
				rv = sequence_makeposition(self, ptracks, ptr);
				break;
			}
			++c;
			ptr = ptr->next;
		}
	}
	return rv;
}

List* sequence_at_offset(Sequence* self, SequenceTracks* tracknode, beat_t offset)
{
	beat_t curroffset = 0.0f;	
	List* p = 0;

	if (tracknode) {		
		SequenceTrack* track;

		track = (SequenceTrack*)tracknode->entry;
		p = track->entries;	
		while (p) {
			Pattern* pattern;
			SequenceEntry* entry = (SequenceEntry*) p->entry;
			pattern = patterns_at(self->patterns, entry->pattern);
			if (pattern) {
				if (offset >= curroffset && offset < curroffset + pattern->length) {
					break;
				}
				curroffset += pattern->length;
			}
			p = p->next;
		}
	}
	return p;
}

SequenceTrackIterator sequence_begin(Sequence* self, List* track, beat_t pos)
{		
	return sequence_makeiterator(self, sequence_at_offset(self, track, pos));	
}

void sequencetrackiterator_inc(SequenceTrackIterator* self)
{	
	if (self->patternnode) {		
		self->patternnode = self->patternnode->next;
		if (self->patternnode == NULL) {
			if (self->tracknode->next) {
				SequenceEntry* entry;
				Pattern* pattern;
				self->tracknode = self->tracknode->next;			
				entry = (SequenceEntry*) self->tracknode->entry;
				pattern = patterns_at(self->patterns, entry->pattern);
				self->patternnode = pattern->events;
			}
		}
	}
}

void sequencetrackiterator_incentry(SequenceTrackIterator* self)
{	
	if (self->tracknode) {		
		self->tracknode = self->tracknode->next;
		if (self->tracknode) {
			SequenceEntry* entry;
			Pattern* pattern;

			entry = (SequenceEntry*) self->tracknode->entry;
			pattern = patterns_at(self->patterns, entry->pattern);
			self->patternnode = pattern->events;
		} else {
			self->patternnode = 0;
		}
	}
}

void sequencetrackiterator_decentry(SequenceTrackIterator* self)
{	
	if (self->tracknode) {		
		self->tracknode = self->tracknode->prev;
		if (self->tracknode) {
			SequenceEntry* entry;
			Pattern* pattern;

			entry = (SequenceEntry*) self->tracknode->entry;
			pattern = patterns_at(self->patterns, entry->pattern);
			self->patternnode = pattern->events;
		} else {
			self->patternnode = 0;
		}
	}
}

SequenceTrackIterator sequence_makeiterator(Sequence* self, List* entries)
{
	SequenceTrackIterator rv;
	Pattern* pPattern  = 0;	
	rv.patterns = self->patterns;
	rv.tracknode = entries;
	if (entries) {
		SequenceEntry* entry = (SequenceEntry*) entries->entry;
		pPattern = patterns_at(self->patterns, entry->pattern);
		if (pPattern) {
			rv.patternnode = pPattern->events;
		} else {
			rv.patternnode = 0;
		}
	}		
	return rv;
}

SequencePosition sequence_makeposition(Sequence* self, SequenceTracks* track, List* entries)
{
	SequencePosition rv;

	rv.trackposition = sequence_makeiterator(self, entries);
	rv.track = track;
	return rv;
}

PatternNode* sequencetrackiterator_patternnode(SequenceTrackIterator* self)
{
	return self->patternnode;
}

SequenceEntry* sequencetrackiterator_entry(SequenceTrackIterator* self)
{
	return self->tracknode ? (SequenceEntry*) self->tracknode->entry : 0;
}

PatternEntry* sequencetrackiterator_patternentry(SequenceTrackIterator* self)
{
	return self->patternnode ? (PatternEntry*)(self->patternnode)->entry : 0;
}

beat_t sequencetrackiterator_offset(SequenceTrackIterator* self)
{	
	return sequencetrackiterator_patternentry(self)
		? sequencetrackiterator_entry(self)->offset +
		  sequencetrackiterator_patternentry(self)->offset
		: 0.f;	
}

List* sequence_appendtrack(Sequence* self, SequenceTrack* track)
{	
	return list_append(&self->tracks, track);	
}

List* sequence_removetrack(Sequence* self, SequenceTracks* tracknode)
{		
	return list_remove(&self->tracks, tracknode);
}

unsigned int sequence_sizetracks(Sequence* self)
{
	unsigned int c = 0;	
	SequenceTracks* p;
	
	for (p = self->tracks; p != 0; p = p->next, ++c);
	return c;
}

int sequence_patternused(Sequence* self, unsigned int patternslot)
{
	int rv = 0;
	SequenceTracks* t;	

	t = self->tracks;
	while (t) {
		SequenceTrack* track;
		List* p;

		track = (SequenceTrack*)t->entry;
		p = track->entries;
		while (p) {
			SequenceEntry* entry;

			entry = (SequenceEntry*)p->entry;
			if (entry->pattern == patternslot) {
				rv = 1;
				break;
			}
			p = p->next;
		}
		t = t->next;
	}
	return rv;
}

void sequence_setpatternslot(Sequence* self, SequencePosition position,
	unsigned int slot)
{
	SequenceEntry* entry;

	entry = sequenceposition_entry(&position);
	if (entry) {
		Pattern* pattern;		
		
		pattern = patterns_at(self->patterns, slot);
		if (pattern == 0) {			
			pattern = pattern_allocinit();			
			patterns_insert(self->patterns, slot, pattern);		
		}
		entry->pattern = slot;		
	}
}

beat_t sequence_duration(Sequence* self)
{	
	SequenceTracks* t;
	beat_t duration = 0.f;

	t = self->tracks;
	while (t) {
		SequenceTrack* track;
		List* p;

		track = (SequenceTrack*)t->entry;
		p = track->entries;
		if (p) {			
			Pattern* pattern;
			SequenceEntry* entry;

			p = p->tail;
			entry = (SequenceEntry*) p->entry;
			pattern = patterns_at(self->patterns, entry->pattern);
			if (pattern) {
				if (duration < entry->offset + pattern->length) {
					duration = entry->offset + pattern->length;
				}
			}
		}		
		t = t->next;
	}
	return duration;
}

unsigned int sequence_maxtracksize(Sequence* self)
{
	unsigned int rv = 0;	
	SequenceTracks* t;
		 
	for (t = self->tracks; t != 0; t = t->next) {
		unsigned int c;

		c = sequence_size(self, t);
		if (c > rv) {
			rv = c;
		}		
	}
	return rv;
}

void sequence_setplayselection(Sequence* self, SequenceSelection* selection)
{
	List* p;

	sequence_clearplayselection(self);
	for (p = selection->entries; p != 0; p = p->next) {
		SequenceEntry* entry;

		entry = (SequenceEntry*) p->entry;
		entry->selplay = 1;
	}
}

void sequence_clearplayselection(Sequence* self)
{
	SequenceTracks* t;
	
	for (t = self->tracks; t != 0; t = t->next) {
		SequenceTrack* track;
		List* p;

		track = t->entry;
		for (p = track->entries; p != 0; p = p->next) {
			SequenceEntry* entry;

			entry = (SequenceEntry*) p->entry;
			entry->selplay = 0;
		}
	}
}

