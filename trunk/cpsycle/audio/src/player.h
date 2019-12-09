// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLAYER_H)
#define PLAYER_H

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
	Driver* driver;		
	Song* song;
	Sequencer sequencer;	
	uintptr_t numsongtracks;
	Signal signal_numsongtrackschanged;
	Signal signal_lpbchanged;
	Signal signal_inputevent;
	Library drivermodule;
	EventDrivers eventdrivers;
	Table rms;	
	VUMeterMode vumode;
	int resetvumeters;
	int recordingnotes;
	int multichannelaudition;
	Table notestotracks;
	Table trackstonotes;	
} Player;

// init dispose
void player_init(Player*, Song*, void* systemhandle);
void player_dispose(Player*);
// general
void player_setsong(Player*, Song*);
void player_setnumsongtracks(Player*, uintptr_t numsongtracks);
uintptr_t player_numsongtracks(Player*);
void player_setvumetermode(Player*, VUMeterMode);
VUMeterMode player_vumetermode(Player*);
// sequencer
void player_start(Player*);
void player_stop(Player*);
int player_playing(Player*);
void player_setposition(Player*, beat_t offset);
beat_t player_position(Player*);
void player_setbpm(Player*, beat_t bpm);
beat_t player_bpm(Player*);
void player_setlpb(Player*, uintptr_t lpb);
uintptr_t player_lpb(Player*);
// audio driver
void player_loaddriver(Player*, const char* path);
void player_reloaddriver(Player*, const char* path);
void player_restartdriver(Player*);
// event recording
void player_startrecordingnotes(Player*);
void player_stoprecordingnotes(Player*);
int player_recordingnotes(Player*);
// event driver
void player_loadeventdriver(Player*, const char * path);
void player_addeventdriver(Player*, int id);
void player_removeeventdriver(Player*, int id);
void player_restarteventdriver(Player*, int id);
EventDriver* player_kbddriver(Player*);
EventDriver* player_eventdriver(Player*, int id);
unsigned int player_numeventdrivers(Player*);

#endif
