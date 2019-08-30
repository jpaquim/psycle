// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(VUMETER_H)
#define VUMETER_H

#include "uicomponent.h"
#include "player.h"

typedef struct {	
	ui_component component;
	Player* player;
	float leftavg;
	float rightavg;
} Vumeter;

void InitVumeter(Vumeter*, ui_component* parent, Player* player);

#endif
