// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(TIMEBAR_H)
#define TIMEBAR_H

#include <uibutton.h>
#include <uilabel.h>
#include <player.h>

typedef struct {
	ui_component component;
	ui_label bpmdesc;
	ui_label bpmlabel;
	ui_button lessless;
	ui_button less;
	ui_button more;
	ui_button moremore;
	Player* player;
	psy_dsp_beat_t bpm;
} TimeBar;

void timerbar_init(TimeBar*, ui_component* parent, Player*);

#endif
