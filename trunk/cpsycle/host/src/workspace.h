// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(WORKSPACE_H)
#define WORKSPACE_H

#include <song.h>
#include <player.h>
#include <propertiesio.h>
#include <signal.h>
#include <plugincatcher.h>
#include <machinefactory.h>
#include <uicomponent.h>
#include <uiapp.h>
#include "undoredo.h"
#include <sequence.h>
#include <notestab.h>

// Workspace
//
// connects the player with the host ui and configures both
//
// psy_ui_MachineCallback <>---- Player
//      ^                 <>---- MachineFactory
//      |                 <>---- psy_audio_PluginCatcher
//      |                 <>---- psy_audio_Song
//  Workspace             <>---- History
//

#ifdef __cplusplus
extern "C" {
#endif

enum {
	TABPAGE_MACHINEVIEW = 0,
	TABPAGE_PATTERNVIEW = 1,
	TABPAGE_SAMPLESVIEW = 2,
	TABPAGE_INSTRUMENTSVIEW = 3,
	TABPAGE_PROPERTIESVIEW = 4,
	TABPAGE_SETTINGSVIEW = 5,
	TABPAGE_HELPVIEW = 6,
	TABPAGE_RENDERVIEW = 7,
	TABPAGE_CHECKUNSAVED = 8
};

typedef enum {
	PATTERNDISPLAY_TRACKER,
	PATTERNDISPLAY_PIANOROLL,
	PATTERNDISPLAY_TRACKER_PIANOROLL_VERTICAL,
	PATTERNDISPLAY_TRACKER_PIANOROLL_HORIZONTAL,
	PATTERNDISPLAY_NUM
} PatternDisplayType;

enum {
	PROPERTY_ID_REGENERATEPLUGINCACHE = 1,
	PROPERTY_ID_ENABLEAUDIO,
	PROPERTY_ID_LOADSKIN,
	PROPERTY_ID_DEFAULTSKIN,
	PROPERTY_ID_LOADCONTROLSKIN,
	PROPERTY_ID_ADDEVENTDRIVER,
	PROPERTY_ID_REMOVEEVENTDRIVER,
	PROPERTY_ID_EVENTDRIVERCONFIGDEFAULTS,
	PROPERTY_ID_EVENTDRIVERCONFIGLOAD,
	PROPERTY_ID_EVENTDRIVERCONFIGKEYMAPSAVE,
	PROPERTY_ID_DEFAULTFONT,
	PROPERTY_ID_DEFAULTLINES,
	PROPERTY_ID_DRAWVUMETERS,
	PROPERTY_ID_PATTERNDISPLAY,
	PROPERTY_ID_PATTERNDISPLAY_TRACKER,
	PROPERTY_ID_PATTERNDISPLAY_PIANOROLL,
	PROPERTY_ID_PATTERNDISPLAY_TRACKER_PIANOROLL_VERTICAL,
	PROPERTY_ID_PATTERNDISPLAY_TRACKER_PIANOROLL_HORIZONTAL,
	PROPERTY_ID_LANG,
	PROPERTY_ID_SHOWSTEPSEQUENCER,
	PROPERTY_ID_TRACKSCOPES,
	PROPERTY_ID_AUDIODRIVERS,
	PROPERTY_ID_ACTIVEEVENTDRIVERS,	
	PROPERTY_ID_ADDCONTROLLERMAP,
	PROPERTY_ID_REMOVECONTROLLERMAP
};

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
	int viewid;
	int sequenceentryid;
} HistoryEntry;

typedef struct {
	psy_List* container;
	int prevented;
} History;

void history_init(History*);
void history_dispose(History*);

typedef struct {
	bool row0;
	bool row1;
	bool row2;
	bool trackscopes;
	psy_ui_Value sequenceviewrestorewidth;
	bool maximized;
} MaximizedView;

typedef struct {
	// Signals
	psy_Signal signal_octavechanged;
	psy_Signal signal_songchanged;
	psy_Signal signal_configchanged;
	psy_Signal signal_skinchanged;
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
	psy_audio_MachineCallback machinecallback;
	// Psycle settings
	psy_Property config;
	// Sections of the configuration
	psy_Property* language;
	psy_Property* recentsongs;
	psy_Property* recentfiles;
	psy_Property* global;
	psy_Property* general;
	psy_Property* inputoutput;
	psy_Property* eventinputs;
	psy_Property* keyboard;
	psy_Property* keyboard_misc;
	psy_Property* directories;
	psy_Property* midicontrollers;
	psy_Property* compatibility;
	psy_Property* driverconfigure;
	psy_Property* driverconfigurations;	
	psy_Property* eventdriverconfigure;
	psy_Property* eventdriverconfigurations;
	psy_Property* theme;
	psy_Property* cmds;
	psy_Property* patternviewtheme;
	psy_Property* machineviewtheme;
	psy_Property* paramtheme;	
	psy_ui_Component* mainhandle;	
	History history;
	psy_audio_PatternCursor patterneditposition;
	psy_audio_SequenceSelection sequenceselection;
	int cursorstep;
	int hasplugincache;
	char* filename;
	int followsong;
	int recordtweaks;
	psy_audio_SequenceEntry* lastentry;
	psy_audio_Pattern patternpaste;
	psy_List* sequencepaste;	
	int currview;
	psy_List* currnavigation;
	int navigating;
	// ui
	MaximizedView maximizeview;
	int fontheight;
	char* dialbitmappath;
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
void workspace_loadsong(Workspace*, const char* path, bool play);
void workspace_savesong(Workspace*, const char* path);

INLINE psy_audio_Song* workspace_song(Workspace* self)
{
	return self->song;
}

INLINE psy_audio_Player* workspace_player(Workspace* self)
{
	return &self->player;
}

void workspace_loadskin(Workspace*, const char* path);
void workspace_loadcontrolskin(Workspace*, const char* path);
void workspace_scanplugins(Workspace*);
psy_Property* workspace_pluginlist(Workspace*);
psy_Property* workspace_recentsongs(Workspace*);
void workspace_load_recentsongs(Workspace*);
void workspace_save_recentsongs(Workspace*);
void workspace_clearrecentsongs(Workspace*);
void workspace_setoctave(Workspace*, int octave);
int workspace_octave(Workspace*);
int workspace_showsonginfoonload(Workspace*);
int workspace_showaboutatstart(Workspace*);
int workspace_showmaximizedatstart(Workspace*);
int workspace_saverecentsongs(Workspace*);
int workspace_playsongafterload(Workspace*);
int workspace_showingpatternnames(Workspace*);
void workspace_showpatternnames(Workspace*);
void workspace_showpatternids(Workspace*);
int workspace_showplaylisteditor(Workspace*);
int workspace_showstepsequencer(Workspace*);
int workspace_showgriddefaults(Workspace*);
int workspace_showlinenumbers(Workspace*);
int workspace_showbeatoffset(Workspace*);
int workspace_showlinenumbercursor(Workspace*);
int workspace_showlinenumbersinhex(Workspace*);
int workspace_showwideinstcolumn(Workspace*);
int workspace_showtrackscopes(Workspace*);
int workspace_showmachineindexes(Workspace*);
int workspace_showwirehover(Workspace*);
int workspace_showparamviewaswindow(Workspace*);
bool workspace_savereminder(Workspace*);
bool workspace_patdefaultlines(Workspace*);
void workspace_togglepatdefaultline(Workspace*);
bool workspace_allowmultipleinstances(Workspace*);
void workspace_configurationchanged(Workspace*, psy_Property* property);
int workspace_wraparound(Workspace*);
void workspace_undo(Workspace*);
void workspace_redo(Workspace*);
void workspace_setpatterncursor(Workspace*, psy_audio_PatternCursor);
psy_audio_PatternCursor workspace_patterncursor(Workspace*);
void workspace_setsequenceselection(Workspace*, psy_audio_SequenceSelection);
psy_audio_SequenceSelection workspace_sequenceselection(Workspace*);
void workspace_setcursorstep(Workspace*, int step);
int workspace_cursorstep(Workspace*);
int workspace_hasplugincache(Workspace*);
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
bool workspace_enableaudio(Workspace*);
bool workspace_ft2home(Workspace*);
bool workspace_ft2delete(Workspace*);
bool workspace_effcursoralwayssdown(Workspace*);
bool workspace_playstartwithrctrl(Workspace*);
bool workspace_movecursoronestep(Workspace*);
void workspace_recordtweaks(Workspace*);
void workspace_stoprecordtweaks(Workspace*);
int workspace_recordingtweaks(Workspace*);
int workspace_recordtweaksastws(Workspace*);
int workspace_advancelineonrecordtweak(Workspace*);
void workspace_onviewchanged(Workspace*, int view);
void workspace_back(Workspace*);
void workspace_forward(Workspace*);
void workspace_updatecurrview(Workspace*);
int workspace_currview(Workspace*);
void workspace_addhistory(Workspace*);
void workspace_setloadnewblitz(Workspace*, int mode);
int workspace_loadnewblitz(Workspace*);
const char* workspace_songs_directory(Workspace*);
const char* workspace_samples_directory(Workspace*);
const char* workspace_plugins_directory(Workspace*);
const char* workspace_luascripts_directory(Workspace*);
const char* workspace_vsts32_directory(Workspace*);
const char* workspace_vsts64_directory(Workspace*);
const char* workspace_ladspas_directory(Workspace*);
const char* workspace_skins_directory(Workspace*);
const char* workspace_doc_directory(Workspace*);
const char* workspace_config_directory(Workspace*);
const char* workspace_userpresets_directory(Workspace*);
void workspace_changedefaultfontsize(Workspace*, int size);
// this is the unzoomed font height
INLINE int workspace_fontheight(Workspace* self)
{
	return self->fontheight;
}

INLINE void workspace_zoom(Workspace* self, double factor)
{
	workspace_changedefaultfontsize(self, (int)(factor *
		workspace_fontheight(self)));
}

const char* workspace_dialbitmap_path(Workspace*);
void workspace_dockview(Workspace*, psy_ui_Component* view);
int workspace_ismovecursorwhenpaste(Workspace*);
void workspace_movecursorwhenpaste(Workspace*, bool on);
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
void workspace_songposdec(Workspace*);
void workspace_songposinc(Workspace*);
PatternDisplayType workspace_patterndisplaytype(Workspace*);
void workspace_selectpatterndisplay(Workspace*, PatternDisplayType);

INLINE psy_Property* workspace_patternviewtheme(Workspace* self)
{
	return self->patternviewtheme;
}

INLINE psy_Property* workspace_machineviewtheme(Workspace* self)
{
	return self->machineviewtheme;
}

INLINE psy_Property* workspace_paramtheme(Workspace* self)
{
	return self->paramtheme;
}

#ifdef __cplusplus
}
#endif

#endif /* WORKSPACE_H */
