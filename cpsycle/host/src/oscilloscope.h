// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(OSCILLOSCOPE_H)
#define OSCILLOSCOPE_H

#include "uicomponent.h"
#include "workspace.h"

typedef struct {	
	psy_ui_Component component;
	psy_audio_Wire wire;
	psy_dsp_amp_t invol;
	float mult;
	int scope_peak_rate;
	bool hold;	
	// memories for vu-meter
	psy_dsp_amp_t peakL, peakR;
	int peakLifeL, peakLifeR;
	uintptr_t scope_spec_samples;
	uintptr_t scope_spec_begin;
	Workspace* workspace;
	psy_audio_Buffer* hold_buffer;
	uintptr_t hold_buffer_numsamples;
} Oscilloscope;

void oscilloscope_init(Oscilloscope*, psy_ui_Component* parent, psy_audio_Wire wire, Workspace*);
void oscilloscope_stop(Oscilloscope*);
void oscilloscope_setzoom(Oscilloscope*, float rate);
void oscilloscope_setspecbegin(Oscilloscope*, float begin);
void oscilloscope_hold(Oscilloscope*);
void oscilloscope_continue(Oscilloscope*);
bool oscilloscope_stopped(Oscilloscope*);

#endif /* Oscilloscope */
