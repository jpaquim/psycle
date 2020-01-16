// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLEEDITOR_H)
#define SAMPLEEDITOR_H

#include "uibutton.h"
#include "scrollzoom.h"
#include "wavebox.h"
#include "workspace.h"

#include <sampler.h>
#include <sample.h>

typedef struct {
	psy_ui_Component component;
	psy_ui_Button loop;
	psy_ui_Button play;
	psy_ui_Button stop;
	psy_ui_Button pause;
	Workspace* workspace;
} SampleEditorPlayBar;

void sampleeditorplaybar_init(SampleEditorPlayBar*, psy_ui_Component* parent,
	Workspace*);

typedef struct {
	float samplewidth;	
	int visisteps;	
	int visiwidth;	
	psy_dsp_beat_t stepwidth;
} SampleEditorMetrics;

typedef struct {
	psy_ui_Component component;           	
	psy_dsp_beat_t bpl;
	int lpb;
	struct SampleEditor* view;   
	int scrollpos;	
} SampleEditorHeader;


typedef struct SampleEditor {	
	psy_ui_Component component;
	SampleEditorPlayBar playbar;
	SampleEditorHeader header;
	WaveBox samplebox;
	ScrollZoom zoom;
	SampleEditorMetrics metrics;
	psy_audio_Sample* sample;
	psy_audio_Sampler sampler;
	psy_audio_Buffer samplerbuffer;
	psy_audio_PatternEntry samplerentry;
	psy_List* samplerevents;
	Workspace* workspace;
} SampleEditor;

void sampleeditor_init(SampleEditor*, psy_ui_Component* parent, Workspace*);
void sampleeditor_setsample(SampleEditor*, psy_audio_Sample*);

#endif
