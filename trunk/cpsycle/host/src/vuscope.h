// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(VUSCOPE_H)
#define VUSCOPE_H

#include "uicomponent.h"
#include "workspace.h"

// aim: displays the Left (coloured blue) and Right (green) volumes in
//      decibels.

typedef struct {	
	psy_ui_Component component;
	psy_audio_Wire wire;
	psy_dsp_amp_t leftavg;
	psy_dsp_amp_t rightavg;
	float invol;
	float mult;
	int scope_peak_rate;
	int hold;
	int running;
	// memories for vu-meter
	psy_dsp_amp_t peakL, peakR;
	int peakLifeL, peakLifeR;
	Workspace* workspace;	
} VuScope;

void vuscope_init(VuScope*, psy_ui_Component* parent, psy_audio_Wire wire, Workspace*);
void vuscope_start(VuScope*);
void vuscope_stop(VuScope*);
void vuscope_disconnect(VuScope*);

#endif
