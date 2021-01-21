// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PLAYER_H
#define psy_audio_PLAYER_H

#include "../../driver/audiodriver.h"
#include "eventdrivers.h"
#include "song.h"
#include "machinefactory.h"
#include "midiinput.h"
#include "sequencer.h"
#include <signal.h>
#include "library.h"

#include <dither.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	VUMETER_NONE,
	VUMETER_PEAK,
	VUMETER_RMS,	
} VUMeterMode;

typedef struct psy_audio_Player {
	psy_AudioDriver* driver;
	psy_audio_Song* song;
	// empty song for lock minimized
	// song switching	
	psy_audio_MachineFactory machinefactory;
	psy_audio_Song emptysong;
	psy_audio_Sequencer sequencer;
	psy_Signal signal_numsongtrackschanged;
	psy_Signal signal_lpbchanged;
	psy_Signal signal_inputevent;
	psy_Signal signal_stop;	
	psy_Library drivermodule;
	psy_audio_EventDrivers eventdrivers;	
	VUMeterMode vumode;
	int recordingnotes;
	bool recordnoteoff;
	int multichannelaudition;
	psy_Table notestotracks;
	psy_Table trackstonotes;
	psy_Table worked;
	psy_audio_Pattern patterndefaults;
	psy_dsp_Dither dither;
	bool dodither;
	uint8_t octave;
	uintptr_t resyncplayposinsamples;
	psy_dsp_big_beat_t resyncplayposinbeats;
	psy_audio_MidiInput midiinput;
	psy_audio_ActiveChannels playon;
	bool measure_cpu_usage;
} psy_audio_Player;

// init dispose
void psy_audio_player_init(psy_audio_Player*, psy_audio_Song*, void* systemhandle);
void psy_audio_player_dispose(psy_audio_Player*);
// general
void psy_audio_player_setsong(psy_audio_Player*, psy_audio_Song*);
INLINE psy_audio_Song* psy_audio_player_song(psy_audio_Player* self)
{
	return self->song;
}
void psy_audio_player_setnumsongtracks(psy_audio_Player*, uintptr_t numsongtracks);

INLINE uintptr_t psy_audio_player_numsongtracks(psy_audio_Player* self)
{
	return self->sequencer.numsongtracks;
}

INLINE psy_audio_MidiConfig* psy_audio_player_midiconfig(psy_audio_Player* self)
{
	return &self->midiinput.midiconfig;
}



void psy_audio_player_setoctave(psy_audio_Player*, uint8_t octave);

INLINE uintptr_t psy_audio_player_octave(psy_audio_Player* self)
{
	return self->octave;
}

void psy_audio_player_setvumetermode(psy_audio_Player*, VUMeterMode);
VUMeterMode psy_audio_player_vumetermode(psy_audio_Player*);
void psy_audio_player_enabledither(psy_audio_Player*);
void psy_audio_player_disabledither(psy_audio_Player*);
void psy_audio_player_setdither(psy_audio_Player*, uintptr_t depth,
	psy_dsp_DitherPdf, psy_dsp_DitherNoiseShape);

// sequencer
void psy_audio_player_start(psy_audio_Player*);
void psy_audio_player_stop(psy_audio_Player*);

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

INLINE void psy_audio_player_setbpm(psy_audio_Player* self, psy_dsp_big_beat_t bpm)
{	
	psy_audio_sequencer_setbpm(&self->sequencer, bpm);
	if (self->song) {
		psy_audio_song_setbpm(self->song, psy_audio_sequencer_bpm(
			&self->sequencer));
	}
}

INLINE psy_dsp_big_beat_t psy_audio_player_bpm(psy_audio_Player* self)
{
	return psy_audio_sequencer_bpm(&self->sequencer);
}

INLINE void psy_audio_player_setticksperbeat(psy_audio_Player* self, uintptr_t ticks)
{
	psy_audio_sequencer_setextraticksperbeat(&self->sequencer, ticks);
	if (self->song) {
		self->song->properties.tpb = ticks;
	}
}

INLINE psy_dsp_big_beat_t psy_audio_player_ticksperbeat(psy_audio_Player* self)
{
	return (psy_dsp_big_beat_t)self->sequencer.tpb;
}

INLINE void psy_audio_player_setextraticksperbeat(psy_audio_Player* self, uintptr_t ticks)
{
	psy_audio_sequencer_setextraticksperbeat(&self->sequencer, ticks);
	if (self->song) {
		self->song->properties.extraticksperbeat = ticks;
	}
}

INLINE psy_dsp_big_beat_t psy_audio_player_extraticksperbeat(psy_audio_Player* self)
{
	return (psy_dsp_big_beat_t)self->sequencer.extraticks;
}

///\returns lines per beat
void psy_audio_player_setlpb(psy_audio_Player*, uintptr_t lpb);

INLINE uintptr_t psy_audio_player_lpb(psy_audio_Player* self)
{
	return psy_audio_sequencer_lpb(&self->sequencer);
}

///\returns beats per line
INLINE psy_dsp_big_beat_t psy_audio_player_bpl(psy_audio_Player* self)
{
	return (psy_dsp_big_beat_t)(1.0) /
		psy_audio_sequencer_lpb(&self->sequencer);
}

INLINE psy_dsp_big_beat_t psy_audio_player_samplerate(psy_audio_Player* self)
{
	return psy_audio_sequencer_samplerate(&self->sequencer);
}

INLINE psy_dsp_percent_t psy_audio_player_playlist_rowprogress(psy_audio_Player* self)
{
	return psy_audio_sequencer_playlist_rowprogress(&self->sequencer);
}

// cpu measure
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

// audio driver
void psy_audio_player_setaudiodriver(psy_audio_Player*, psy_AudioDriver*);
psy_AudioDriver* psy_audio_player_audiodriver(psy_audio_Player*);
void psy_audio_player_loaddriver(psy_audio_Player*, const char* path, psy_Property* config, bool open);
void psy_audio_player_unloaddriver(psy_audio_Player*);
void psy_audio_player_reloaddriver(psy_audio_Player*, const char* path, psy_Property* config);
void psy_audio_player_restartdriver(psy_audio_Player*, psy_Property* config);
// event recording
void psy_audio_player_startrecordingnotes(psy_audio_Player*);
void psy_audio_player_stoprecordingnotes(psy_audio_Player*);
int psy_audio_player_recordingnotes(psy_audio_Player*);

INLINE void psy_audio_player_recordnoteoff(psy_audio_Player* self)
{
	self->recordnoteoff = TRUE;
}

INLINE void psy_audio_player_preventrecordnoteoff(psy_audio_Player* self)
{
	self->recordnoteoff = FALSE;
}

INLINE bool psy_audio_player_recordingnoteoff(const psy_audio_Player* self)
{
	return self->recordnoteoff;
}

INLINE psy_audio_SequencerTime* psy_audio_player_sequencertime(psy_audio_Player* self)
{
	return &self->sequencer.seqtime;
}

// event driver
psy_EventDriver* psy_audio_player_loadeventdriver(psy_audio_Player*, const char* path);
void psy_audio_player_removeeventdriver(psy_audio_Player*, intptr_t id);
void psy_audio_player_restarteventdriver(psy_audio_Player*, intptr_t id,
	psy_Property* configuration);
psy_EventDriver* psy_audio_player_kbddriver(psy_audio_Player*);
psy_EventDriver* psy_audio_player_eventdriver(psy_audio_Player*, intptr_t id);
uintptr_t psy_audio_player_numeventdrivers(psy_audio_Player*);
void psy_audio_player_write_eventdrivers(psy_audio_Player*, psy_EventDriverInput input);
void psy_audio_player_workmachine(psy_audio_Player*, uintptr_t amount,
	uintptr_t slot);
void psy_audio_player_setemptysong(psy_audio_Player*);
void psy_audio_player_midiconfigure(psy_audio_Player*, psy_Property*
	configuration, bool datastr);
void psy_audio_player_idle(psy_audio_Player*);
void psy_audio_player_sendcmd(psy_audio_Player*, const char* section,
	psy_EventDriverCmd cmd);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PLAYER_H */
