// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(STEREOPHASE_H)
#define STEREOPHASE_H

#include "uicomponent.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

// Displays the stereo phase of the signal 

typedef struct {
	// inherits
	psy_ui_Component component;
	psy_audio_Wire wire;
	psy_dsp_amp_t leftavg;
	psy_dsp_amp_t rightavg;
	float invol;
	float mult;
	// float* pSamplesL;
	// float* pSamplesR;
	int scope_peak_rate;
	int scope_phase_rate;
	int hold;	
	// memories for vu-meter
	psy_dsp_amp_t peakL, peakR;
	int peakLifeL, peakLifeR;
	//Memories for phase
	float o_mvc, o_mvpc, o_mvl, o_mvdl, o_mvpl, o_mvdpl, o_mvr, o_mvdr, o_mvpr, o_mvdpr;
	Workspace* workspace;	
} StereoPhase;

void stereophase_init(StereoPhase*, psy_ui_Component* parent, psy_audio_Wire wire, Workspace*);
void stereophase_stop(StereoPhase*);

#ifdef __cplusplus
}
#endif

#endif /* STEREOPHASE_H */
