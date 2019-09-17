// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MAINFRAME_H)
#define MAINFRAME_H

#include "workspace.h"
#include <uibutton.h>
#include <uinotebook.h>
#include "uistatusbar.h"
#include "machinebar.h"
#include "machineview.h"
#include "patternview.h"
#include "sequenceview.h"
#include "samplesview.h"
#include "instrumentsview.h"
#include "settingsview.h"
#include "songproperties.h"
#include "properties.h"
#include "noteinputs.h"
#include "tabbar.h"
#include "playbar.h"
#include "vumeter.h"
#include "volslider.h"
#include "clipbox.h"
#include "timebar.h"
#include "linesperbeatbar.h"
#include "octavebar.h"
#include "songtrackbar.h"
#include "greet.h"

typedef struct {
	ui_component component;
	ui_button newsongbutton;
	ui_button loadsongbutton;
	ui_component splitbar;
	TabBar tabbar;	
	ui_notebook notebook;
	MachineBar machinebar;
	PlayBar playbar;
	SequenceView sequenceview;
	MachineView machineview;
	PatternView patternview;		
	SamplesView samplesview;
	InstrumentsView instrumentsview;
	SongProperties songproperties;
	SettingsView settingsview;
	Greet greet;
	Vumeter vumeter;
	VolSlider volslider;
	ClipBox clipbox;
	SongTrackBar songtrackbar;
	TimeBar timebar;
	LinesPerBeatBar linesperbeatbar;
	OctaveBar octavebar;	
	ui_statusbar statusbar;
	ui_component* activeview;
	Player* player;
	NoteInputs noteinputs;	
	Workspace workspace;
	int firstshow;
	int toolbarheight;
	int tabbarheight;
	int resize;
} MainFrame;

void InitMainFrame(MainFrame* self);

#endif
