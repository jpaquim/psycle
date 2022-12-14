/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PLAYER_H
#define psy_audio_PLAYER_H

/* local */
#include "eventdrivers.h"
#include "library.h"
#include "machinefactory.h"
#include "midiinput.h"
#include "sequencer.h"
#include "song.h"
#include "sampler.h"
/* dsp */
#include <dither.h>
/* driver */
#include "../../driver/audiodriver.h"
/* container */
#include <signal.h>
#include "../../thread/src/lock.h"

#ifdef __cplusplus
extern "C" {
#endif

void psy_audio_init(void);
void psy_audio_dispose(void);

typedef enum {
	VUMETER_NONE,
	VUMETER_PEAK,
	VUMETER_RMS,	
} VUMeterMode;

typedef struct psy_audio_MachineWork {
	uintptr_t amount;
	uintptr_t slot;
} psy_audio_MachineWork;

typedef struct psy_audio_PatternDefaults {
	psy_audio_Pattern* pattern;
	psy_audio_Patterns patterns;
	psy_audio_Sequence sequence;
} psy_audio_PatternDefaults;

void psy_audio_patterndefaults_init(psy_audio_PatternDefaults*);
void psy_audio_patterndefaults_dispose(psy_audio_PatternDefaults*);

psy_audio_PatternEvent psy_audio_patterndefaults_event(const
	psy_audio_PatternDefaults*, uintptr_t);
psy_audio_PatternEvent psy_audio_patterndefaults_fill_event(const
	psy_audio_PatternDefaults*, uintptr_t track,
	psy_audio_PatternEvent src);

/* psy_audio_Player */
typedef struct psy_audio_Player {
	/* inherits */
	psy_audio_CustomMachine custommachine;
	/* signals */
	psy_Signal signal_song_changed;
	/* internal */
	psy_AudioDriver* driver;
	psy_audio_Song* song;	
	psy_audio_MachineFactory machinefactory;
	psy_audio_PluginCatcher plugincatcher;
	psy_audio_Song emptysong; /* dummy song while song switching */
	psy_audio_Sequencer sequencer;	
	psy_Signal signal_lpbchanged;
	psy_Signal signal_inputevent;
	psy_Signal signal_stop;
	psy_Signal signal_octavechanged;
	psy_Library drivermodule;
	psy_audio_EventDrivers eventdrivers;	
	VUMeterMode vumode;
	int recordingnotes;
	bool recordnoteoff;
	int multichannelaudition;
	uint8_t active_note;
	psy_Table notestotracks;
	psy_Table trackstonotes;
	psy_Table worked;
	psy_audio_PatternDefaults patterndefaults;	
	psy_dsp_Dither dither;	
	uint8_t octave;
	uintptr_t resyncplayposinsamples;
	psy_dsp_big_beat_t resyncplayposinbeats;
	psy_audio_MidiInput midiinput;
	psy_audio_ActiveChannels playon;
	bool measure_cpu_usage;
	psy_audio_Sampler sampler;
	uintptr_t thread_count;
	psy_List* threads_;
	bool stop_requested_;
	psy_List* nodes_queue_;
	psy_Lock mutex;
	psy_Lock block;
	uintptr_t waiting;
	/* parameters */
	psy_audio_CustomMachineParam tempo_param;
	psy_audio_CustomMachineParam lpb_param;
} psy_audio_Player;

void psy_audio_player_init(psy_audio_Player*, psy_audio_MachineCallback*,
	void* systemhandle);
void psy_audio_player_dispose(psy_audio_Player*);

void psy_audio_player_set_song(psy_audio_Player*, psy_audio_Song*);
INLINE psy_audio_Song* psy_audio_player_song(psy_audio_Player* self)
{
	return self->song;
}

INLINE psy_audio_MidiConfig* psy_audio_player_midiconfig(psy_audio_Player* self)
{
	return &self->midiinput.midiconfig;
}

void psy_audio_player_set_octave(psy_audio_Player*, uint8_t octave);

INLINE uint8_t psy_audio_player_octave(const psy_audio_Player* self)
{
	return self->octave;
}

void psy_audio_player_set_vu_meter_mode(psy_audio_Player*, VUMeterMode);
VUMeterMode psy_audio_player_vumetermode(psy_audio_Player*);
void psy_audio_player_enable_dither(psy_audio_Player*);
void psy_audio_player_disabledither(psy_audio_Player*);
void psy_audio_player_configure_dither(psy_audio_Player*, psy_dsp_DitherSettings);
psy_dsp_DitherSettings psy_audio_player_dither_configuration(const psy_audio_Player*);

/* sequencer */
void psy_audio_player_start(psy_audio_Player*);
void psy_audio_player_start_begin(psy_audio_Player*);
void psy_audio_player_start_currseqpos(psy_audio_Player*);
void psy_audio_player_stop(psy_audio_Player*);
void psy_audio_player_pause(psy_audio_Player*);
void psy_audio_player_resume(psy_audio_Player*);


INLINE int psy_audio_player_playing(psy_audio_Player* self)
{
	return psy_audio_sequencer_playing(&self->sequencer);
}

void psy_audio_player_setposition(psy_audio_Player*, psy_dsp_big_beat_t offset);

INLINE psy_dsp_big_beat_t psy_audio_player_position(psy_audio_Player* self)
{
	return psy_audio_sequencer_position(&self->sequencer);
}

INLINE uintptr_t psy_audio_player_playlist_position(const psy_audio_Player*
	self)
{
	return psy_audio_sequencer_playlist_position(&self->sequencer);
}

INLINE void psy_audio_player_setbpm(psy_audio_Player* self,
	psy_dsp_big_beat_t bpm)
{	
	psy_audio_sequencer_setbpm(&self->sequencer, bpm);
	if (self->song) {
		psy_audio_song_set_bpm(self->song, psy_audio_sequencer_bpm(
			&self->sequencer));
	}
}

INLINE psy_dsp_big_beat_t psy_audio_player_bpm(const psy_audio_Player* self)
{
	return psy_audio_sequencer_bpm(&self->sequencer);
}

psy_dsp_big_beat_t psy_audio_player_realbpm(const psy_audio_Player*);

INLINE void psy_audio_player_setticksperbeat(psy_audio_Player* self,
	uintptr_t ticks)
{
	psy_audio_sequencer_set_extra_ticks_per_beat(&self->sequencer, ticks);
	if (self->song) {
		self->song->properties.tpb = ticks;
	}
}

INLINE psy_dsp_big_beat_t psy_audio_player_ticksperbeat(psy_audio_Player* self)
{
	return (psy_dsp_big_beat_t)self->sequencer.tpb;
}

INLINE void psy_audio_player_setextraticksperbeat(psy_audio_Player* self,
	uintptr_t ticks)
{
	psy_audio_sequencer_set_extra_ticks_per_beat(&self->sequencer, ticks);
	if (self->song) {
		psy_audio_song_set_extra_ticks_per_beat(self->song, ticks);		
	}
}

INLINE psy_dsp_big_beat_t psy_audio_player_extraticksperbeat(
	psy_audio_Player* self)
{
	return (psy_dsp_big_beat_t)self->sequencer.extraticks;
}

INLINE void psy_audio_player_set_sampler_index(psy_audio_Player* self,
	uintptr_t sampler_index)
{
	self->sequencer.sample_event.mach = (uint8_t)sampler_index;
	if (self->song) {
		psy_audio_song_set_sampler_index(self->song, sampler_index);
	}
}

/* \returns lines per beat */
void psy_audio_player_set_lpb(psy_audio_Player*, uintptr_t lpb);

INLINE uintptr_t psy_audio_player_lpb(psy_audio_Player* self)
{
	return psy_audio_sequencer_lpb(&self->sequencer);
}

/* \returns beats per line */
INLINE psy_dsp_big_beat_t psy_audio_player_bpl(psy_audio_Player* self)
{
	return (psy_dsp_big_beat_t)(1.0) /
		psy_audio_sequencer_lpb(&self->sequencer);
}

INLINE psy_dsp_big_beat_t psy_audio_player_samplerate(psy_audio_Player* self)
{
	return psy_audio_sequencer_sample_rate(&self->sequencer);
}

INLINE psy_dsp_percent_t psy_audio_player_rowprogress(
	psy_audio_Player* self, uintptr_t track)
{
	return psy_audio_sequencer_row_progress(&self->sequencer, track);
}

/* cpu measure */
INLINE void psy_audio_player_measure_cpu_usage(psy_audio_Player* self)
{
	self->measure_cpu_usage = TRUE;
}

INLINE void psy_audio_player_stop_measure_cpu_usage(psy_audio_Player* self)
{
	self->measure_cpu_usage = FALSE;
}

INLINE bool psy_audio_player_measuring_cpu_usage(const psy_audio_Player* self)
{
	return self->measure_cpu_usage;
}

/* audio driver */
void psy_audio_player_setaudiodriver(psy_audio_Player*, psy_AudioDriver*);
psy_AudioDriver* psy_audio_player_audiodriver(psy_audio_Player*);
void psy_audio_player_loaddriver(psy_audio_Player*, const char* path,
	psy_Property* config, bool open);
void psy_audio_player_unloaddriver(psy_audio_Player*);
void psy_audio_player_reloaddriver(psy_audio_Player*, const char* path,
	psy_Property* config);
void psy_audio_player_restart_driver(psy_audio_Player*,
	const psy_Property* config);
/* event recording */
void psy_audio_player_startrecordingnotes(psy_audio_Player*);
void psy_audio_player_stoprecordingnotes(psy_audio_Player*);
int psy_audio_player_recordingnotes(psy_audio_Player*);
void psy_audio_player_inputpatternevent(psy_audio_Player*,
	const psy_audio_PatternEvent*);
void psy_audio_player_playevent(psy_audio_Player*,
	const psy_audio_PatternEvent* ev);
psy_audio_PatternEvent psy_audio_player_pattern_event(psy_audio_Player*,
	uint8_t note);

INLINE void psy_audio_player_recordnoteoff(psy_audio_Player* self)
{
	self->recordnoteoff = TRUE;
}

INLINE void psy_audio_player_preventrecordnoteoff(psy_audio_Player* self)
{ 
	self->recordnoteoff = FALSE;
}

INLINE bool psy_audio_player_recording_noteoff(const psy_audio_Player* self)
{
	return self->recordnoteoff;
}

INLINE psy_audio_SequencerTime* psy_audio_player_sequencertime(
	psy_audio_Player* self)
{
	return &self->sequencer.seqtime;
}

/* event driver */
psy_EventDriver* psy_audio_player_loadeventdriver(psy_audio_Player*,
	const char* path);
void psy_audio_player_remove_event_driver(psy_audio_Player*, intptr_t id);
void psy_audio_player_restart_event_driver(psy_audio_Player*, intptr_t id,
	psy_Property* configuration);
psy_EventDriver* psy_audio_player_kbddriver(psy_audio_Player*);
psy_EventDriver* psy_audio_player_eventdriver(psy_audio_Player*, intptr_t id);
uintptr_t psy_audio_player_numeventdrivers(psy_audio_Player*);
void psy_audio_player_write_eventdrivers(psy_audio_Player*,
	psy_EventDriverInput input);
void psy_audio_player_work_machine(psy_audio_Player*, uintptr_t amount,
	uintptr_t slot);
void psy_audio_player_setemptysong(psy_audio_Player*);
void psy_audio_player_midi_configure(psy_audio_Player*, psy_Property*
	configuration, bool datastr);
void psy_audio_player_idle(psy_audio_Player*);
void psy_audio_player_sendcmd(psy_audio_Player*, const char* section,
	psy_EventDriverCmd cmd);
/* metronome */
INLINE void psy_audio_player_activatemetronome(psy_audio_Player* self)
{
	self->sequencer.metronome.active = TRUE;
}

INLINE void psy_audio_player_deactivatemetronome(psy_audio_Player* self)
{
	self->sequencer.metronome.active = FALSE;
}

void psy_audio_player_start_threads(psy_audio_Player*, uintptr_t thread_count);
void psy_audio_player_stop_threads(psy_audio_Player*);

uintptr_t psy_audio_player_numthreads(const psy_audio_Player*);

bool psy_audio_player_is_active_key(const psy_audio_Player*, uint8_t key);

void psy_audio_player_enable_audio(psy_audio_Player*);
void psy_audio_player_disable_audio(psy_audio_Player*);
bool psy_audio_player_enabled(const psy_audio_Player*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PLAYER_H */
