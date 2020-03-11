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
#include "undoredo.h"
#include <sequence.h>

enum {
	TABPAGE_MACHINEVIEW = 0,
	TABPAGE_PATTERNVIEW = 1,
	TABPAGE_SAMPLESVIEW = 2,
	TABPAGE_INSTRUMENTSVIEW = 3,
	TABPAGE_PROPERTIESVIEW = 4,
	TABPAGE_SETTINGSVIEW = 5,
	TABPAGE_HELPVIEW = 6,
	TABPAGE_RENDERVIEW = 7
};

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
	int row0;
	int row1;
	int row2;
	int sequenceviewrestorewidth;
	int maximized;
} MaximizedView;

typedef struct {	
	psy_audio_Song* song;
	psy_audio_Song* songcbk;
	psy_audio_Player player;
	psy_Properties* recentsongs;
	psy_Properties* recentfiles;
	psy_Properties* config;
	psy_Properties* general;
	psy_Properties* inputoutput;
	psy_Properties* midi;
	psy_Properties* keyboard;
	psy_Properties* directories;
	psy_Properties* compatibility;
	psy_Properties* properties;
	psy_Properties* lang;	
	psy_Properties* driverconfigure;
	psy_Properties* driverconfigurations;
	psy_Properties* midiconfigure;
	psy_Properties* theme;
	psy_Properties* cmds;
	psy_Properties* patternviewtheme;
	psy_Properties* machineviewtheme;
	psy_Properties* paramtheme;
	psy_audio_PluginCatcher plugincatcher;
	psy_audio_MachineFactory machinefactory;
	int octave;
	psy_Signal signal_octavechanged;
	psy_Signal signal_songchanged;	
	psy_Signal signal_configchanged;
	psy_Signal signal_skinchanged;
	psy_Signal signal_changecontrolskin;
	psy_Signal signal_patterneditpositionchanged;
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
	psy_ui_Component* mainhandle;
	UndoRedo undoredo;
	History history;
	PatternEditPosition patterneditposition;
	SequenceSelection sequenceselection;
	int cursorstep;
	int hasplugincache;
	char* filename;
	int followsong;
	int recordtweaks;
	SequenceEntry* lastentry;
	psy_audio_Pattern patternpaste;
	psy_List* sequencepaste;
	MaximizedView maximizeview;
	int currview;
	psy_List* currnavigation;
	int navigating;
	int chordmode;
	int fontheight;
	char* dialbitmappath;
} Workspace;

void workspace_init(Workspace*, void* handle);
void workspace_dispose(Workspace*);
void workspace_disposesequencepaste(Workspace*);
void workspace_newsong(Workspace*);
void workspace_loadsong(Workspace*, const char* path);
void workspace_savesong(Workspace*, const char* path);
void workspace_loadskin(Workspace*, const char* path);
void workspace_loadcontrolskin(Workspace*, const char* path);
void workspace_scanplugins(Workspace*);
psy_Properties* workspace_pluginlist(Workspace*);
psy_Properties* workspace_recentsongs(Workspace*);
void workspace_load_configuration(Workspace*);
void workspace_save_configuration(Workspace*);
void workspace_load_recentsongs(Workspace*);
void workspace_save_recentsongs(Workspace*);
void workspace_clearrecentsongs(Workspace*);
void workspace_setoctave(Workspace*, int octave);
int workspace_octave(Workspace*);
int workspace_showsonginfoonload(Workspace*);
int workspace_showaboutatstart(Workspace*);
int workspace_showmaximizedatstart(Workspace*);
int workspace_saverecentsongs(Workspace*);
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
int workspace_showparamviewaswindow(Workspace*);
void workspace_configchanged(Workspace*, psy_Properties* property,
	psy_Properties* choice);
int workspace_wraparound(Workspace*);
int workspace_doublemidline(Workspace*);
void workspace_undo(Workspace*);
void workspace_redo(Workspace*);
void workspace_setpatterneditposition(Workspace*, PatternEditPosition);
PatternEditPosition workspace_patterneditposition(Workspace*);
void workspace_setsequenceselection(Workspace*, SequenceSelection);
SequenceSelection workspace_sequenceselection(Workspace*);
void workspace_setcursorstep(Workspace*, int step);
int workspace_cursorstep(Workspace*);
const char* workspace_translate(Workspace*, const char* key);
int workspace_hasplugincache(Workspace*);
psy_EventDriver* workspace_kbddriver(Workspace*);
int workspace_followingsong(Workspace*);
void workspace_followsong(Workspace*);
void workspace_stopfollowsong(Workspace*);
void workspace_idle(Workspace*);
void workspace_showparameters(Workspace*, uintptr_t machineslot);
void workspace_selectview(Workspace*, int view, uintptr_t section, int option);
void workspace_parametertweak(Workspace*, int slot, uintptr_t tweak, float value);
void workspace_recordtweaks(Workspace*);
void workspace_stoprecordtweaks(Workspace*);
int workspace_recordingtweaks(Workspace*);
int workspace_recordtweaksastws(Workspace*);
int workspace_advancelineonrecordtweak(Workspace*);
void workspace_onviewchanged(Workspace*, int view);
void workspace_back(Workspace*);
void workspace_forward(Workspace*);
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
void workspace_changedefaultfontsize(Workspace*, int size);
const char* workspace_dialbitmap_path(Workspace*);
void workspace_dockview(Workspace*, psy_ui_Component* view);

#endif
