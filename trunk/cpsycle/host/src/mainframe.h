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
#include "uistatusbar.h"
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
#include "songbar.h"
#include "helpview.h"
#include "stepbox.h"
#include "zoombox.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Component top;	
	psy_ui_Component toprow0;
	psy_ui_Component toprow1;
	psy_ui_Component toprow2;
	psy_ui_Component tabbars;
	psy_ui_Component client;
	psy_ui_Terminal terminal;
	psy_ui_SplitBar splitbar;	
	psy_ui_ProgressBar progressbar;
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
	StepsequencerView stepsequencerview;
	SequenceView sequenceview;
	MachineView machineview;
	PatternView patternview;		
	SamplesView samplesview;
	InstrumentView instrumentsview;
	SongPropertiesView songpropertiesview;
	RenderView renderview;
	SettingsView settingsview;
	HelpView helpview;	
	Gear gear;
	PluginEditor plugineditor;
	VuBar vubar;
	psy_ui_Component statusbar;
	psy_ui_Notebook viewtabbars;
	psy_ui_Notebook viewbars;
	ZoomBox zoombox;
	psy_ui_Component left;
	psy_ui_Component* activeview;
	Workspace workspace;
	psy_ui_Label label;	
	psy_ui_Label statusbarlabel;
	PatternViewBar patternbar;
	MachineViewBar machineviewbar;
} MainFrame;

void mainframe_init(MainFrame*);
int mainframe_showmaximizedatstart(MainFrame*);

#endif
