// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequence.h"
#include "sequencer.h" // calculate duration
#include <stdlib.h>

static sequenceentryid = 1;

static void sequenceselection_addeditposition(SequenceSelection*);
static psy_audio_PatternNode* SequenceTrackIterator_next(SequenceTrackIterator*);
static void SequenceTrackIterator_unget(SequenceTrackIterator*);
static void sequence_reposition(psy_audio_Sequence* self, SequenceTrack*);
static SequenceTrackIterator sequence_makeiterator(psy_audio_Sequence*, psy_List* entries);

void sequenceselection_init(SequenceSelection* self, psy_audio_Sequence* sequence)
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
	psy_signal_init(&self->signal_editpositionchanged);
}

void sequenceselection_dispose(SequenceSelection* self)
{
	psy_signal_dispose(&self->signal_editpositionchanged);
	psy_list_free(self->entries);
}

void sequenceselection_addeditposition(SequenceSelection* self)
{						
	if (self->editposition.trackposition.tracknode) {				
		psy_list_append(&self->entries, 
			self->editposition.trackposition.tracknode->entry);	
	}
	sequence_setplayselection(self->sequence, self);
}

void sequenceselection_seteditposition(SequenceSelection* self,
	SequencePosition position)
{
	psy_List* p;
	int append = 1;

	if (self->selectionmode == SELECTIONMODE_SINGLE) {
		psy_list_free(self->entries);
		self->entries = 0;
	} else
	if ((p = psy_list_findentry(self->entries, 
			position.trackposition.tracknode->entry)) != 0) {
		psy_list_remove(&self->entries, p);
		append = 0;
	}
	self->editposition = position;
	if (append) {
		if (self->editposition.trackposition.tracknode) {
			psy_list_append(&self->entries,
			self->editposition.trackposition.tracknode->entry);
		}
	} else {		
		SequenceTrack* track;
		psy_List* p = psy_list_last(self->entries);
			
		if (p) {
			track = (SequenceTrack*)position.track->entry;
			p = psy_list_findentry(track->entries, p->entry);			
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
	psy_audio_Sequence* sequence)
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

void sequenceentry_init(SequenceEntry* self, uintptr_t pattern, psy_dsp_big_beat_t offset)
{
	self->pattern = pattern;
	self->offset = offset;
	self->selplay = 0;
	self->id = sequenceentryid++;
}

SequenceEntry* sequenceentry_alloc(void)
{
	return (SequenceEntry*) malloc(sizeof(SequenceEntry));
}

SequenceEntry* sequenceentry_allocinit(uintptr_t pattern, psy_dsp_big_beat_t offset)
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
	psy_list_deallocate(&self->entries, (psy_fp_disposefunc)NULL);
}

void sequence_init(psy_audio_Sequence* self, psy_audio_Patterns* patterns)
{
	self->tracks = 0;
	self->patterns = patterns;	
}

void sequence_dispose(psy_audio_Sequence* self)
{
	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		sequencetrack_dispose);
}

SequenceTrackNode* sequence_insert(psy_audio_Sequence* self, SequencePosition position,
	int pattern)
{		
	psy_List* rv = 0;
	if (position.track) {
		SequenceEntry* entry;
		SequenceTrack* track;

		entry = sequenceentry_allocinit(pattern, (psy_dsp_big_beat_t) 0.f);	
		track = (SequenceTrack*) position.track->entry;						
		if (track->entries) {		
			if (position.trackposition.tracknode) {			
				rv = psy_list_insert(&track->entries, position.trackposition.tracknode,
					entry);			
				sequence_reposition(self, track);						
			} 
		} else {		
			rv = track->entries = psy_list_create(entry);		
		}	
		entry->node = rv;
	}
	return rv;
}

SequenceTrackNode* sequence_remove(psy_audio_Sequence* self, SequencePosition position)
{					
	SequenceTrackNode* rv = 0;
	if (position.track) {
		SequenceTrack* track;
		SequenceEntry* entry;		

		track = (SequenceTrack*)position.track->entry;		
		if (position.trackposition.tracknode) {
			entry = (SequenceEntry*)position.trackposition.tracknode->entry;
			rv = psy_list_remove(&track->entries, 
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

void sequence_clear(psy_audio_Sequence* self)
{
	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		sequencetrack_dispose);	
}

SequenceTrackIterator sequence_last(psy_audio_Sequence* self,
	psy_List* tracknode)
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
		psy_audio_Pattern* pattern;
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

void sequence_reposition(psy_audio_Sequence* self, SequenceTrack* track)
{
	psy_dsp_big_beat_t curroffset = 0.0f;	
	psy_List* p;	
			
	for (p = track->entries; p != NULL; psy_list_next(&p)) {
		psy_audio_Pattern* pattern;
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

unsigned int sequence_size(psy_audio_Sequence* self, psy_List* tracknode)
{	
	unsigned int rv = 0;

	if (tracknode) {
		SequenceTrack* track;
		psy_List* p;

		track = (SequenceTrack*)(tracknode->entry);
		for (p = track->entries; p != NULL; psy_list_next(&p), ++rv);
	}
	return rv;
}

SequencePosition sequence_at(psy_audio_Sequence* self, unsigned int trackindex,
	unsigned int position)
{
	SequencePosition rv;	
	psy_List* ptr;	
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

psy_List* sequenceentry_at_offset(psy_audio_Sequence* self,
	SequenceTracks* tracknode, psy_dsp_big_beat_t offset)
{
	psy_dsp_big_beat_t curroffset = 0.0f;	
	psy_List* p = 0;

	if (tracknode) {		
		SequenceTrack* track;

		track = (SequenceTrack*)tracknode->entry;
		p = track->entries;	
		while (p) {
			psy_audio_Pattern* pattern;
			SequenceEntry* entry = (SequenceEntry*) p->entry;
			pattern = patterns_at(self->patterns, entry->pattern);
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

SequenceTrackIterator sequence_begin(psy_audio_Sequence* self, psy_List* track,
	psy_dsp_big_beat_t pos)
{		
	SequenceTrackIterator rv;		
	SequenceEntry* entry;	

	rv.patterns = self->patterns;
	rv.tracknode = sequenceentry_at_offset(self, track, pos);
	if (rv.tracknode) {
		psy_audio_Pattern* pattern;

		entry = (SequenceEntry*) rv.tracknode->entry;
		pattern = patterns_at(self->patterns, entry->pattern);
		rv.patternnode = psy_audio_pattern_greaterequal(pattern, pos - entry->offset);
	} else {
		rv.patternnode = 0;
	}
	return rv;	
}

void sequencetrackiterator_inc(SequenceTrackIterator* self)
{	
	if (self->patternnode) {		
		self->patternnode = self->patternnode->next;
		if (self->patternnode == NULL) {
			if (self->tracknode->next) {
				SequenceEntry* entry;
				psy_audio_Pattern* pattern;
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
			psy_audio_Pattern* pattern;

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
			psy_audio_Pattern* pattern;

			entry = (SequenceEntry*) self->tracknode->entry;
			pattern = patterns_at(self->patterns, entry->pattern);
			self->patternnode = pattern->events;
		} else {
			self->patternnode = 0;
		}
	}
}

SequenceTrackIterator sequence_makeiterator(psy_audio_Sequence* self, psy_List* entries)
{
	SequenceTrackIterator rv;
	psy_audio_Pattern* pPattern  = 0;	
	
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

SequencePosition sequence_makeposition(psy_audio_Sequence* self, SequenceTracks* track, psy_List* entries)
{
	SequencePosition rv;

	rv.trackposition = sequence_makeiterator(self, entries);
	rv.track = track;
	return rv;
}

SequencePosition sequence_positionfromid(psy_audio_Sequence* self, int id)
{
	SequencePosition rv;	
	SequenceTracks* t;	

	rv.track = 0;
	rv.trackposition.patternnode = 0;
	rv.trackposition.tracknode = 0;
	rv.trackposition.patterns = 0;
	t = self->tracks;
	while (t) {
		SequenceTrack* track;
		psy_List* p;

		track = (SequenceTrack*)t->entry;
		p = track->entries;
		while (p) {
			SequenceEntry* entry;

			entry = (SequenceEntry*)p->entry;
			if (entry->id == id) {
				rv.track = t;
				rv.trackposition = sequence_makeiterator(self, p);
				break;
			}
			psy_list_next(&p);
		}
		t = t->next;
	}
	return rv;
}

psy_List* sequence_appendtrack(psy_audio_Sequence* self, SequenceTrack* track)
{	
	return psy_list_append(&self->tracks, track);	
}

psy_List* sequence_removetrack(psy_audio_Sequence* self, SequenceTracks* tracknode)
{		
	return psy_list_remove(&self->tracks, tracknode);
}

uintptr_t sequence_sizetracks(psy_audio_Sequence* self)
{
	uintptr_t c = 0;	
	SequenceTracks* p;
	
	for (p = self->tracks; p != NULL; psy_list_next(&p), ++c);
	return c;
}

int sequence_patternused(psy_audio_Sequence* self, unsigned int patternslot)
{
	int rv = 0;
	SequenceTracks* t;	

	t = self->tracks;
	while (t) {
		SequenceTrack* track;
		psy_List* p;

		track = (SequenceTrack*)t->entry;
		p = track->entries;
		while (p) {
			SequenceEntry* entry;

			entry = (SequenceEntry*)p->entry;
			if (entry->pattern == patternslot) {
				rv = 1;
				break;
			}
			psy_list_next(&p);
		}
		t = t->next;
	}
	return rv;
}

void sequence_setpatternslot(psy_audio_Sequence* self, SequencePosition position,
	unsigned int slot)
{
	SequenceEntry* entry;

	entry = sequenceposition_entry(&position);
	if (entry) {
		psy_audio_Pattern* pattern;		
		
		pattern = patterns_at(self->patterns, slot);
		if (pattern == 0) {			
			pattern = psy_audio_pattern_allocinit();
			patterns_insert(self->patterns, slot, pattern);		
		}
		entry->pattern = slot;		
	}
}

psy_dsp_big_beat_t sequence_duration(psy_audio_Sequence* self)
{	
	SequenceTracks* t;
	psy_dsp_big_beat_t duration = 0.f;

	t = self->tracks;
	while (t) {
		SequenceTrack* track;
		psy_List* p;

		track = (SequenceTrack*)t->entry;
		p = track->entries;
		if (p) {			
			psy_audio_Pattern* pattern;
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

uintptr_t sequence_maxtracksize(psy_audio_Sequence* self)
{
	uintptr_t rv = 0;
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

void sequence_setplayselection(psy_audio_Sequence* self, SequenceSelection* selection)
{
	psy_List* p;

	sequence_clearplayselection(self);
	for (p = selection->entries; p != NULL; psy_list_next(&p)) {
		SequenceEntry* entry;

		entry = (SequenceEntry*) p->entry;
		entry->selplay = 1;
	}
}

void sequence_clearplayselection(psy_audio_Sequence* self)
{
	SequenceTracks* t;
	
	for (t = self->tracks; t != 0; t = t->next) {
		SequenceTrack* track;
		psy_List* p;

		track = t->entry;
		for (p = track->entries; p != NULL; psy_list_next(&p)) {
			SequenceEntry* entry;

			entry = (SequenceEntry*) p->entry;
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
