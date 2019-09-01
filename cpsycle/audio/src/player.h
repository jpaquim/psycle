// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(PLAYER_H)
#define PLAYER_H

#include "../../driver/driver.h"
#include "song.h"
#include "sequencer.h"

typedef struct {	
	Driver* driver;
	Driver* silentdriver;
	Song* song;
	Sequencer sequencer;
	float pos; 
	int playing;
	float t;
	unsigned int lpb;
	HMODULE module;
} Player;

void player_init(Player*, Song*, const char* driverpath);
void player_dispose(Player*);
void player_start(Player*);
void player_stop(Player*);
float player_position(Player*);
void player_setbpm(Player*, float bpm);
void player_setlpb(Player*, unsigned int lpb);

#endif