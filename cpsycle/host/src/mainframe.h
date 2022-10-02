/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MAINFRAME_H)
#define MAINFRAME_H

/* host */
#include "mainviews.h"
#include "mainstatusbar.h"
// #include "confirmbox.h"
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
#include "metronomebar.h"
#include "midimonitor.h"
#include "paramgear.h"
#include "playbar.h"
#include "playposbar.h"
#include "plugineditor.h"
#include "patternview.h"
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
#include "styleview.h"
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
	psy_ui_Component pane;
	psy_ui_Component top;	
	psy_ui_Component toprows;
	psy_ui_Component toprow0;
	psy_ui_Component toprow0_client;	
	psy_ui_Component toprow1;	
	psy_ui_Component client;
	psy_ui_Button settings_btn;
	psy_ui_Button help_btn;
	/* includes tabbar */
	MainViews mainviews;
	/* excludes tabbar and includes bottom views */
	//psy_ui_Component mainpane;	
	psy_ui_Terminal terminal;
	psy_ui_Splitter splitbar;
	psy_ui_Splitter splitbarterminal;
	StartScript startscript;
	Links links;
	psy_ui_TabBar scripttabbar;	
	psy_ui_Button togglescripts;
	FileBar filebar;
	UndoRedoBar undoredobar;
	MachineBar machinebar;
	SongBar songbar;
	PlayBar playbar;
	MetronomeBar metronomebar;
	PlayPosBar playposbar;
	TrackScopeView trackscopeview;	
	StepsequencerView stepsequencerview;
	psy_ui_Component keyboardview;
	KeyboardState keyboardstate;
	PianoKeyboard keyboard;
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
	StyleView styleview;
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
	FileView fileview;
	MainStatusBar statusbar;	
	psy_ui_Component left;
	psy_ui_Component right;
	Workspace workspace;	
	ConfirmBox checkunsavedbox;	
	Interpreter interpreter;	
	bool titlemodified;
	ParamViews paramviews;
	psy_ui_RealPoint frame_drag_offset;
	bool allow_frame_move;	
	bool starting;	
} MainFrame;

void mainframe_init(MainFrame*);

MainFrame* mainframe_alloc(void);
MainFrame* mainframe_allocinit(void);

INLINE int mainframe_showmaximizedatstart(MainFrame* self)
{	
	return generalconfig_show_maximized_at_start(
		psycleconfig_general(workspace_conf(&self->workspace)));
}

void mainframe_add_link(MainFrame*, Link*);

INLINE psy_ui_Component* mainframe_base(MainFrame* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MAINFRAME_H */
