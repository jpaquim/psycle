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
static void makecurrtracks(Sequencer*, beat_t offset);
static beat_t skiptrackiterators(Sequencer*, beat_t offset);
static void clearcurrtracks(Sequencer*);
static void advanceposition(Sequencer* self, beat_t width);
static void addsequenceevent(Sequencer*, SequencerTrack*, beat_t offset);
static void maketweakslideevents(Sequencer*, PatternEntry*);
static void makeretriggerevents(Sequencer*, SequencerTrack*, PatternEntry*);
static void makeretriggercontinueevents(Sequencer* self, SequencerTrack*, PatternEntry*);
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
	self->lpbspeed = (beat_t) 1.f;
	self->playing = 0;
	self->looping = 1;
	self->position = 0;	
	self->window = 0;
	self->seqlinetickcount = (beat_t) (1.f / self->lpb);
	self->events = 0;
	self->delayedevents = 0;
	self->inputevents = 0;
	self->currtracks = 0;
	self->mode = SEQUENCERPLAYMODE_PLAYALL;
	compute_beatsprosample(self);
	makecurrtracks(self, (beat_t) 0.f);
}

void sequencer_dispose(Sequencer* self)
{
	clearevents(self);
	cleardelayed(self);
	clearinputevents(self);
	clearcurrtracks(self);
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
	clearcurrtracks(self);
	self->position = offset;
	self->window = 0.0f;
	makecurrtracks(self, offset);
}

beat_t sequencer_position(Sequencer* self)
{	
	return self->position;
}

void sequencer_start(Sequencer* self)
{	
	self->lpbspeed = (beat_t) 1.f;
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

void sequencer_setlpb(Sequencer* self, uintptr_t lpb)
{	
	self->lpb = lpb;
	self->lpbspeed = (beat_t) 1.f;
	self->seqlinetickcount = 1.f / lpb;
	compute_beatsprosample(self);
}

uintptr_t sequencer_lpb(Sequencer* self)
{
	return self->lpb;
}

List* sequencer_tickevents(Sequencer* self)
{
	return self->events;
}

List* sequencer_machinetickevents(Sequencer* self, size_t slot)
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

void clearcurrtracks(Sequencer* self)
{
	List* p;

	for (p = self->currtracks; p != 0; p = p->next) {
		SequencerTrack* track;

		track = (SequencerTrack*) p->entry;
		free(track->iterator);
		free(track);
	}
	list_free(self->currtracks);
	self->currtracks = 0;
}

void makecurrtracks(Sequencer* self, beat_t offset)
{
	SequenceTracks* p;

	for (p = self->sequence->tracks; p != 0; p = p->next) {
		SequencerTrack* track;		

		track = malloc(sizeof(SequencerTrack));		
		track->iterator =
			(SequenceTrackIterator*)malloc(sizeof(SequenceTrackIterator));
		*track->iterator = sequence_begin(self->sequence, p, offset);		
		track->state.jump = 0;
		track->state.jumpat = 0;
		track->state.jumpto = 0;
		track->state.retriggeroffset = 0;
		track->state.retriggerstep = 0;
		list_append(&self->currtracks, track);
	}
}

beat_t skiptrackiterators(Sequencer* self, beat_t offset)
{
	List* p;		
		
	int first = 1;
	beat_t newplayposition = offset;
	for (p = self->currtracks; p != 0; p = p->next) {
		SequencerTrack* track;
		SequenceTrackIterator* it;
		int skip = 0;

		track = (SequencerTrack*) p->entry;
		it = track->iterator;
		while (it->tracknode && !sequencetrackiterator_entry(it)->selplay) {			
			sequencetrackiterator_incentry(it);			
			skip = 1;
		}
		if (first && it->tracknode && skip) {
			newplayposition = sequencetrackiterator_offset(it);
		}		
	}
	return newplayposition;
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
	sequencer_tick(self, sequencer_frametooffset(self, numframes));
}

void sequencer_tick(Sequencer* self, beat_t width)
{
	if (self->playing) {
		advanceposition(self, width);		
	}
	clearevents(self);
	insertinputevents(self);
	if (self->playing) {
		if (self->mode == SEQUENCERPLAYMODE_PLAYSEL) {
			self->position =
				skiptrackiterators(self, self->position);
		}
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
		machine->vtable->sequencertick(machine);
		if (linetick) {
			machine->vtable->sequencerlinetick(machine);
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
					
					insert = machine->vtable->sequencerinsert(machine, events);
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
	int continueplaying = 0;

	for (p = self->currtracks; p != 0; p = p->next) {
		SequencerTrack* track;
		SequenceTrackIterator* it;

		track = (SequencerTrack*) p->entry;
		it = track->iterator;
		if (!continueplaying && it->tracknode) {
			SequenceEntry* entry;
			Pattern* pattern;

			entry = sequencetrackiterator_entry(it);
			pattern = patterns_at(it->patterns, entry->pattern);
			if (pattern) {
				if (self->position <= entry->offset + pattern->length) {
					continueplaying = 1;
				} else 
				if (it->tracknode->next) {
					sequencetrackiterator_incentry(it);
					entry = sequencetrackiterator_entry(it);
					if (self->position <= entry->offset + pattern->length) {
						continueplaying = 1;
					}
				}
			}
			while (sequencetrackiterator_patternentry(it)) {
				beat_t offset;
				
				offset = sequencetrackiterator_offset(it);
				if (isoffsetinwindow(self, offset)) {
/*					PatternEntry* patternentry;

					patternentry = 
						sequencetrackiterator_patternentry(it);					
					if (patternentry->event.cmd == JUMP_TO_ORDER) {
						SequencePosition position;
						SequenceEntry* jumpsequenceentry;

						position = sequence_at(self->sequence,
							0, patternentry->event.cmd);
						if (position.trackposition.tracknode) {
							jumpsequenceentry = (SequenceEntry*)
								position.trackposition.tracknode->entry;
							it->state.jump = 1;
							it->state.jumpat = offset + (1 / (self->lpb * self->lpbspeed));
							it->state.jumpto = jumpsequenceentry;
						}
					}*/					
					addsequenceevent(self, track, offset);
					sequencetrackiterator_inc(it);					
				} else {			
					break;
				}				
			}
		}				
	}	
	if (self->looping && !continueplaying) {
		self->position = (beat_t) 0.f;
		clearevents(self);
		cleardelayed(self);
		clearcurrtracks(self);
		makecurrtracks(self, 0.f);
		self->window = (beat_t) 0.f;
	} else {
		self->playing = continueplaying;
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

void addsequenceevent(Sequencer* self, SequencerTrack* track, beat_t offset)
{	
	PatternEntry* patternentry;	

	patternentry = sequencetrackiterator_patternentry(track->iterator);
	if (!patternentry || (patternentry && patterns_istrackmuted(
			self->sequence->patterns, patternentry->track))) {
		return;
	}
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
			entry->delta = offset + entry->event.parameter / 
				(self->lpb * self->lpbspeed * 256.f);
			list_append(&self->delayedevents, entry);
		} else
		if (entry->event.cmd == RETRIGGER) {
			entry->delta = offset - self->position;
			list_append(&self->events, entry);
			makeretriggerevents(self, track, patternentry);
		} else
		if (entry->event.cmd == RETR_CONT) {			
			entry->delta = offset + track->state.retriggeroffset -
				self->position;
			list_append(&self->delayedevents, entry);
			makeretriggercontinueevents(self, track, patternentry);
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
	if (machine &&
			entry->event.inst < machine->vtable->numparameters(machine) > 0) {
		int param = entry->event.inst;
		int minval;
		int maxval;		
		int slides = sequencer_frames(self, 1.f/(self->lpb * self->lpbspeed)) / 64;		
		int dest = ((entry->event.cmd << 8) + entry->event.parameter);
		int start = machine->vtable->parametervalue(machine, param);
		int slide;
		float delta;
		float curr;

		machine->vtable->parameterrange(machine, entry->event.parameter, &minval,
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

void makeretriggerevents(Sequencer* self, SequencerTrack* track,
	PatternEntry* entry)
{
	beat_t retriggerstep;
	beat_t retriggeroffset;		

	retriggerstep = entry->event.parameter /
		(self->lpb * self->lpbspeed * 256.f);
	retriggeroffset = retriggerstep;
	while (retriggeroffset < 1 / (self->lpb * self->lpbspeed)) {
		PatternEntry* retriggerentry;

		retriggerentry = patternentry_clone(entry);
		retriggerentry->event.cmd = 0;
		retriggerentry->event.parameter = 0;
		retriggerentry->delta = entry->delta + retriggeroffset;
		list_append(&self->delayedevents, retriggerentry);
		retriggeroffset += retriggerstep;
	}
	track->state.retriggeroffset = 		
		retriggeroffset - (1 / (self->lpb * self->lpbspeed));
	track->state.retriggerstep = retriggerstep;
}

void makeretriggercontinueevents(Sequencer* self, SequencerTrack* track,
	PatternEntry* entry)
{
	beat_t retriggerstep;
	beat_t retriggeroffset;		

	if (entry->event.parameter == 0) {
		retriggerstep = track->state.retriggerstep;		
	} else {
		retriggerstep = track->state.retriggerstep;
	}
	retriggeroffset = track->state.retriggeroffset + retriggerstep;
	while (retriggeroffset < 1 / (self->lpb * self->lpbspeed)) {
		PatternEntry* retriggerentry;

		retriggerentry = patternentry_clone(entry);
		retriggerentry->event.cmd = 0;
		retriggerentry->event.parameter = 0;
		retriggerentry->delta = entry->delta + retriggeroffset;
		list_append(&self->delayedevents, retriggerentry);
		retriggeroffset += retriggerstep;
	}
	track->state.retriggeroffset =
		retriggeroffset - (1 / (self->lpb * self->lpbspeed));
	track->state.retriggerstep = retriggerstep;
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

beat_t sequencer_frametooffset(Sequencer* self, int numsamples)
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

List* sequencer_timedevents(Sequencer* self, size_t slot, unsigned int amount)
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
	uintptr_t track)
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

void sequencer_setplaymode(Sequencer* self, SequencerPlayMode mode)
{
	self->mode = mode;
}

SequencerPlayMode sequencer_playmode(Sequencer* self)
{
	return self->mode;
}

void sequencer_loop(Sequencer* self)
{
	self->looping = 1;
}

void sequencer_stoploop(Sequencer* self)
{
	self->looping = 0;
}

int sequencer_looping(Sequencer* self)
{
	return self->looping;
}
