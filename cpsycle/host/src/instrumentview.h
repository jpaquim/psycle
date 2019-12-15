// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTRUMENTVIEW_H)
#define INSTRUMENTVIEW_H

#include "envelopeview.h"
#include "tabbar.h"
#include "instrumentsbox.h"
#include "samplesbox.h"
#include "workspace.h"

#include <uinotebook.h>
#include <uilabel.h>
#include <uiedit.h>
#include <uibutton.h>
#include <uislider.h>
#include <uigroupbox.h>

typedef struct {
	ui_component component;	
	ui_label namelabel;
	ui_edit nameedit;	
	ui_button prevbutton;
	ui_button nextbutton;	
	Instrument* instrument;
	Instruments* instruments;
} InstrumentHeaderView;

typedef struct {
	ui_component component;	
	int dy;
} InstrumentKeyboardView;

void instrumentkeyboardview_init(InstrumentKeyboardView*,
	ui_component* parent);

typedef struct {
	ui_component component;	
	int dy;
	Instrument* instrument;
} InstrumentEntryView;

void instrumententryview_init(InstrumentEntryView*,
	ui_component* parent);

typedef struct {
	ui_component component;
	ui_button load;
	ui_button save;
	ui_button duplicate;
	ui_button del;
} InstrumentViewButtons;

typedef struct {
	ui_component component;
	ui_component nna;
	ui_label nnaheaderlabel;
	ui_button nnacutbutton;
	ui_button nnareleasebutton;
	ui_button nnanonebutton;
	SamplesBox samplesbox;
	InstrumentKeyboardView keyboard;
	InstrumentEntryView entryview;
	Instrument* instrument;
	Instruments* instruments;
} InstrumentGeneralView;

typedef struct {
	ui_component component;	
	ui_groupbox groupbox;
	EnvelopeView envelopeview;
	ui_slider attack;
	ui_slider decay;
	ui_slider sustain;
	ui_slider release;
	Player* player;
	Instrument* instrument;
	Instruments* instruments;
} InstrumentVolumeView;

typedef struct {
	ui_component component;		
	Instrument* instrument;
	Instruments* instruments;
} InstrumentPanView;

typedef struct {
	ui_component component;	
	ui_groupbox groupbox;
	EnvelopeView envelopeview;
	ui_slider attack;
	ui_slider decay;
	ui_slider sustain;
	ui_slider release;
	ui_slider cutoff;
	ui_slider res;
	ui_slider modamount;
	Player* player;
	Instrument* instrument;
	Instruments* instruments;
} InstrumentFilterView;

typedef struct {
	ui_component component;		
	Instrument* instrument;
	Instruments* instruments;
} InstrumentPitchView;

typedef struct {
	ui_component component;
	ui_notebook notebook;
	TabBar tabbar;
	ui_component left;
	ui_label label;
	InstrumentViewButtons buttons;
	InstrumentsBox instrumentsbox;
	ui_component client;
	InstrumentHeaderView header;
	InstrumentGeneralView general;
	InstrumentVolumeView volume;
	InstrumentPanView pan;
	InstrumentFilterView filter;
	InstrumentPitchView pitch;	
	Player* player;
} InstrumentView;

void instrumentview_init(InstrumentView*, ui_component* parent, Workspace*);

#endif
