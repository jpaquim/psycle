// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(OSCILLOSCOPE_H)
#define OSCILLOSCOPE_H

#include "uicomponent.h"
#include "workspace.h"
#include "uibutton.h"
#include "uilabel.h"
#include "zoombox.h"

#ifdef __cplusplus
extern "C" {
#endif

// aim: displays the Left and Right signals as a realtime waveform graph.

typedef enum {
	OSCILLOSCOPE_CHMODE_LEFT = 0,
	OSCILLOSCOPE_CHMODE_RIGHT = 1
} OscilloscopeChannelMode;

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
	uintptr_t scope_view_samples;
	uintptr_t scope_begin;
	psy_dsp_amp_t ampzoom;
	Workspace* workspace;
	psy_audio_Buffer* hold_buffer;
	uintptr_t hold_buffer_numsamples;
	OscilloscopeChannelMode channelmode;
	bool running;
} Oscilloscope;

void oscilloscope_init(Oscilloscope*, psy_ui_Component* parent,
	psy_audio_Wire wire, Workspace*);
void oscilloscope_stop(Oscilloscope*);
void oscilloscope_setzoom(Oscilloscope*, float rate);
void oscilloscope_setbegin(Oscilloscope*, float begin);
void oscilloscope_hold(Oscilloscope*);
void oscilloscope_continue(Oscilloscope*);
bool oscilloscope_stopped(Oscilloscope*);


typedef struct OscilloscopeControls {
	psy_ui_Component component;
	Oscilloscope* oscilloscope;
	psy_ui_Label channellbl;
	psy_ui_Button channelmode;
	psy_ui_Label amplbl;
	ZoomBox ampzoom;
} OscilloscopeControls;

void oscilloscopecontrols_init(OscilloscopeControls*, psy_ui_Component* parent,
	Oscilloscope* oscilloscope);

typedef struct OscilloscopeView {
	psy_ui_Component component;
	Oscilloscope oscilloscope;
	OscilloscopeControls oscilloscopecontrols;
} OscilloscopeView;

void oscilloscopeview_init(OscilloscopeView*, psy_ui_Component* parent,
	psy_audio_Wire wire, Workspace*);
void oscilloscopeview_start(OscilloscopeView*);
void oscilloscopeview_stop(OscilloscopeView*);

#ifdef __cplusplus
}
#endif

#endif /* Oscilloscope */
