/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(WORKSPACE_H)
#define WORKSPACE_H

/* host */
#include "config.h"
#include "confirmbox.h"
#include "fileview.h"
#include "hostmachinecallback.h"
#include "inputhandler.h"
#include "pluginscanthread.h"
#include "undoredo.h"
#include "viewhistory.h"
/* audio */
#include <machinefactory.h>
#include <plugincatcher.h>
#include <song.h>
#include <sequence.h>
#include <signal.h>
/* dsp */
#include <notestab.h>
/* ui */
#include <uicomponent.h>
#include <uiapp.h>
/* file */
#include <logger.h>
#include <playlist.h>

/*
** Workspace
**
** connects the player with the psycle host ui and configures both
**
** Workspace
**           <>---- HostMachineCallback
**           <>---- PsycleConfig;
**           <>---- ViewHistory
**           <>---- psy_audio_Player;
**           <>---- psy_audio_MachineFactory
**           <>---- psy_audio_PluginCatcher
**           <>---- psy_audio_Song
*/

#ifdef __cplusplus
extern "C" {
#endif


struct Workspace;

typedef void (*fp_workspace_output)(void* context,
	struct Workspace* sender, const char* text);
typedef void (*fp_workspace_songloadprogress)(void* context,
	struct Workspace* sender, intptr_t progress);

/* MachinesInsert */

typedef struct MachinesInsert {
	uintptr_t replace_mac;
	psy_audio_Wire wire;
	bool random_position;
} MachinesInsert;

void machinesinsert_init(MachinesInsert*);

void machineinsert_reset(MachinesInsert*);
void machineinsert_append(MachinesInsert*, psy_audio_Wire);
void machineinsert_replace(MachinesInsert*, uintptr_t replace_mac);


/* Workspace */

struct ParamViews;

typedef struct Workspace {	
	HostMachineCallback hostmachinecallback;
	/* signals */		
	psy_Signal signal_load_progress;	
	psy_Signal signal_viewselected;	
	psy_Signal signal_status_out;	
	psy_Signal signal_machineeditresize;
	psy_Signal signal_buschanged;
	psy_audio_Player player;
	psy_audio_Song* song;
	PsycleConfig config;
	psy_Playlist playlist;
	psy_ui_Component* main;	
	ViewHistory view_history;
	ViewIndex restoreview;
	psy_audio_SequencePaste sequencepaste;	
	bool modified_without_undo;
	/* UndoRedo */
	psy_UndoRedo undoredo;
	uintptr_t undo_save_point;
	uintptr_t machines_undo_save_point;		
	PluginScanThread pluginscanthread;	
	InputHandler inputhandler;	
	struct ParamViews* paramviews;
	psy_Logger* terminal_output;
	uintptr_t terminalstyleid;
	FileView* fileview;
	ConfirmBox* confirm;
	MachinesInsert insert;
	int dbg;
	psy_Property load;
	psy_Property save;
} Workspace;

void workspace_init(Workspace*, psy_ui_Component* handle);
void workspace_dispose(Workspace*);

void workspace_load_configuration(Workspace*);
void workspace_start_audio(Workspace*);
void workspace_clear_sequence_paste(Workspace*);
void workspace_save_styles(Workspace*);
void workspace_new_song(Workspace*);
void workspace_load_song_fileselect(Workspace*);
void workspace_load_song(Workspace*, const char*, bool play);
bool workspace_save_song_fileselect(Workspace*);
void workspace_save_song(Workspace*, const char*);
bool workspace_export_song(Workspace*);
void workspace_export_module(Workspace*, const char* path);
bool workspace_export_midi_fileselect(Workspace*);
bool workspace_export_ly_fileselect(Workspace*);
void workspace_export_midi_file(Workspace*, const char* path);
void workspace_export_ly_file(Workspace*, const char* path);
void workspace_scan_plugins(Workspace*);
void workspace_mark_song_modified(Workspace*);

INLINE void workspace_set_param_views(Workspace* self, struct ParamViews* paramviews)
{
	self->paramviews = paramviews;
}

INLINE PsycleConfig* workspace_conf(Workspace* self) { return &self->config; }

INLINE void workspace_configure_host(Workspace* self)
{
	// psycleconfig_notifyall_changed(&self->config);	
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

psy_Playlist* workspace_playlist(Workspace*);
void workspace_undo(Workspace*);
void workspace_redo(Workspace*);
void workspace_edit_quantize_change(Workspace*, int diff);
int workspace_has_plugin_cache(const Workspace*);
psy_audio_PluginCatcher* workspace_plugincatcher(Workspace*);
psy_EventDriver* workspace_kbd_driver(Workspace*);
void workspace_idle(Workspace*);
void workspace_select_view(Workspace*, ViewIndex);
void workspace_save_view(Workspace*);
void workspace_restore_view(Workspace*);
void workspace_add_view(Workspace*, ViewIndex);
void workspace_back(Workspace*);
void workspace_forward(Workspace*);
void workspace_update_currview(Workspace*);
ViewIndex workspace_current_view(Workspace*);
bool workspace_song_modified(const Workspace*);
bool workspace_currview_has_undo(Workspace*);
bool workspace_currview_has_redo(Workspace*);
void workspace_clear_undo(Workspace*);
void workspace_output_warning(Workspace*, const char* text);
void workspace_output_error(Workspace*, const char* text);
void workspace_output(Workspace*, const char* text);
void workspace_output_status(Workspace*, const char* text);
void workspace_connect_status(Workspace*, void* context, fp_workspace_output);
void workspace_connect_load_progress(Workspace*, void* context,
	fp_workspace_songloadprogress);
void workspace_app_title(Workspace*, char* rv_title, uintptr_t max_len);
const char* workspace_song_title(const Workspace*);
void workspace_select_start_view(Workspace*);
void workspace_on_input(Workspace*, uintptr_t cmd);
void workspace_confirm_new(Workspace*);
void workspace_confirm_load(Workspace*);
void workspace_confirm_close(Workspace*);
void workspace_confirm_seqclear(Workspace*);

INLINE bool workspace_song_has_file(const Workspace* self)
{
	if (self->song) {
		return self->song->song_has_file;
	}
	return FALSE;
}

INLINE const psy_audio_HostSequencerTime* workspace_host_seq_time(
	const Workspace* self)
{
	return &self->player.sequencer.hostseqtime;
}

INLINE void workspace_set_terminal_output(Workspace* self,
	psy_Logger* terminal_output)
{
	self->terminal_output = terminal_output;
	self->hostmachinecallback.logger = terminal_output;
}

#ifdef __cplusplus
}
#endif

#endif /* WORKSPACE_H */
