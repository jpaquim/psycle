// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(TIMEBAR_H)
#define TIMEBAR_H

#include "uilabel.h"
#include "uibutton.h"
#include "player.h"

typedef struct {
	ui_component component;
	ui_label bpmdesclabel;
	ui_label bpmlabel;
	ui_button lesslessbutton;
	ui_button lessbutton;
	ui_button morebutton;
	ui_button moremorebutton;
	Player* player;
	float bpm;
} TimeBar;


void InitTimeBar(TimeBar*, ui_component* parent,Player* player);

#endif