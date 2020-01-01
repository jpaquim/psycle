// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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
	psy_dsp_beat_t retriggeroffset;
	psy_dsp_beat_t retriggerstep;
} SequencerTrackState;

typedef struct {
	SequencerTrackState state;
	SequenceTrackIterator* iterator;
} SequencerTrack;

typedef struct {
	int active;
	int count;
	psy_dsp_beat_t offset;	
} SequencerLoop;

typedef struct {
	int active;	
	psy_dsp_beat_t offset;
} SequencerJump;

typedef struct {
	int active;
	psy_dsp_beat_t rowspeed; // line delay
} SequenceRowDelay;

typedef struct {
	psy_audio_Sequence* sequence;
	psy_audio_Machines* machines;
	psy_dsp_beat_t bpm;
	unsigned int samplerate;
	psy_dsp_beat_t beatspersample;	
	unsigned int lpb; // global
	psy_dsp_beat_t lpbspeed; // pattern	
	int playing;
	psy_dsp_beat_t position;	
	psy_dsp_beat_t window;	
	psy_List* currtracks;	
	psy_List* events;	
	psy_List* delayedevents;
	psy_List* inputevents;	
	SequencerPlayMode mode;	
	int looping;	
	psy_dsp_beat_t linetickcount;
	SequencerJump jump;
	SequenceRowDelay rowdelay;
	SequencerLoop loop;
	psy_Table lastmachine;
} psy_audio_Sequencer;

void sequencer_init(psy_audio_Sequencer*, psy_audio_Sequence*, psy_audio_Machines*);
void sequencer_dispose(psy_audio_Sequencer*);
void sequencer_reset(psy_audio_Sequencer*, psy_audio_Sequence*, psy_audio_Machines*);
void sequencer_frametick(psy_audio_Sequencer*, uintptr_t numsamples);
void sequencer_tick(psy_audio_Sequencer*, psy_dsp_beat_t offset);
void sequencer_linetick(psy_audio_Sequencer*);
void sequencer_setposition(psy_audio_Sequencer*, psy_dsp_beat_t position);
psy_dsp_beat_t sequencer_position(psy_audio_Sequencer*);
void sequencer_start(psy_audio_Sequencer*);
void sequencer_stop(psy_audio_Sequencer*);
psy_List* sequencer_tickevents(psy_audio_Sequencer*);
psy_List* sequencer_machinetickevents(psy_audio_Sequencer*, uintptr_t slot);
psy_List* sequencer_timedevents(psy_audio_Sequencer*, uintptr_t slot,
	uintptr_t amount);
void sequencer_append(psy_audio_Sequencer*, psy_List* events);
void sequencer_setsamplerate(psy_audio_Sequencer*, unsigned int samplerate);
unsigned int sequencer_samplerate(psy_audio_Sequencer*);
void sequencer_setbpm(psy_audio_Sequencer*, psy_dsp_beat_t bpm);
psy_dsp_beat_t sequencer_bpm(psy_audio_Sequencer*);
void sequencer_setlpb(psy_audio_Sequencer*, uintptr_t lpb);
uintptr_t sequencer_lpb(psy_audio_Sequencer*);
unsigned int sequencer_frames(psy_audio_Sequencer*, psy_dsp_beat_t offset);
psy_dsp_beat_t sequencer_frametooffset(psy_audio_Sequencer*, int numsamples);
int sequencer_playing(psy_audio_Sequencer*);
void sequencer_addinputevent(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*, uintptr_t track);
void sequencer_recordinputevent(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*, uintptr_t track,
	psy_dsp_beat_t playposition);
void sequencer_setplaymode(psy_audio_Sequencer*, SequencerPlayMode);
void sequencer_loop(psy_audio_Sequencer*);
void sequencer_stoploop(psy_audio_Sequencer*);
int sequencer_looping(psy_audio_Sequencer*);
SequencerPlayMode sequencer_playmode(psy_audio_Sequencer*);
psy_dsp_beat_t sequencer_beatspersample(psy_audio_Sequencer*);

#endif
