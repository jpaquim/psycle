/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "workspace.h"
/* local */
#include "cmdproperties.h"
#include "cmdsgeneral.h"
#include "sequencehostcmds.h"
#include "styles.h"
#include "trackercmds.h"
#include "paramviews.h"
/* dsp */
#include <operations.h>
/* audio */
#include <exclusivelock.h>
#include <kbddriver.h>
#include <luaplayer.h>
#include <songio.h>
/* ui */
#include <uiopendialog.h>
#include <uisavedialog.h>
/* platform */
#include "../../detail/os.h"
#include "../../detail/portable.h"

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#include <unistd.h>
#endif


/* MachinesInsert */

/* implementation */
void machinesinsert_init(MachinesInsert* self)
{	
	assert(self);
	
	machineinsert_reset(self);
	self->random_position = TRUE;
}

void machineinsert_reset(MachinesInsert* self)
{
	self->replace_mac = psy_INDEX_INVALID;
	self->wire = psy_audio_wire_zero();	
}

void machineinsert_append(MachinesInsert* self, psy_audio_Wire wire)
{
	assert(self);
	
	self->replace_mac = psy_INDEX_INVALID;
	self->wire = wire;
}

void machineinsert_replace(MachinesInsert* self, uintptr_t replace_mac)
{
	assert(self);
	
	self->replace_mac = replace_mac;
	self->wire = psy_audio_wire_zero();
}


/* Workspace */

/* prototypes */
static void workspace_init_views(Workspace*);
static void workspace_init_audio(Workspace*);
static void workspace_init_player(Workspace*);
static void workspace_init_config(Workspace*);
static void workspace_init_song(Workspace*);
static void workspace_init_signals(Workspace*);
static void workspace_dispose_signals(Workspace*);
static void workspace_update_play_status(Workspace*, bool follow_song);
static void workspace_reset_views(Workspace*);
/* config */
static void workspace_set_song(Workspace*, psy_audio_Song*);
static void workspace_on_load_progress(Workspace*, psy_audio_Song*, int progress);
/* configure actions */
static void workspace_update_save_point(Workspace*);
/* terminal */
static void workspace_on_terminal_warning(Workspace*,
	psy_audio_SongFile* sender, const char* text);
static void workspace_on_terminal_output(Workspace*,
	psy_audio_SongFile* sender, const char* text);
static void workspace_on_terminal_error(Workspace*,
	psy_audio_SongFile* sender, const char* text);
/* song */
static void workspace_on_song_changed(Workspace*, psy_audio_Player* sender);
static void workspace_connect_song(Workspace*, psy_audio_Song*);
static void workspace_on_machine_insert(Workspace*,
	psy_audio_Machines* sender, uintptr_t slot);
static void workspace_on_machine_removed(Workspace*,
	psy_audio_Machines* sender, uintptr_t slot);
static void workspace_reset_undo_save_point(Workspace*);
/* file */
static void workspace_on_load(Workspace*, psy_Property* sender);
static void workspace_on_save(Workspace*, psy_Property* sender);
static void workspace_on_confirm_accept_new(Workspace*);
static void workspace_on_confirm_reject_new(Workspace*);
static void workspace_on_confirm_accept_load(Workspace*);
static void workspace_on_confirm_reject_load(Workspace*);
static void workspace_confirm_accept_close(Workspace*);
static void workspace_confirm_reject_close(Workspace*);
static void workspace_on_confirm_continue(Workspace*);
static void workspace_confirm_accept_seqclear(Workspace*);
static void workspace_confirm_reject_seqclear(Workspace*);


/* implementation */
void workspace_init(Workspace* self, psy_ui_Component* main)
{
	assert(self);
	
	self->dbg = 0;	
	self->main = main;
	machinesinsert_init(&self->insert);
	workspace_init_views(self);
	workspace_init_player(self);	
	workspace_init_song(self);	
	workspace_init_config(self);	
	workspace_init_signals(self);
	workspace_init_audio(self);	
}

void workspace_init_views(Workspace* self)
{
	self->undo_save_point = 0;	
	self->machines_undo_save_point = 0;	
	self->terminalstyleid = STYLE_TERM_BUTTON;	
	self->restoreview = viewindex_make_all(VIEW_ID_MACHINEVIEW,
		SECTION_ID_MACHINEVIEW_WIRES, psy_INDEX_INVALID, psy_INDEX_INVALID);
	self->modified_without_undo = FALSE;
	self->paramviews = NULL;
	self->terminal_output = NULL;
	self->fileview = NULL;
	self->confirm = NULL;
	psy_audio_sequencepaste_init(&self->sequencepaste);
	viewhistory_init(&self->view_history);
	psy_playlist_init(&self->playlist);
	psy_playlist_load(&self->playlist);
	psy_undoredo_init(&self->undoredo);	
}

void workspace_init_player(Workspace* self)
{
	assert(self);
		
	psy_audio_init();
	hostmachinecallback_init(&self->hostmachinecallback,
		psycleconfig_directories(&self->config),
		&self->signal_machineeditresize,
		&self->signal_buschanged);		
	psy_audio_player_init(&self->player,
		hostmachinecallback_base(&self->hostmachinecallback),		
		/* mainwindow platform handle for directx driver */
		(self->main)
		? psy_ui_component_platform(self->main)
		: NULL);	
	psy_audio_eventdrivers_setcmds(&self->player.eventdrivers,
		cmdproperties_create());
	psy_audio_luabind_setplayer(&self->player);	
	pluginscanthread_init(&self->pluginscanthread,
		&self->player.plugincatcher);	
	psy_signal_connect(&self->player.signal_song_changed, self,
		workspace_on_song_changed);
}

void workspace_init_song(Workspace* self)
{
	assert(self);
	
	self->song = psy_audio_song_allocinit(&self->player.machinefactory);
	psy_audio_player_set_song(&self->player, self->song);
	psy_property_init_type(&self->load, "load",
		PSY_PROPERTY_TYPE_STRING);
	psy_property_connect(&self->load, self, workspace_on_load);
	psy_property_init_type(&self->save, "save",
		PSY_PROPERTY_TYPE_STRING);
	psy_property_connect(&self->save, self, workspace_on_save);
}

void workspace_on_load(Workspace* self, psy_Property* sender)
{	
	if (psy_strlen(psy_property_item_str(sender)) == 0) {
		workspace_restore_view(self);
		return;
	}	
	if (keyboardmiscconfig_savereminder(&self->config.misc) &&
			workspace_song_modified(self)) {
		workspace_confirm_load(self);
	} else {
		char path[4096];	
		
		fileview_filename(self->fileview, path, 4096);		
		workspace_load_song(self, path,
			generalconfig_playsongafterload(psycleconfig_general(
				workspace_conf(self))));
	}
}

void workspace_on_save(Workspace* self, psy_Property* sender)
{
	char path[4096];
	
	if (psy_strlen(psy_property_item_str(sender)) == 0) {
		workspace_restore_view(self);
		return;
	}
	psy_ui_component_show_align(fileview_base(self->fileview));
	fileview_filename(self->fileview, path, 4096);
	workspace_save_song(self, path);
}

void workspace_init_config(Workspace* self)
{
	psycleconfig_init(&self->config, &self->player, &self->pluginscanthread,
		&self->player.machinefactory);
	psy_audio_plugincatcher_set_directories(&self->player.plugincatcher,
		psycleconfig_directories(&self->config)->directories);
	psy_audio_plugincatcher_load(&self->player.plugincatcher);
}

void workspace_init_audio(Workspace* self)
{
	audioconfig_driverconfigure_section(&self->config.audio);
	eventdriverconfig_update_active(&self->config.input);
	eventdriverconfig_show_active(&self->config.input, 0);
	eventdriverconfig_register_event_drivers(&self->config.input);
	inputhandler_init(&self->inputhandler);
	psy_audio_eventdrivers_connect(&self->player.eventdrivers,
		&self->inputhandler, (fp_eventdriver_input)
		inputhandler_event_driver_input);
}

void workspace_init_signals(Workspace* self)
{
	assert(self);
		
	psy_signal_init(&self->signal_load_progress);	
	psy_signal_init(&self->signal_viewselected);	
	psy_signal_init(&self->signal_status_out);
	psy_signal_init(&self->signal_machineeditresize);
	psy_signal_init(&self->signal_buschanged);	
}

void workspace_dispose(Workspace* self)
{
	assert(self);
	
	psycleconfig_save_configuration(&self->config);		
	psy_audio_player_dispose(&self->player);
	if (self->song) {
		psy_audio_song_deallocate(self->song);
		self->song = NULL;
	}
	psycleconfig_dispose(&self->config);		
	psy_undoredo_dispose(&self->undoredo);
	viewhistory_dispose(&self->view_history);
	workspace_dispose_signals(self);	
	psy_audio_sequencepaste_dispose(&self->sequencepaste);
	psy_playlist_dispose(&self->playlist);
	psy_audio_dispose();	
	inputhandler_dispose(&self->inputhandler);
	psy_property_dispose(&self->load);
	psy_property_dispose(&self->save);
}

void workspace_dispose_signals(Workspace* self)
{
	assert(self);
			
	psy_signal_dispose(&self->signal_load_progress);			
	psy_signal_dispose(&self->signal_viewselected);
	psy_signal_dispose(&self->signal_status_out);	
	psy_signal_dispose(&self->signal_machineeditresize);
	psy_signal_dispose(&self->signal_buschanged);	
}

void workspace_clear_sequence_paste(Workspace* self)
{
	assert(self);

	psy_audio_sequencepaste_clear(&self->sequencepaste);
}

void workspace_on_song_changed(Workspace* self, psy_audio_Player* sender)
{
	assert(self);
	
	workspace_connect_song(self, psy_audio_player_song(sender));
}

void workspace_connect_song(Workspace* self, psy_audio_Song* song)
{
	assert(self);
	
	if (song) {
		psy_signal_connect(&song->machines.signal_insert, self,
			workspace_on_machine_insert);
		psy_signal_connect(&song->machines.signal_removed, self,
			workspace_on_machine_removed);
	}
}

void workspace_on_machine_insert(Workspace* self,
	psy_audio_Machines* sender, uintptr_t slot)
{	
	assert(self);
	
	if (psy_audio_machines_at(sender, slot)) {
		char text[128];

		psy_snprintf(text, 128, "%s inserted at slot %u",
			psy_audio_machine_editname(psy_audio_machines_at(sender, slot)),
			(unsigned int)slot);
		workspace_output_status(self, text);
	}
}

void workspace_on_machine_removed(Workspace* self,
	psy_audio_Machines* sender, uintptr_t slot)
{	
	char text[128];
		
	psy_snprintf(text, 128, "Machine removed from slot %u",
		(unsigned int)slot);
	workspace_output_status(self, text);
}

void workspace_new_song(Workspace* self)
{
	assert(self);

	psy_audio_exclusivelock_enter();
	workspace_set_song(self, psy_audio_song_allocinit(
		&self->player.machinefactory));
	psy_audio_player_set_sampler_index(&self->player,
		seqeditconfig_machine(&self->config.seqedit));	
	psy_audio_exclusivelock_leave();
	workspace_select_view(self, viewindex_make_all(VIEW_ID_MACHINEVIEW, 0,
		psy_INDEX_INVALID, 0));
}

void workspace_confirm_new(Workspace* self)
{
	assert(self);
	assert(self->confirm);
		
	confirmbox_set_labels(self->confirm, "msg.newsong", "msg.savenew",
		"msg.nosavenew");	
	confirmbox_set_callbacks(self->confirm,
		psy_slot_make(self, workspace_on_confirm_accept_new),
		psy_slot_make(self, workspace_on_confirm_reject_new),
		psy_slot_make(self, workspace_on_confirm_continue));
	workspace_select_view(self, viewindex_make(VIEW_ID_CHECKUNSAVED));
}

void workspace_on_confirm_accept_new(Workspace* self)
{
	assert(self);
	
	if (workspace_save_song_fileselect(self)) {
		workspace_new_song(self);
	}
}

void workspace_on_confirm_reject_new(Workspace* self)
{
	assert(self);

	workspace_reset_undo_save_point(self);
	workspace_new_song(self);
}

void workspace_confirm_load(Workspace* self)
{
	assert(self);
	assert(self->confirm);
	
	confirmbox_set_labels(self->confirm, "msg.loadsong", "msg.saveload",
		"msg.nosaveload");	
	confirmbox_set_callbacks(self->confirm,
		psy_slot_make(self, workspace_on_confirm_accept_load),
		psy_slot_make(self, workspace_on_confirm_reject_load),
		psy_slot_make(self, workspace_on_confirm_continue));
	workspace_select_view(self, viewindex_make(VIEW_ID_CHECKUNSAVED));
}

void workspace_on_confirm_accept_load(Workspace* self)
{
	assert(self);
	
	if (keyboardmiscconfig_ft2_file_view(psycleconfig_misc(
			workspace_conf(self)))) {
		char path[4096];
		
		fileview_filename(self->fileview, path, 4096);
		workspace_load_song(self, path,
			generalconfig_playsongafterload(psycleconfig_general(
				workspace_conf(self))));
	} else {
		workspace_load_song_fileselect(self);
	}
}

void workspace_on_confirm_reject_load(Workspace* self)
{
	char path[4096];
		
	assert(self);

	fileview_filename(self->fileview, path, 4096);
	workspace_load_song(self, path,
		generalconfig_playsongafterload(psycleconfig_general(
			workspace_conf(self))));
}

void workspace_on_confirm_continue(Workspace* self)
{
	assert(self);
	
	workspace_restore_view(self);
}

void workspace_confirm_close(Workspace* self)
{
	assert(self);
	assert(self->confirm);
	
	confirmbox_set_labels(self->confirm, "msg.psyexit", "msg.saveexit",
		"msg.nosaveexit");
	confirmbox_set_callbacks(self->confirm,
		psy_slot_make(self, workspace_confirm_accept_close),
		psy_slot_make(self, workspace_confirm_reject_close),
		psy_slot_make(self, workspace_on_confirm_continue));
	workspace_select_view(self, viewindex_make(VIEW_ID_CHECKUNSAVED));
}

void workspace_confirm_accept_close(Workspace* self)
{
	assert(self);

	if (workspace_save_song_fileselect(self)) {
		psy_ui_app_close(psy_ui_app());
	}
}

void workspace_confirm_reject_close(Workspace* self)
{
	assert(self);
		
	workspace_reset_undo_save_point(self);
	psy_ui_app_close(psy_ui_app());
}

void workspace_reset_undo_save_point(Workspace* self)
{
	assert(self);

	self->modified_without_undo = FALSE;
	self->undo_save_point = psy_list_size(self->undoredo.undo);
	if (self->song) {
		self->machines_undo_save_point = psy_list_size(self->song->machines.undoredo.undo);
	} else {
		self->machines_undo_save_point = 0;
	}
}

void workspace_confirm_seqclear(Workspace* self)
{
	assert(self);
	assert(self->confirm);
	
	if (!workspace_song(self)) {
		return;
	}
	confirmbox_set_labels(self->confirm, "msg.seqclear", "msg.yes", "msg.no");	
	confirmbox_set_callbacks(self->confirm,
		psy_slot_make(self, workspace_confirm_accept_seqclear),
		psy_slot_make(self, workspace_confirm_reject_seqclear),
		psy_slot_make(self, workspace_on_confirm_continue));
	workspace_select_view(self, viewindex_make(VIEW_ID_CHECKUNSAVED));
}

void workspace_confirm_accept_seqclear(Workspace* self)
{
	SequenceCmds cmds;
	
	assert(self);
		
	workspace_restore_view(self);
	sequencecmds_init(&cmds, self);
	sequencecmds_clear(&cmds);	
}

void workspace_confirm_reject_seqclear(Workspace* self)
{
	assert(self);
	
	workspace_restore_view(self);
}

void workspace_load_song_fileselect(Workspace* self)
{
	assert(self);
	
	if (!keyboardmiscconfig_ft2_file_view(psycleconfig_misc(
			workspace_conf(self)))) {
		psy_Property types;
		
		assert(self);
					
		fileview_set_callbacks(self->fileview, &self->load, &self->save);
		psy_property_init_type(&types, "types", PSY_PROPERTY_TYPE_CHOICE);		
		psy_property_set_text(psy_property_set_id(
			psy_property_append_str(&types, "psy", "*.psy"),
			FILEVIEWFILTER_PSY), "Psycle");
		psy_property_set_text(psy_property_set_id(
			psy_property_append_str(&types, "mod", "*.mod"),
			FILEVIEWFILTER_MOD), "Module");	
		fileview_set_filter(self->fileview, &types);
		psy_property_dispose(&types);
		workspace_save_view(self);
		workspace_select_view(self, viewindex_make_all(
			VIEW_ID_FILEVIEW, 0, 0, psy_INDEX_INVALID));
	} else {			
		psy_ui_OpenDialog dialog;

		psy_ui_opendialog_init_all(&dialog, NULL,
			psy_ui_translate("file.loadsong"),
			psy_audio_songfile_loadfilter(),
			psy_audio_songfile_standardloadfilter(),
			dirconfig_songs(psycleconfig_directories(
				workspace_conf(self))));
		if (psy_ui_opendialog_execute(&dialog)) {
			workspace_load_song(self,
				psy_path_full(psy_ui_opendialog_path(&dialog)),
				generalconfig_playsongafterload(psycleconfig_general(
					workspace_conf(self))));
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void workspace_load_song(Workspace* self, const char* filename, bool play)
{
	psy_audio_Song* song;

	assert(self);

	psy_audio_exclusivelock_enter();
	song = psy_audio_song_allocinit(&self->player.machinefactory);	
	if (song) {
		psy_audio_SongFile songfile;

		psy_signal_connect(&song->signal_load_progress, self,
			workspace_on_load_progress);
		psy_signal_emit(&song->signal_load_progress, self, 1, -1);
		psy_audio_songfile_init(&songfile);
		psy_signal_connect(&songfile.signal_warning, self,
			workspace_on_terminal_warning);
		psy_signal_connect(&songfile.signal_output, self,
			workspace_on_terminal_output);
		songfile.song = song;
		songfile.file = 0;
		psy_audio_player_setemptysong(&self->player);
		psy_audio_machinecallback_set_song(&self->hostmachinecallback.machinecallback, song);
		if (psy_audio_songfile_load(&songfile, filename) != PSY_OK) {
			psy_audio_song_deallocate(song);
			workspace_output_error(self, songfile.serr);
			workspace_output_error(self, "\n");			
			psy_audio_songfile_dispose(&songfile);
			play = FALSE;
		} else {
			psy_audio_song_set_file(song, filename);					
			workspace_set_song(self, song);
			psy_audio_songfile_dispose(&songfile);
			if (generalconfig_saving_recent_songs(psycleconfig_general(
					workspace_conf(self)))) {
				psy_playlist_add(&self->playlist, filename);
			}			
		}
		workspace_clear_undo(self);
		workspace_output(self, "ready\n");
		if (play) {
			psy_audio_player_stop(&self->player);
			psy_audio_player_setposition(&self->player, 0.0);
			psy_audio_player_start(&self->player);
		}
	}
	psy_audio_exclusivelock_leave();
}

void workspace_on_load_progress(Workspace* self, psy_audio_Song* sender,
	int progress)
{
	assert(self);

	psy_signal_emit(&self->signal_load_progress, self, 1, progress);
}

void workspace_set_song(Workspace* self, psy_audio_Song* song)
{
	psy_audio_Song* old;
	
	assert(self);

	if (self->song == song) {
		return;
	}
	old = self->song;
	self->song = song;		
	psy_audio_player_set_song(&self->player, self->song);		
	if (old) {
		psy_audio_song_deallocate(old);
	}
	workspace_reset_views(self);	
}

void workspace_reset_views(Workspace* self)
{
	ViewIndex view;	
	psy_audio_SequenceCursor cursor;
		
	assert(self);
	
	workspace_clear_sequence_paste(self);
	workspace_clear_undo(self);
	self->modified_without_undo = FALSE;		
	view = viewhistory_current(&self->view_history);
	viewhistory_clear(&self->view_history);
	viewhistory_add(&self->view_history, view);
	psy_audio_sequenceselection_clear(&self->song->sequence.selection);
	psy_audio_sequenceselection_select_first(&self->song->sequence.selection,
		psy_audio_orderindex_make(0, 0));		
	psy_audio_sequencecursor_init(&cursor);
	psy_audio_sequence_set_cursor(&self->song->sequence, cursor);
}

bool workspace_save_song_fileselect(Workspace* self)
{
	psy_ui_SaveDialog dialog;
	int success;

	assert(self);
	
	psy_ui_savedialog_init_all(&dialog, NULL,
		psy_ui_translate("file.savesong"),
		psy_audio_songfile_savefilter(),
		psy_audio_songfile_standardsavefilter(),
		dirconfig_songs(psycleconfig_directories(
			workspace_conf(self))));
	if (success = psy_ui_savedialog_execute(&dialog)) {
		workspace_save_song(self,
			psy_path_full(psy_ui_savedialog_path(&dialog)));
	}
	psy_ui_savedialog_dispose(&dialog);
	return success;
}

bool workspace_export_song(Workspace* self)
{
	psy_ui_SaveDialog dialog;
	int success;
	static const char export_filters[] =
		"All Songs (*.xm)" "|*.xm|"
		"FastTracker II Songs (*.xm)"       "|*.xm";

	assert(self);
	
	psy_ui_savedialog_init_all(&dialog, NULL,
		psy_ui_translate("file.savesong"),
		export_filters, "xm",
		dirconfig_songs(psycleconfig_directories(
			workspace_conf(self))));
	if (success = psy_ui_savedialog_execute(&dialog)) {
		workspace_export_module(self,
			psy_path_full(psy_ui_savedialog_path(&dialog)));
	}
	psy_ui_savedialog_dispose(&dialog);
	return success;
}

void workspace_export_module(Workspace* self, const char* path)
{
	psy_audio_SongFile songfile;

	assert(self);

	psy_audio_songfile_init(&songfile);
	songfile.file = 0;
	songfile.song = self->song;
	if (psy_audio_songfile_exportmodule(&songfile, path)) {
		workspace_output_error(self, songfile.serr);			
	} else {
		self->undo_save_point = psy_list_size(self->undoredo.undo);
		self->machines_undo_save_point = psy_list_size(self->undoredo.undo);
	}
	psy_audio_songfile_dispose(&songfile);
	workspace_output(self, "ready\n");
}

bool workspace_export_midi_fileselect(Workspace* self)
{
	psy_ui_SaveDialog dialog;
	int success;
	static const char export_filters[] =
		"All Songs (*.mid)" "|*.mid|"
		"Midi File Songs (*.mid)"       "|*.mid";

	psy_ui_savedialog_init_all(&dialog, NULL,
		psy_ui_translate("file.savesong"),
		export_filters, "mid",
		dirconfig_songs(psycleconfig_directories(
			workspace_conf(self))));
	if (success = psy_ui_savedialog_execute(&dialog)) {
		workspace_export_midi_file(self,
			psy_path_full(psy_ui_savedialog_path(&dialog)));
	}
	psy_ui_savedialog_dispose(&dialog);
	return success;
}

bool workspace_export_ly_fileselect(Workspace* self)
{
	psy_ui_SaveDialog dialog;
	int success;
	static const char export_filters[] =
		"All Songs (*.ly)" "|*.ly|"
		"LilyPond File Songs (*.ly)"       "|*.ly";

	psy_ui_savedialog_init_all(&dialog, NULL,
		psy_ui_translate("file.savesong"),
		export_filters, "ly",
		dirconfig_songs(psycleconfig_directories(
			workspace_conf(self))));
	if (success = psy_ui_savedialog_execute(&dialog)) {
		workspace_export_ly_file(self,
			psy_path_full(psy_ui_savedialog_path(&dialog)));
	}
	psy_ui_savedialog_dispose(&dialog);
	return success;
}


void workspace_export_midi_file(Workspace* self, const char* path)
{
	psy_audio_SongFile songfile;

	assert(self);

	psy_audio_songfile_init(&songfile);
	songfile.file = 0;
	songfile.song = self->song;	

	if (psy_audio_songfile_export_midi_file(&songfile, path)) {
		workspace_output_error(self, songfile.serr);
	} else {
		self->undo_save_point = psy_list_size(self->undoredo.undo);
		self->machines_undo_save_point = psy_list_size(self->undoredo.undo);
	}
	psy_audio_songfile_dispose(&songfile);
	workspace_output(self, "ready\n");
}

void workspace_export_ly_file(Workspace* self, const char* path)
{
	psy_audio_SongFile songfile;

	assert(self);

	psy_audio_songfile_init_song(&songfile, self->song);	
	if (psy_audio_songfile_export_ly_file(&songfile, path)) {
		workspace_output_error(self, songfile.serr);
	} else {
		workspace_update_save_point(self);
	}
	psy_audio_songfile_dispose(&songfile);
	workspace_output(self, "ready\n");
}

void workspace_save_song(Workspace* self, const char* path)
{
	int status;
	
	assert(self);
	
	if (!self->song) {
		return;
	}			
	status = psy_audio_song_save(self->song, path, self,
		(fp_string_output)workspace_output_error);
	if (status == PSY_OK) {
		workspace_update_save_point(self);
		workspace_output(self, "ready\n");
	}
}

void workspace_update_save_point(Workspace* self)
{
	self->undo_save_point = psy_list_size(self->undoredo.undo);
	self->machines_undo_save_point = psy_list_size(self->undoredo.undo);	
}

psy_Playlist* workspace_playlist(Workspace* self)
{
	assert(self);

	return &self->playlist;
}

void workspace_load_configuration(Workspace* self)
{
	psy_Path path;
	const char* driverkey;	
	psy_PropertyReader propertyreader;

	assert(self);

	psy_path_init(&path, NULL);
	psy_path_set_prefix(&path, dirconfig_config_dir(&self->config.directories));
	psy_path_set_name(&path, PSYCLE_INI);
	psy_propertyreader_init(&propertyreader, &self->config.config,
		psy_path_full(&path));
	psy_propertyreader_load(&propertyreader);
	visualconfig_reset_app_theme(&self->config.visual);	
	psy_audio_player_midi_configure(&self->player,
		self->config.midi.controllers, TRUE /* use controllerdata */);
	midiviewconfig_make_controllers(
		psycleconfig_midi(&self->config));
	if (compatconfig_loadnewblitz(psycleconfig_compat(
		workspace_conf(self)))) {
		psy_audio_machinefactory_loadnewgamefxandblitzifversionunknown(
			&self->player.machinefactory);
	} else {
		psy_audio_machinefactory_loadoldgamefxandblitzifversionunknown(
			&self->player.machinefactory);
	}
	psy_audio_player_set_sampler_index(&self->player,
		seqeditconfig_machine(&self->config.seqedit));

	driverkey = audioconfig_driverkey(psycleconfig_audio(&self->config));
	audioconfig_makeconfiguration_driverkey(psycleconfig_audio(&self->config),
		driverkey);	
	psy_propertyreader_load(&propertyreader);	
	{
		psy_Property* driversection = NULL;

		psy_audio_player_loaddriver(&self->player, audioconfig_driver_path(
			&self->config.audio), NULL /*no config*/,
			FALSE /* don't open yet*/);
		if (psy_audiodriver_configuration(self->player.driver)) {
			driversection = psy_property_find(
				self->config.audio.driverconfigurations,
				psy_property_key(
					psy_audiodriver_configuration(self->player.driver)),
				PSY_PROPERTY_TYPE_NONE);
		}
		if (globalconfig_audio_enabled(&self->config.global)) {
			psy_audiodriver_close(self->player.driver);
			psy_audiodriver_configure(self->player.driver, driversection);
		} else if (self->player.driver) {
			psy_audiodriver_close(self->player.driver);
			psy_audiodriver_configure(self->player.driver, driversection);
		}
		audioconfig_driverconfigure_section(&self->config.audio);
	}
	eventdriverconfig_config(&self->config.input);
	psy_audio_eventdrivers_restart_all(&self->player.eventdrivers);
	eventdriverconfig_update_active(&self->config.input);
	eventdriverconfig_make(&self->config.input);
	psy_propertyreader_load(&propertyreader);
	eventdriverconfig_read(&self->config.input);
	psy_audio_eventdrivers_restart_all(&self->player.eventdrivers);
	eventdriverconfig_show_active(&self->config.input,
		eventdriverconfig_current(&self->config.input));	
	psy_path_dispose(&path);
	psy_propertyreader_load(&propertyreader);
	psy_propertyreader_dispose(&propertyreader);
	psycleconfig_load_driver_configurations(&self->config);
	if (globalconfig_audio_enabled(&self->config.global)) {
		workspace_start_audio(self);
		workspace_output_status(self, psy_ui_translate("msg.audiostarted"));
	}
}

void workspace_start_audio(Workspace* self)
{
	if (self->player.driver) {
		psy_audiodriver_open(self->player.driver);
		psy_audio_player_stop_threads(&self->player);
		psy_audio_player_start_threads(&self->player,
			audioconfig_num_threads(&self->config.audio));
	}
}

void workspace_undo(Workspace* self)
{
	assert(self);

	if (workspace_current_view(self).id == VIEW_ID_MACHINEVIEW) {
		psy_undoredo_undo(&self->song->machines.undoredo);
	} else {
		psy_undoredo_undo(&self->undoredo);
	}
}

void workspace_redo(Workspace* self)
{
	assert(self);

	if (workspace_current_view(self).id == VIEW_ID_MACHINEVIEW) {
		psy_undoredo_redo(&self->song->machines.undoredo);
	} else {
		psy_undoredo_redo(&self->undoredo);
	}
}

bool workspace_currview_has_undo(Workspace* self)
{
	assert(self);

	if (workspace_current_view(self).id == VIEW_ID_MACHINEVIEW) {
		return psy_list_size(self->song->machines.undoredo.undo) != 0;
	}
	return psy_list_size(self->undoredo.undo) != 0;
}

bool workspace_currview_has_redo(Workspace* self)
{
	assert(self);

	if (workspace_current_view(self).id == VIEW_ID_MACHINEVIEW) {
		return psy_list_size(self->song->machines.undoredo.redo) !=
			self->machines_undo_save_point;
	}
	return psy_list_size(self->undoredo.redo) != self->undo_save_point;
}

void workspace_clear_undo(Workspace* self)
{
	assert(self);

	psy_undoredo_dispose(&self->undoredo);
	psy_undoredo_init(&self->undoredo);
	self->undo_save_point = 0;
	if (self->song) {
		psy_undoredo_dispose(&self->song->machines.undoredo);
		psy_undoredo_init(&self->song->machines.undoredo);
	}
	self->machines_undo_save_point = 0;
}

ViewIndex workspace_current_view(Workspace* self)
{
	assert(self);

	return viewhistory_current(&self->view_history);
}

int workspace_has_plugin_cache(const Workspace* self)
{
	assert(self);

	return psy_audio_plugincatcher_hascache(&self->player.plugincatcher);
}

psy_audio_PluginCatcher* workspace_plugincatcher(Workspace* self)
{
	assert(self);

	return &self->player.plugincatcher;
}

void workspace_edit_quantize_change(Workspace* self, int diff)
{
	const int total = 17;
	const int nextsel = (total + keyboardmiscconfig_cursor_step(
		&self->config.misc) + diff) % total;

	assert(self);

	keyboardmiscconfig_setcursorstep(&self->config.misc, nextsel);
}

psy_EventDriver* workspace_kbd_driver(Workspace* self)
{
	assert(self);

	return psy_audio_player_kbddriver(&self->player);
}

void workspace_idle(Workspace* self)
{
	assert(self);

	workspace_update_play_status(self, keyboardmiscconfig_following_song(
		&self->config.misc));
	pluginscanthread_idle(&self->pluginscanthread);
	psy_audio_player_idle(&self->player);	
}

void workspace_update_play_status(Workspace* self, bool follow_song)
{	
	bool restore_prevented;

	assert(self);	

	restore_prevented = viewhistory_prevented(&self->view_history);
	viewhistory_prevent(&self->view_history);
	psy_audio_sequencer_update_host_seq_time(&self->player.sequencer,
		follow_song);	
	if (!restore_prevented) {
		viewhistory_enable(&self->view_history);
	}
}

void workspace_save_view(Workspace* self)
{
	assert(self);

	self->restoreview = workspace_current_view(self);
}

void workspace_restore_view(Workspace* self)
{
	assert(self);

	workspace_select_view(self, self->restoreview);	
}

void workspace_select_view(Workspace* self, ViewIndex view_index)
{
	assert(self);

	if (view_index.id == VIEW_ID_CHECKUNSAVED &&
			workspace_current_view(self).id != VIEW_ID_CHECKUNSAVED &&
			workspace_current_view(self).id != VIEW_ID_CONFIRM) {
		workspace_save_view(self);		
	}
	psy_signal_emit(&self->signal_viewselected, self, 3, view_index.id,
		view_index.section, view_index.option);
}

void workspace_add_view(Workspace* self, ViewIndex view)
{
	assert(self);

	viewhistory_add(&self->view_history, view);
}

void workspace_back(Workspace* self)
{
	ViewIndex view;

	assert(self);

	view = viewhistory_current(&self->view_history);
	if (viewhistory_back(&self->view_history)) {
		if (!viewhistory_equal(&self->view_history, view)) {
			workspace_update_currview(self);
		}
	}
}

void workspace_forward(Workspace* self)
{
	ViewIndex view;

	assert(self);

	view = viewhistory_current(&self->view_history);
	if (viewhistory_forward(&self->view_history)) {
		if (!viewhistory_equal(&self->view_history, view)) {
			workspace_update_currview(self);
		}
	}
}

void workspace_update_currview(Workspace* self)
{
	ViewIndex view;
	int prevented;

	assert(self);

	view = viewhistory_current(&self->view_history);
	prevented = viewhistory_prevented(&self->view_history);
	viewhistory_prevent(&self->view_history);
	workspace_select_view(self, view);
	/* if (view.seqpos != -1 &&
		self->sequenceselection.editposition.trackposition.sequencentrynode) {
		psy_audio_SequencePosition position;

		position = psy_audio_sequence_positionfromid(&self->song->sequence,
			view.seqpos);
		psy_audio_sequenceselection_seteditposition(&self->sequenceselection,
			position);
		workspace_setsequenceselection(self, self->sequenceselection);
	} */
	if (!prevented) {
		viewhistory_enable(&self->view_history);
	}
}

void workspace_on_terminal_warning(Workspace* self, psy_audio_SongFile* sender,
	const char* text)
{
	assert(self);

	workspace_output_warning(self, text);
}

void workspace_on_terminal_error(Workspace* self, psy_audio_SongFile* sender,
	const char* text)
{
	assert(self);

	workspace_output_error(self, text);
}

void workspace_on_terminal_output(Workspace* self, psy_audio_SongFile* sender,
	const char* text)
{
	assert(self);

	workspace_output(self, text);
}

void workspace_output(Workspace* self, const char* text)
{
	assert(self);

	if (self->terminal_output) {
		psy_logger_output(self->terminal_output, text);		
	}
}

void workspace_output_warning(Workspace* self, const char* text)
{
	assert(self);

	if (self->terminal_output) {
		if (self->terminalstyleid == STYLE_TERM_BUTTON) {
			self->terminalstyleid = STYLE_TERM_BUTTON_WARNING;
		}
		psy_logger_warn(self->terminal_output, text);
	}
}

void workspace_output_error(Workspace* self, const char* text)
{
	assert(self);

	if (self->terminal_output) {
		self->terminalstyleid = STYLE_TERM_BUTTON_ERROR;
		psy_logger_error(self->terminal_output, text);
	}
}

void workspace_output_status(Workspace* self, const char* text)
{
	assert(self);

	psy_signal_emit(&self->signal_status_out, self, 1, text);
}

bool workspace_song_modified(const Workspace* self)
{
	assert(self);

	return self->modified_without_undo ||
		psy_list_size(self->undoredo.undo) != self->undo_save_point ||
		psy_list_size(self->song->machines.undoredo.undo) !=
			self->machines_undo_save_point;
}

void workspace_scan_plugins(Workspace* self)
{
	pluginscanthread_start(&self->pluginscanthread);
}

void workspace_mark_song_modified(Workspace* self)
{
	assert(self);

	self->modified_without_undo = TRUE;
}

void workspace_connect_status(Workspace* self,
	void* context, fp_workspace_output status)
{
	assert(self);

	psy_signal_connect(&self->signal_status_out, context, status);
}

void workspace_connect_load_progress(Workspace* self, void* context,
	fp_workspace_songloadprogress fp)
{
	assert(self);

	psy_signal_connect(&self->signal_load_progress, context, fp);
}

void workspace_on_input(Workspace* self, uintptr_t cmdid)
{
	assert(self);

	switch (cmdid) {
	case CMD_IMM_ADDMACHINE:
		workspace_select_view(self, 
			viewindex_make_all(VIEW_ID_MACHINEVIEW,
			SECTION_ID_MACHINEVIEW_NEWMACHINE,
			psy_INDEX_INVALID, psy_INDEX_INVALID));			
		break;
	case CMD_IMM_EDITMACHINE:
		if (workspace_current_view(self).id != VIEW_ID_MACHINEVIEW) {
			workspace_select_view(self, viewindex_make_all(VIEW_ID_MACHINEVIEW,
				psy_INDEX_INVALID, psy_INDEX_INVALID, 0));
		} else {
			if (workspace_current_view(self).section == SECTION_ID_MACHINEVIEW_WIRES) {
				workspace_select_view(self, viewindex_make_all(
					VIEW_ID_MACHINEVIEW,
					SECTION_ID_MACHINEVIEW_STACK,
					psy_INDEX_INVALID, 0));
			} else {
				workspace_select_view(self,
					viewindex_make_all(VIEW_ID_MACHINEVIEW,
					SECTION_ID_MACHINEVIEW_WIRES, psy_INDEX_INVALID, 0));
			}
		}
		break;
	case CMD_IMM_HELP:
		workspace_select_view(self, viewindex_make_all(
			VIEW_ID_HELPVIEW, SECTION_ID_HELPVIEW_HELP, 0,
			psy_INDEX_INVALID));
		break;
	case CMD_IMM_EDITPATTERN: {
		ViewIndex view;
		
		view = workspace_current_view(self);		
		if (view.id != VIEW_ID_PATTERNVIEW) {
			workspace_select_view(self, viewindex_make_all(
				VIEW_ID_PATTERNVIEW, psy_INDEX_INVALID,
				psy_INDEX_INVALID, 0));
		} else if (view.section == SECTION_ID_PATTERNVIEW_TRACKER) {				
			workspace_select_view(self, viewindex_make_all(
				VIEW_ID_PATTERNVIEW, SECTION_ID_PATTERNVIEW_PIANO,
				psy_INDEX_INVALID, 0));
		} else {				
			workspace_select_view(self, viewindex_make_all(
				VIEW_ID_PATTERNVIEW, SECTION_ID_PATTERNVIEW_TRACKER,
				psy_INDEX_INVALID, 0));		
		}				
		break; }
	case CMD_IMM_EDITINSTR:		
		workspace_select_view(self, viewindex_make_all(
			VIEW_ID_INSTRUMENTSVIEW, psy_INDEX_INVALID,
			psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_EDITSAMPLE:
		workspace_select_view(self, viewindex_make_all(
			VIEW_ID_SAMPLESVIEW, psy_INDEX_INVALID,
			psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_EDITWAVE:
		workspace_select_view(self, viewindex_make_all(
			VIEW_ID_SAMPLESVIEW, 2, 
				psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_INSTRDEC:
		if (self->song) {
			psy_audio_instruments_dec(&self->song->instruments);
		}
		break;
	case CMD_IMM_INSTRINC:
		if (self->song) {
			psy_audio_instruments_inc(&self->song->instruments);
		}
		break;
	case CMD_IMM_ENABLEAUDIO:
		globalconfig_enable_audio(&self->config.global,
			!globalconfig_audio_enabled(&self->config.global));
		break;
	case CMD_IMM_SETTINGS:
		workspace_select_view(self, viewindex_make_all(VIEW_ID_SETTINGSVIEW, 0, 0, 0));
		break;
	case CMD_IMM_LOADSONG:
		if (keyboardmiscconfig_savereminder(&self->config.misc) &&
				workspace_song_modified(self)) {
			workspace_confirm_load(self);
		} else {
			workspace_load_song_fileselect(self);
		}
		break;
	case CMD_IMM_INFOMACHINE:
		if (self->song && self->paramviews) {
			paramviews_show(self->paramviews, psy_audio_machines_selected(
				&self->song->machines));
		}
		break;
	case CMD_IMM_PATTERNINC: {
		SequenceCmds cmds;

		sequencecmds_init(&cmds, self);
		sequencecmds_incpattern(&cmds);
		break; }
	case CMD_IMM_PATTERNDEC: {
		SequenceCmds cmds;

		sequencecmds_init(&cmds, self);
		sequencecmds_decpattern(&cmds);
		break; }
	case CMD_IMM_SONGPOSDEC:
		if (self->song) {
			psy_audio_sequence_dec_seqpos(&self->song->sequence);
		}
		break;
	case CMD_IMM_SONGPOSINC:
		if (self->song) {
			psy_audio_sequence_inc_seqpos(&self->song->sequence);
		}
		break;
	case CMD_IMM_PLAYSONG:
		psy_audio_player_start_begin(&self->player);		
		break;
	case CMD_IMM_PLAYSTART:
		psy_audio_player_start_currseqpos(&self->player);
		break;
	case CMD_IMM_PLAYSTOP:
		psy_audio_player_stop(&self->player);
		break;
	case CMD_IMM_PLAYROWTRACK:
		/* todo */
		break;
	case CMD_IMM_PLAYROWPATTERN:
		/* todo */
		break;
	case CMD_IMM_PLAYFROMPOS:
		/* todo */
		break;
	case CMD_IMM_SAVESONG:
		workspace_save_song_fileselect(self);
		break;
	case CMD_IMM_FOLLOWSONG:
		if (keyboardmiscconfig_following_song(&self->config.misc)) {
			keyboardmiscconfig_stop_follow_song(&self->config.misc);
		} else {
			keyboardmiscconfig_follow_song(&self->config.misc);			
		}
		break;
	case CMD_COLUMN_0:
	case CMD_COLUMN_1:
	case CMD_COLUMN_2:
	case CMD_COLUMN_3:
	case CMD_COLUMN_4:
	case CMD_COLUMN_5:
	case CMD_COLUMN_6:
	case CMD_COLUMN_7:
	case CMD_COLUMN_8:
	case CMD_COLUMN_9:
	case CMD_COLUMN_A:
	case CMD_COLUMN_B:
	case CMD_COLUMN_C:
	case CMD_COLUMN_D:
	case CMD_COLUMN_E:
	case CMD_COLUMN_F:
		if (self->song && psy_audio_song_num_song_tracks(self->song) >=
				(uintptr_t)(cmdid - CMD_COLUMN_0)) {
			self->song->sequence.cursor.track = (cmdid - CMD_COLUMN_0);			
			psy_audio_sequence_set_cursor(psy_audio_song_sequence(self->song),
				self->song->sequence.cursor);
		}
		break;
	case CMD_IMM_TAB1:
		workspace_select_view(self,
			viewindex_make_all(0, psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB2:
		workspace_select_view(self,
			viewindex_make_all(1, psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB3:
		workspace_select_view(self,
			viewindex_make_all(2, psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB4:
		workspace_select_view(self,
			viewindex_make_all(3, psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB5:
		workspace_select_view(self,
			viewindex_make_all(4, psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB6:
		workspace_select_view(self,
			viewindex_make_all(psy_INDEX_INVALID, 0, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB7:
		workspace_select_view(self,
			viewindex_make_all(psy_INDEX_INVALID, 1, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB8:
		workspace_select_view(self,
			viewindex_make_all(psy_INDEX_INVALID, 2, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB9:
		workspace_select_view(self,
			viewindex_make_all(psy_INDEX_INVALID, 3, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	default:
		break;
	}
}

void workspace_app_title(Workspace* self, char* rv_title, uintptr_t max_len)
{	
	static const char* app = "Psycle Modular Music Creation Studio";
	
	assert(self);

	rv_title[0] = '\n';
	if (self->song) {
		psy_Path path;

		psy_path_init(&path, self->song->filename);
		psy_snprintf(rv_title, max_len,
			(workspace_song_modified(self))
			? "[%s.%s *] %s %s"
			: "[%s.%s] %s %s",
			psy_path_name(&path), psy_path_ext(&path),
			app, " (" PSYCLE__VERSION ")");
		psy_path_dispose(&path);
	} else {
		psy_snprintf(rv_title, max_len,	"%s %s", app, " (" PSYCLE__VERSION ")");
	}
}

const char* workspace_song_title(const Workspace* self)
{
	assert(self);

	if (self->song) {
		return psy_audio_song_title(self->song);
	}
	return "";		
}

void workspace_select_start_view(Workspace* self)
{	
	assert(self);
	
	workspace_select_view(self, generalconfig_start_view(
		psycleconfig_general(&self->config)));
}


/*

bool playrow;
psy_audio_SequencerPlayMode restoreplaymode;
psy_dsp_big_beat_t restorenumplaybeats;
bool restoreloop;	

self->playrow = FALSE;
self->restoreplaymode = psy_audio_SEQUENCERPLAYMODE_PLAYALL;
self->restorenumplaybeats = 4.0;
self->restoreloop = TRUE;

// idle
if (self->playrow && !psy_audio_player_playing(&self->player)) {
	self->playrow = FALSE;
	psy_audio_sequencer_set_play_mode(&self->player.sequencer,
		self->restoreplaymode);
	psy_audio_sequencer_set_num_play_beats(&self->player.sequencer,
		self->restorenumplaybeats);
	if (self->restoreloop) {
		psy_audio_sequencer_loop(&self->player.sequencer);
	} else {
		psy_audio_sequencer_stop_loop(&self->player.sequencer);
	}
	self->player.sequencer.playtrack = psy_INDEX_INVALID;
}	
	
case CMD_IMM_PLAYROWTRACK: {
psy_dsp_big_beat_t playposition = 0;
psy_audio_SequencePatternEntry* entry;

psy_audio_exclusivelock_enter();
psy_audio_player_stop(&self->workspace.player);
entry = psy_audio_sequenceposition_entry(&self->workspace.sequenceselection.editposition);
playposition = (entry ? entry->offset : 0) +
	(psy_dsp_big_beat_t)self->workspace.patterneditposition.offset;
self->restoreplaymode = psy_audio_sequencer_play_mode(&self->workspace.player.sequencer);
self->restorenumplaybeats = self->workspace.player.sequencer.numplaybeats;
self->restoreloop = psy_audio_sequencer_looping(&self->workspace.player.sequencer);
psy_audio_sequencer_stop_loop(&self->workspace.player.sequencer);
psy_audio_sequencer_set_play_mode(&self->workspace.player.sequencer,
	psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS);
psy_audio_sequencer_set_num_play_beats(&self->workspace.player.sequencer,
	psy_audio_player_bpl(&self->workspace.player));
self->workspace.player.sequencer.playtrack = self->workspace.patterneditposition.track;
psy_audio_player_setposition(&self->workspace.player, playposition);
psy_audio_player_start(&self->workspace.player);
self->playrow = TRUE;
psy_audio_exclusivelock_leave();
	break;  }
case CMD_IMM_PLAYROWPATTERN: {
psy_dsp_big_beat_t playposition = 0;
psy_audio_SequencePatternEntry* entry;

psy_audio_exclusivelock_enter();
psy_audio_player_stop(&self->workspace.player);
entry = psy_audio_sequenceposition_entry(&self->workspace.sequenceselection.editposition);
playposition = (entry ? entry->offset : 0) +
	(psy_dsp_big_beat_t)self->workspace.patterneditposition.offset;
self->restoreplaymode = psy_audio_sequencer_play_mode(&self->workspace.player.sequencer);
self->restorenumplaybeats = self->workspace.player.sequencer.numplaybeats;
self->restoreloop = psy_audio_sequencer_looping(&self->workspace.player.sequencer);
psy_audio_sequencer_stop_loop(&self->workspace.player.sequencer);
psy_audio_sequencer_set_play_mode(&self->workspace.player.sequencer,
	psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS);
psy_audio_sequencer_set_num_play_beats(&self->workspace.player.sequencer,
	psy_audio_player_bpl(&self->workspace.player));
psy_audio_player_setposition(&self->workspace.player, playposition);
psy_audio_player_start(&self->workspace.player);
self->playrow = TRUE;
psy_audio_exclusivelock_leave();
	break; }
case CMD_IMM_PLAYFROMPOS: {
psy_dsp_big_beat_t playposition = 0;
psy_audio_SequencePatternEntry* entry;

entry = psy_audio_sequenceposition_entry(&self->workspace.sequenceselection.editposition);
playposition = (entry ? entry->offset : 0) +
	(psy_dsp_big_beat_t)self->workspace.patterneditposition.offset;
psy_audio_player_setposition(&self->workspace.player, playposition);
psy_audio_player_start(&self->workspace.player);
	break; }
*/
