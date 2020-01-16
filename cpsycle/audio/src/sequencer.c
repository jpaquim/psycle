// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequencer.h"
#include "pattern.h"
#include "instruments.h"
#include <stdlib.h> 

static void psy_audio_clearevents(psy_audio_Sequencer*);
static void psy_audio_cleardelayed(psy_audio_Sequencer*);
static void psy_audio_clearinputevents(psy_audio_Sequencer*);
static void freeentries(psy_List* events);
static void psy_audio_makecurrtracks(psy_audio_Sequencer*, psy_dsp_beat_t offset);
static void psy_audio_sequencer_setbarloop(psy_audio_Sequencer*);
static psy_dsp_beat_t skiptrackiterators(psy_audio_Sequencer*, psy_dsp_beat_t offset);
static void psy_audio_clearcurrtracks(psy_audio_Sequencer*);
static void psy_audio_advanceposition(psy_audio_Sequencer* self, psy_dsp_beat_t width);
static void psy_audio_addsequenceevent(psy_audio_Sequencer*, psy_audio_SequencerTrack*,
	psy_dsp_beat_t offset);
//static void addvolume(psy_audio_Sequencer* self, psy_audio_PatternEntry*,
//	int delayed);
static void psy_audio_addgate(psy_audio_Sequencer*, psy_audio_PatternEntry*);
static void psy_audio_maketweakslideevents(psy_audio_Sequencer*, psy_audio_PatternEntry*, psy_dsp_beat_t offset);
static void psy_audio_makeretriggerevents(psy_audio_Sequencer*, psy_audio_SequencerTrack*,
	psy_audio_PatternEntry*, psy_dsp_beat_t offset);
static void psy_audio_makeretriggercontinueevents(psy_audio_Sequencer*,
	psy_audio_SequencerTrack*, psy_audio_PatternEntry*, psy_dsp_beat_t offset);
static int psy_audio_isoffsetinwindow(psy_audio_Sequencer*, psy_dsp_beat_t offset);
static void psy_audio_insertevents(psy_audio_Sequencer*);
static void psy_audio_insertinputevents(psy_audio_Sequencer*);
static void psy_audio_insertdelayedevents(psy_audio_Sequencer*);
static int psy_audio_sequencerinsert(psy_audio_Sequencer*);
static void psy_audio_compute_beatspersample(psy_audio_Sequencer*);
static void psy_audio_notifysequencertick(psy_audio_Sequencer*, psy_dsp_beat_t width);
static psy_dsp_beat_t psy_audio_sequencer_speed(psy_audio_Sequencer*);
static uintptr_t psy_audio_sequencer_currframesperline(psy_audio_Sequencer*);
static void psy_audio_sequencer_sortevents(psy_audio_Sequencer*);
static void psy_audio_assertorder(psy_audio_Sequencer*);

void psy_audio_sequencer_init(psy_audio_Sequencer* self, psy_audio_Sequence*
	sequence, psy_audio_Machines* machines)
{
	self->sequence = sequence;
	self->machines = machines;
	self->samplerate = 44100;
	self->bpm = 125.f;
	self->lpb = 4;
	self->lpbspeed = (psy_dsp_beat_t) 1.f;
	self->playing = 0;
	self->looping = 1;
	self->numplaybeats = (psy_dsp_beat_t) 4.0f;
	self->position = 0;	
	self->window = 0;
	self->events = 0;
	self->delayedevents = 0;
	self->inputevents = 0;
	self->currtracks = 0;
	self->mode = psy_audio_SEQUENCERPLAYMODE_PLAYALL;
	self->jump.active = 0;
	self->jump.offset = (psy_dsp_beat_t) 0.f;
	self->rowdelay.active = 0;
	self->rowdelay.rowspeed = (psy_dsp_beat_t) 1.f;
	self->loop.active = 0;
	self->loop.count = 0;
	self->loop.offset = (psy_dsp_beat_t) 0.f;
	self->linetickcount = (psy_dsp_beat_t) 0.f;
	psy_audio_compute_beatspersample(self);
	psy_audio_makecurrtracks(self, (psy_dsp_beat_t) 0.f);
	psy_table_init(&self->lastmachine);
	psy_signal_init(&self->signal_linetick);
}

void psy_audio_sequencer_dispose(psy_audio_Sequencer* self)
{
	psy_audio_clearevents(self);
	psy_audio_cleardelayed(self);
	psy_audio_clearinputevents(self);
	psy_audio_clearcurrtracks(self);
	self->sequence = 0;
	self->machines = 0;
	psy_table_dispose(&self->lastmachine);
	psy_signal_dispose(&self->signal_linetick);
}

void psy_audio_sequencer_reset(psy_audio_Sequencer* self, psy_audio_Sequence*
	sequence, psy_audio_Machines* machines)
{	

	psy_audio_clearevents(self);
	psy_audio_cleardelayed(self);
	psy_audio_clearinputevents(self);
	psy_audio_clearcurrtracks(self);	
	self->sequence = sequence;
	self->machines = machines;
	self->samplerate = 44100;
	self->bpm = 125.f;
	self->lpb = 4;
	self->lpbspeed = (psy_dsp_beat_t) 1.f;
	self->playing = 0;
	self->looping = 1;
	self->numplaybeats = (psy_dsp_beat_t) 4.0f;
	self->position = 0;	
	self->window = 0;
	self->events = 0;
	self->delayedevents = 0;
	self->inputevents = 0;
	self->currtracks = 0;
	self->mode = psy_audio_SEQUENCERPLAYMODE_PLAYALL;
	self->jump.active = 0;
	self->jump.offset = (psy_dsp_beat_t) 0.f;
	self->rowdelay.active = 0;
	self->rowdelay.rowspeed = (psy_dsp_beat_t) 1.f;
	self->loop.active = 0;
	self->loop.count = 0;
	self->loop.offset = (psy_dsp_beat_t) 0.f;
	self->linetickcount = (psy_dsp_beat_t) 0.f;
	psy_audio_compute_beatspersample(self);
	psy_audio_makecurrtracks(self, (psy_dsp_beat_t) 0.f);
	psy_table_clear(&self->lastmachine);
}

void psy_audio_sequencer_setposition(psy_audio_Sequencer* self, psy_dsp_beat_t
	offset)
{
	psy_audio_clearevents(self);
	psy_audio_cleardelayed(self);		
	psy_audio_clearcurrtracks(self);
	self->position = offset;
	self->window = 0.0f;
	psy_audio_makecurrtracks(self, offset);
}

psy_dsp_beat_t psy_audio_sequencer_position(psy_audio_Sequencer* self)
{	
	return self->position;
}

void psy_audio_sequencer_start(psy_audio_Sequencer* self)
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
	psy_audio_compute_beatspersample(self);
	if (self->mode == psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS) {
		psy_audio_sequencer_setbarloop(self);
	}
	self->playing = 1;
}

void psy_audio_sequencer_setbarloop(psy_audio_Sequencer* self)
{	
	self->playbeatloopstart = (psy_dsp_beat_t) ((int) self->position);
	self->playbeatloopend = self->playbeatloopstart +
		(psy_dsp_beat_t) self->numplaybeats;
	psy_audio_sequencer_setposition(self, self->playbeatloopstart);
}

void psy_audio_sequencer_stop(psy_audio_Sequencer* self)
{
	self->playing = 0;
}

void psy_audio_sequencer_setnumplaybeats(psy_audio_Sequencer* self,
	psy_dsp_beat_t num)
{
	self->numplaybeats = num;
	self->playbeatloopend = self->playbeatloopstart + num;
}

void psy_audio_sequencer_setsamplerate(psy_audio_Sequencer* self,
	unsigned int samplerate)
{
	self->samplerate = samplerate;
	psy_audio_compute_beatspersample(self);
}

unsigned int psy_audio_sequencer_samplerate(psy_audio_Sequencer* self)
{
	return self->samplerate;
}

void psy_audio_sequencer_setbpm(psy_audio_Sequencer* self, psy_dsp_beat_t bpm)
{	
	if (bpm < 32) {
		self->bpm = 32;
	} else
	if (bpm > 999) {
		self->bpm = 999;
	} else {
		self->bpm = bpm;
	}
	psy_audio_compute_beatspersample(self);
}

psy_dsp_beat_t psy_audio_sequencer_bpm(psy_audio_Sequencer* self)
{
	return self->bpm;
}

void psy_audio_sequencer_setlpb(psy_audio_Sequencer* self, uintptr_t lpb)
{	
	self->lpb = lpb;
	self->lpbspeed = (psy_dsp_beat_t) 1.f;
	psy_audio_compute_beatspersample(self);
}

uintptr_t psy_audio_sequencer_lpb(psy_audio_Sequencer* self)
{
	return self->lpb;
}

psy_List* psy_audio_sequencer_tickevents(psy_audio_Sequencer* self)
{
	return self->events;
}

psy_List* psy_audio_sequencer_machinetickevents(psy_audio_Sequencer* self,
	uintptr_t slot)
{
	psy_List* rv = 0;
	psy_List* p;
		
	for (p = self->events; p != 0; p = p->next) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*) p->entry;		
		if (patternentry_front(entry)->mach == slot) {			
			psy_list_append(&rv, entry);						
		}		
	}
	return rv;
}

void psy_audio_clearcurrtracks(psy_audio_Sequencer* self)
{
	psy_List* p;

	for (p = self->currtracks; p != 0; p = p->next) {
		psy_audio_SequencerTrack* track;

		track = (psy_audio_SequencerTrack*) p->entry;
		free(track->iterator);
		free(track);
	}
	psy_list_free(self->currtracks);
	self->currtracks = 0;
}

void psy_audio_makecurrtracks(psy_audio_Sequencer* self, psy_dsp_beat_t offset)
{
	SequenceTracks* p;

	for (p = self->sequence->tracks; p != 0; p = p->next) {
		psy_audio_SequencerTrack* track;

		track = malloc(sizeof(psy_audio_SequencerTrack));
		track->iterator =
			(SequenceTrackIterator*)malloc(sizeof(SequenceTrackIterator));
		*track->iterator = sequence_begin(self->sequence, p, offset);		
		track->state.retriggeroffset = 0;
		track->state.retriggerstep = 0;		
		psy_list_append(&self->currtracks, track);
	}
}

psy_dsp_beat_t psy_audio_skiptrackiterators(psy_audio_Sequencer* self,
	psy_dsp_beat_t offset)
{
	psy_List* p;		
		
	int first = 1;
	psy_dsp_beat_t newplayposition = offset;
	for (p = self->currtracks; p != 0; p = p->next) {
		psy_audio_SequencerTrack* track;
		SequenceTrackIterator* it;
		int skip = 0;

		track = (psy_audio_SequencerTrack*) p->entry;
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

void psy_audio_clearevents(psy_audio_Sequencer* self)
{
	freeentries(self->events);
	psy_list_free(self->events);
	self->events = 0;	
}

void psy_audio_cleardelayed(psy_audio_Sequencer* self)
{	
	freeentries(self->delayedevents);
	psy_list_free(self->delayedevents);
	self->delayedevents = 0;	
}

void psy_audio_clearinputevents(psy_audio_Sequencer* self)
{
	freeentries(self->inputevents);
	psy_list_free(self->inputevents);
	self->inputevents = 0;	
}

void freeentries(psy_List* events)
{
	psy_List* p;
	
	for (p = events; p != 0; p = p->next) {
		patternentry_dispose((psy_audio_PatternEntry*) p->entry);
		free(p->entry);
	}
}

void psy_audio_sequencer_frametick(psy_audio_Sequencer* self, uintptr_t
	numframes)
{	
	psy_audio_sequencer_tick(self, psy_audio_sequencer_frametooffset(self,
		numframes));
}

void psy_audio_sequencer_tick(psy_audio_Sequencer* self,
	psy_dsp_beat_t width)
{
	if (self->playing) {
		psy_audio_advanceposition(self, width);		
	}
	psy_audio_clearevents(self);
	psy_audio_insertinputevents(self);
	if (self->playing) {
		if (self->mode == psy_audio_SEQUENCERPLAYMODE_PLAYSEL) {
			self->position =
				psy_audio_skiptrackiterators(self, self->position);
		} else
		if (self->mode == psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS) {
			if (self->position >= self->playbeatloopend - 
					1.f / (psy_dsp_beat_t) self->lpb) {
				self->jump.active = 1;
				self->jump.offset = self->playbeatloopstart;
			}
		}		
		psy_audio_insertevents(self);		
		psy_audio_insertdelayedevents(self);
	}
	psy_audio_notifysequencertick(self, width);
	if (self->playing && psy_audio_sequencerinsert(self)) {
		psy_audio_insertdelayedevents(self);		
	}	
	psy_audio_sequencer_sortevents(self);	
}

void psy_audio_advanceposition(psy_audio_Sequencer* self, psy_dsp_beat_t width)
{	
	self->position += self->window;
	self->window = width;	
}

void psy_audio_notifysequencertick(psy_audio_Sequencer* self, psy_dsp_beat_t width)
{
	psy_TableIterator it;	
	
	for (it = machines_begin(self->machines);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {		
		psy_audio_machine_sequencertick((psy_audio_Machine*)
			psy_tableiterator_value(&it));
	}
}

void psy_audio_sequencer_onlinetick(psy_audio_Sequencer* self)
{
	if (self->jump.active) {
		self->jump.active = 0;
		psy_audio_sequencer_setposition(self, self->jump.offset);
	}
	if (self->rowdelay.active) {
		self->rowdelay.active = 0;
		psy_audio_compute_beatspersample(self);
	}
	psy_signal_emit(&self->signal_linetick, self, 0);
}

int psy_audio_sequencerinsert(psy_audio_Sequencer* self) {
	PatternNode* p;
	int rv = 0;
	
	for (p = psy_audio_sequencer_tickevents(self); p != 0; p = p->next) {
		psy_audio_PatternEntry* entry;			
		
		entry = psy_audio_patternnode_entry(p);
		if (patternentry_front(entry)->mach != NOTECOMMANDS_EMPTY) {
			psy_audio_Machine* machine;				
				
			machine = machines_at(self->machines,
				patternentry_front(entry)->mach);
			if (machine) {
				psy_List* events;

				events = psy_audio_sequencer_machinetickevents(self,
					patternentry_front(entry)->mach);
				if (events) {					
					PatternNode* insert;
					
					insert = psy_audio_machine_sequencerinsert(machine, events);
					if (insert) {
						psy_audio_sequencer_append(self, insert);
						psy_list_free(insert);
						rv = 1;
					}
					psy_list_free(events);
				}					
			}									
		}
	}
	return rv;
}

void psy_audio_insertevents(psy_audio_Sequencer* self)
{
	psy_List* p;	
	int continueplaying = 0;

	for (p = self->currtracks; p != 0; p = p->next) {
		psy_audio_SequencerTrack* track;
		SequenceTrackIterator* it;

		track = (psy_audio_SequencerTrack*) p->entry;
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
				if (psy_audio_isoffsetinwindow(self, offset)) {
					psy_audio_PatternEntry* patternentry;

					patternentry = 
						sequencetrackiterator_patternentry(it);
					
					if (patternentry_front(patternentry)->cmd == EXTENDED) {
						if ((patternentry_front(patternentry)->parameter & 0xF0) ==
								PATTERN_DELAY) {
							psy_dsp_beat_t rows;

							rows = (psy_dsp_beat_t)
								(patternentry_front(patternentry)->parameter & 0x0F);																			
							if (rows > 0) {
								self->rowdelay.active = 1;	
								self->rowdelay.rowspeed = (psy_dsp_beat_t) 1.f / rows;
							} else {
								self->rowdelay.rowspeed = (psy_dsp_beat_t) 1.f;
								self->rowdelay.active = 0;
							}
							psy_audio_compute_beatspersample(self);
						} else
						if ((patternentry_front(patternentry)->parameter & 0xF0) ==
							FINE_PATTERN_DELAY) {
							psy_dsp_beat_t ticks;

							ticks = (psy_dsp_beat_t)
								(patternentry_front(patternentry)->parameter & 0x0F);
							self->rowdelay.active = 1;
							self->rowdelay.rowspeed = 
								(psy_dsp_beat_t)0.5 / 15 * (psy_dsp_beat_t)(30 - ticks);
							psy_audio_compute_beatspersample(self);
						} else
						if ((patternentry_front(patternentry)->parameter & 0xB0) ==
								PATTERN_LOOP) {
							if (!self->loop.active) {
								self->loop.count =
									patternentry_front(patternentry)->parameter & 0x0F;
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
					if (patternentry_front(patternentry)->cmd == JUMP_TO_ORDER) {
						SequencePosition position;

						position = sequence_at(self->sequence,
							0, patternentry_front(patternentry)->parameter);
						if (position.trackposition.tracknode) {
							SequenceEntry* orderentry;

							orderentry = (SequenceEntry*)
								position.trackposition.tracknode->entry;
							self->jump.active = 1;
							self->jump.offset = orderentry->offset;
						}
					} else					
					if (patternentry_front(patternentry)->cmd == BREAK_TO_LINE) {
						SequenceTrackNode* next = it->tracknode->next;
						if (next) {						
							SequenceEntry* orderentry;

							orderentry = (SequenceEntry*)
									next->entry;
							self->jump.active = 1;
							self->jump.offset = orderentry->offset +
								patternentry_front(patternentry)->parameter *
								((psy_dsp_beat_t)1.f / self->lpb);
						}
					} else {						
						psy_audio_addsequenceevent(self, track, offset);
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
		psy_audio_clearevents(self);
		psy_audio_cleardelayed(self);
		psy_audio_clearcurrtracks(self);
		psy_audio_makecurrtracks(self, 0.f);
		self->window = (psy_dsp_beat_t) 0.f;
	} else {
		self->playing = continueplaying;
	}
}

void psy_audio_sequencer_append(psy_audio_Sequencer* self, psy_List* events)
{
	PatternNode* p;

	for (p = events; p != 0; p = p->next) {
		psy_audio_PatternEntry* entry;

		entry = psy_audio_patternnode_entry(p);
		entry->delta += self->position;
		psy_list_append(&self->delayedevents, entry);
		// if (patternentry_front(entry)->cmd == NOTE_DELAY) {
		// 	psy_list_append(&self->delayedevents, entry);
		// } else {			
		// 	psy_list_append(&self->events, entry);
		// }
	}
}

int psy_audio_isoffsetinwindow(psy_audio_Sequencer* self, psy_dsp_beat_t offset)
{
	return offset >= self->position && offset < self->position + self->window;
}

void psy_audio_addsequenceevent(psy_audio_Sequencer* self,
	psy_audio_SequencerTrack* track, psy_dsp_beat_t offset)
{	
	psy_audio_PatternEntry* patternentry;	

	patternentry = sequencetrackiterator_patternentry(track->iterator);
	if (!patternentry || (patternentry && patterns_istrackmuted(
			self->sequence->patterns, patternentry->track))) {
		return;
	}
	if (patternentry_front(patternentry)->cmd == SET_TEMPO
		&& !(patternentry_front(patternentry)->note >= NOTECOMMANDS_RELEASE &&
			 patternentry_front(patternentry)->note <= NOTECOMMANDS_INVALID)) {
		self->bpm = patternentry_front(patternentry)->parameter;
		psy_audio_compute_beatspersample(self);
	} else 
	if (patternentry_front(patternentry)->cmd == EXTENDED 
		&& !(patternentry_front(patternentry)->note >= NOTECOMMANDS_RELEASE &&
			 patternentry_front(patternentry)->note <= NOTECOMMANDS_INVALID)) {
		if (patternentry_front(patternentry)->parameter < SET_LINESPERBEAT1) {
			self->lpbspeed = patternentry_front(patternentry)->parameter /
				(psy_dsp_beat_t)self->lpb;
			psy_audio_compute_beatspersample(self);
		}
	}
	if (patternentry_front(patternentry)->note == NOTECOMMANDS_TWEAKSLIDE) {
		psy_audio_maketweakslideevents(self, patternentry, offset);
	} else {
		psy_audio_PatternEntry* entry;
		entry =	patternentry_clone(patternentry);
		entry->bpm = self->bpm;
		if (patternentry_front(patternentry)->cmd == NOTE_DELAY) {
			entry->delta = offset + patternentry_front(entry)->parameter / 
				(self->lpb * psy_audio_sequencer_speed(self) * 256.f);
			psy_list_append(&self->delayedevents, entry);
			// addvolume(self, entry, 1);
			
		} else
		if (patternentry_front(patternentry)->cmd == RETRIGGER) {
			entry->delta = offset - self->position;
			psy_list_append(&self->events, entry);
			psy_audio_makeretriggerevents(self, track, patternentry, offset);
		} else
		if (patternentry_front(patternentry)->cmd == RETR_CONT) {			
			 entry->delta = offset + track->state.retriggeroffset -
				self->position;
			psy_list_append(&self->delayedevents, entry);
			psy_audio_makeretriggercontinueevents(self, track, patternentry,
				offset);
		} else {
			entry->delta = offset - self->position;
			if (patternentry_front(patternentry)->note <
					NOTECOMMANDS_RELEASE) {
				psy_table_insert(&self->lastmachine, entry->track,
					(void*)(uintptr_t)patternentry_front(patternentry)->mach);
			}
			psy_list_append(&self->events, entry);
			// addvolume(self, entry, 0);
			// addgate(self, entry);
		}
	}	
}

//void addvolume(psy_audio_Sequencer* self, psy_audio_PatternEntry* entry,
//	int delayed)
//{
	/*if (patternentry_front(entry)->vol != NOTECOMMANDS_VOL_EMPTY) {
		if (patternentry_front(entry)->cmd == 0 &&
			patternentry_front(entry)->parameter == 0) {
			patternentry_front(entry)->cmd = 0x0C;
			patternentry_front(entry)->parameter = patternentry_front(entry)->vol;
		} else {
			psy_audio_PatternEntry* volentry;

			volentry = patternentry_clone(entry);		
			patternentry_front(volentry)->note = NOTECOMMANDS_EMPTY;
			patternentry_front(volentry)->cmd = 0x0C;
			patternentry_front(volentry)->parameter =
				patternentry_front(entry)->vol;
			if (delayed) {
				psy_list_append(&self->delayedevents, volentry);
			} else {
				psy_list_append(&self->events, volentry);
			}
		}
	}*/
//}

void psy_audio_addgate(psy_audio_Sequencer* self, psy_audio_PatternEntry*
	entry)
{
	uint8_t gate;
	psy_List* p;
		
	gate = (uint8_t) NOTECOMMANDS_GATE_EMPTY;
	p = entry->events;
	while (p != 0) {
		psy_audio_PatternEvent* event;

		event = (psy_audio_PatternEvent*) p->entry;
		if (event->cmd == GATE) {
			gate = event->parameter;
			break;
		}
		p = p->next;
	}
	if (gate != NOTECOMMANDS_GATE_EMPTY) {		
		psy_audio_PatternEntry* noteoff;

		noteoff = patternentry_allocinit();
		patternentry_front(noteoff)->note = NOTECOMMANDS_RELEASE;
		patternentry_front(noteoff)->mach = patternentry_front(entry)->mach;
		noteoff->delta += entry->offset + gate / 
			(self->lpb * psy_audio_sequencer_speed(self) * 128.f);
		psy_list_append(&self->delayedevents, noteoff);
	}
}

void psy_audio_maketweakslideevents(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* entry, psy_dsp_beat_t offset)
{
	psy_audio_Machine* machine;
	uintptr_t param;
	
	machine = machines_at(self->machines, patternentry_front(entry)->mach);
	if (!machine) {
		return;
	}
	param = patternentry_front(entry)->inst;
	if (param >= psy_audio_machine_numparameters(machine)) {
		return;	
	}
	
	{
		int minval;
		int maxval;		
		int numslides;
		int dest;
		int start;
		int slide;
		float delta;
		float curr;

		numslides = psy_audio_sequencer_currframesperline(self) / 64;
		if (numslides == 0) {
			return;
		}
		psy_audio_machine_parameterrange(machine, param, &minval, &maxval);		
		start = psy_audio_machine_parametervalue(machine, param);
		if (start < minval) {
			start = minval;
		} else
		if (start > maxval) {
			start = maxval;
		}		
		start -= minval;
		dest = ((patternentry_front(entry)->cmd << 8) |
			patternentry_front(entry)->parameter) & 0xFF;		
		if (dest > maxval - minval) { 
			dest = maxval - minval;
		}
		if (dest == start) {
			psy_audio_PatternEntry* slideentry;
			
			slideentry = patternentry_clone(entry);
			patternentry_front(slideentry)->note = NOTECOMMANDS_TWEAK;
			slideentry->bpm = self->bpm;
			psy_list_append(&self->events, slideentry);
		} else {
			delta = (dest - start) / (float) numslides;
			if (delta < 0) {
				delta = delta;
			}
			curr = (float) start;
			for (slide = 0; slide < numslides; ++slide) {
				psy_audio_PatternEntry* slideentry;
				int cmd;
				uintptr_t parameter;				
				int nv;

				if (slide == numslides -1) {
					curr = (float) dest;
				}	
				if (curr > maxval - minval) {
					nv = maxval - minval;
				} else
				if (curr < 0) {
					nv = 0;
				} else {
					nv = (int) curr;
				}				
				cmd = nv >> 8;
				parameter = nv & 0xFF;
				curr += delta;
				slideentry = patternentry_clone(entry);
				slideentry->bpm = self->bpm;
				patternentry_front(slideentry)->note = NOTECOMMANDS_TWEAK;
				patternentry_front(slideentry)->cmd = cmd;
				patternentry_front(slideentry)->parameter = parameter;				
				slideentry->delta = offset +
					psy_audio_sequencer_frametooffset(self, slide * 64);
				psy_list_append(&self->delayedevents, slideentry);
			}
		}
	}
}

void psy_audio_makeretriggerevents(psy_audio_Sequencer* self,
	psy_audio_SequencerTrack* track, psy_audio_PatternEntry* entry,
	psy_dsp_beat_t offset)
{
	psy_dsp_beat_t retriggerstep;
	psy_dsp_beat_t retriggeroffset;		
	
	retriggerstep = patternentry_front(entry)->parameter /
		(self->lpb * psy_audio_sequencer_speed(self) * 256.f);
	retriggeroffset = retriggerstep;
	while (retriggeroffset < 1 / 
			(self->lpb * psy_audio_sequencer_speed(self))) {
		psy_audio_PatternEntry* retriggerentry;
		
		retriggerentry = patternentry_clone(entry);
		patternentry_front(retriggerentry)->cmd = 0;
		patternentry_front(retriggerentry)->parameter = 0;
		retriggerentry->delta = offset + entry->delta + retriggeroffset;
		psy_list_append(&self->delayedevents, retriggerentry);
		// addvolume(self, retriggerentry, 1);
		retriggeroffset += retriggerstep;
	}
	track->state.retriggeroffset = 		
		retriggeroffset - (1 / (self->lpb * psy_audio_sequencer_speed(self)));
	track->state.retriggerstep = retriggerstep;
}

void psy_audio_makeretriggercontinueevents(psy_audio_Sequencer* self,
	psy_audio_SequencerTrack* track, psy_audio_PatternEntry* entry,
	psy_dsp_beat_t offset)
{
	psy_dsp_beat_t retriggerstep;
	psy_dsp_beat_t retriggeroffset;		

	if (patternentry_front(entry)->parameter == 0) {
		retriggerstep = track->state.retriggerstep;		
	} else {
		retriggerstep = track->state.retriggerstep;
	}
	retriggeroffset = track->state.retriggeroffset + retriggerstep;
	while (retriggeroffset < 1 / (self->lpb *
			psy_audio_sequencer_speed(self))) {
		psy_audio_PatternEntry* retriggerentry;

		retriggerentry = patternentry_clone(entry);
		patternentry_front(retriggerentry)->cmd = 0;
		patternentry_front(retriggerentry)->parameter = 0;
		retriggerentry->delta = offset + entry->delta + retriggeroffset;
		psy_list_append(&self->delayedevents, retriggerentry);
		// addvolume(self, retriggerentry, 1);
		retriggeroffset += retriggerstep;
	}
	track->state.retriggeroffset =
		retriggeroffset - (1 / (self->lpb * psy_audio_sequencer_speed(self)));
	track->state.retriggerstep = retriggerstep;
}

void psy_audio_insertdelayedevents(psy_audio_Sequencer* self)
{
	PatternNode* p;	
	
	p = self->delayedevents;
	while (p != 0) {	
		psy_audio_PatternEntry* delayed;
		
		delayed = psy_audio_patternnode_entry(p);		
		if (psy_audio_isoffsetinwindow(self, delayed->delta)) {
			delayed->delta -= self->position;
			psy_list_append(&self->events, delayed);
			p = psy_list_remove(&self->delayedevents, p);
		} else {
			p = p->next;
		}
	}	
}

void psy_audio_insertinputevents(psy_audio_Sequencer* self)
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

void psy_audio_compute_beatspersample(psy_audio_Sequencer* self)
{
	self->beatspersample = (self->bpm * psy_audio_sequencer_speed(self)) /
		(self->samplerate * 60.0f);
}

psy_dsp_beat_t psy_audio_sequencer_beatspersample(psy_audio_Sequencer* self)
{
	return self->beatspersample;
}

psy_dsp_beat_t psy_audio_sequencer_frametooffset(psy_audio_Sequencer* self,
	uintptr_t numsamples)
{
	return numsamples * self->beatspersample;
}

uintptr_t psy_audio_sequencer_frames(psy_audio_Sequencer* self, psy_dsp_beat_t
	offset)
{
	return (uintptr_t)(offset / self->beatspersample);
}

int psy_audio_sequencer_playing(psy_audio_Sequencer* self)
{
	return self->playing;
}

psy_List* psy_audio_sequencer_timedevents(psy_audio_Sequencer* self, uintptr_t
	slot, uintptr_t amount)
{
	psy_List* rv = 0;
	psy_List* p;

	rv = psy_audio_sequencer_machinetickevents(self, slot);
	for (p = rv ; p != 0; p = p->next) {		
		psy_audio_PatternEntry* entry;
		psy_dsp_beat_t beatspersample;
		uintptr_t deltaframes;			

		entry = (psy_audio_PatternEntry*) p->entry;
		beatspersample = (entry->bpm * psy_audio_sequencer_speed(self)) /
			(self->samplerate * 60.0f);
		deltaframes = (uintptr_t) (entry->delta / self->beatspersample);
		if (deltaframes >= amount) {
			deltaframes = amount - 1;
		}
		entry->delta = (psy_dsp_beat_t) deltaframes;						
	}
	return rv;
}

void psy_audio_sequencer_addinputevent(psy_audio_Sequencer* self,
	const psy_audio_PatternEvent* event, uintptr_t track)
{
	if (event) {
		psy_audio_PatternEntry* entry;
	
		entry = patternentry_alloc();
		patternentry_init_all(entry, event, 0, 0, self->bpm, track);		
		psy_list_append(&self->inputevents, entry);
	}
}

void psy_audio_sequencer_recordinputevent(psy_audio_Sequencer* self,
	const psy_audio_PatternEvent* event, uintptr_t track,
	psy_dsp_beat_t playposition)
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

void psy_audio_sequencer_setplaymode(psy_audio_Sequencer* self,
	psy_audio_SequencerPlayMode mode)
{
	self->mode = mode;
}

psy_audio_SequencerPlayMode psy_audio_sequencer_playmode(psy_audio_Sequencer*
	self)
{
	return self->mode;
}

void psy_audio_sequencer_loop(psy_audio_Sequencer* self)
{
	self->looping = 1;
}

void psy_audio_sequencer_stoploop(psy_audio_Sequencer* self)
{
	self->looping = 0;
}

int psy_audio_sequencer_looping(psy_audio_Sequencer* self)
{
	return self->looping;
}

psy_dsp_beat_t psy_audio_sequencer_speed(psy_audio_Sequencer* self)
{
	psy_dsp_beat_t rv;

	rv = self->lpbspeed;
	if (self->rowdelay.active) {
		rv *= self->rowdelay.rowspeed;
	}
	return rv;
}

void psy_audio_sequencer_checkiterators(psy_audio_Sequencer* self,
	PatternNode* node)
{
	psy_List* p;

	for (p = self->currtracks; p != 0; p = p->next) {
		psy_audio_SequencerTrack* track;
		SequenceTrackIterator* it;

		track = (psy_audio_SequencerTrack*) p->entry;
		it = track->iterator;
		if (it->patternnode == node) {
			sequencetrackiterator_incentry(it);
		}
	}
}

psy_dsp_beat_t psy_audio_sequencer_currbeatsperline(psy_audio_Sequencer* self)
{
	return 1 / (self->lpb * psy_audio_sequencer_speed(self));
}

uintptr_t psy_audio_sequencer_currframesperline(psy_audio_Sequencer* self)
{
	return psy_audio_sequencer_frames(self,
		psy_audio_sequencer_currbeatsperline(self));	
}

void psy_audio_sequencer_sortevents(psy_audio_Sequencer* self)
{
	if (self->events && self->events != self->events->tail) {
		psy_List* sorted;
		psy_List* p;
		psy_List* q;

		sorted = 0;
		for (p = self->events; p != 0; p = p->next) {
			int inserted = 0;
			psy_audio_PatternEntry* entry;

			entry = psy_audio_patternnode_entry(p);
			q = sorted;
			while (q) {
				psy_audio_PatternEntry* sortedentry;

				sortedentry = psy_audio_patternnode_entry(q);
				if (sortedentry->delta > entry->delta) {
					psy_list_insert(&sorted, q->prev, entry);
					inserted = 1;
					break;
				}
				q = q->next;
			}
			if (!inserted) {
				psy_list_append(&sorted, entry);
			}
		}	
		psy_list_free(self->events);
		self->events = sorted;
	}
	// assertorder(self);
}

void psy_audio_assertorder(psy_audio_Sequencer* self)
{
	psy_List* p;
	float last = 0;
	
	p = self->events;
	while (p) {
		psy_audio_PatternEntry* entry;
		
		entry = psy_audio_patternnode_entry(p);
		assert(entry->delta >= last);
		last = entry->delta;			
		p = p->next;
	}
}
