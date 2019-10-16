// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLAYER_H)
#define PLAYER_H

#include "../../driver/driver.h"
#include "../../driver/eventdriver.h"
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
	EventDriver* eventdriver;
	Library* library;
} EventDriverEntry;

typedef struct {	
	Driver* driver;	
	EventDriver* kbddriver;
	Song* song;
	Sequencer sequencer;	
	unsigned int numsongtracks;
	Signal signal_numsongtrackschanged;
	Signal signal_lpbchanged;
	Signal signal_inputevent;
	Library drivermodule;	
	List* eventdrivers;	
	Table rms;	
	VUMeterMode vumode;
	int resetvumeters;
} Player;

void player_init(Player*, Song*, void* handle);
void player_dispose(Player*);
void player_start(Player*);
void player_stop(Player*);
int player_playing(Player*);
void player_setsong(Player*, Song*);
beat_t player_position(Player*);
void player_setbpm(Player*, beat_t bpm);
beat_t player_bpm(Player*);
void player_setlpb(Player*, unsigned int lpb);
unsigned int player_lpb(Player*);
void player_setnumsongtracks(Player*, unsigned int numsongtracks);
unsigned int player_numsongtracks(Player*);
void player_loaddriver(Player*, const char* path);
void player_loadeventdriver(Player * self, const char * path);
void player_addeventdriver(Player * self, int id);
void player_removeeventdriver(Player * self, int id);
void player_restartdriver(Player*);
void player_restarteventdriver(Player*, int id);
void player_reloaddriver(Player*, const char* path);
void player_setvumetermode(Player*, VUMeterMode);
VUMeterMode player_vumetermode(Player*);
EventDriver* player_kbddriver(Player*);
EventDriver* player_eventdriver(Player*, int id);
unsigned int player_numeventdrivers(Player*);

#endif