// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLEEDITOR_H)
#define SAMPLEEDITOR_H

#include "uibutton.h"
#include "wavebox.h"
#include "workspace.h"

#include <sampler.h>
#include <sample.h>


typedef struct {
	ui_component component;
	ui_button loop;
	ui_button play;
	ui_button stop;
	ui_button pause;
	Workspace* workspace;
} SampleEditorPlayBar;

void sampleeditorplaybar_init(SampleEditorPlayBar*, ui_component* parent,
	Workspace*);

typedef struct {
	float samplewidth;	
	int visisteps;	
	int visiwidth;	
	beat_t stepwidth;	
} SampleEditorMetrics;

typedef struct {
	ui_component component;           	
	beat_t bpl;
	int lpb;
	struct SampleEditor* view;   
	int scrollpos;	
} SampleEditorHeader;

typedef struct {	
	ui_component component;	
	Sample* sample;
	float zoomleft;
	float zoomright;	
	int dragmode;
	int dragoffset;
	Signal signal_zoom;
} SampleZoom;

typedef struct SampleEditor {	
	ui_component component;
	SampleEditorPlayBar playbar;
	SampleEditorHeader header;
	WaveBox samplebox;
	SampleZoom zoom;
	SampleEditorMetrics metrics;
	Sample* sample;
	Sampler sampler;
	Buffer samplerbuffer;
	PatternEvent samplerevent;
	List* samplerevents;
	Workspace* workspace;
} SampleEditor;

void sampleeditor_init(SampleEditor*, ui_component* parent, Workspace*);
void sampleeditor_setsample(SampleEditor*, Sample*);

#endif
