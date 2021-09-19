/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MAINFRAME_H)
#define MAINFRAME_H

/* host */
#include "mainstatusbar.h"
#include "minmaximize.h"
#include "confirmbox.h"
#include "cpuview.h"
#include "filebar.h"
#include "fileview.h"
#include "exportview.h"
#include "gear.h"
#include "helpview.h"
#include "instrumentview.h"
#include "interpreter.h"
#include "kbdhelp.h"
#include "machinebar.h"
#include "machineview.h"
#include "machineviewbar.h"
#include "metronomebar.h"
#include "midimonitor.h"
#include "navigation.h"
#include "paramgear.h"
#include "playbar.h"
#include "playposbar.h"
#include "plugineditor.h"
#include "patternview.h"
#include "recentview.h"
#include "renderview.h"
#include "sequencerbar.h"
#include "samplesview.h"
#include "sequenceview.h"
#include "seqeditor.h"
#include "startscript.h"
#include "stepbox.h"
#include "stepsequencerview.h"
#include "songbar.h"
#include "songproperties.h"
#include <uitabbar.h>
#include "trackscopeview.h"
#include "undoredobar.h"
#include "vubar.h"
#include "workspace.h"
/* ui */
#include <uiframe.h>
#include <uiterminal.h>
#include <uinotebook.h>
#include <uisplitbar.h>
/* container */
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** MainFrame
**
**  The root component of all other components of psycle. Initializes the
**  workspace. First component created after program start by psycle.c and
**  last destroyed at program termination.
*/

typedef struct MainFrame {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component top;	
	psy_ui_Component toprows;
	psy_ui_Component toprow0;
	psy_ui_Component toprow0_client;
	psy_ui_Component toprow0_bars;
	psy_ui_Component toprow1;
	psy_ui_Component toprow2;
	psy_ui_Component maximize;
	psy_ui_Button maximizebtn;
	psy_ui_Component tabbars;
	psy_ui_Component topspacer;
	psy_ui_Component tabspacer;
	psy_ui_Component client;
	/* includes tabbar */
	psy_ui_Component mainviews;
	/* excludes tabbar and includes bottom views */
	psy_ui_Component mainpane;
	psy_ui_Component spacerleft;
	psy_ui_Component spacerright;
	psy_ui_Terminal terminal;
	psy_ui_Splitter splitbar;
	psy_ui_Splitter splitbarterminal;
	StartScript startscript;
	Navigation navigation;
	psy_ui_TabBar tabbar;
	psy_ui_TabBar scripttabbar;
	psy_ui_Button togglescripts;
	psy_ui_TabBar helpsettingstabbar;
	psy_ui_Notebook notebook;
	FileBar filebar;
	UndoRedoBar undoredobar;
	MachineBar machinebar;
	SongBar songbar;
	PlayBar playbar;
	MetronomeBar metronomebar;
	PlayPosBar playposbar;
	TrackScopeView trackscopeview;	
	StepsequencerView stepsequencerview;
	SeqView sequenceview;
	SequencerBar sequencerbar;
	psy_ui_Splitter splitbarparamrack;
	ParamRack paramrack;
	psy_ui_Splitter splitseqeditor;
	SeqEditor seqeditor;
	MachineView machineview;
	PatternView patternview;		
	SamplesView samplesview;
	InstrumentView instrumentsview;
	SongPropertiesView songpropertiesview;
	ExportView exportview;
	RenderView renderview;	
	PropertiesView settingsview;
	HelpView helpview;	
	KbdHelp kbdhelp;
	Gear gear;
	psy_ui_Splitter gearsplitter;
	PluginEditor plugineditor;
	psy_ui_Splitter splitbarplugineditor;
	VuBar vubar;
	CPUView cpuview;
	psy_ui_Splitter cpusplitter;
	MidiMonitor midimonitor;
	psy_ui_Splitter midisplitter;
	PlaylistView playlist;
	psy_ui_Splitter playlistsplitter;
	FileView fileview;
	MainStatusBar statusbar;
	MinMaximize minmaximize;
	psy_ui_Notebook viewtabbars;
	psy_ui_Component left;
	psy_ui_Component right;
	Workspace workspace;
	psy_ui_Label label;		
	PatternViewBar patternbar;
	MachineViewBar machineviewbar;
	InstrumentsViewBar instrumentsviewbar;	
	ConfirmBox checkunsavedbox;		
	Interpreter interpreter;	
	bool titlemodified;
} MainFrame;

void mainframe_init(MainFrame*);

MainFrame* mainframe_alloc(void);
MainFrame* mainframe_allocinit(void);	

INLINE int mainframe_showmaximizedatstart(MainFrame* self)
{	
	return generalconfig_showmaximizedatstart(
		psycleconfig_general(workspace_conf(&self->workspace)));
}

INLINE psy_ui_Component* mainframe_base(MainFrame* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MAINFRAME_H */
