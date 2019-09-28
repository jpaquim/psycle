// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "sequencer.h"
#include "pattern.h"
#include <stdlib.h> 

static void clearevents(Sequencer*);
static void cleardelayed(Sequencer*);
static void maketrackiterators(Sequencer*, float offset);
static void cleartrackiterators(Sequencer*);
static void advanceposition(Sequencer* self, float width);
static void addcurrevent(Sequencer*, SequenceTrackIterator*, float offset);
static int isoffsetinwindow(Sequencer* self, float offset);
static void insertevents(Sequencer* self);
static void insertdelayedevents(Sequencer*);

void sequencer_init(Sequencer* self, Sequence* sequence)
{
	self->samplerate = 44100;
	self->bpm = 125;
	self->sequence = sequence;
	self->position = 0;	
	self->window = 0;
	self->events = 0;
	self->delayedevents = 0;
	self->currtrackiterators = 0;
}

void sequencer_dispose(Sequencer* self)
{
	clearevents(self);
	cleardelayed(self);
	cleartrackiterators(self);
	self->sequence = 0;
}

void sequencer_reset(Sequencer* self, Sequence* sequence)
{
	sequencer_dispose(self);
	sequencer_init(self, sequence);
}

void sequencer_setposition(Sequencer* self, float offset)
{
	clearevents(self);
	cleardelayed(self);
	self->position = 0.0f;
	self->window = 0.0f;
	cleartrackiterators(self);
	maketrackiterators(self, offset);
}

void cleartrackiterators(Sequencer* self)
{
	List* p;

	for (p = self->currtrackiterators; p != 0; p = p->next) {
		free(p->entry);
	}
	list_free(self->currtrackiterators);
	self->currtrackiterators = 0;
}

void maketrackiterators(Sequencer* self, float offset)
{
	SequenceTracks* p;

	for (p = self->sequence->tracks; p != 0; p = p->next) {		
		SequenceTrackIterator* iterator;

		iterator =
			(SequenceTrackIterator*)malloc(sizeof(SequenceTrackIterator));
		*iterator = sequence_begin(self->sequence, p, 0.0f);		
		if (self->currtrackiterators == 0) {
			self->currtrackiterators = list_create(iterator);
		} else {
			list_append(self->currtrackiterators, iterator);	
		}
	}
}

void clearevents(Sequencer* self)
{
	list_free(self->events);
	self->events = 0;	
}

void cleardelayed(Sequencer* self)
{
	list_free(self->delayedevents);
	self->delayedevents = 0;	
}

void sequencer_tick(Sequencer* self, float width)
{			
	advanceposition(self, width);
	clearevents(self);
	insertevents(self);
	insertdelayedevents(self);
}

void advanceposition(Sequencer* self, float width)
{
	self->position += self->window;
	self->window = width;
}

void insertevents(Sequencer* self)
{
	List* p;

	for (p = self->currtrackiterators; p != 0; p = p->next) {
		SequenceTrackIterator* iterator;

		iterator = (SequenceTrackIterator*)p->entry;
		while (sequencetrackiterator_patternentry(iterator)) {
			float offset;			
			offset = sequencetrackiterator_offset(iterator); 
			if (isoffsetinwindow(self, offset)) {
				addcurrevent(self, iterator, offset);
				sequencetrackiterator_inc(iterator);
			} else {			
				break;
			}				
		}
	}
}

int isoffsetinwindow(Sequencer* self, float offset)
{
  return offset >= self->position && offset < self->position + self->window;
}

void addcurrevent(Sequencer* self, SequenceTrackIterator* trackiterator,
	float offset)
{		
	PatternEntry* entry = sequencetrackiterator_patternentry(trackiterator);
	if (entry->event.cmd == NOTE_DELAY) {
		int lpb = 4;
		entry->delta = offset + entry->event.parameter / (lpb*256.f);
		if (!self->delayedevents) {
			self->delayedevents = list_create(entry);
		} else {
			list_append(self->delayedevents, entry);
		}
	} else {
		entry->delta = offset - self->position;	
		if (!self->events) {
			self->events = list_create(entry);
		} else {
			list_append(self->events, entry);
		}
	}
}

void insertdelayedevents(Sequencer* self)
{
	List* p;
	
	for (p = self->delayedevents; p != 0; p = p->next) {
		PatternEntry* delayed = (PatternEntry*)p->entry;
		if (isoffsetinwindow(self, delayed->offset + delayed->delta)) {						
			List* q;
			int inserted = 0;
			q = self->events;
			while (q) {
				PatternEntry* entry = (PatternEntry*)q->entry;
				if (delayed->offset >= entry->offset) {					
					break;
				}
				q = q->next;
			}			
			p = list_remove(&self->delayedevents, p);
			delayed->delta = delayed->offset + delayed->delta - self->position;	
			if (q) {
				q = list_insert(&self->events, q, delayed);								
			} else {
				if (!self->events) {
					self->events = list_create(delayed);
				} else {
					list_append(self->events, delayed);
				}				
			}			
			if (!p) {
				break;
			}
		}		
	}
}
