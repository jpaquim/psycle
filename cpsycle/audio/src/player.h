// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_AUDIO_PLAYER_H)
#define PSY_AUDIO_PLAYER_H

#include "../../driver/driver.h"
#include "eventdrivers.h"
#include "song.h"
#include "sequencer.h"
#include <signal.h>
#include "library.h"

typedef enum {
	VUMETER_NONE,
	VUMETER_PEAK,
	VUMETER_RMS,	
} VUMeterMode;

typedef struct {	
	psy_AudioDriver* driver;
	psy_audio_Song* song;
	psy_audio_Sequencer sequencer;	
	uintptr_t numsongtracks;
	psy_Signal signal_numsongtrackschanged;
	psy_Signal signal_lpbchanged;
	psy_Signal signal_inputevent;
	psy_Signal signal_stop;	
	psy_Library drivermodule;
	EventDrivers eventdrivers;
	psy_Table rms;	
	VUMeterMode vumode;
	int resetvumeters;
	int recordingnotes;
	int multichannelaudition;	
	psy_Table notestotracks;
	psy_Table trackstonotes;
	psy_audio_Pattern patterndefaults;
} psy_audio_Player;

// init dispose
void player_init(psy_audio_Player*, psy_audio_Song*, void* systemhandle);
void player_dispose(psy_audio_Player*);
// general
void player_setsong(psy_audio_Player*, psy_audio_Song*);
void player_setnumsongtracks(psy_audio_Player*, uintptr_t numsongtracks);
uintptr_t player_numsongtracks(psy_audio_Player*);
void player_setvumetermode(psy_audio_Player*, VUMeterMode);
VUMeterMode player_vumetermode(psy_audio_Player*);
// sequencer
void player_start(psy_audio_Player*);
void player_stop(psy_audio_Player*);
int player_playing(psy_audio_Player*);
void player_setposition(psy_audio_Player*, psy_dsp_beat_t offset);
psy_dsp_beat_t player_position(psy_audio_Player*);
void player_setbpm(psy_audio_Player*, psy_dsp_beat_t bpm);
psy_dsp_beat_t player_bpm(psy_audio_Player*);
void player_setlpb(psy_audio_Player*, uintptr_t lpb);
uintptr_t player_lpb(psy_audio_Player*);
// audio driver
void player_setaudiodriver(psy_audio_Player*, psy_AudioDriver* driver);
psy_AudioDriver* player_audiodriver(psy_audio_Player*);
void player_loaddriver(psy_audio_Player*, const char* path, psy_Properties* config);
void player_reloaddriver(psy_audio_Player*, const char* path, psy_Properties* config);
void player_restartdriver(psy_audio_Player*, psy_Properties* config);
// event recording
void player_startrecordingnotes(psy_audio_Player*);
void player_stoprecordingnotes(psy_audio_Player*);
int player_recordingnotes(psy_audio_Player*);
// event driver
void player_loadeventdriver(psy_audio_Player*, const char * path);
void player_addeventdriver(psy_audio_Player*, int id);
void player_removeeventdriver(psy_audio_Player*, int id);
void player_restarteventdriver(psy_audio_Player*, int id);
psy_EventDriver* player_kbddriver(psy_audio_Player*);
psy_EventDriver* player_eventdriver(psy_audio_Player*, int id);
unsigned int player_numeventdrivers(psy_audio_Player*);

#endif
