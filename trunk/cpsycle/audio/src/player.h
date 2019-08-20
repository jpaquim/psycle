// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(PLAYER_H)
#define PLAYER_H

#include "driver.h"
#include "song.h"
#include "sequencer.h"

typedef struct {	
	Driver* driver;
	Song* song;
	Sequencer sequencer;
	float pos; 
	int playing;
	float t;
} Player;

void player_init(Player* player, Song*);
void player_dispose(Player* player);
void player_start(Player* player);
void player_stop(Player* player);
float player_position(Player* player);

#endif