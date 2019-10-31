// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequencer.h"
#include "pattern.h"
#include <stdlib.h> 

static void clearevents(Sequencer*);
static void cleardelayed(Sequencer*);
static void clearinputevents(Sequencer*);
static void freeentries(List* events);
static void maketrackiterators(Sequencer*, beat_t offset);
static void cleartrackiterators(Sequencer*);
static void advanceposition(Sequencer* self, beat_t width);
static void addsequenceevent(Sequencer*, SequenceTrackIterator*, beat_t offset);
static void maketweakslideevents(Sequencer*, PatternEntry*);
static int isoffsetinwindow(Sequencer*, beat_t offset);
static void insertevents(Sequencer*);
static void insertinputevents(Sequencer*);
static void insertdelayedevents(Sequencer*);
static void sequencerinsert(Sequencer*);
static void compute_beatsprosample(Sequencer*);
static void notifysequencertick(Sequencer*, beat_t width);

void sequencer_init(Sequencer* self, Sequence* sequence, Machines* machines)
{
	self->sequence = sequence;
	self->machines = machines;
	self->samplerate = 44100;
	self->bpm = 125.f;
	self->lpb = 4;
	self->lpbspeed = 1.f;
	self->playing = 0;	
	self->position = 0;	
	self->window = 0;
	self->seqlinetickcount = 1.f / self->lpb;
	self->events = 0;
	self->delayedevents = 0;
	self->inputevents = 0;
	self->currtrackiterators = 0;
	compute_beatsprosample(self);
}

void sequencer_dispose(Sequencer* self)
{
	clearevents(self);
	cleardelayed(self);
	clearinputevents(self);
	cleartrackiterators(self);
	self->sequence = 0;
	self->machines = 0;
}

void sequencer_reset(Sequencer* self, Sequence* sequence, Machines* machines)
{
	sequencer_dispose(self);
	sequencer_init(self, sequence, machines);
}

void sequencer_setposition(Sequencer* self, beat_t offset)
{
	clearevents(self);
	cleardelayed(self);
	self->position = 0.0f;
	self->window = 0.0f;
	cleartrackiterators(self);
	maketrackiterators(self, offset);
}

beat_t sequencer_position(Sequencer* self)
{	
	return self->position;
}

void sequencer_start(Sequencer* self)
{	
	self->lpbspeed = 1.f;
	compute_beatsprosample(self);
	self->playing = 1;
}

void sequencer_stop(Sequencer* self)
{
	self->playing = 0;
}

void sequencer_setsamplerate(Sequencer* self, unsigned int samplerate)
{
	self->samplerate = samplerate;
	compute_beatsprosample(self);
}

unsigned int sequencer_samplerate(Sequencer* self)
{
	return self->samplerate;
}

void sequencer_setbpm(Sequencer* self, beat_t bpm)
{	
	if (bpm < 32) {
		self->bpm = 32;
	} else
	if (bpm > 999) {
		self->bpm = 999;
	} else {
		self->bpm = bpm;
	}
	compute_beatsprosample(self);
}

beat_t sequencer_bpm(Sequencer* self)
{
	return self->bpm;
}

void sequencer_setlpb(Sequencer* self, unsigned int lpb)
{	
	self->lpb = lpb;
	self->lpbspeed = 1.f;
	self->seqlinetickcount = 1.f / lpb;
	compute_beatsprosample(self);
}

unsigned int sequencer_lpb(Sequencer* self)
{
	return self->lpb;
}

List* sequencer_tickevents(Sequencer* self)
{
	return self->events;
}

List* sequencer_machinetickevents(Sequencer* self, unsigned int slot)
{
	List* rv = 0;
	List* p;
		
	for (p = self->events; p != 0; p = p->next) {
		PatternEntry* entry = (PatternEntry*) p->entry;		
		if (entry->event.mach == slot) {			
			list_append(&rv, entry);						
		}		
	}
	return rv;
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

void maketrackiterators(Sequencer* self, beat_t offset)
{
	SequenceTracks* p;

	for (p = self->sequence->tracks; p != 0; p = p->next) {		
		SequenceTrackIterator* iterator;

		iterator =
			(SequenceTrackIterator*)malloc(sizeof(SequenceTrackIterator));
		*iterator = sequence_begin(self->sequence, p, 0.0f);				
		list_append(&self->currtrackiterators, iterator);		
	}
}

void clearevents(Sequencer* self)
{
	freeentries(self->events);
	list_free(self->events);
	self->events = 0;	
}

void cleardelayed(Sequencer* self)
{	
	freeentries(self->delayedevents);
	list_free(self->delayedevents);
	self->delayedevents = 0;	
}

void clearinputevents(Sequencer* self)
{
	freeentries(self->inputevents);
	list_free(self->inputevents);
	self->inputevents = 0;	
}

void freeentries(List* events)
{
	List* p;
	
	for (p = events; p != 0; p = p->next) {
		free(p->entry);
	}
}

void sequencer_frametick(Sequencer* self, unsigned int numframes)
{	
	sequencer_tick(self, sequencer_offset(self, numframes));
}

void sequencer_tick(Sequencer* self, beat_t width)
{
	if (self->playing) {
		advanceposition(self, width);		
	}
	clearevents(self);
	insertinputevents(self);
	if (self->playing) {
		insertevents(self);
		insertdelayedevents(self);		
	}
	notifysequencertick(self, width);
	if (self->playing) {
		sequencerinsert(self);
	}	
}

void notifysequencertick(Sequencer* self, beat_t width)
{
	TableIterator it;
	int linetick;
	
	linetick = self->seqlinetickcount <= 0;
	for (it = machines_begin(self->machines); !tableiterator_equal(&it, table_end());		
			tableiterator_inc(&it)) {			
		Machine* machine;

		machine = (Machine*)tableiterator_value(&it);
		machine->sequencertick(machine);
		if (linetick) {
			machine->sequencerlinetick(machine);
		}		
	}
	if (linetick) {
		self->seqlinetickcount = 1.f / (self->lpb * self->lpbspeed);
	} else {
		self->seqlinetickcount -= width;
	}
}

void sequencerinsert(Sequencer* self) {
	List* p;

	for (p = sequencer_tickevents(self); p != 0; p = p->next) {			
		PatternEntry* entry;			
		
		entry = (PatternEntry*) p->entry;
		if (entry->event.mach != NOTECOMMANDS_EMPTY) {
			Machine* machine;				
				
			machine = machines_at(self->machines, entry->event.mach);
			if (machine) {
				List* events;

				events = sequencer_machinetickevents(self, entry->event.mach);
				if (events) {					
					List* insert;
					
					insert = machine->sequencerinsert(machine, events);
					if (insert) {
						sequencer_append(self, insert);					
						list_free(insert);
					}
					list_free(events);
				}					
			}									
		}
	}
}

void advanceposition(Sequencer* self, beat_t width)
{	
	self->position += self->window;
	self->window = width;	
}

void insertevents(Sequencer* self)
{
	List* p;	

	for (p = self->currtrackiterators; p != 0; p = p->next) {
		SequenceTrackIterator* it;

		it = (SequenceTrackIterator*)p->entry;
		while (sequencetrackiterator_patternentry(it)) {
			beat_t offset;
			
			offset = sequencetrackiterator_offset(it);
			if (isoffsetinwindow(self, offset)) {
				addsequenceevent(self, it, offset);			
				sequencetrackiterator_inc(it);				
			} else {			
				break;
			}				
		}
	}
}

void sequencer_append(Sequencer* self, List* events)
{
	List* p;

	for (p = events; p != 0; p = p->next) {
		PatternEntry* entry;

		entry = (PatternEntry*) p->entry;
		if (entry->event.cmd == NOTE_DELAY) {
			list_append(&self->delayedevents, entry);
		} else {
			list_append(&self->events, entry);
		}
	}
}

int isoffsetinwindow(Sequencer* self, beat_t offset)
{
  return offset >= self->position && offset < self->position + self->window;
}

void addsequenceevent(Sequencer* self, SequenceTrackIterator* trackiterator,
	beat_t offset)
{	
	PatternEntry* patternentry;

	patternentry = sequencetrackiterator_patternentry(trackiterator);
	if (patternentry->event.cmd == SET_TEMPO) {
		self->bpm = patternentry->event.parameter;
		compute_beatsprosample(self);		
	} else 
	if (patternentry->event.cmd == EXTENDED) {
		if (patternentry->event.parameter < SET_LINESPERBEAT1) {
			self->lpbspeed = patternentry->event.parameter / (beat_t)self->lpb;
			compute_beatsprosample(self);
		}
	}
	if (patternentry->event.note == NOTECOMMANDS_TWEAKSLIDE) {
		maketweakslideevents(self, patternentry);
	} else {
		PatternEntry* entry;
		entry =	patternentry_clone(patternentry);
		entry->bpm = self->bpm;
		if (entry->event.cmd == NOTE_DELAY) {		
			entry->delta = offset + entry->event.parameter / (self->lpb * self->lpbspeed * 256.f);			
			list_append(&self->delayedevents, entry);
		} else {
			entry->delta = offset - self->position;			
			list_append(&self->events, entry);
		}
	}	
}

void maketweakslideevents(Sequencer* self, PatternEntry* entry)
{
	Machine* machine;		
		
	machine = machines_at(self->machines, entry->event.mach);
	if (machine && machine->info(machine) &&
			entry->event.inst < machine->numparameters(machine) > 0) {
		int param = entry->event.inst;
		int minval;
		int maxval;		
		int slides = sequencer_frames(self, 1.f/(self->lpb * self->lpbspeed)) / 64;		
		int dest = ((entry->event.cmd << 8) + entry->event.parameter);
		int start = machine->value(machine, param);
		int slide;
		float delta;
		float curr;

		machine->parameterrange(machine, entry->event.parameter, &minval,
			&maxval);		
		dest += minval;
		if (slides == 0) {
			return;
		}
		if (dest > maxval) { 
			dest = maxval;
		}
		if (dest == start) {
			PatternEntry* slideentry;
			slideentry = patternentry_clone(entry);
			slideentry->event.note = NOTECOMMANDS_TWEAK;
			slideentry->bpm = self->bpm;
			list_append(&self->events, slideentry);
		} else {
			delta = (dest - start) / (float)slides;			
			curr = (float)start + minval;
			for (slide = 0; slide < slides; ++slide) {
				PatternEntry* slideentry;
				int cmd;
				int parameter;				
				int nv;

				if (slide == slides -1) {
					curr = (float)dest;
				}				
				nv = (int) curr + minval;
				if (nv > maxval) nv = maxval;
				cmd = nv >> 8;
				parameter = nv & 0xFF;
				curr += delta;
				slideentry = patternentry_clone(entry);
				slideentry->bpm = self->bpm;
				slideentry->event.note = NOTECOMMANDS_TWEAK;
				slideentry->event.cmd = cmd;
				slideentry->event.parameter = parameter;				
				slideentry->delta += slide * 64 * self->beatsprosample;				
				list_append(&self->delayedevents, slideentry);				
			}
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
				list_append(&self->events, delayed);								
			}			
			if (!p) {
				break;
			}
		}		
	}
}

void insertinputevents(Sequencer* self)
{
	List* p;
	List* q;
	
	for (p = self->inputevents; p != 0; p = q) {
		PatternEntry* entry = (PatternEntry*)p->entry;
		
		q = p->next;
		entry->delta = 0;
		list_append(&self->events, entry);
		if (q) {
			q->tail = self->inputevents->tail;
			q->prev = 0;
		}
		self->inputevents = q;
		free(p);		
	}
}

void compute_beatsprosample(Sequencer* self)
{
	self->beatsprosample = (self->bpm * self->lpbspeed) / (self->samplerate * 60.0f);
}

beat_t sequencer_offset(Sequencer* self, int numsamples)
{
	return numsamples * self->beatsprosample;
}

unsigned int sequencer_frames(Sequencer* self, beat_t offset)
{
	return (unsigned int)(offset / self->beatsprosample);
}

int sequencer_playing(Sequencer* self)
{
	return self->playing;
}

List* sequencer_timedevents(Sequencer* self, unsigned int slot,
	unsigned int amount)
{
	List* rv = 0;
	List* p;

	rv = sequencer_machinetickevents(self, slot);
	for (p = rv ; p != 0; p = p->next) {		
		PatternEntry* entry;
		beat_t beatsprosample;
		unsigned int deltaframes;			

		entry = (PatternEntry*) p->entry;
		beatsprosample = (entry->bpm * self->lpbspeed) / (self->samplerate * 60.0f);			
		deltaframes = (unsigned int) (entry->delta / self->beatsprosample);
		if (deltaframes >= amount) {
			deltaframes = amount - 1;
		}
		entry->delta = (beat_t) deltaframes;						
	}
	return rv;
}

void sequencer_addinputevent(Sequencer* self, const PatternEvent* event,
	unsigned int track)
{
	if (event) {
		PatternEntry* entry;
	
		entry = (PatternEntry*) malloc(sizeof(PatternEntry));
			entry->event = *event;
			entry->track = track;
			entry->bpm = self->bpm;
			entry->delta = 0;
			entry->offset = 0;
		list_append(&self->inputevents, entry);
	}
}

void sequencer_recordinputevent(Sequencer* self, const PatternEvent* event,
	unsigned int track, beat_t playposition)
{
	SequenceTrackIterator it;

	it = sequence_begin(self->sequence, self->sequence->tracks, playposition);
	if (it.tracknode) {
		SequenceEntry* entry;
		Pattern* pattern;		
		
		entry = (SequenceEntry*) it.tracknode->entry;
		pattern = patterns_at(self->sequence->patterns, entry->pattern);
		if (pattern) {			
			beat_t quantizedpatternoffset;
			PatternNode* prev;
			PatternNode* node;

			quantizedpatternoffset = ((int)((playposition - entry->offset) *
				self->lpb)) / (beat_t)self->lpb;			
			node = pattern_findnode(pattern, 0, quantizedpatternoffset, 0, 
				1.f / self->lpb, &prev);
			if (node) {					
				pattern_setevent(pattern, node, event);
			} else {
				pattern_insert(pattern, prev, 0, quantizedpatternoffset,
					event);
			}
		}
	}
}
