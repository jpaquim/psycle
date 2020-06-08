// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(TIMEBAR_H)
#define TIMEBAR_H

#include <uibutton.h>
#include <uilabel.h>
#include <player.h>

// aim: Sets the speed of the track in beats per minute, ranging from 33 to
//      999 BPM

typedef struct {
	psy_ui_Component component;
	psy_ui_Label bpmdesc;
	psy_ui_Label bpmlabel;
	psy_ui_Button lessless;
	psy_ui_Button less;
	psy_ui_Button more;
	psy_ui_Button moremore;
	psy_audio_Player* player;
	psy_dsp_beat_t bpm;
} TimeBar;

void timerbar_init(TimeBar*, psy_ui_Component* parent, psy_audio_Player*);

#endif
