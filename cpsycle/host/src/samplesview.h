// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLESVIEW_H)
#define SAMPLESVIEW_H

#include <uinotebook.h>
#include <uibutton.h>
#include <uicombobox.h>
#include <uiedit.h>
#include <uilabel.h>
#include "samplesbox.h"
#include "sampleeditor.h"
#include <uislider.h>
#include "wavebox.h"
#include "tabbar.h"
#include "notestab.h"
#include "workspace.h"

typedef struct {
	ui_component component;
	struct SamplesView* view;
	Sample* sample;
	Instruments* instruments;	
	ui_label namelabel;
	ui_edit nameedit;
	ui_button prevbutton;
	ui_button nextbutton;
	ui_label srlabel;
	ui_edit sredit;
	ui_label numsamplesheaderlabel;
	ui_label numsampleslabel;
	ui_label channellabel;	
} SamplesHeaderView;

typedef struct {
	ui_component component;
	Sample* sample;
	psy_dsp_NotesTabMode notestabmode;
	ui_slider defaultvolume;
	ui_slider globalvolume;
	ui_slider panposition;
	ui_slider samplednote; 
	ui_slider pitchfinetune;
} SamplesGeneralView;

typedef struct {
	ui_component component;
	Sample* sample;	
	ui_label waveformheaderlabel;
	ui_combobox waveformbox;
	ui_slider attack;
	ui_slider speed;
	ui_slider depth;
	Player* player;
} SamplesVibratoView;

typedef struct {
	ui_component component;
	struct SamplesView* view;
	Sample* sample;	
	ui_component cont;
	ui_label loopheaderlabel;
	ui_combobox loopdir;
	ui_label loopstartlabel;
	ui_edit loopstartedit;
	ui_label loopendlabel;
	ui_edit loopendedit;
	ui_component sustain;
	ui_label sustainloopheaderlabel;
	ui_combobox sustainloopdir;
	ui_label sustainloopstartlabel;
	ui_edit sustainloopstartedit;
	ui_label sustainloopendlabel;
	ui_edit sustainloopendedit;		
} SamplesWaveLoopView;

typedef struct {
	ui_component component;
	ui_button loadbutton;
	ui_button savebutton;
	ui_button duplicatebutton;
	ui_button deletebutton;
} SamplesViewButtons;

void samplesviewbuttons_init(SamplesViewButtons*, ui_component* parent);

typedef struct {
	ui_component component;
	ui_component header;
	ui_label label;
	ui_label songname;
	ui_button browse;
	SamplesBox samplesbox;
	ui_component bar;
	ui_button add;
	Song* source;
	struct SamplesView* view;
	Workspace* workspace;
} SamplesSongImportView;

typedef struct SamplesView {
	ui_component component;	
	TabBar clienttabbar;
	ui_notebook clientnotebook;
	ui_component mainview;
	ui_component importview;
	ui_notebook notebook;
	SamplesBox samplesbox;
	ui_component left;
	SamplesViewButtons buttons;	
	ui_component client;
	SamplesSongImportView songimport;
	SampleEditor sampleeditor;
	SamplesHeaderView header;
	TabBar tabbar;
	SamplesGeneralView general;
	SamplesVibratoView vibrato;
	ui_component loop;
	SamplesWaveLoopView waveloop;	
	WaveBox wavebox;	
	Workspace* workspace;
} SamplesView;

void samplesview_init(SamplesView*, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace);

#endif
