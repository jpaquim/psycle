// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(TIMEBAR_H)
#define TIMEBAR_H

// host
#include "workspace.h"
// audio
#include <player.h>
// ui
#include <uibutton.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

// TimeBar
//
// Sets the speed of the player in beats per minute, ranging from 33 to 999 BPM
// If you want to increase or decrease the BPM by 10, you can press the CTRL
// key while you click the button
// The older two button version can be set with #define PSYCLE_TIMEBAR_OLD
// see details/psyconf.h

typedef struct TimeBar {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Label desc;
	psy_ui_Label bpmlabel;
	psy_ui_Button lessless;
	psy_ui_Button less;
	psy_ui_Button more;
	psy_ui_Button moremore;
	// data members	
	psy_dsp_big_beat_t bpm;
	psy_dsp_big_beat_t realbpm;
	// references
	psy_audio_Player* player;
} TimeBar;

void timebar_init(TimeBar*, psy_ui_Component* parent, psy_audio_Player*);

INLINE psy_ui_Component* timebar_base(TimeBar* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* TIMEBAR_H */
