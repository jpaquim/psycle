// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "sequencer.h"
#include "pattern.h"

static void clearevents(Sequencer*);
static void cleardelayed(Sequencer*);
static void advanceposition(Sequencer* self, float width);
static void addcurrevent(Sequencer*, float offset);
static int isoffsetinwindow(Sequencer* self, float offset);
static void insertevents(Sequencer* self);
static void insertdelayedevents(Sequencer*);

void sequencer_init(Sequencer* self)
{
	self->samplerate = 44100;
	self->bpm = 125;
	self->sequence = 0;
	self->position = 0;	
	self->window = 0;
	self->events = 0;
	self->delayedevents = 0;
}

void sequencer_dispose(Sequencer* self)
{
	clearevents(self);
	cleardelayed(self);
}

void sequencer_setposition(Sequencer* self, float position)
{
	clearevents(self);
	cleardelayed(self);
	self->position = 0.0f;
	self->window = 0.0f;
	self->curr = sequence_begin(self->sequence, 0.0f);	
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
	while (sequenceiterator_patternentry(&self->curr)) {
		float offset;
		
		offset = sequenceiterator_offset(&self->curr); 
		if (isoffsetinwindow(self, offset)) {
			addcurrevent(self, offset);
			sequenceiterator_inc(&self->curr);
		} else {			
			break;
		}				
	}
}

int isoffsetinwindow(Sequencer* self, float offset)
{
  return offset >= self->position && offset < self->position + self->window;
}

void addcurrevent(Sequencer* self, float offset)
{		
	PatternEntry* entry = sequenceiterator_patternentry(&self->curr);
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

SequenceIterator sequencer_curr(Sequencer* self)
{
	return self->curr;
}
