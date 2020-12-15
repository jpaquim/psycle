// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(WORKSPACE_H)
#define WORKSPACE_H

// host
#include "config.h"
#include "undoredo.h"
#include "viewhistory.h"
// audio
#include <machinefactory.h>
#include <player.h>
#include <plugincatcher.h>
#include <song.h>
#include <sequence.h>
#include <signal.h>
// dsp
#include <notestab.h>
// ui
#include <uicomponent.h>
#include <uiapp.h>
// file
#include <playlist.h>
#include <propertiesio.h>

// Workspace
//
// connects the player with the psycle host ui and configures both
//
// psy_ui_MachineCallback
//         ^
//         |
//       Workspace
//             <>---- PsycleConfig;	              host
//             <>---- ViewHistory
//             <>---- psy_audio_Player;           audio imports
//             <>---- psy_audio_MachineFactory
//             <>---- psy_audio_PluginCatcher
//             <>---- psy_audio_Song

#ifdef __cplusplus
extern "C" {
#endif

// The view id belongs to a component of the client notebook of the mainframe
// view_id = component insert order
enum {
	VIEW_ID_MACHINEVIEW			= 0,
	VIEW_ID_PATTERNVIEW			= 1,
	VIEW_ID_SAMPLESVIEW			= 2,
	VIEW_ID_INSTRUMENTSVIEW		= 3,
	VIEW_ID_SONGPROPERTIES		= 4,
	VIEW_ID_SETTINGSVIEW		= 5,
	VIEW_ID_HELPVIEW			= 6,
	VIEW_ID_RENDERVIEW			= 7,
	VIEW_ID_CHECKUNSAVED		= 8,
	VIEW_NUM
};

// The patternview display modes
typedef enum {
	PATTERN_DISPLAYMODE_TRACKER,					// only tracker visible
	PATTERN_DISPLAYMODE_PIANOROLL,					// only pianoroll visible
	PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL,	// both of them visible
	PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL,
	PATTERN_DISPLAYMODE_NUM
} PatternDisplayMode;

typedef enum {
	CHECKUNSAVE_CLOSE,
	CHECKUNSAVE_LOAD,
	CHECKUNSAVE_NEW
} CheckUnsaveMode;

enum {
	WORKSPACE_NEWSONG,
	WORKSPACE_LOADSONG
};

typedef struct {
	bool row0;
	bool row1;
	bool row2;
	bool trackscopes;
	psy_ui_Value sequenceviewrestorewidth;
	bool maximized;
} MaximizedView;

typedef struct {
	// implements
	psy_audio_MachineCallback machinecallback;
	// Signals
	psy_Signal signal_octavechanged;
	psy_Signal signal_songchanged;
	psy_Signal signal_configchanged;	
	psy_Signal signal_changecontrolskin;
	psy_Signal signal_patterncursorchanged;
	psy_Signal signal_sequenceselectionchanged;
	psy_Signal signal_loadprogress;
	psy_Signal signal_scanprogress;
	psy_Signal signal_beforesavesong;
	psy_Signal signal_showparameters;
	psy_Signal signal_viewselected;
	psy_Signal signal_parametertweak;
	psy_Signal signal_terminal_error;
	psy_Signal signal_terminal_out;
	psy_Signal signal_terminal_warning;
	psy_Signal signal_followsongchanged;
	psy_Signal signal_dockview;
	psy_Signal signal_defaultfontchanged;
	psy_Signal signal_togglegear;
	psy_Signal signal_selectpatterndisplay;
	psy_Signal signal_floatsection;
	psy_Signal signal_docksection;
	// audio
	psy_audio_Song* song;
	psy_audio_Player player;
	psy_audio_PluginCatcher plugincatcher;
	psy_audio_MachineFactory machinefactory;	
	// Psycle settings	
	PsycleConfig config;
	psy_Playlist playlist;	
	psy_ui_Component* mainhandle;	
	ViewHistory viewhistory;
	psy_audio_PatternCursor patterneditposition;
	psy_audio_SequenceSelection sequenceselection;
	int cursorstep;	
	char* filename;
	int followsong;
	int recordtweaks;
	psy_audio_SequenceEntry* lastentry;
	psy_audio_Pattern patternpaste;
	psy_List* sequencepaste;	
	int navigating;
	// ui
	MaximizedView maximizeview;
	int fontheight;	
	bool hasnewline;
	// UndoRedo
	psy_UndoRedo undoredo;
	uintptr_t undosavepoint;
	uintptr_t machines_undosavepoint;
} Workspace;

void workspace_init(Workspace*, void* handle);
void workspace_dispose(Workspace*);
void workspace_disposesequencepaste(Workspace*);
void workspace_load_configuration(Workspace*);
void workspace_save_configuration(Workspace*);
void workspace_newsong(Workspace*);
void workspace_loadsong_fileselect(Workspace*);
void workspace_loadsong(Workspace*, const char* path, bool play);
bool workspace_savesong_fileselect(Workspace*);
void workspace_savesong(Workspace*, const char* path);

INLINE PsycleConfig* workspace_conf(Workspace* self) { return &self->config; }

INLINE void workspace_configure_host(Workspace* self)
{
	psycleconfig_notifyall_changed(&self->config);
	psy_signal_emit(&self->signal_configchanged,
		self, 1, &self->config.config);
}

INLINE psy_audio_Song* workspace_song(Workspace* self) { return self->song; }

INLINE psy_audio_Player* workspace_player(Workspace* self)
{
	return &self->player;
}

void workspace_scanplugins(Workspace*);
psy_Property* workspace_pluginlist(Workspace*);
psy_Property* workspace_recentsongs(Workspace*);
void workspace_load_recentsongs(Workspace*);
void workspace_save_recentsongs(Workspace*);
void workspace_clearrecentsongs(Workspace*);
void workspace_setoctave(Workspace*, int octave);
int workspace_octave(Workspace*);
void workspace_showpatternnames(Workspace*);
void workspace_showpatternids(Workspace*);
void workspace_togglepatdefaultline(Workspace*);
void workspace_configurationchanged(Workspace*, psy_Property* property);
void workspace_undo(Workspace*);
void workspace_redo(Workspace*);
void workspace_setpatterncursor(Workspace*, psy_audio_PatternCursor);
psy_audio_PatternCursor workspace_patterncursor(Workspace*);
void workspace_setsequenceselection(Workspace*, psy_audio_SequenceSelection);
psy_audio_SequenceSelection workspace_sequenceselection(Workspace*);
void workspace_setcursorstep(Workspace*, int step);
int workspace_cursorstep(Workspace*);
void workspace_editquantizechange(Workspace*, int diff);
int workspace_hasplugincache(const Workspace*);
psy_EventDriver* workspace_kbddriver(Workspace*);
int workspace_followingsong(Workspace*);
void workspace_followsong(Workspace*);
void workspace_stopfollowsong(Workspace*);
void workspace_idle(Workspace*);
void workspace_showparameters(Workspace*, uintptr_t machineslot);
void workspace_selectview(Workspace*, int view, uintptr_t section, int option);
void workspace_floatsection(Workspace*, int view, uintptr_t section);
void workspace_docksection(Workspace*, int view, uintptr_t section);
void workspace_parametertweak(Workspace*, int slot, uintptr_t tweak, float value);
void workspace_recordtweaks(Workspace*);
void workspace_stoprecordtweaks(Workspace*);
int workspace_recordingtweaks(Workspace*);
void workspace_onviewchanged(Workspace*, int view);
void workspace_back(Workspace*);
void workspace_forward(Workspace*);
void workspace_updatecurrview(Workspace*);
int workspace_currview(Workspace*);
void workspace_addhistory(Workspace*);
void workspace_changedefaultfontsize(Workspace*, int size);
// unzoomed font height
INLINE int workspace_fontheight(Workspace* self)
{
	return self->fontheight;
}

INLINE void workspace_zoom(Workspace* self, double factor)
{
	workspace_changedefaultfontsize(self, (int)(factor *
		workspace_fontheight(self)));
}

void workspace_dockview(Workspace*, psy_ui_Component*);
void workspace_connectasmixersend(Workspace*);
void workspace_connectasmixerinput(Workspace*);
bool workspace_isconnectasmixersend(const Workspace*);
void workspace_togglegear(Workspace*);
bool workspace_songmodified(const Workspace*);
bool workspace_currview_hasundo(Workspace*);
bool workspace_currview_hasredo(Workspace*);
psy_dsp_NotesTabMode workspace_notetabmode(Workspace*);
void workspace_playstart(Workspace*);
void workspace_outputwarning(Workspace*, const char* text);
void workspace_outputerror(Workspace*, const char* text);
void workspace_output(Workspace*, const char* text);
void workspace_patterndec(Workspace*);
void workspace_patterninc(Workspace*);
void workspace_songposdec(Workspace*);
void workspace_songposinc(Workspace*);
PatternDisplayMode workspace_patterndisplaytype(Workspace*);
void workspace_selectpatterndisplay(Workspace*, PatternDisplayMode);

#ifdef __cplusplus
}
#endif

#endif /* WORKSPACE_H */
