// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQUENCER_H)
#define SEQUENCER_H

#include "sequence.h"
#include "machines.h"
#include <list.h>

typedef enum {
	SEQUENCERPLAYMODE_PLAYALL,
	SEQUENCERPLAYMODE_PLAYSEL
} SequencerPlayMode;

typedef struct {
	beat_t retriggeroffset;
	beat_t retriggerstep;	
} SequencerTrackState;

typedef struct {
	SequencerTrackState state;
	SequenceTrackIterator* iterator;
} SequencerTrack;

typedef struct {
	int active;	
	beat_t offset;
} SequencerJump;

typedef struct {
	int active;
	beat_t rowspeed; // line delay
} SequenceRowDelay;

typedef struct {
	Sequence* sequence;
	Machines* machines;
	beat_t bpm;
	unsigned int samplerate;
	beat_t beatsprosample;	
	unsigned int lpb; // global
	beat_t lpbspeed; // pattern	
	int playing;
	beat_t position;	
	beat_t window;	
	List* currtracks;	
	List* events;	
	List* delayedevents;
	List* inputevents;	
	SequencerPlayMode mode;	
	int looping;
	beat_t linetickcount;
	SequencerJump jump;
	SequenceRowDelay rowdelay;
} Sequencer;

void sequencer_init(Sequencer*, Sequence*, Machines*);
void sequencer_dispose(Sequencer*);
void sequencer_reset(Sequencer*, Sequence*, Machines*);
void sequencer_frametick(Sequencer*, unsigned int numsamples);
void sequencer_tick(Sequencer*, beat_t offset);
void sequencer_linetick(Sequencer*);
void sequencer_setposition(Sequencer*, beat_t position);
beat_t sequencer_position(Sequencer*);
void sequencer_start(Sequencer*);
void sequencer_stop(Sequencer*);
List* sequencer_tickevents(Sequencer*);
List* sequencer_machinetickevents(Sequencer*, size_t slot);
List* sequencer_timedevents(Sequencer*, size_t slot, unsigned int amount);
void sequencer_append(Sequencer*, List* events);
void sequencer_setsamplerate(Sequencer*, unsigned int samplerate);
unsigned int sequencer_samplerate(Sequencer*);
void sequencer_setbpm(Sequencer*, beat_t bpm);
beat_t sequencer_bpm(Sequencer*);
void sequencer_setlpb(Sequencer*, uintptr_t lpb);
uintptr_t sequencer_lpb(Sequencer*);
unsigned int sequencer_frames(Sequencer*, beat_t offset);
beat_t sequencer_frametooffset(Sequencer*, int numsamples);
int sequencer_playing(Sequencer*);
void sequencer_addinputevent(Sequencer*, const PatternEvent*, uintptr_t track);
void sequencer_recordinputevent(Sequencer*, const PatternEvent*,
	unsigned int track, beat_t playposition);
void sequencer_setplaymode(Sequencer*, SequencerPlayMode);
void sequencer_loop(Sequencer*);
void sequencer_stoploop(Sequencer*);
int sequencer_looping(Sequencer*);
SequencerPlayMode sequencer_playmode(Sequencer*);

#endif
