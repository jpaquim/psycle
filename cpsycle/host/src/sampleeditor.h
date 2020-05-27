// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLEEDITOR_H)
#define SAMPLEEDITOR_H

#include <uibutton.h>
#include <uicheckbox.h>
#include <uiedit.h>
#include <uilabel.h>
#include <uislider.h>
#include <uinotebook.h>
#include "scrollzoom.h"
#include "wavebox.h"
#include "workspace.h"

#include <uilistbox.h>

#include <sampler.h>
#include <sample.h>

struct SampleEditor;

typedef struct {
	psy_ui_Component component;
	psy_ui_CheckBox selecttogether;
	psy_ui_CheckBox doublecontloop;
	psy_ui_CheckBox doublesustainloop;
	psy_ui_Label selstartlabel;
	psy_ui_Edit selstartedit;
	psy_ui_Label selendlabel;
	psy_ui_Edit selendedit;
	struct SampleEditor* editor;
	Workspace* workspace;
} SampleEditorBar;

void  sampleeditorbar_init(SampleEditorBar*, psy_ui_Component* parent,
	struct SampleEditor* editor,
	Workspace*);
void sampleeditorbar_setselection(SampleEditorBar* self, uintptr_t selectionstart,
	uintptr_t selectionend);
void sampleeditorbar_clearselection(SampleEditorBar* self);

typedef struct {
	psy_ui_Component component;
	psy_ui_Button cut;
	psy_ui_Button crop;
	psy_ui_Button copy;
	psy_ui_Button paste;
	psy_ui_Button del;
	Workspace* workspace;
} SampleEditorOperations;

void sampleeditoroperations_init(SampleEditorOperations*, psy_ui_Component* parent, Workspace*);

typedef struct {
	psy_ui_Component component;
	psy_ui_Label header;
	psy_ui_Slider gain;
	psy_dsp_amp_t gainvalue;
	psy_ui_Label dbdisplay;
	Workspace* workspace;
} SampleEditorAmplify;

void sampleeditoramplify_init(SampleEditorAmplify*, psy_ui_Component* parent,
	Workspace* workspace);

typedef struct {
	psy_ui_Component component;
	SampleEditorOperations copypaste;
	psy_ui_Button process;
	psy_ui_ListBox processors;
	Workspace* workspace;
	psy_ui_Notebook notebook;
	psy_ui_Component emptypage1;
	psy_ui_Component emptypage2;
	psy_ui_Component emptypage3;
	psy_ui_Component emptypage4;
	psy_ui_Component emptypage5;
	psy_ui_Component emptypage6;
	SampleEditorAmplify amplify;
} SampleEditorProcessView;

void sampleprocessview_init(SampleEditorProcessView*, psy_ui_Component* parent,
	Workspace* workspace);

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
	SampleEditorProcessView processview;
	psy_ui_Component samplebox;
	psy_Table waveboxes;
	ScrollZoom zoom;
	SampleEditorMetrics metrics;
	psy_audio_Sample* sample;
	psy_audio_Sampler sampler;
	psy_audio_Buffer samplerbuffer;
	psy_audio_PatternEntry samplerentry;
	psy_List* samplerevents;
	Workspace* workspace;
	SampleEditorBar sampleeditortbar;
	psy_Signal signal_samplemodified;
	bool showdoubleloop;
} SampleEditor;

void sampleeditor_init(SampleEditor*, psy_ui_Component* parent, Workspace*);
void sampleeditor_setsample(SampleEditor*, psy_audio_Sample*);
void sampleeditor_showdoubleloop(SampleEditor*);
void sampleeditor_showsingleloop(SampleEditor*);

#endif
