// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MAINFRAME_H)
#define MAINFRAME_H

#include "workspace.h"
#include <uilabel.h>
#include <uibutton.h>
#include <uiframe.h>
#include <uinotebook.h>
#include <uiprogressbar.h>
#include "uistatusbar.h"
#include "filebar.h"
#include "undoredobar.h"
#include "navigation.h"
#include "machinebar.h"
#include "machineview.h"
#include "gear.h"
#include "patternview.h"
#include "sequenceview.h"
#include "samplesview.h"
#include "instrumentsview.h"
#include "settingsview.h"
#include "songproperties.h"
#include "renderview.h"
#include "properties.h"
#include "tabbar.h"
#include "playbar.h"
#include "playposbar.h"
#include "vubar.h"
#include "songbar.h"
#include "helpview.h"
#include "stepbox.h"

typedef struct {
	ui_component component;
	ui_component top;	
	ui_component toprow0;
	ui_component toprow1;
	ui_component toprow2;
	ui_component tabbars;
	ui_component client;	
	// ui_button updatedriver;
	ui_component splitbar;
	ui_progressbar progressbar;
	Navigation navigation;
	TabBar tabbar;	
	ui_notebook notebook;
	FileBar filebar;
	UndoRedoBar undoredobar;
	MachineBar machinebar;
	SongBar songbar;
	PlayBar playbar;
	PlayPosBar playposbar;
	SequenceView sequenceview;
	MachineView machineview;
	PatternView patternview;		
	SamplesView samplesview;
	InstrumentsView instrumentsview;
	SongPropertiesView songpropertiesview;
	RenderView renderview;
	SettingsView settingsview;
	HelpView helpview;	
	Gear gear;
	VuBar vubar;	
	ui_component statusbar;
	ui_notebook viewbars;
	ui_component* activeview;	
	Workspace workspace;
	ui_label label;
	int firstshow;	
	int resize;
	ui_label statusbarlabel;
	PatternViewBar patternbar;
	MachineViewBar machineviewbar;
} MainFrame;

void mainframe_init(MainFrame*);
int mainframe_showmaximizedatstart(MainFrame*);

#endif
