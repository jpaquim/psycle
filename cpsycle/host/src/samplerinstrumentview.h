// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLERINSTRUMENTVIEW_H)
#define SAMPLERINSTRUMENTVIEW_H

#include "tabbar.h"
#include "slidergroup.h"
#include "envelopeview.h"
#include <uinotebook.h>
#include <uilabel.h>
#include <uiedit.h>
#include <uibutton.h>
#include <uilistbox.h>
#include <uigroupbox.h>
#include "workspace.h"

typedef struct {
	ui_component component;	
	ui_label namelabel;
	ui_edit nameedit;	
	ui_button previnstrumentbutton;
	ui_button nextinstrumentbutton;
	ui_button deleteinstrumentbutton;
	Instrument* instrument;
	Instruments* instruments;
} SamplerInstrumentHeaderView;

typedef struct {
	ui_component component;		
	ui_label nnaheaderlabel;
	ui_button nnacutbutton;
	ui_button nnareleasebutton;
	ui_button nnanonebutton;
	Instrument* instrument;
	Instruments* instruments;
} SamplerInstrumentGeneralView;

typedef struct {
	ui_component component;	
	ui_groupbox groupbox;
	EnvelopeView envelopeview;
	SliderGroup attack;
	SliderGroup decay;
	SliderGroup sustain;
	SliderGroup release;
	Player* player;
	Instrument* instrument;
	Instruments* instruments;
} SamplerInstrumentVolumeView;

typedef struct {
	ui_component component;		
	Instrument* instrument;
	Instruments* instruments;
} SamplerInstrumentPanView;

typedef struct {
	ui_component component;	
	ui_groupbox groupbox;
	EnvelopeView envelopeview;
	SliderGroup attack;
	SliderGroup decay;
	SliderGroup sustain;
	SliderGroup release;
	SliderGroup cutoff;
	SliderGroup res;
	SliderGroup modamount;
	Player* player;
	Instrument* instrument;
	Instruments* instruments;
} SamplerInstrumentFilterView;

typedef struct {
	ui_component component;		
	Instrument* instrument;
	Instruments* instruments;
} SamplerInstrumentPitchView;

typedef struct {
	ui_component component;	
	ui_notebook notebook;
	TabBar tabbar;
	ui_listbox instrumentlist;	
	SamplerInstrumentHeaderView header;
	SamplerInstrumentGeneralView general;
	SamplerInstrumentVolumeView volume;
	SamplerInstrumentPanView pan;
	SamplerInstrumentFilterView filter;
	SamplerInstrumentPitchView pitch;
	Player* player;
} SamplerInstrumentView;

void InitSamplerInstrumentView(SamplerInstrumentView*, ui_component* parent, Workspace*);

#endif