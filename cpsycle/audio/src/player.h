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
	float pos; 
	int playing;
	float t;
	unsigned int lpb;	
	unsigned int numsongtracks;
	Signal signal_numsongtrackschanged;
	Signal signal_lpbchanged;
	Library drivermodule;
	float volume;
} Player;

void player_init(Player*, Song*, const char* driverpath);
void player_dispose(Player*);
void player_start(Player*);
void player_stop(Player*);
void player_setsong(Player*, Song*);
float player_position(Player*);
void player_setbpm(Player*, float bpm);
void player_setlpb(Player*, unsigned int lpb);
void player_initmaster(Player* self);
void player_setnumsongtracks(Player* self, unsigned int numsongtracks);
unsigned int player_numsongtracks(Player* self);
void player_setvolume(Player*, float volume);
float player_volume(Player*);

#endif