// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "sequence.h"
#include <stdlib.h>

static PatternNode* SequenceTrackIterator_next(SequenceTrackIterator* self);
static void SequenceTrackIterator_unget(SequenceTrackIterator* self);
static void sequence_reposition(Sequence* self, SequenceTrack* track);
static SequenceTrackIterator sequence_makeiterator(Sequence* self, List* entries);
static SequencePosition sequence_makeposition(Sequence* self, SequenceTracks* track, List* entries);

void sequencetrack_init(SequenceTrack* self)
{
	self->entries = 0;
}

void sequenceposition_init(SequencePosition* self)
{
	self->track = 0;
	self->trackposition.patternnode = 0;
	self->trackposition.tracknode = 0;
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
	self->editposition = sequence_makeposition(self, 0, 0);
	signal_init(&self->signal_editpositionchanged);
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
	signal_dispose(&self->signal_editpositionchanged);
}

SequenceEntry* sequence_insert(Sequence* self, SequencePosition position, int pattern)
{		
	SequenceEntry* entry = (SequenceEntry*) malloc(sizeof(SequenceEntry));
	SequenceTrack* track = (SequenceTrack*) position.track->entry;	
	entry->pattern = pattern;
	entry->offset = 0;
	if (track->entries) {
		List* ptr = position.trackposition.tracknode;
		if (ptr) {			
			self->editposition = sequence_makeposition(self, 
				position.track, list_insert(&track->entries, position.trackposition.tracknode, entry));
			sequence_reposition(self, track);						
		}
	} else {		
		track->entries = list_create(entry);		
		self->editposition = sequence_makeposition(self, position.track,
			track->entries);
	}
	signal_emit(&self->signal_editpositionchanged, self, 0);
	return entry;
}

void sequence_remove(Sequence* self, SequencePosition position)
{					
	if (position.track) {
		SequenceTrack* track;
		SequenceEntry* entry;
		List* ptr;

		track = (SequenceTrack*)position.track->entry;
		ptr = position.trackposition.tracknode;	
		entry = (SequenceEntry*)ptr->entry;
		ptr = list_remove(&track->entries, ptr);	
		if (track->entries != NULL) {
			SequencePosition newposition;
			
			newposition.track = position.track;
			sequence_reposition(self, track);	
			if (ptr) {						
				newposition.trackposition = sequence_begin(self, position.track, entry->offset);				
			} else {
				newposition.trackposition = sequence_last(self, position.track);				
			}
			self->editposition = newposition;
		} else {
			self->editposition = sequence_makeposition(self, position.track, 0);
		}
		signal_emit(&self->signal_editpositionchanged, self, 0);
	}
}

void sequence_clear(Sequence* self)
{
	SequenceTracks* p;	
	
	for (p = self->tracks; p != 0; p = p->next) {		
		sequencetrack_dispose((SequenceTrack*)p->entry);		
	}
	list_free(self->tracks);
	self->tracks = 0;
	sequenceposition_init(&self->editposition);	
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
	float curroffset = 0.0f;	
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
	unsigned int c = 0;

	if (tracknode) {
		List* p;

		SequenceTrack* track = (SequenceTrack*)(tracknode->entry);		
		p = track->entries;
		while (p) {		
			p = p->next;
			++c;
		}
	}
	return c;
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

List* sequence_at_offset(Sequence* self, SequenceTracks* tracknode, float offset)
{
	float curroffset = 0.0f;	
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

SequenceTrackIterator sequence_begin(Sequence* self, List* track, float pos)
{		
	return sequence_makeiterator(self, sequence_at_offset(self, track, pos));	
}

void sequencetrackiterator_inc(SequenceTrackIterator* self) {	
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

void sequence_seteditposition(Sequence* self, SequencePosition position)
{
	self->editposition = position;	
	signal_emit(&self->signal_editpositionchanged, self, 0);
}

SequencePosition sequence_editposition(Sequence* self)
{
	return self->editposition;
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

static SequencePosition sequence_makeposition(Sequence* self, SequenceTracks* track, List* entries)
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

float sequencetrackiterator_offset(SequenceTrackIterator* self)
{	
	return sequencetrackiterator_patternentry(self)
		? sequencetrackiterator_entry(self)->offset +
		  sequencetrackiterator_patternentry(self)->offset
		: 0.f;	
}

List* sequence_appendtrack(Sequence* self, SequenceTrack* track)
{
	List* rv;

	if (self->tracks == 0) {
		self->tracks = list_create(track);
		rv = self->tracks;
	} else {
		rv = list_append(self->tracks, track);		
	}
	return rv;
}

unsigned int sequence_sizetracks(Sequence* self)
{
	unsigned int c = 0;	
	SequenceTracks* p;
	
	for (p = self->tracks; p != 0; p = p->next, ++c);
	return c;
}
