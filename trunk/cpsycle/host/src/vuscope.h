// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(VUSCOPE_H)
#define VUSCOPE_H

// host
#include "workspace.h"
// ui
#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

// VuScope
//
// Display a blue left and green right volumebar in decibels.

typedef struct VuScope {
	// inherits
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

void vuscope_idle(VuScope*);

INLINE psy_ui_Component* vuscope_base(VuScope* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* VUSCOPE_H */
