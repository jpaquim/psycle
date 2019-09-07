// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLESVIEW_H)
#define SAMPLESVIEW_H

#include <player.h>
#include <uibutton.h>
#include <uicombobox.h>
#include <uiedit.h>
#include <uilabel.h>
#include <uilistbox.h>
#include <uislider.h>
#include "wavebox.h"
#include "slidergroup.h"
#include "tabbar.h"

typedef struct {
	ui_component component;
	Sample* sample;
	Instruments* instruments;
	ui_listbox* samplelist;
	ui_label namelabel;
	ui_edit nameedit;
	ui_button prevbutton;
	ui_button nextbutton;
	ui_button deletebutton;
	ui_label srlabel;
	ui_edit sredit;
	ui_label numsamplesheaderlabel;
	ui_label numsampleslabel;
	ui_label channellabel;	
} SamplesHeaderView;

typedef struct {
	ui_component component;
	Sample* sample;	
	SliderGroup defaultvolume;
	SliderGroup globalvolume;
	SliderGroup panposition;
	SliderGroup samplednote; 
	SliderGroup pitchfinetune;	
} SamplesGeneralView;

typedef struct {
	ui_component component;
	Sample* sample;	
	ui_label waveformheaderlabel;
	ui_combobox waveformbox;
	SliderGroup attack;
	SliderGroup speed;
	SliderGroup depth;
	Player* player;
} SamplesVibratoView;

typedef struct {
	ui_component component;
	Sample* sample;	
	ui_label loopheaderlabel;
	ui_combobox loopdir;
	ui_label loopstartlabel;
	ui_edit loopstartedit;
	ui_label loopendlabel;
	ui_edit loopendedit;
	ui_label sustainloopheaderlabel;
	ui_combobox sustainloopdir;
	ui_label sustainloopstartlabel;
	ui_edit sustainloopstartedit;
	ui_label sustainloopendlabel;
	ui_edit sustainloopendedit;		
} SamplesWaveLoopView;

typedef struct {
	ui_component component;	
	ui_listbox samplelist;
	ui_button loadbutton;
	ui_button savebutton;
	ui_button duplicatebutton;
	ui_button deletebutton;	
	ui_component client;
	SamplesHeaderView header;
	TabBar tabbar;
	SamplesGeneralView general;
	SamplesVibratoView vibrato;
	SamplesWaveLoopView waveloop;
	ui_button waveeditorbutton;
	WaveBox wavebox;
	Player* player;
} SamplesView;

void InitSamplesView(SamplesView*, ui_component* parent, Player* player);

#endif