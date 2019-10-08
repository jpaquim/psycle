// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLAYER_H)
#define PLAYER_H

#include "../../driver/driver.h"
#include "song.h"
#include "sequencer.h"
#include <signal.h>
#include "library.h"

typedef struct {	
	Driver* driver;
	Driver* silentdriver;
	Song* song;
	Sequencer sequencer;	
	int playing;
	beat_t t;
	unsigned int lpb;	
	unsigned int numsongtracks;
	Signal signal_numsongtrackschanged;
	Signal signal_lpbchanged;
	Library drivermodule;
	beat_t seqtickcount;
	Table rmsvol;
} Player;

void player_init(Player*, Song*, void* handle);
void player_dispose(Player*);
void player_start(Player*);
void player_stop(Player*);
void player_setsong(Player*, Song*);
beat_t player_position(Player*);
void player_setbpm(Player*, beat_t bpm);
beat_t player_bpm(Player*);
void player_setlpb(Player*, unsigned int lpb);
unsigned int player_lpb(Player*);
void player_setnumsongtracks(Player*, unsigned int numsongtracks);
unsigned int player_numsongtracks(Player*);
void player_loaddriver(Player*, const char* path);
void player_unloaddriver(Player*);
void player_restartdriver(Player*);
void player_reloaddriver(Player*, const char* path);

#endif