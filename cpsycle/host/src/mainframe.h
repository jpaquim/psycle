/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MAINFRAME_H)
#define MAINFRAME_H

/* host */
#include "mainviews.h"
#include "mainstatusbar.h"
#include "cpuview.h"
#include "filebar.h"
#include "fileview.h"
#include "gear.h"
#include "interpreter.h"
#include "kbdhelp.h"
#include "keyboardview.h"
#include "machinebar.h"

#include "metronomebar.h"
#include "midimonitor.h"
#include "paramgear.h"
#include "playbar.h"
#include "playposbar.h"
#include "plugineditor.h"
#include "sequencerbar.h"
#include "sequenceview.h"
#include "seqeditor.h"
#include "startscript.h"
#include "stepbox.h"
#include "stepsequencerview.h"
#include "songbar.h"
#include "trackscopeview.h"
#include "undoredobar.h"
#include "vubar.h"
#include "workspace.h"
/* ui */
#include <uiframe.h>
#include <uitabbar.h>
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
	psy_ui_Component left;
	FrameDrag frame_drag;
	psy_ui_Button settings_btn;
	psy_ui_Button help_btn;	
	MainViews mainviews;		
	psy_ui_Terminal terminal;
	psy_ui_Splitter splitbar;
	psy_ui_Splitter splitbarterminal;
	StartScript startscript;		
	FileBar filebar;
	UndoRedoBar undoredobar;
	MachineBar machinebar;
	SongBar songbar;
	PlayBar playbar;
	MetronomeBar metronomebar;
	PlayPosBar playposbar;
	TrackScopeView trackscopeview;	
	StepsequencerView stepsequencerview;
	KeyboardView keyboardview;	
	SeqView sequenceview;
	SequencerBar sequencerbar;
	psy_ui_Splitter splitbarparamrack;
	ParamRack paramrack;
	psy_ui_Splitter splitseqeditor;
	SeqEditor seqeditor;
	KbdHelp kbdhelp;
	Gear gear;
	psy_ui_Splitter gearsplitter;
#ifdef PSYCLE_USE_PLUGIN_EDITOR
	PluginEditor plugineditor;
	psy_ui_Splitter splitbarplugineditor;
#endif	
	VuBar vubar;
	CPUView cpuview;
	psy_ui_Splitter cpusplitter;
	MidiMonitor midimonitor;
	psy_ui_Splitter midisplitter;	
	MainStatusBar statusbar;	
	Workspace workspace;	
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

INLINE psy_ui_Component* mainframe_base(MainFrame* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MAINFRAME_H */
