// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLAYPOSBAR_H)
#define PLAYPOSBAR_H

#include "uilabel.h"
#include "uibutton.h"
#include "player.h"

typedef struct {
	ui_component component;
	ui_label header;
	ui_label position;		
	beat_t lastposition;
	Player* player;
} PlayPosBar;

void playposbar_init(PlayPosBar*, ui_component* parent,Player* player);

#endif
