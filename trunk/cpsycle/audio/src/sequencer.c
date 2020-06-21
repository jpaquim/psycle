// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequencer.h"
#include "pattern.h"
#include "instruments.h"
#include <qsort.h>
#include <assert.h>
#include <stdlib.h>
#include "../../detail/portable.h"

static void psy_audio_sequencerjump_init(psy_audio_SequencerJump* self)
{
	self->active = 0;
	self->offset = (psy_dsp_beat_t) 0.f;
}

static void psy_audio_sequencerjump_activate(psy_audio_SequencerJump* self,
	psy_dsp_beat_t offset)
{
	self->active = 1;
	self->offset = offset;
}

static void psy_audio_sequencerloop_init(psy_audio_SequencerLoop* self)
{
	self->active = 0;
	self->count = 0;
	self->offset = (psy_dsp_beat_t) 0.f;
}

static void psy_audio_sequencerrowdelay_init(psy_audio_SequencerRowDelay* self)
{
	self->active = 0;
	// line delay
	self->rowspeed = (psy_dsp_beat_t) 1.f;
}

static void psy_audio_sequencer_reset_common(psy_audio_Sequencer*,
	psy_audio_Sequence*, psy_audio_Machines*, uintptr_t samplerate);
static void psy_audio_sequencer_clearevents(psy_audio_Sequencer*);
static void psy_audio_sequencer_cleardelayed(psy_audio_Sequencer*);
static void psy_audio_sequencer_clearinputevents(psy_audio_Sequencer*);
static void psy_audio_sequencer_makecurrtracks(psy_audio_Sequencer*,
	psy_dsp_beat_t offset);
static void psy_audio_sequencer_setbarloop(psy_audio_Sequencer*);
static psy_dsp_beat_t psy_audio_sequencer_skiptrackiterators(
	psy_audio_Sequencer*, psy_dsp_beat_t offset);
static void psy_audio_sequencer_clearcurrtracks(psy_audio_Sequencer*);
static void psy_audio_sequencer_advanceposition(psy_audio_Sequencer*,
	psy_dsp_beat_t width);
static void psy_audio_sequencer_updateplaymodeposition(psy_audio_Sequencer*);
static void psy_audio_sequencer_addsequenceevent(psy_audio_Sequencer*, 
	psy_audio_PatternEntry*,  psy_audio_SequencerTrack*,
	psy_dsp_beat_t offset);
static void psy_audio_addgate(psy_audio_Sequencer*, psy_audio_PatternEntry*);
static void psy_audio_sequencer_maketweakslideevents(psy_audio_Sequencer*,
	psy_audio_PatternEntry*, psy_dsp_beat_t offset);
static void psy_audio_sequencer_makeretriggerevents(psy_audio_Sequencer*,
	psy_audio_SequencerTrack*, psy_audio_PatternEntry*, psy_dsp_beat_t offset);
static void psy_audio_sequencer_makeretriggercontinueevents(psy_audio_Sequencer*,
	psy_audio_SequencerTrack*, psy_audio_PatternEntry*, psy_dsp_beat_t offset);
static int psy_audio_sequencer_isoffsetinwindow(psy_audio_Sequencer*,
	psy_dsp_beat_t offset);
static void psy_audio_sequencer_insertevents(psy_audio_Sequencer*);
static void psy_audio_sequencer_insertinputevents(psy_audio_Sequencer*);
static void psy_audio_sequencer_insertdelayedevents(psy_audio_Sequencer*);
static int psy_audio_sequencer_sequencerinsert(psy_audio_Sequencer*);
static void psy_audio_sequencer_compute_beatspersample(psy_audio_Sequencer*);
static void psy_audio_sequencer_notifysequencertick(psy_audio_Sequencer*,
	psy_dsp_beat_t width);
static uintptr_t psy_audio_sequencer_currframesperline(psy_audio_Sequencer*);
static void psy_audio_sequencer_sortevents(psy_audio_Sequencer*);
static int psy_audio_sequencer_comp_events(PatternNode* lhs, PatternNode* rhs);
static void psy_audio_sequencer_assertorder(psy_audio_Sequencer*);
static void psy_audio_sequencer_jumpto(psy_audio_Sequencer*,
	psy_dsp_beat_t position);
static void psy_audio_sequencer_executejump(psy_audio_Sequencer*);
static void psy_audio_sequencer_notedelay(psy_audio_Sequencer*,
	psy_audio_PatternEntry*, psy_dsp_beat_t offset);
static void psy_audio_sequencer_retrigger(psy_audio_Sequencer*,
	psy_audio_PatternEntry*, psy_audio_SequencerTrack*,
	psy_dsp_beat_t offset);
static void psy_audio_sequencer_retriggercont(psy_audio_Sequencer*,
	psy_audio_PatternEntry*, psy_audio_SequencerTrack*,
	psy_dsp_beat_t offset);
static void psy_audio_sequencer_note(psy_audio_Sequencer*,
	psy_audio_PatternEntry* patternentry, psy_dsp_beat_t offset);
static void psy_audio_sequencer_tweak(psy_audio_Sequencer*,
	psy_audio_PatternEntry*, psy_dsp_beat_t offset);
static void psy_audio_sequencer_executeline(psy_audio_Sequencer*,
	psy_audio_SequencerTrack*, psy_dsp_beat_t offset);
static bool psy_audio_sequencer_executeglobalcommands(psy_audio_Sequencer*,
	psy_audio_PatternEntry*, psy_audio_SequencerTrack*,
	psy_dsp_beat_t offset);
static void psy_audio_sequencer_patterndelay(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*);
void psy_audio_sequencer_finepatterndelay(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*);
static void psy_audio_sequencer_patternloop(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*, psy_dsp_beat_t offset);
static void psy_audio_sequencer_jumptoorder(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*);
static void psy_audio_sequencer_breaktoline(psy_audio_Sequencer*,
	SequenceTrackIterator*, const psy_audio_PatternEvent*);

void psy_audio_sequencer_init(psy_audio_Sequencer* self, psy_audio_Sequence*
	sequence, psy_audio_Machines* machines)
{
	self->events = NULL;
	self->delayedevents = NULL;
	self->inputevents = NULL;
	self->currtracks = NULL;
	psy_table_init(&self->lastmachine);
	psy_signal_init(&self->signal_linetick);
	psy_audio_sequencer_reset_common(self, sequence, machines, 44100);	
}

void psy_audio_sequencer_dispose(psy_audio_Sequencer* self)
{
	psy_audio_sequencer_clearevents(self);
	psy_audio_sequencer_cleardelayed(self);
	psy_audio_sequencer_clearinputevents(self);
	psy_audio_sequencer_clearcurrtracks(self);	
	psy_table_dispose(&self->lastmachine);
	psy_signal_dispose(&self->signal_linetick);
	self->sequence = NULL;
	self->machines = NULL;
}

void psy_audio_sequencer_reset(psy_audio_Sequencer* self, psy_audio_Sequence*
	sequence, psy_audio_Machines* machines, uintptr_t samplerate)
{		
	psy_audio_sequencer_reset_common(self, sequence, machines, samplerate);	
	psy_table_clear(&self->lastmachine);
}

void psy_audio_sequencer_reset_common(psy_audio_Sequencer* self,
	psy_audio_Sequence* sequence, psy_audio_Machines* machines,
	uintptr_t samplerate)
{
	self->sequence = sequence;
	self->machines = machines;
	self->samplerate = samplerate;
	self->bpm = (psy_dsp_beat_t)125.f;
	self->lpb = 4;
	self->lpbspeed = (psy_dsp_beat_t)1.f;
	self->playing = FALSE;
	self->looping = TRUE;
	self->numplaybeats = (psy_dsp_beat_t)4.0f;
	self->position = (psy_dsp_beat_t)0.f;
	self->playcounter = 0;
	self->window = (psy_dsp_beat_t)0.f;
	self->linetickcount = (psy_dsp_beat_t)0.f;		
	self->mode = psy_audio_SEQUENCERPLAYMODE_PLAYALL;
	self->calcduration = FALSE;
	psy_audio_sequencer_clearevents(self);
	psy_audio_sequencer_cleardelayed(self);
	psy_audio_sequencer_clearinputevents(self);
	psy_audio_sequencer_clearcurrtracks(self);
	psy_audio_sequencer_makecurrtracks(self, (psy_dsp_beat_t)0.f);
	psy_audio_sequencer_compute_beatspersample(self);
	psy_audio_sequencerjump_init(&self->jump);
	psy_audio_sequencerrowdelay_init(&self->rowdelay);
	psy_audio_sequencerloop_init(&self->loop);
}

void psy_audio_sequencer_setposition(psy_audio_Sequencer* self,
	psy_dsp_beat_t offset)
{
	psy_audio_sequencer_clearevents(self);
	psy_audio_sequencer_cleardelayed(self);
	psy_audio_sequencer_clearcurrtracks(self);
	self->position = offset;
	self->window = (psy_dsp_beat_t)0.f;
	psy_audio_sequencer_makecurrtracks(self, offset);
}

void psy_audio_sequencer_start(psy_audio_Sequencer* self)
{		
	self->playcounter = 0;
	psy_audio_sequencerjump_init(&self->jump);
	psy_audio_sequencerrowdelay_init(&self->rowdelay);
	psy_audio_sequencerloop_init(&self->loop);
	self->lpbspeed = (psy_dsp_beat_t)1.f;
	psy_audio_sequencer_compute_beatspersample(self);
	self->linetickcount = 0.f;
	if (self->mode == psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS) {
		psy_audio_sequencer_setbarloop(self);
	}
	self->playing = TRUE;
}

void psy_audio_sequencer_setbarloop(psy_audio_Sequencer* self)
{	
	self->playbeatloopstart = (psy_dsp_beat_t)((int) self->position);
	self->playbeatloopend = self->playbeatloopstart +
		(psy_dsp_beat_t) self->numplaybeats;
	psy_audio_sequencer_setposition(self, self->playbeatloopstart);
}

void psy_audio_sequencer_setnumplaybeats(psy_audio_Sequencer* self,
	psy_dsp_beat_t num)
{
	self->numplaybeats = num;
	self->playbeatloopend = self->playbeatloopstart + num;
}

void psy_audio_sequencer_setsamplerate(psy_audio_Sequencer* self,
	uintptr_t samplerate)
{
	self->samplerate = samplerate;
	psy_audio_sequencer_compute_beatspersample(self);
}

void psy_audio_sequencer_setbpm(psy_audio_Sequencer* self,
	psy_dsp_beat_t bpm)
{	
	if (bpm < 32) {
		self->bpm = 32;
	} else
	if (bpm > 999) {
		self->bpm = 999;
	} else {
		self->bpm = bpm;
	}
	psy_audio_sequencer_compute_beatspersample(self);
}

void psy_audio_sequencer_setlpb(psy_audio_Sequencer* self, uintptr_t lpb)
{	
	self->lpb = lpb;
	self->lpbspeed = (psy_dsp_beat_t)1.f;
	psy_audio_sequencer_compute_beatspersample(self);
}

psy_List* psy_audio_sequencer_machinetickevents(psy_audio_Sequencer* self,
	uintptr_t slot)
{
	psy_List* rv = 0;
	psy_List* p;
		
	for (p = self->events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*) p->entry;		
		if (patternentry_front(entry)->mach == slot) {			
			psy_list_append(&rv, entry);						
		}		
	}
	return rv;
}

void psy_audio_sequencer_clearcurrtracks(psy_audio_Sequencer* self)
{
	psy_List* p;

	for (p = self->currtracks; p != NULL; psy_list_next(&p)) {
		psy_audio_SequencerTrack* track;

		track = (psy_audio_SequencerTrack*) p->entry;
		free(track->iterator);
		free(track);
	}
	psy_list_free(self->currtracks);
	self->currtracks = 0;
}

void psy_audio_sequencer_makecurrtracks(psy_audio_Sequencer* self, psy_dsp_beat_t offset)
{
	SequenceTracks* p;

	for (p = self->sequence->tracks; p != NULL; psy_list_next(&p)) {
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

psy_dsp_beat_t psy_audio_sequencer_skiptrackiterators(psy_audio_Sequencer* self,
	psy_dsp_beat_t offset)
{
	psy_List* p;		
		
	int first = 1;
	psy_dsp_beat_t newplayposition = offset;
	for (p = self->currtracks; p != NULL; psy_list_next(&p)) {
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

void psy_audio_sequencer_clearevents(psy_audio_Sequencer* self)
{
	psy_list_deallocate(&self->events, (psy_fp_disposefunc)
		patternentry_dispose);	
}

void psy_audio_sequencer_cleardelayed(psy_audio_Sequencer* self)
{	
	psy_list_deallocate(&self->delayedevents, (psy_fp_disposefunc)
		patternentry_dispose);	
}

void psy_audio_sequencer_clearinputevents(psy_audio_Sequencer* self)
{
	psy_list_deallocate(&self->inputevents, (psy_fp_disposefunc)
		patternentry_dispose);	
}

void psy_audio_sequencer_frametick(psy_audio_Sequencer* self, uintptr_t
	numframes)
{	
	if (self->playing) {
		self->playcounter += numframes;
	}
	psy_audio_sequencer_tick(self, psy_audio_sequencer_frametooffset(self,
		numframes));
}

void psy_audio_sequencer_tick(psy_audio_Sequencer* self,
	psy_dsp_beat_t width)
{
	if (psy_audio_sequencer_playing(self)) {
		psy_audio_sequencer_advanceposition(self, width);		
	}
	psy_audio_sequencer_clearevents(self);
	psy_audio_sequencer_insertinputevents(self);
	if (psy_audio_sequencer_playing(self)) {
		psy_audio_sequencer_updateplaymodeposition(self);
		psy_audio_sequencer_insertevents(self);
		psy_audio_sequencer_insertdelayedevents(self);
	}
	psy_audio_sequencer_notifysequencertick(self, width);
	if (psy_audio_sequencer_playing(self) &&
			psy_audio_sequencer_sequencerinsert(self)) {
		psy_audio_sequencer_insertdelayedevents(self);
	}	
	psy_audio_sequencer_sortevents(self);	
}

void psy_audio_sequencer_advanceposition(psy_audio_Sequencer* self,
	psy_dsp_beat_t width)
{	
	self->position += self->window;
	self->window = width;	
}

void psy_audio_sequencer_updateplaymodeposition(psy_audio_Sequencer* self)
{
	switch (psy_audio_sequencer_playmode(self)) {
	case psy_audio_SEQUENCERPLAYMODE_PLAYSEL:
		self->position = psy_audio_sequencer_skiptrackiterators(self,
			self->position);
		break;
	case psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS:
		if (self->position >= self->playbeatloopend -
			1.f / (psy_dsp_beat_t)self->lpb) {
			psy_audio_sequencer_jumpto(self, self->playbeatloopstart);
		}
		break;
	default:
		break;
	}
}

void psy_audio_sequencer_notifysequencertick(psy_audio_Sequencer* self,
	psy_dsp_beat_t width)
{
	if (self->machines) {
		psy_TableIterator it;

		for (it = psy_audio_machines_begin(self->machines);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_machine_sequencertick((psy_audio_Machine*)
				psy_tableiterator_value(&it));
		}
	}
}

uintptr_t psy_audio_sequencer_updatelinetickcount(psy_audio_Sequencer* self, uintptr_t amount)
{
	uintptr_t rv;

	self->linetickcount -= psy_audio_sequencer_frametooffset(self, amount);
	if (self->linetickcount <= 0) {
		rv = psy_audio_sequencer_frames(self, -self->linetickcount);
	} else {
		rv = amount;
	}
	return rv;
}

void psy_audio_sequencer_onlinetick(psy_audio_Sequencer* self)
{
	if (self->jump.active) {
		psy_audio_sequencer_executejump(self);
	}
	if (self->rowdelay.active) {
		self->rowdelay.active = 0;
		psy_audio_sequencer_compute_beatspersample(self);
	}
	psy_signal_emit(&self->signal_linetick, self, 0);	
	self->linetickcount = psy_audio_sequencer_currbeatsperline(self);	
}

void psy_audio_sequencer_executejump(psy_audio_Sequencer* self)
{
	self->jump.active = 0;
	psy_audio_sequencer_setposition(self, self->jump.offset);
}

int psy_audio_sequencer_sequencerinsert(psy_audio_Sequencer* self) {
	PatternNode* p;
	int rv = 0;
	
	for (p = psy_audio_sequencer_tickevents(self); p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* entry;			
		
		entry = psy_audio_patternnode_entry(p);
		if (patternentry_front(entry)->mach != NOTECOMMANDS_EMPTY && self->machines) {
			psy_audio_Machine* machine;				
				
			machine = psy_audio_machines_at(self->machines,
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

void psy_audio_sequencer_insertevents(psy_audio_Sequencer* self)
{	
	bool continueplaying = FALSE;
	bool work = TRUE;

	while (work) {
		psy_List* p;

		work = FALSE;		
		for (p = self->currtracks; p != NULL; psy_list_next(&p)) {
			psy_audio_SequencerTrack* track;
			psy_dsp_beat_t offset;

			track = (psy_audio_SequencerTrack*) p->entry;
			if (track->iterator->tracknode) {
				SequenceEntry* entry;
				psy_audio_Pattern* pattern;

				entry = sequencetrackiterator_entry(track->iterator);
				pattern = patterns_at(track->iterator->patterns, entry->pattern);
				if (pattern) {
					if (self->position + self->window < entry->offset + pattern->length) {
						continueplaying = TRUE;
					} else
					if (track->iterator->tracknode->next) {
						sequencetrackiterator_incentry(track->iterator);
						entry = sequencetrackiterator_entry(track->iterator);
						if (self->position <= entry->offset + pattern->length) {
							continueplaying = TRUE;
						}
					}
				}				
				if (track->iterator->patternnode) {
					offset = sequencetrackiterator_offset(track->iterator);
					if (psy_audio_sequencer_isoffsetinwindow(self, offset)) {
						psy_audio_sequencer_executeline(self, track, offset);
						work = TRUE;
					}
				}
			}
		}		
	}	
	if (self->looping && !continueplaying) {
		psy_audio_sequencer_setposition(self, 0.f);
	} else {
		self->playing = continueplaying;
	}
}

void psy_audio_sequencer_executeline(psy_audio_Sequencer* self,
	psy_audio_SequencerTrack* track,
	psy_dsp_beat_t offset)
{
	psy_dsp_beat_t rowoffset;
	PatternNode* events;
	PatternNode* p;		

	// First execute global events on the same offset (e.g. tracker row start)
	// before adding them to the current event list that time changes are
	// applied to all of them independend of the channel order
	rowoffset = offset;
	events = NULL;
	while (track->iterator->patternnode &&
			rowoffset == sequencetrackiterator_offset(track->iterator)) {
		psy_audio_PatternEntry* entry;

		entry = sequencetrackiterator_patternentry(track->iterator);
		if (entry && !patterns_istrackmuted(self->sequence->patterns,
				entry->track)) {
			psy_audio_sequencer_executeglobalcommands(self,
				entry, track, offset);
			psy_list_append(&events, entry);
		}
		sequencetrackiterator_inc(track->iterator);		
	}
	// global events are applied, now execute the events
	for (p = events; p != NULL; psy_list_next(&p)) {
		psy_audio_sequencer_addsequenceevent(self,
			psy_audio_patternnode_entry(p), track, rowoffset);
	}
	psy_list_free(events);
}

bool psy_audio_sequencer_executeglobalcommands(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* patternentry, psy_audio_SequencerTrack* track,
	psy_dsp_beat_t offset)
{
	psy_List* p;
	bool done;
	
	done = FALSE;
	for (p = patternentry->events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEvent* ev;

		ev = (psy_audio_PatternEvent*) p->entry;
		if (ev->note < NOTECOMMANDS_TWEAK || ev->note == NOTECOMMANDS_EMPTY) {
			switch (ev->cmd) {
				case EXTENDED:
					if ((ev->parameter & 0xF0) == PATTERN_DELAY) {
						psy_audio_sequencer_patterndelay(self, ev);
						done = TRUE;
					} else
					if ((ev->parameter & 0xF0) == FINE_PATTERN_DELAY) {
						psy_audio_sequencer_finepatterndelay(self, ev);
						done = TRUE;
					} else
					if (ev->parameter < SET_LINESPERBEAT1) {
						self->lpbspeed = ev->parameter /
							(psy_dsp_beat_t)self->lpb;
						psy_audio_sequencer_compute_beatspersample(self);
						done = TRUE;
					}
				break;
				case SET_TEMPO:
					self->bpm = patternentry_front(patternentry)->parameter;
					psy_audio_sequencer_compute_beatspersample(self);
					done = TRUE;
				break;
				case JUMP_TO_ORDER:
					if (!self->calcduration) {
						psy_audio_sequencer_jumptoorder(self, ev);
					}
					done = TRUE;
				break;
				case BREAK_TO_LINE:
					if (!self->calcduration) {
						psy_audio_sequencer_breaktoline(self, track->iterator, ev);						
					}
					done = TRUE;
				break;
				default:
				break;
			}
		}
	}	
	return done;
}

void psy_audio_sequencer_patterndelay(psy_audio_Sequencer* self,
	const psy_audio_PatternEvent* ev)
{
	psy_dsp_beat_t rows;

	rows = (psy_dsp_beat_t)(ev->parameter & 0x0F);
	if (rows > 0) {
		self->rowdelay.active = 1;
		self->rowdelay.rowspeed = (psy_dsp_beat_t)1.f / rows;
	}
	else {
		self->rowdelay.rowspeed = (psy_dsp_beat_t)1.f;
		self->rowdelay.active = 0;
	}
	psy_audio_sequencer_compute_beatspersample(self);
}

void psy_audio_sequencer_finepatterndelay(psy_audio_Sequencer* self,
	const psy_audio_PatternEvent* ev)
{
	psy_dsp_beat_t ticks;

	ticks = (psy_dsp_beat_t)(ev->parameter & 0x0F);
	self->rowdelay.active = 1;
	self->rowdelay.rowspeed =
		(psy_dsp_beat_t) 0.5 / 15 * (psy_dsp_beat_t)(30 - ticks);
	psy_audio_sequencer_compute_beatspersample(self);
}

void psy_audio_sequencer_patternloop(psy_audio_Sequencer* self,
	const psy_audio_PatternEvent* ev, psy_dsp_beat_t offset)
{
	if (!self->loop.active) {
		self->loop.count = ev->parameter & 0x0F;
		if (self->loop.count > 0) {
			self->loop.active = 1;
			psy_audio_sequencer_jumpto(self, self->loop.offset);
		}
		else {
			self->loop.offset = offset;
		}
	}
	else
	if (self->loop.count > 0 &&
		offset != self->loop.offset) {
		--self->loop.count;
		if (self->loop.count > 0) {
			psy_audio_sequencer_jumpto(self, self->loop.offset);
		}
		else {
			self->loop.active = 0;
			self->loop.offset = offset +
				(psy_dsp_beat_t)1.f / self->lpb;
		}
	}
	else
	if (self->loop.count == 0) {
		self->loop.active = 0;
		self->loop.offset = offset;
	}
}

void psy_audio_sequencer_jumptoorder(psy_audio_Sequencer* self, const psy_audio_PatternEvent* ev)
{
	SequencePosition position;

	position = sequence_at(self->sequence, 0, ev->parameter);
	if (position.trackposition.tracknode) {
		SequenceEntry* orderentry;

		orderentry = (SequenceEntry*)position.trackposition.tracknode->entry;
		psy_audio_sequencer_jumpto(self, orderentry->offset);
	}
}

void psy_audio_sequencer_breaktoline(psy_audio_Sequencer* self,
	SequenceTrackIterator* it, const psy_audio_PatternEvent* ev)
{
	SequenceTrackNode* next = it->tracknode->next;
	if (next) {
		SequenceEntry* orderentry;

		orderentry = (SequenceEntry*) next->entry;
		psy_audio_sequencer_jumpto(self, orderentry->offset + ev->parameter *
			((psy_dsp_beat_t)1.f / self->lpb));
	}
}

void psy_audio_sequencer_append(psy_audio_Sequencer* self, psy_List* events)
{
	PatternNode* p;

	for (p = events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* entry;

		entry = psy_audio_patternnode_entry(p);
		entry->delta += self->position;
		psy_list_append(&self->delayedevents, entry);		
	}
}

INLINE int psy_audio_sequencer_isoffsetinwindow(psy_audio_Sequencer* self,
	psy_dsp_beat_t offset)
{
	return offset >= self->position && offset < self->position + self->window;
}

void psy_audio_sequencer_addsequenceevent(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* patternentry,
	psy_audio_SequencerTrack* track, psy_dsp_beat_t offset)
{			
	if (patternentry_front(patternentry)->note == NOTECOMMANDS_TWEAKSLIDE) {
		psy_audio_sequencer_maketweakslideevents(self, patternentry, offset);
	} else
	if (patternentry_front(patternentry)->note < NOTECOMMANDS_TWEAK ||
			patternentry_front(patternentry)->note == NOTECOMMANDS_EMPTY) {
		if (self->machines) {
			psy_audio_Machine* machine;

			machine = psy_audio_machines_at(self->machines, patternentry_front(patternentry)->mach);
			if (!machine) {
				if (patternentry_front(patternentry)->mach == NOTECOMMANDS_MACH_EMPTY) {
					uintptr_t lastmachine;
					lastmachine = (uintptr_t)psy_table_at(&self->lastmachine, patternentry->track);
					machine = psy_audio_machines_at(self->machines, lastmachine);
				}
			}
			// Does this machine really exist and is not muted?
			if (machine && !psy_audio_machine_muted(machine)) {
				if (patternentry_front(patternentry)->cmd == NOTE_DELAY) {
					psy_audio_sequencer_notedelay(self, patternentry, offset);
				} else if (patternentry_front(patternentry)->cmd == RETRIGGER) {
					psy_audio_sequencer_retrigger(self, patternentry, track, offset);
				} else if (patternentry_front(patternentry)->cmd == RETR_CONT) {
					psy_audio_sequencer_retriggercont(self, patternentry, track, offset);
				} else if (patternentry_front(patternentry)->cmd != SET_VOLUME) {
					psy_audio_sequencer_note(self, patternentry, offset);
				}
			} 
		}
		if (patternentry_front(patternentry)->cmd == SET_VOLUME) {
			psy_audio_PatternEntry* entry;

			// volume column used to store mach
			entry = patternentry_clone(patternentry);			
			patternentry_front(entry)->vol = 
				(patternentry_front(entry)->mach == NOTECOMMANDS_MACH_EMPTY)
				? psy_audio_MASTER_INDEX
				: patternentry_front(entry)->mach;
			// because master handles all wires volumes
			patternentry_front(entry)->mach = psy_audio_MASTER_INDEX;
			patternentry_setbpm(entry, self->bpm);
			entry->delta = offset - self->position;
			psy_list_append(&self->events, entry);				
		}		
	} else
	if (patternentry_front(patternentry)->note == NOTECOMMANDS_TWEAK) {
		psy_audio_sequencer_tweak(self, patternentry, offset);
	} else
	if ((patternentry_front(patternentry)->note == NOTECOMMANDS_MIDICC) &&
			(patternentry_front(patternentry)->inst < 0x80)) {
		psy_audio_PatternEntry* entry;

		entry = patternentry_clone(patternentry);
		patternentry_setbpm(entry, self->bpm);
		entry->delta = offset - self->position;
		entry->track = patternentry_front(patternentry)->inst;
		patternentry_front(entry)->note = NOTECOMMANDS_EMPTY;
		patternentry_front(entry)->inst = NOTECOMMANDS_INST_EMPTY;
		psy_list_append(&self->events, entry);
	}		
}

void psy_audio_sequencer_notedelay(psy_audio_Sequencer* self, psy_audio_PatternEntry* patternentry,
	psy_dsp_beat_t offset)
{
	psy_audio_PatternEntry* entry;

	entry = patternentry_clone(patternentry);
	patternentry_setbpm(entry, self->bpm);
	entry->delta = offset + patternentry_front(entry)->parameter *
		(psy_audio_sequencer_currbeatsperline(self) / 256.f);
	psy_list_append(&self->delayedevents, entry);
}

void psy_audio_sequencer_retrigger(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* patternentry, psy_audio_SequencerTrack* track,
	psy_dsp_beat_t offset)
{
	psy_audio_PatternEntry* entry;

	entry = patternentry_clone(patternentry);
	patternentry_setbpm(entry, self->bpm);
	entry->delta = offset - self->position;
	psy_list_append(&self->events, entry);
	psy_audio_sequencer_makeretriggerevents(self, track, entry, offset);
}

void psy_audio_sequencer_retriggercont(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* patternentry, psy_audio_SequencerTrack* track,
	psy_dsp_beat_t offset)
{
	psy_audio_PatternEntry* entry;

	entry = patternentry_clone(patternentry);
	patternentry_setbpm(entry, self->bpm);
	entry->delta = offset + track->state.retriggeroffset;
	psy_list_append(&self->delayedevents, entry);
	psy_audio_sequencer_makeretriggercontinueevents(self, track, entry,
		offset);
}

void psy_audio_sequencer_note(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* patternentry, psy_dsp_beat_t offset)
{
	psy_audio_PatternEntry* entry;

	entry = patternentry_clone(patternentry);
	if (patternentry_front(entry)->mach == NOTECOMMANDS_MACH_EMPTY) {
		uintptr_t lastmachine;

		lastmachine = (uintptr_t)psy_table_at(&self->lastmachine,
			patternentry->track);
		patternentry_front(entry)->mach = (uint8_t)lastmachine;
	}
	patternentry_setbpm(entry, self->bpm);
	entry->delta = offset - self->position;
	if (patternentry_front(patternentry)->note <
		NOTECOMMANDS_RELEASE) {
		psy_table_insert(&self->lastmachine, entry->track,
			(void*)(uintptr_t)patternentry_front(patternentry)->mach);
	}
	psy_list_append(&self->events, entry);
}

void psy_audio_sequencer_tweak(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* patternentry, psy_dsp_beat_t offset)
{
	psy_audio_PatternEntry* entry;

	entry = patternentry_clone(patternentry);
	patternentry_setbpm(entry, self->bpm);
	entry->delta = offset - self->position;
	patternentry_front(entry)->vol = 0;
	psy_list_append(&self->events, entry);
}

void psy_audio_addgate(psy_audio_Sequencer* self, psy_audio_PatternEntry*
	entry)
{
	uint8_t gate;
	psy_List* p;
		
	gate = (uint8_t) NOTECOMMANDS_GATE_EMPTY;
	p = entry->events;
	while (p != NULL) {
		psy_audio_PatternEvent* event;

		event = (psy_audio_PatternEvent*) p->entry;
		if (event->cmd == GATE) {
			gate = event->parameter;
			break;
		}
		psy_list_next(&p);
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

void psy_audio_sequencer_maketweakslideevents(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* entry, psy_dsp_beat_t offset)
{
	psy_audio_Machine* machine;
	uintptr_t tweak;
	psy_audio_MachineParam* param;
	uintptr_t slide;
	uintptr_t framesperslide = 64;
	uintptr_t numslides;
	
	if (!self->machines) {
		return;
	}
	machine = psy_audio_machines_at(self->machines, patternentry_front(entry)->mach);
	if (!machine) {
		return;
	}
	tweak = patternentry_front(entry)->inst;
	if (tweak >= psy_audio_machine_numtweakparameters(machine)) {
		return;	
	}
	param = psy_audio_machine_tweakparameter(machine, tweak);
	if (!param) {
		return;
	}	
	numslides = psy_audio_sequencer_currframesperline(self) / framesperslide;
	if (numslides == 0) {
		return;
	}
	if (numslides > UINT16_MAX) {
		numslides = UINT16_MAX;
	}			
	for (slide = 0; slide < numslides; ++slide) {
		psy_audio_PatternEntry* slideentry;
				
		slideentry = patternentry_clone(entry);
		patternentry_front(slideentry)->note = NOTECOMMANDS_TWEAK;
		patternentry_front(slideentry)->vol = (uint16_t)(numslides - slide);
		patternentry_setbpm(slideentry, self->bpm);		
		slideentry->delta = offset +
			psy_audio_sequencer_frametooffset(self, slide * framesperslide);
		slideentry->priority = 1; // not used right now
		psy_list_append(&self->delayedevents, slideentry);
	}			
}

void psy_audio_sequencer_makeretriggerevents(psy_audio_Sequencer* self,
	psy_audio_SequencerTrack* track, psy_audio_PatternEntry* entry,
	psy_dsp_beat_t offset)
{
	psy_dsp_beat_t retriggerstep;
	psy_dsp_beat_t retriggeroffset;		
	
	retriggerstep = patternentry_front(entry)->parameter *
		(psy_audio_sequencer_currbeatsperline(self) / 256.f);		
	retriggeroffset = retriggerstep;
	while (retriggeroffset < psy_audio_sequencer_currbeatsperline(self)) {
		psy_audio_PatternEntry* retriggerentry;
		
		retriggerentry = patternentry_clone(entry);
		patternentry_front(retriggerentry)->cmd = 0;
		patternentry_front(retriggerentry)->parameter = 0;
		retriggerentry->delta = offset + entry->delta + retriggeroffset;
		psy_list_append(&self->delayedevents, retriggerentry);
		retriggeroffset += retriggerstep;
	}
	track->state.retriggeroffset = retriggeroffset -
		psy_audio_sequencer_currbeatsperline(self);
	track->state.retriggerstep = retriggerstep;
}

void psy_audio_sequencer_makeretriggercontinueevents(psy_audio_Sequencer* self,
	psy_audio_SequencerTrack* track, psy_audio_PatternEntry* entry,
	psy_dsp_beat_t offset)
{
	psy_dsp_beat_t retriggerstep;
	psy_dsp_beat_t retriggeroffset;
	
	if ((patternentry_front(entry)->parameter & 0xf0) != 0) {
		uint8_t retriggerrate; // x / 16 = row duration per trigger

		retriggerrate = patternentry_front(entry)->parameter & 0xf0;
		// convert retriggerrate to beat unit
		retriggerstep = retriggerrate *
			(psy_audio_sequencer_currbeatsperline(self) / 256.f);
	} else {
		// use current retriggerrate	
		retriggerstep = track->state.retriggerstep;		
	}
	retriggeroffset = track->state.retriggeroffset + retriggerstep;
	while (retriggeroffset < psy_audio_sequencer_currbeatsperline(self)) {
		psy_audio_PatternEntry* retriggerentry;

		retriggerentry = patternentry_clone(entry);
		patternentry_front(retriggerentry)->cmd = 0;
		patternentry_front(retriggerentry)->parameter = 0;
		retriggerentry->delta = entry->delta + retriggeroffset;
		psy_list_append(&self->delayedevents, retriggerentry);		
		retriggeroffset += retriggerstep;
	}
	track->state.retriggeroffset = retriggeroffset -
		psy_audio_sequencer_currbeatsperline(self);
	track->state.retriggerstep = retriggerstep;
}

void psy_audio_sequencer_insertdelayedevents(psy_audio_Sequencer* self)
{
	PatternNode* p;	
	
	p = self->delayedevents;
	while (p != NULL) {	
		psy_audio_PatternEntry* delayed;
		
		delayed = psy_audio_patternnode_entry(p);		
		if (psy_audio_sequencer_isoffsetinwindow(self, delayed->delta)) {
			delayed->delta -= self->position;
			psy_list_append(&self->events, delayed);
			p = psy_list_remove(&self->delayedevents, p);
		} else {
			psy_list_next(&p);
		}
	}	
}

void psy_audio_sequencer_insertinputevents(psy_audio_Sequencer* self)
{
	psy_List* p;
	psy_List* q;
	
	for (p = self->inputevents; p != NULL; p = q) {
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

void psy_audio_sequencer_compute_beatspersample(psy_audio_Sequencer* self)
{
	assert(self->samplerate != 0);
	self->beatspersample = (self->bpm * psy_audio_sequencer_speed(self)) /
		(self->samplerate * 60.0f);
}

psy_List* psy_audio_sequencer_timedevents(psy_audio_Sequencer* self, uintptr_t
	slot, uintptr_t amount)
{
	psy_List* rv = 0;
	psy_List* p;

	rv = psy_audio_sequencer_machinetickevents(self, slot);
	for (p = rv ; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* entry;
		psy_dsp_beat_t beatspersample;
		uintptr_t deltaframes;			

		entry = (psy_audio_PatternEntry*) p->entry;
		beatspersample = (entry->bpm * psy_audio_sequencer_speed(self)) /
			(self->samplerate * 60.0f);
		deltaframes = (uintptr_t)(entry->delta / self->beatspersample);
		if (deltaframes >= amount) {
			deltaframes = amount - 1;
		}
		entry->delta = (psy_dsp_beat_t)deltaframes;						
	}
	return rv;
}

void psy_audio_sequencer_addinputevent(psy_audio_Sequencer* self,
	const psy_audio_PatternEvent* ev, uintptr_t track)
{
	if (ev) {
		psy_list_append(&self->inputevents,
			patternentry_allocinit_all(ev, 0, 0, self->bpm, track));
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
			node = psy_audio_pattern_findnode(pattern, 0, quantizedpatternoffset,
				1.f / self->lpb, &prev);
			if (node) {					
				psy_audio_pattern_setevent(pattern, node, event);
			} else {
				psy_audio_pattern_insert(pattern, prev, 0, quantizedpatternoffset,
					event);
			}
		}
	}
}

void psy_audio_sequencer_checkiterators(psy_audio_Sequencer* self,
	const PatternNode* node)
{
	psy_List* p;

	for (p = self->currtracks; p != NULL; psy_list_next(&p)) {
		psy_audio_SequencerTrack* track;
		SequenceTrackIterator* it;

		track = (psy_audio_SequencerTrack*) p->entry;
		it = track->iterator;
		if (it->patternnode == node) {
			sequencetrackiterator_incentry(it);
		}
	}
}

uintptr_t psy_audio_sequencer_currframesperline(psy_audio_Sequencer* self)
{
	return psy_audio_sequencer_frames(self,
		psy_audio_sequencer_currbeatsperline(self));	
}

void psy_audio_sequencer_sortevents(psy_audio_Sequencer* self)
{	
	if (self->events && self->events != self->events->tail) {
		PatternNode** nodes;
		uintptr_t num;
		uintptr_t i;
		PatternNode* sorted = 0;
		PatternNode* p;		
			
		num = psy_list_size(self->events);		
		nodes = malloc(sizeof(PatternNode*) * num);
		for (i = 0, p = self->events; p != NULL && i < num; psy_list_next(&p), ++i) {
			nodes[i] = p;
		}
		psy_qsort(nodes, 0, num - 1, psy_audio_sequencer_comp_events);		
		for (i = 0; i < num; ++i) {			
			psy_list_append(&sorted, nodes[i]->entry);			
		}		
		psy_list_free(self->events);
		self->events = sorted;
		free(nodes);
	}
	// psy_audio_sequencer_assertorder(self);
}

int psy_audio_sequencer_comp_events(PatternNode* lhsnode, PatternNode* rhsnode)
{
	int rv;
	psy_audio_PatternEntry* lhs;
	psy_audio_PatternEntry* rhs;

	lhs = psy_audio_patternnode_entry(lhsnode);
	rhs = psy_audio_patternnode_entry(rhsnode);	
	if (lhs->delta == rhs->delta) {
		if (lhs->track < rhs->track) {
			rv = -1;
		} else {
			rv = 0;
		}
	} else
	if (lhs->delta < rhs->delta) {
		rv = -1;
	} else {
		rv = 1;
	}
	return rv;		
}	

// void psy_audio_sequencer_assertorder(psy_audio_Sequencer* self)
// {
//		psy_List* p;
//		psy_dsp_beat_t last = 0;
// 	
//		p = self->events;
// 		while (p) {
// 			psy_audio_PatternEntry* entry;
// 		
// 			entry = psy_audio_patternnode_entry(p);
// 			assert(entry->delta >= last);
// 			last = entry->delta;			
// 			psy_list_next(&p);
// 		}
// }

void psy_audio_sequencer_jumpto(psy_audio_Sequencer* self,
	psy_dsp_beat_t position)
{
	psy_audio_sequencerjump_activate(&self->jump, position);
}
