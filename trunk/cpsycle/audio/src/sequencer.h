// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SEQUENCER_H
#define psy_audio_SEQUENCER_H

// audio
#include "sequence.h"
#include "machines.h"
// container
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	psy_audio_SEQUENCERPLAYMODE_PLAYALL,
	psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS,
	psy_audio_SEQUENCERPLAYMODE_PLAYSEL
} psy_audio_SequencerPlayMode;

typedef struct {
	psy_dsp_big_beat_t retriggeroffset;
	psy_dsp_big_beat_t retriggerstep;
} psy_audio_SequencerTrackState;

typedef struct {
	psy_audio_SequencerTrackState state;
	psy_audio_SequenceTrackIterator* iterator;
	uintptr_t channeloffset;
} psy_audio_SequencerTrack;

typedef struct {
	bool active;
	int count;	
	psy_dsp_big_beat_t offset;
} psy_audio_SequencerLoop;

typedef struct {
	bool active;	
	psy_dsp_big_beat_t offset;
} psy_audio_SequencerJump;

typedef struct {
	bool active;
	psy_dsp_big_beat_t rowspeed; // line delay
} psy_audio_SequencerRowDelay;

typedef struct {
	psy_audio_Sequence* sequence;
	psy_audio_Machines* machines;
	psy_dsp_big_beat_t bpm;
	uintptr_t samplerate;
	psy_dsp_big_beat_t beatspersample;	
	unsigned int lpb; // global
	psy_dsp_big_beat_t lpbspeed; // pattern	
	bool playing;
	psy_dsp_big_beat_t position;
	psy_dsp_big_beat_t window;
	uintptr_t currrowposition;
	psy_List* currtracks;
	psy_audio_PatternNode* events;
	psy_audio_PatternNode* globalevents;
	psy_audio_PatternNode* delayedevents;
	psy_audio_PatternNode* inputevents;
	psy_audio_SequencerPlayMode mode;	
	bool looping;	
	psy_dsp_big_beat_t linetickcount;
	psy_audio_SequencerJump jump;
	psy_audio_SequencerRowDelay rowdelay;
	psy_audio_SequencerLoop loop;
	psy_Table lastmachine;
	psy_dsp_big_beat_t playbeatloopstart;
	psy_dsp_big_beat_t playbeatloopend;
	psy_dsp_big_beat_t numplaybeats;
	psy_Signal signal_newline;
	uintptr_t playcounter;
	uintptr_t ppqncounter;
	bool calcduration;	
} psy_audio_Sequencer;

void psy_audio_sequencer_init(psy_audio_Sequencer*, psy_audio_Sequence*,
	psy_audio_Machines*);
void psy_audio_sequencer_dispose(psy_audio_Sequencer*);
void psy_audio_sequencer_reset(psy_audio_Sequencer*, psy_audio_Sequence*,
	psy_audio_Machines*, uintptr_t samplerate);
void psy_audio_sequencer_frametick(psy_audio_Sequencer*,
	uintptr_t numsamples);
void psy_audio_sequencer_tick(psy_audio_Sequencer*,
	psy_dsp_big_beat_t offset);
uintptr_t psy_audio_sequencer_updatelinetickcount(psy_audio_Sequencer*,
	uintptr_t amount);
// clock
void psy_audio_sequencer_clockstart(psy_audio_Sequencer*);
void psy_audio_sequencer_clock(psy_audio_Sequencer*);
void psy_audio_sequencer_clockcontinue(psy_audio_Sequencer*);
void psy_audio_sequencer_clockstop(psy_audio_Sequencer*);

void psy_audio_sequencer_onnewline(psy_audio_Sequencer*);
void psy_audio_sequencer_setposition(psy_audio_Sequencer*,
	psy_dsp_big_beat_t position);
INLINE psy_dsp_big_beat_t psy_audio_sequencer_position(psy_audio_Sequencer*
	self)
{
	return self->position;
}

INLINE uintptr_t psy_audio_sequencer_playlist_position(const psy_audio_Sequencer*
	self)
{
	return self->currrowposition;
}

void psy_audio_sequencer_start(psy_audio_Sequencer*);

INLINE void psy_audio_sequencer_stop(psy_audio_Sequencer* self)
{
	self->playing = FALSE;
}

void psy_audio_sequencer_setnumplaybeats(psy_audio_Sequencer*,
	psy_dsp_big_beat_t);

INLINE psy_List* psy_audio_sequencer_tickevents(psy_audio_Sequencer* self)
{
	return self->events;
}

psy_List* psy_audio_sequencer_machinetickevents(psy_audio_Sequencer*,
	uintptr_t slot);
psy_List* psy_audio_sequencer_timedevents(psy_audio_Sequencer*,
	uintptr_t slot, uintptr_t amount);
void psy_audio_sequencer_append(psy_audio_Sequencer*, psy_List* events);
void psy_audio_sequencer_setsamplerate(psy_audio_Sequencer*,
	uintptr_t samplerate);

INLINE uintptr_t psy_audio_sequencer_samplerate(psy_audio_Sequencer* self)
{
	return self->samplerate;
}
void psy_audio_sequencer_setbpm(psy_audio_Sequencer*, psy_dsp_big_beat_t bpm);

INLINE psy_dsp_big_beat_t psy_audio_sequencer_bpm(psy_audio_Sequencer* self)
{
	return self->bpm;
}

void psy_audio_sequencer_setlpb(psy_audio_Sequencer*, uintptr_t lpb);

INLINE uintptr_t psy_audio_sequencer_lpb(psy_audio_Sequencer* self)
{
	return self->lpb;
}

INLINE uintptr_t psy_audio_sequencer_frames(psy_audio_Sequencer* self,
	psy_dsp_big_beat_t offset)
{
	return (uintptr_t)(offset / self->beatspersample);
}

INLINE psy_dsp_big_beat_t psy_audio_sequencer_frametooffset(
	psy_audio_Sequencer* self, uintptr_t numsamples)
{
	return numsamples * self->beatspersample;
}

INLINE int psy_audio_sequencer_playing(psy_audio_Sequencer* self)
{
	return self->playing;
}

void psy_audio_sequencer_addinputevent(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*, uintptr_t track);
void psy_audio_sequencer_recordinputevent(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*, uintptr_t track, psy_dsp_big_beat_t
	playposition);

INLINE void psy_audio_sequencer_setplaymode(psy_audio_Sequencer* self,
	psy_audio_SequencerPlayMode mode)
{
	self->mode = mode;
}

INLINE void psy_audio_sequencer_loop(psy_audio_Sequencer* self)
{
	self->looping = TRUE;
}

INLINE void psy_audio_sequencer_stoploop(psy_audio_Sequencer* self)
{
	self->looping = FALSE;
}

INLINE bool psy_audio_sequencer_looping(psy_audio_Sequencer* self)
{
	return self->looping != FALSE;
}

INLINE psy_audio_SequencerPlayMode psy_audio_sequencer_playmode(
	psy_audio_Sequencer* self)
{
	return self->mode;
}

INLINE psy_dsp_big_beat_t psy_audio_sequencer_speed(psy_audio_Sequencer* self)
{
	psy_dsp_big_beat_t rv;

	rv = self->lpbspeed;
	if (self->rowdelay.active) {
		rv *= self->rowdelay.rowspeed;
	}
	return rv;
}

INLINE psy_dsp_big_beat_t psy_audio_sequencer_beatspersample(
	psy_audio_Sequencer* self)
{
	return self->beatspersample;
}

INLINE psy_dsp_big_beat_t psy_audio_sequencer_currbeatsperline(
	psy_audio_Sequencer* self)
{
	return 1.0 / (self->lpb * psy_audio_sequencer_speed(self));
}

void psy_audio_sequencer_checkiterators(psy_audio_Sequencer*,
	const psy_audio_PatternNode*);

// elapsed playtime in seconds
INLINE psy_dsp_seconds_t psy_audio_sequencer_currplaytime(
	psy_audio_Sequencer* self)
{
	return self->playcounter / (psy_dsp_seconds_t)self->samplerate;
}


psy_dsp_percent_t psy_audio_sequencer_playlist_rowprogress(psy_audio_Sequencer* self);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SEQUENCER_H */
