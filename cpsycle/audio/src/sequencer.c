// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequencer.h"
#include "pattern.h"
#include "instruments.h"
#include <stdlib.h> 

static void clearevents(psy_audio_Sequencer*);
static void cleardelayed(psy_audio_Sequencer*);
static void clearinputevents(psy_audio_Sequencer*);
static void freeentries(psy_List* events);
static void makecurrtracks(psy_audio_Sequencer*, psy_dsp_beat_t offset);
static psy_dsp_beat_t skiptrackiterators(psy_audio_Sequencer*, psy_dsp_beat_t offset);
static void clearcurrtracks(psy_audio_Sequencer*);
static void advanceposition(psy_audio_Sequencer* self, psy_dsp_beat_t width);
static void addsequenceevent(psy_audio_Sequencer*, SequencerTrack*, psy_dsp_beat_t offset);
static void maketweakslideevents(psy_audio_Sequencer*, psy_audio_PatternEntry*);
static void makeretriggerevents(psy_audio_Sequencer*, SequencerTrack*, psy_audio_PatternEntry*);
static void makeretriggercontinueevents(psy_audio_Sequencer* self, SequencerTrack*, psy_audio_PatternEntry*);
static int isoffsetinwindow(psy_audio_Sequencer*, psy_dsp_beat_t offset);
static void insertevents(psy_audio_Sequencer*);
static void insertinputevents(psy_audio_Sequencer*);
static void insertdelayedevents(psy_audio_Sequencer*);
static void sequencerinsert(psy_audio_Sequencer*);
static void compute_beatspersample(psy_audio_Sequencer*);
static void notifysequencertick(psy_audio_Sequencer*, psy_dsp_beat_t width);
static psy_dsp_beat_t sequencer_speed(psy_audio_Sequencer*);

void sequencer_init(psy_audio_Sequencer* self, psy_audio_Sequence* sequence, psy_audio_Machines* machines)
{
	self->sequence = sequence;
	self->machines = machines;
	self->samplerate = 44100;
	self->bpm = 125.f;
	self->lpb = 4;
	self->lpbspeed = (psy_dsp_beat_t) 1.f;
	self->playing = 0;
	self->looping = 1;
	self->position = 0;	
	self->window = 0;
	self->events = 0;
	self->delayedevents = 0;
	self->inputevents = 0;
	self->currtracks = 0;
	self->mode = SEQUENCERPLAYMODE_PLAYALL;
	self->jump.active = 0;
	self->jump.offset = (psy_dsp_beat_t) 0.f;
	self->rowdelay.active = 0;
	self->rowdelay.rowspeed = (psy_dsp_beat_t) 1.f;
	self->loop.active = 0;
	self->loop.count = 0;
	self->loop.offset = (psy_dsp_beat_t) 0.f;
	self->linetickcount = (psy_dsp_beat_t) 0.f;
	compute_beatspersample(self);
	makecurrtracks(self, (psy_dsp_beat_t) 0.f);
	psy_table_init(&self->lastmachine);
}

void sequencer_dispose(psy_audio_Sequencer* self)
{
	clearevents(self);
	cleardelayed(self);
	clearinputevents(self);
	clearcurrtracks(self);
	self->sequence = 0;
	self->machines = 0;
	psy_table_dispose(&self->lastmachine);
}

void sequencer_reset(psy_audio_Sequencer* self, psy_audio_Sequence* sequence, psy_audio_Machines* machines)
{	
	sequencer_dispose(self);
	sequencer_init(self, sequence, machines);
}

void sequencer_setposition(psy_audio_Sequencer* self, psy_dsp_beat_t offset)
{
	clearevents(self);
	cleardelayed(self);		
	clearcurrtracks(self);
	self->position = offset;
	self->window = 0.0f;
	makecurrtracks(self, offset);
}

psy_dsp_beat_t sequencer_position(psy_audio_Sequencer* self)
{	
	return self->position;
}

void sequencer_start(psy_audio_Sequencer* self)
{	
	self->linetickcount = 0;
	self->jump.active = 0;
	self->jump.offset = (psy_dsp_beat_t) 0.f;
	self->rowdelay.active = 0;
	self->rowdelay.rowspeed = (psy_dsp_beat_t) 1.f;
	self->loop.active = 0;
	self->loop.count = 0;
	self->loop.offset = (psy_dsp_beat_t) 0.f;
	self->lpbspeed = (psy_dsp_beat_t) 1.f;
	compute_beatspersample(self);
	self->playing = 1;
}

void sequencer_stop(psy_audio_Sequencer* self)
{
	self->playing = 0;
}

void sequencer_setsamplerate(psy_audio_Sequencer* self, unsigned int samplerate)
{
	self->samplerate = samplerate;
	compute_beatspersample(self);
}

unsigned int sequencer_samplerate(psy_audio_Sequencer* self)
{
	return self->samplerate;
}

void sequencer_setbpm(psy_audio_Sequencer* self, psy_dsp_beat_t bpm)
{	
	if (bpm < 32) {
		self->bpm = 32;
	} else
	if (bpm > 999) {
		self->bpm = 999;
	} else {
		self->bpm = bpm;
	}
	compute_beatspersample(self);
}

psy_dsp_beat_t sequencer_bpm(psy_audio_Sequencer* self)
{
	return self->bpm;
}

void sequencer_setlpb(psy_audio_Sequencer* self, uintptr_t lpb)
{	
	self->lpb = lpb;
	self->lpbspeed = (psy_dsp_beat_t) 1.f;
	compute_beatspersample(self);
}

uintptr_t sequencer_lpb(psy_audio_Sequencer* self)
{
	return self->lpb;
}

psy_List* sequencer_tickevents(psy_audio_Sequencer* self)
{
	return self->events;
}

psy_List* sequencer_machinetickevents(psy_audio_Sequencer* self, size_t slot)
{
	psy_List* rv = 0;
	psy_List* p;
		
	for (p = self->events; p != 0; p = p->next) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*) p->entry;		
		if (entry->event.mach == slot) {			
			psy_list_append(&rv, entry);						
		}		
	}
	return rv;
}

void clearcurrtracks(psy_audio_Sequencer* self)
{
	psy_List* p;

	for (p = self->currtracks; p != 0; p = p->next) {
		SequencerTrack* track;

		track = (SequencerTrack*) p->entry;
		free(track->iterator);
		free(track);
	}
	psy_list_free(self->currtracks);
	self->currtracks = 0;
}

void makecurrtracks(psy_audio_Sequencer* self, psy_dsp_beat_t offset)
{
	SequenceTracks* p;

	for (p = self->sequence->tracks; p != 0; p = p->next) {
		SequencerTrack* track;		

		track = malloc(sizeof(SequencerTrack));		
		track->iterator =
			(SequenceTrackIterator*)malloc(sizeof(SequenceTrackIterator));
		*track->iterator = sequence_begin(self->sequence, p, offset);		
		track->state.retriggeroffset = 0;
		track->state.retriggerstep = 0;		
		psy_list_append(&self->currtracks, track);
	}
}

psy_dsp_beat_t skiptrackiterators(psy_audio_Sequencer* self, psy_dsp_beat_t offset)
{
	psy_List* p;		
		
	int first = 1;
	psy_dsp_beat_t newplayposition = offset;
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

void clearevents(psy_audio_Sequencer* self)
{
	freeentries(self->events);
	psy_list_free(self->events);
	self->events = 0;	
}

void cleardelayed(psy_audio_Sequencer* self)
{	
	freeentries(self->delayedevents);
	psy_list_free(self->delayedevents);
	self->delayedevents = 0;	
}

void clearinputevents(psy_audio_Sequencer* self)
{
	freeentries(self->inputevents);
	psy_list_free(self->inputevents);
	self->inputevents = 0;	
}

void freeentries(psy_List* events)
{
	psy_List* p;
	
	for (p = events; p != 0; p = p->next) {
		free(p->entry);
	}
}

void sequencer_frametick(psy_audio_Sequencer* self, unsigned int numframes)
{	
	sequencer_tick(self, sequencer_frametooffset(self, numframes));
}

void sequencer_tick(psy_audio_Sequencer* self, psy_dsp_beat_t width)
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

void advanceposition(psy_audio_Sequencer* self, psy_dsp_beat_t width)
{	
	self->position += self->window;
	self->window = width;	
}

void notifysequencertick(psy_audio_Sequencer* self, psy_dsp_beat_t width)
{
	psy_TableIterator it;	
	
	for (it = machines_begin(self->machines); !psy_tableiterator_equal(&it, psy_table_end());		
			psy_tableiterator_inc(&it)) {			
		psy_audio_Machine* machine;

		machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
		machine->vtable->sequencertick(machine);
	}
}

void sequencer_linetick(psy_audio_Sequencer* self)
{
	if (self->jump.active) {
		self->jump.active = 0;
		sequencer_setposition(self, self->jump.offset);
	}
	if (self->rowdelay.active) {
		self->rowdelay.active = 0;
		compute_beatspersample(self);
	}
}

void sequencerinsert(psy_audio_Sequencer* self) {
	psy_List* p;

	for (p = sequencer_tickevents(self); p != 0; p = p->next) {			
		psy_audio_PatternEntry* entry;			
		
		entry = (psy_audio_PatternEntry*) p->entry;
		if (entry->event.mach != NOTECOMMANDS_EMPTY) {
			psy_audio_Machine* machine;				
				
			machine = machines_at(self->machines, entry->event.mach);
			if (machine) {
				psy_List* events;

				events = sequencer_machinetickevents(self, entry->event.mach);
				if (events) {					
					psy_List* insert;
					
					insert = machine->vtable->sequencerinsert(machine, events);
					if (insert) {
						sequencer_append(self, insert);					
						psy_list_free(insert);
					}
					psy_list_free(events);
				}					
			}									
		}
	}
}

void insertevents(psy_audio_Sequencer* self)
{
	psy_List* p;	
	int continueplaying = 0;

	for (p = self->currtracks; p != 0; p = p->next) {
		SequencerTrack* track;
		SequenceTrackIterator* it;

		track = (SequencerTrack*) p->entry;
		it = track->iterator;
		if (!continueplaying && it->tracknode) {
			SequenceEntry* entry;
			psy_audio_Pattern* pattern;

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
				psy_dsp_beat_t offset;
				
				offset = sequencetrackiterator_offset(it);
				if (isoffsetinwindow(self, offset)) {					
					psy_audio_PatternEntry* patternentry;

					patternentry = 
						sequencetrackiterator_patternentry(it);
					if (patternentry->event.cmd == EXTENDED) {
						if ((patternentry->event.parameter & 0xF0) ==
								PATTERN_DELAY) {
							psy_dsp_beat_t rows;

							rows = (psy_dsp_beat_t)
								(patternentry->event.parameter & 0x0F);																			
							if (rows > 0) {
								self->rowdelay.active = 1;	
								self->rowdelay.rowspeed = (psy_dsp_beat_t) 1.f / rows;
							} else {
								self->rowdelay.rowspeed = (psy_dsp_beat_t) 1.f;
								self->rowdelay.active = 0;
							}
							compute_beatspersample(self);
						} else
						if ((patternentry->event.parameter & 0xF0) ==
							FINE_PATTERN_DELAY) {
							psy_dsp_beat_t ticks;

							ticks = (psy_dsp_beat_t)
								(patternentry->event.parameter & 0x0F);
							self->rowdelay.active = 1;
							self->rowdelay.rowspeed = 
								(psy_dsp_beat_t)0.5 / 15 * (psy_dsp_beat_t)(30 - ticks);
							compute_beatspersample(self);													
						} else
						if ((patternentry->event.parameter & 0xB0) ==
								PATTERN_LOOP) {
							if (!self->loop.active) {
								self->loop.count =
									patternentry->event.parameter & 0x0F;
								if (self->loop.count > 0) {
									self->loop.active = 1;									
									self->jump.active = 1;
									self->jump.offset = self->loop.offset;
								} else {
									self->loop.offset = offset;
								}
							} else
							if (self->loop.count > 0 && 
									offset != self->loop.offset) {
								--self->loop.count;
								if (self->loop.count > 0) {
									self->jump.active = 1;
									self->jump.offset = self->loop.offset;
								} else {
									self->loop.active = 0;
									self->loop.offset = offset +
										(psy_dsp_beat_t) 1.f / self->lpb;
								}
							} else 
							if (self->loop.count == 0) {
								self->loop.active = 0;
								self->loop.offset = offset;
							}
						}
					} else
					if (patternentry->event.cmd == JUMP_TO_ORDER) {
						SequencePosition position;

						position = sequence_at(self->sequence,
							0, patternentry->event.parameter);
						if (position.trackposition.tracknode) {
							SequenceEntry* orderentry;

							orderentry = (SequenceEntry*)
								position.trackposition.tracknode->entry;
							self->jump.active = 1;
							self->jump.offset = orderentry->offset;
						}
					} else					
					if (patternentry->event.cmd == BREAK_TO_LINE) {
						SequenceTrackNode* next = it->tracknode->next;
						if (next) {						
							SequenceEntry* orderentry;

							orderentry = (SequenceEntry*)
									next->entry;
							self->jump.active = 1;
							self->jump.offset = orderentry->offset +
								patternentry->event.parameter *
								((psy_dsp_beat_t)1.f / self->lpb);
						}
					} else {						
						addsequenceevent(self, track, offset);
					}
					sequencetrackiterator_inc(it);
				} else {			
					break;
				}				
			}
		}				
	}	
	if (self->looping && !continueplaying) {
		self->position = (psy_dsp_beat_t) 0.f;
		clearevents(self);
		cleardelayed(self);
		clearcurrtracks(self);
		makecurrtracks(self, 0.f);
		self->window = (psy_dsp_beat_t) 0.f;
	} else {
		self->playing = continueplaying;
	}
}

void sequencer_append(psy_audio_Sequencer* self, psy_List* events)
{
	psy_List* p;

	for (p = events; p != 0; p = p->next) {
		psy_audio_PatternEntry* entry;

		entry = (psy_audio_PatternEntry*) p->entry;
		if (entry->event.cmd == NOTE_DELAY) {
			psy_list_append(&self->delayedevents, entry);
		} else {
			psy_list_append(&self->events, entry);
		}
	}
}

int isoffsetinwindow(psy_audio_Sequencer* self, psy_dsp_beat_t offset)
{
	return offset >= self->position && offset < self->position + self->window;
}

void addsequenceevent(psy_audio_Sequencer* self, SequencerTrack* track, psy_dsp_beat_t offset)
{	
	psy_audio_PatternEntry* patternentry;	

	patternentry = sequencetrackiterator_patternentry(track->iterator);
	if (!patternentry || (patternentry && patterns_istrackmuted(
			self->sequence->patterns, patternentry->track))) {
		return;
	}
	if (patternentry->event.cmd == SET_TEMPO
		&& !(patternentry->event.note >= NOTECOMMANDS_RELEASE &&
			 patternentry->event.note <= NOTECOMMANDS_INVALID)) {
		self->bpm = patternentry->event.parameter;
		compute_beatspersample(self);		
	} else 
	if (patternentry->event.cmd == EXTENDED 
		&& !(patternentry->event.note >= NOTECOMMANDS_RELEASE &&
			 patternentry->event.note <= NOTECOMMANDS_INVALID)) {
		if (patternentry->event.parameter < SET_LINESPERBEAT1) {
			self->lpbspeed = patternentry->event.parameter / (psy_dsp_beat_t)self->lpb;
			compute_beatspersample(self);
		}
	}
	if (patternentry->event.note == NOTECOMMANDS_TWEAKSLIDE) {
		maketweakslideevents(self, patternentry);
	} else {
		psy_audio_PatternEntry* entry;
		entry =	patternentry_clone(patternentry);
		entry->bpm = self->bpm;
		if (entry->event.cmd == NOTE_DELAY) {
			entry->delta = offset + entry->event.parameter / 
				(self->lpb * sequencer_speed(self) * 256.f);
			psy_list_append(&self->delayedevents, entry);
		} else
		if (entry->event.cmd == RETRIGGER) {
			entry->delta = offset - self->position;
			psy_list_append(&self->events, entry);
			makeretriggerevents(self, track, patternentry);
		} else
		if (entry->event.cmd == RETR_CONT) {			
			entry->delta = offset + track->state.retriggeroffset -
				self->position;
			psy_list_append(&self->delayedevents, entry);
			makeretriggercontinueevents(self, track, patternentry);
		} else {
			entry->delta = offset - self->position;
			if (entry->event.note < NOTECOMMANDS_RELEASE) {
				psy_table_insert(&self->lastmachine, entry->track,
					(void*)(uintptr_t)entry->event.mach);				
			}
			psy_list_append(&self->events, entry);
		}
	}	
}

void maketweakslideevents(psy_audio_Sequencer* self, psy_audio_PatternEntry* entry)
{
	psy_audio_Machine* machine;		
		
	machine = machines_at(self->machines, entry->event.mach);
	if (machine &&
			entry->event.inst < machine->vtable->numparameters(machine) > 0) {
		uintptr_t param = entry->event.inst;
		int minval;
		int maxval;		
		int slides = sequencer_frames(self, 1.f/(self->lpb * sequencer_speed(self))) / 64;		
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
			psy_audio_PatternEntry* slideentry;
			slideentry = patternentry_clone(entry);
			slideentry->event.note = NOTECOMMANDS_TWEAK;
			slideentry->bpm = self->bpm;
			psy_list_append(&self->events, slideentry);
		} else {
			delta = (dest - start) / (float)slides;			
			curr = (float)start + minval;
			for (slide = 0; slide < slides; ++slide) {
				psy_audio_PatternEntry* slideentry;
				int cmd;
				uintptr_t parameter;				
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
				slideentry->delta += slide * 64 * self->beatspersample;				
				psy_list_append(&self->delayedevents, slideentry);
			}
		}
	}
}

void makeretriggerevents(psy_audio_Sequencer* self, SequencerTrack* track,
	psy_audio_PatternEntry* entry)
{
	psy_dsp_beat_t retriggerstep;
	psy_dsp_beat_t retriggeroffset;		

	retriggerstep = entry->event.parameter /
		(self->lpb * sequencer_speed(self) * 256.f);
	retriggeroffset = retriggerstep;
	while (retriggeroffset < 1 / (self->lpb * sequencer_speed(self))) {
		psy_audio_PatternEntry* retriggerentry;

		retriggerentry = patternentry_clone(entry);
		retriggerentry->event.cmd = 0;
		retriggerentry->event.parameter = 0;
		retriggerentry->delta = entry->delta + retriggeroffset;
		psy_list_append(&self->delayedevents, retriggerentry);
		retriggeroffset += retriggerstep;
	}
	track->state.retriggeroffset = 		
		retriggeroffset - (1 / (self->lpb * sequencer_speed(self)));
	track->state.retriggerstep = retriggerstep;
}

void makeretriggercontinueevents(psy_audio_Sequencer* self, SequencerTrack* track,
	psy_audio_PatternEntry* entry)
{
	psy_dsp_beat_t retriggerstep;
	psy_dsp_beat_t retriggeroffset;		

	if (entry->event.parameter == 0) {
		retriggerstep = track->state.retriggerstep;		
	} else {
		retriggerstep = track->state.retriggerstep;
	}
	retriggeroffset = track->state.retriggeroffset + retriggerstep;
	while (retriggeroffset < 1 / (self->lpb * sequencer_speed(self))) {
		psy_audio_PatternEntry* retriggerentry;

		retriggerentry = patternentry_clone(entry);
		retriggerentry->event.cmd = 0;
		retriggerentry->event.parameter = 0;
		retriggerentry->delta = entry->delta + retriggeroffset;
		psy_list_append(&self->delayedevents, retriggerentry);
		retriggeroffset += retriggerstep;
	}
	track->state.retriggeroffset =
		retriggeroffset - (1 / (self->lpb * sequencer_speed(self)));
	track->state.retriggerstep = retriggerstep;
}

void insertdelayedevents(psy_audio_Sequencer* self)
{
	psy_List* p;
	
	for (p = self->delayedevents; p != 0; p = p->next) {
		psy_audio_PatternEntry* delayed = (psy_audio_PatternEntry*)p->entry;

		if (isoffsetinwindow(self, delayed->offset + delayed->delta)) {						
			psy_List* q;
			int inserted = 0;
			q = self->events;
			while (q) {
				psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)q->entry;
				if (delayed->offset >= entry->offset) {					
					break;
				}
				q = q->next;
			}			
			p = psy_list_remove(&self->delayedevents, p);
			delayed->delta = delayed->offset + delayed->delta - self->position;	
			if (q) {
				q = psy_list_insert(&self->events, q, delayed);								
			} else {				
				psy_list_append(&self->events, delayed);								
			}			
			if (!p) {
				break;
			}
		}		
	}
}

void insertinputevents(psy_audio_Sequencer* self)
{
	psy_List* p;
	psy_List* q;
	
	for (p = self->inputevents; p != 0; p = q) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)p->entry;
		
		q = p->next;
		entry->delta = 0;
		psy_list_append(&self->events, entry);
		if (q) {
			q->tail = self->inputevents->tail;
			q->prev = 0;
		}
		self->inputevents = q;
		free(p);		
	}
}

void compute_beatspersample(psy_audio_Sequencer* self)
{
	self->beatspersample = (self->bpm * sequencer_speed(self)) / (self->samplerate * 60.0f);
}

psy_dsp_beat_t sequencer_beatspersample(psy_audio_Sequencer* self)
{
	return self->beatspersample;
}

psy_dsp_beat_t sequencer_frametooffset(psy_audio_Sequencer* self, int numsamples)
{
	return numsamples * self->beatspersample;
}

unsigned int sequencer_frames(psy_audio_Sequencer* self, psy_dsp_beat_t offset)
{
	return (unsigned int)(offset / self->beatspersample);
}

int sequencer_playing(psy_audio_Sequencer* self)
{
	return self->playing;
}

psy_List* sequencer_timedevents(psy_audio_Sequencer* self, size_t slot, unsigned int amount)
{
	psy_List* rv = 0;
	psy_List* p;

	rv = sequencer_machinetickevents(self, slot);
	for (p = rv ; p != 0; p = p->next) {		
		psy_audio_PatternEntry* entry;
		psy_dsp_beat_t beatspersample;
		unsigned int deltaframes;			

		entry = (psy_audio_PatternEntry*) p->entry;
		beatspersample = (entry->bpm * sequencer_speed(self)) / (self->samplerate * 60.0f);			
		deltaframes = (unsigned int) (entry->delta / self->beatspersample);
		if (deltaframes >= amount) {
			deltaframes = amount - 1;
		}
		entry->delta = (psy_dsp_beat_t) deltaframes;						
	}
	return rv;
}

void sequencer_addinputevent(psy_audio_Sequencer* self, const psy_audio_PatternEvent* event,
	uintptr_t track)
{
	if (event) {
		psy_audio_PatternEntry* entry;
	
		entry = (psy_audio_PatternEntry*) malloc(sizeof(psy_audio_PatternEntry));
			entry->event = *event;
			entry->track = track;
			entry->bpm = self->bpm;
			entry->delta = 0;
			entry->offset = 0;
		psy_list_append(&self->inputevents, entry);
	}
}

void sequencer_recordinputevent(psy_audio_Sequencer* self, const psy_audio_PatternEvent* event,
	unsigned int track, psy_dsp_beat_t playposition)
{
	SequenceTrackIterator it;

	it = sequence_begin(self->sequence, self->sequence->tracks, playposition);
	if (it.tracknode) {
		SequenceEntry* entry;
		psy_audio_Pattern* pattern;		
		
		entry = (SequenceEntry*) it.tracknode->entry;
		pattern = patterns_at(self->sequence->patterns, entry->pattern);
		if (pattern) {			
			psy_dsp_beat_t quantizedpatternoffset;
			PatternNode* prev;
			PatternNode* node;

			quantizedpatternoffset = ((int)((playposition - entry->offset) *
				self->lpb)) / (psy_dsp_beat_t)self->lpb;			
			node = pattern_findnode(pattern, 0, quantizedpatternoffset, 
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

void sequencer_setplaymode(psy_audio_Sequencer* self, SequencerPlayMode mode)
{
	self->mode = mode;
}

SequencerPlayMode sequencer_playmode(psy_audio_Sequencer* self)
{
	return self->mode;
}

void sequencer_loop(psy_audio_Sequencer* self)
{
	self->looping = 1;
}

void sequencer_stoploop(psy_audio_Sequencer* self)
{
	self->looping = 0;
}

int sequencer_looping(psy_audio_Sequencer* self)
{
	return self->looping;
}

psy_dsp_beat_t sequencer_speed(psy_audio_Sequencer* self)
{
	psy_dsp_beat_t rv;

	rv = self->lpbspeed;
	if (self->rowdelay.active) {
		rv *= self->rowdelay.rowspeed;
	}
	return rv;
}
