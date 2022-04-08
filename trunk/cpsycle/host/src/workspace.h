/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(WORKSPACE_H)
#define WORKSPACE_H

/* host */
#include "config.h"
#include "inputhandler.h"
#include "undoredo.h"
#include "viewhistory.h"
/* audio */
#include <machinefactory.h>
#include <player.h>
#include <plugincatcher.h>
#include <song.h>
#include <sequence.h>
#include <signal.h>
/* dsp */
#include <notestab.h>
/* ui */
#include <uicomponent.h>
#include <uiapp.h>
/* thread */
#include <lock.h>
#include <thread.h>
/* file */
#include <playlist.h>
#include <propertiesio.h>

/*
** Workspace
**
**  connects the player with the psycle host ui and configures both
**
**  psy_audio_MachineCallback
**         ^
**         |
**       Workspace
**             <>---- PsycleConfig;	              host
**             <>---- ViewHistory
**             <>---- psy_audio_Player;           audio imports
**             <>---- psy_audio_MachineFactory
**             <>---- psy_audio_PluginCatcher
**             <>---- psy_audio_Song
*/

#ifdef __cplusplus
extern "C" {
#endif

/* The patternview display modes */
typedef enum {
	PATTERN_DISPLAYMODE_TRACKER,					/* only tracker visible */
	PATTERN_DISPLAYMODE_PIANOROLL,					/* only pianoroll visible */
	PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL,	/* both of them visible */
	PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL,
	PATTERN_DISPLAYMODE_NUM
} PatternDisplayMode;

enum {
	WORKSPACE_NEWSONG,
	WORKSPACE_LOADSONG
};

typedef struct HostSequencerTime {	
	bool currplaying;
	psy_audio_SequenceCursor lastplaycursor;
	psy_audio_SequenceCursor currplaycursor;
} HostSequencerTime;

void hostsequencertime_init(HostSequencerTime*);

void hostsequencertime_set_play_cursor(HostSequencerTime*, 
	psy_audio_SequenceCursor);
void hostsequencertime_update_last_play_cursor(HostSequencerTime*);

INLINE bool hostsequencertime_playing(const HostSequencerTime* self)
{
	return (self->currplaying);
}

INLINE bool hostsequencertime_play_line_changed(const HostSequencerTime* self)
{
	return (self->currplaycursor.linecache != self->lastplaycursor.linecache);
}

struct Workspace;

typedef void (*fp_workspace_output)(void* context,
	struct Workspace* sender, const char* text);
typedef void (*fp_workspace_songloadprogress)(void* context,
	struct Workspace* sender, intptr_t progress);

struct ParamViews;

typedef struct Workspace {
	/* implements */
	psy_audio_MachineCallback machinecallback;
	/* signals */
	psy_Signal signal_octavechanged;
	psy_Signal signal_songchanged;
	psy_Signal signal_configchanged;	
	psy_Signal signal_playlinechanged;
	psy_Signal signal_playstatuschanged;
	psy_Signal signal_gotocursor;	
	psy_Signal signal_loadprogress;
	psy_Signal signal_scanprogress;
	psy_Signal signal_scanstart;
	psy_Signal signal_scanend;
	psy_Signal signal_scanfile;
	psy_Signal signal_scantaskstart;
	psy_Signal signal_plugincachechanged;
	psy_Signal signal_beforesavesong;	
	psy_Signal signal_viewselected;
	psy_Signal signal_focusview;
	psy_Signal signal_parametertweak;
	psy_Signal signal_terminal_error;
	psy_Signal signal_terminal_warning;
	psy_Signal signal_terminal_out;		
	psy_Signal signal_status_out;
	psy_Signal signal_followsongchanged;	
	psy_Signal signal_togglegear;
	psy_Signal signal_machineeditresize;
	psy_Signal signal_buschanged;
	psy_Signal signal_gearselect;	
	psy_audio_Song* song;
	psy_audio_Player player;
	psy_audio_PluginCatcher plugincatcher;	
	psy_audio_MachineFactory machinefactory;	
	PsycleConfig config;
	psy_Playlist playlist;	
	psy_ui_Component* main;	
	ViewHistory viewhistory;
	ViewIndex restoreview;	
	char* filename;
	bool songhasfile;
	int followsong;
	int recordtweaks;	
	psy_audio_SequencePaste sequencepaste;
	int navigating;
	bool patternsinglemode;
	/* ui */
	int fontheight;	
	bool hasnewline;
	bool gearvisible;
	bool modified_without_undo;
	/* UndoRedo */
	psy_UndoRedo undoredo;
	uintptr_t undosavepoint;
	uintptr_t machines_undosavepoint;
	psy_Lock pluginscanlock;	
	int filescanned;
	char* scanfilename;
	int scanstart;
	int scanend;
	int scantaskstart;
	int plugincachechanged;
	int scanprogress;
	int scanprogresschanged;
	psy_audio_PluginScanTask lastscantask;
	int scanplugintype;
	bool playrow;
	psy_audio_SequencerPlayMode restoreplaymode;
	psy_dsp_big_beat_t restorenumplaybeats;
	bool restoreloop;
	bool startpage;	
	bool driverconfigloading;
	bool seqviewactive;	
	HostSequencerTime host_sequencer_time;
	InputHandler inputhandler;
	psy_Thread driverconfigloadthread;
	psy_Thread pluginscanthread;
	struct ParamViews* paramviews;
} Workspace;

void workspace_init(Workspace*, psy_ui_Component* handle);
void workspace_dispose(Workspace*);
void workspace_load_configuration(Workspace*);
void workspace_postload_driver_configurations(Workspace*);
void workspace_start_audio(Workspace*);
void workspace_save_configuration(Workspace*);
void workspace_clearsequencepaste(Workspace*);
void workspace_save_styleconfiguration(Workspace*);
void workspace_newsong(Workspace*);
void workspace_loadsong_fileselect(Workspace*);
void workspace_loadsong(Workspace*, const char*, bool play);
bool workspace_savesong_fileselect(Workspace*);
void workspace_save_song(Workspace*, const char*);
bool workspace_exportsong(Workspace*);
void workspace_exportmodule(Workspace*, const char* path);
bool workspace_exportmidifile_fileselect(Workspace*);
bool workspace_exportlyfile_fileselect(Workspace*);
void workspace_exportmidifile(Workspace*, const char* path);
void workspace_exportlyfile(Workspace*, const char* path);
void workspace_scanplugins(Workspace*);
void workspace_mark_song_modified(Workspace*);

INLINE void workspace_setparamviews(Workspace* self, struct ParamViews* paramviews)
{
	self->paramviews = paramviews;
}

INLINE PsycleConfig* workspace_conf(Workspace* self) { return &self->config; }

INLINE void workspace_configure_host(Workspace* self)
{
	psycleconfig_notifyall_changed(&self->config);
	psy_signal_emit(&self->signal_configchanged,
		self, 1, &self->config.config);
}

INLINE psy_audio_Song* workspace_song(Workspace* self) { return self->song; }

INLINE const psy_audio_Song* workspace_song_const(const Workspace* self)
{
	return self->song;
}

INLINE psy_audio_Player* workspace_player(Workspace* self)
{
	return &self->player;
}

psy_Property* workspace_recentsongs(Workspace*);
psy_Playlist* workspace_playlist(Workspace*);
void workspace_load_recentsongs(Workspace*);
void workspace_set_octave(Workspace*, uint8_t octave);
uint8_t workspace_octave(Workspace*);
void workspace_configurationchanged(Workspace*, psy_Property*,
	uintptr_t* rebuild_level);
void workspace_onconfigurationchanged(Workspace*, psy_Property*);
void workspace_undo(Workspace*);
void workspace_redo(Workspace*);
void workspace_edit_quantize_change(Workspace*, int diff);
int workspace_hasplugincache(const Workspace*);
psy_audio_PluginCatcher* workspace_plugincatcher(Workspace*);
psy_EventDriver* workspace_kbd_driver(Workspace*);
bool workspace_following_song(const Workspace*);
void workspace_follow_song(Workspace*);
void workspace_stop_follow_song(Workspace*);
void workspace_idle(Workspace*);
void workspace_show_parameters(Workspace*, uintptr_t machineslot);
void workspace_select_view(Workspace*, ViewIndex view_index);
void workspace_focus_view(Workspace*);
void workspace_save_view(Workspace*);
void workspace_restore_view(Workspace*);
void workspace_parameter_tweak(Workspace*, int slot, uintptr_t tweak, float value);
void workspace_record_tweaks(Workspace*);
void workspace_stop_record_tweaks(Workspace*);
int workspace_recording_tweaks(Workspace*);
void workspace_on_view_changed(Workspace*, ViewIndex);
void workspace_back(Workspace*);
void workspace_forward(Workspace*);
void workspace_update_currview(Workspace*);
ViewIndex workspace_current_view(Workspace*);
void workspace_addhistory(Workspace*);
void workspace_connect_as_mixersend(Workspace*);
void workspace_connect_as_mixer_input(Workspace*);
bool workspace_is_connect_as_mixersend(const Workspace*);
void workspace_toggle_gear(Workspace*);
bool workspace_gear_visible(const Workspace* self);
bool workspace_song_modified(const Workspace*);
bool workspace_currview_has_undo(Workspace*);
bool workspace_currview_has_redo(Workspace*);
void workspace_clearundo(Workspace*);
psy_dsp_NotesTabMode workspace_notetabmode(Workspace*);
void workspace_output_warning(Workspace*, const char* text);
void workspace_output_error(Workspace*, const char* text);
void workspace_output(Workspace*, const char* text);
void workspace_output_status(Workspace*, const char* text);
void workspace_goto_cursor(Workspace*, psy_audio_SequenceCursor);
PatternDisplayMode workspace_pattern_display_type(Workspace*);
void workspace_select_pattern_display(Workspace*, PatternDisplayMode);
void workspace_multi_select_gear(Workspace*, psy_List* slotlist);
void workspace_connect_terminal(Workspace*, void* context,
	fp_workspace_output out,
	fp_workspace_output warning,
	fp_workspace_output error);
void workspace_connect_status(Workspace*, void* context, fp_workspace_output);
void workspace_connect_load_progress(Workspace*, void* context,
	fp_workspace_songloadprogress);
void workspace_app_title(Workspace*, char* rv_title, uintptr_t max_len);
const char* workspace_song_title(const Workspace*);
void workspace_set_start_page(Workspace*);
void workspace_on_input(Workspace*, uintptr_t cmd);

INLINE const HostSequencerTime* workspace_host_sequencer_time(
	const Workspace* self)
{
	return &self->host_sequencer_time;
}

#ifdef __cplusplus
}
#endif

#endif /* WORKSPACE_H */
