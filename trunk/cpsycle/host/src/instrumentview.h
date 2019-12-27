// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTRUMENTVIEW_H)
#define INSTRUMENTVIEW_H

#include "envelopeview.h"
#include "tabbar.h"
#include "instrumentsbox.h"
#include "instnotemapview.h"
#include "samplesbox.h"
#include "workspace.h"

#include <uinotebook.h>
#include <uilabel.h>
#include <uiedit.h>
#include <uibutton.h>
#include <uislider.h>
#include <uigroupbox.h>

typedef struct {	
	psy_ui_Component component;	
	psy_ui_Label namelabel;
	psy_ui_Edit nameedit;	
	psy_ui_Button prevbutton;
	psy_ui_Button nextbutton;	
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
	struct InstrumentView* view;
} InstrumentHeaderView;

typedef struct {
	psy_ui_Component component;
	psy_ui_Component row1;
	psy_ui_Button create;
	psy_ui_Button load;
	psy_ui_Button save;
	psy_ui_Component row2;
	psy_ui_Button duplicate;
	psy_ui_Button del;
} InstrumentViewButtons;

typedef struct {
	psy_ui_Component component;
	psy_ui_Component nna;
	psy_ui_Label nnaheaderlabel;
	psy_ui_Button nnacutbutton;
	psy_ui_Button nnareleasebutton;
	psy_ui_Button nnanonebutton;
	psy_ui_Component left;
	SamplesBox samplesbox;
	InstrumentNoteMapView notemapview;
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
} InstrumentGeneralView;

typedef struct {
	psy_ui_Component component;	
	psy_ui_Groupbox groupbox;
	EnvelopeView envelopeview;
	ui_slider attack;
	ui_slider decay;
	ui_slider sustain;
	ui_slider release;
	psy_audio_Player* player;
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
} InstrumentVolumeView;

typedef struct {
	psy_ui_Component component;		
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
} InstrumentPanView;

typedef struct {
	psy_ui_Component component;	
	psy_ui_Groupbox groupbox;
	EnvelopeView envelopeview;
	ui_slider attack;
	ui_slider decay;
	ui_slider sustain;
	ui_slider release;
	ui_slider cutoff;
	ui_slider res;
	ui_slider modamount;
	psy_audio_Player* player;
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
} InstrumentFilterView;

typedef struct {
	psy_ui_Component component;		
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
} InstrumentPitchView;

typedef struct InstrumentView {
	psy_ui_Component component;
	ui_notebook notebook;
	TabBar tabbar;
	psy_ui_Component left;
	psy_ui_Label label;
	InstrumentViewButtons buttons;
	InstrumentsBox instrumentsbox;
	psy_ui_Component client;
	InstrumentHeaderView header;
	InstrumentGeneralView general;
	InstrumentVolumeView volume;
	InstrumentPanView pan;
	InstrumentFilterView filter;
	InstrumentPitchView pitch;	
	psy_audio_Player* player;
	Workspace* workspace;
} InstrumentView;

void instrumentview_init(InstrumentView*, psy_ui_Component* parent, Workspace*);

#endif
