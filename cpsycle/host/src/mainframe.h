// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(MAINFRAME_H)
#define MAINFRAME_H

#include "workspace.h"
#include "trackscopeview.h"
#include <uilabel.h>
#include <uibutton.h>
#include <uiframe.h>
#include <uiterminal.h>
#include <uinotebook.h>
#include <uiprogressbar.h>
#include <uisplitbar.h>
#include "filebar.h"
#include "undoredobar.h"
#include "navigation.h"
#include "machinebar.h"
#include "machineview.h"
#include "gear.h"
#include "plugineditor.h"
#include "patternview.h"
#include "sequenceview.h"
#include "stepsequencerview.h"
#include "samplesview.h"
#include "instrumentview.h"
#include "settingsview.h"
#include "songproperties.h"
#include "renderview.h"
#include "properties.h"
#include "tabbar.h"
#include "playbar.h"
#include "playposbar.h"
#include "vubar.h"
#include "cpuview.h"
#include "midiview.h"
#include "recentview.h"
#include "songbar.h"
#include "helpview.h"
#include "kbdhelp.h"
#include "stepbox.h"
#include "zoombox.h"
#include "checkunsaved.h"
#include "interpreter.h"

// MainFrame
//
// aim: The root component of all other components of psycle. Initializes the
//      workspace. First component created at program start by psycle.c and
//      last destroyed at program termination.

typedef struct {
	psy_ui_Component component;
	psy_ui_Component top;
	psy_ui_Component topright;
	psy_ui_Component toprow0;
	psy_ui_Component toprow1;
	psy_ui_Component toprow2;
	psy_ui_Component tabbars;
	psy_ui_Component client;
	psy_ui_Terminal terminal;
	psy_ui_SplitBar splitbar;
	psy_ui_SplitBar splitbarterminal;
	psy_ui_ProgressBar progressbar;
	psy_ui_Button toggleterminal;
	Navigation navigation;
	TabBar tabbar;	
	psy_ui_Notebook notebook;
	FileBar filebar;
	UndoRedoBar undoredobar;
	MachineBar machinebar;
	SongBar songbar;
	PlayBar playbar;
	PlayPosBar playposbar;
	TrackScopeView trackscopeview;
	psy_ui_Component paramviews;
	StepsequencerView stepsequencerview;
	SequenceView sequenceview;
	MachineView machineview;
	PatternView patternview;		
	SamplesView samplesview;
	InstrumentView instrumentsview;
	SongPropertiesView songpropertiesview;
	RenderView renderview;
	PropertiesView settingsview;
	HelpView helpview;	
	KbdHelp kbdhelp;
	Gear gear;
	PluginEditor plugineditor;
	psy_ui_SplitBar splitbarplugineditor;
	VuBar vubar;
	CPUView cpuview;
	MidiView midiview;
	RecentView recentview;
	psy_ui_Component statusbar;
	psy_ui_Notebook viewtabbars;
	psy_ui_Notebook viewstatusbars;
	ZoomBox zoombox;
	psy_ui_Component left;
	psy_ui_Component* activeview;
	Workspace workspace;
	psy_ui_Label label;	
	psy_ui_Label statusbarlabel;
	PatternViewBar patternbar;
	MachineViewBar machineviewbar;
	CheckUnsavedBox checkunsavedbox;
	bool startpage;
	bool terminalhasmessage;
	bool terminalhaswarning;
	bool terminalhaserror;
	Interpreter interpreter;
} MainFrame;

void mainframe_init(MainFrame*);
int mainframe_showmaximizedatstart(MainFrame*);

#endif
