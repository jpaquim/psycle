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

/* HostSequencerTime */

void hostsequencertime_init(HostSequencerTime* self)
{
	psy_audio_sequencecursor_init(&self->currplaycursor);
	psy_audio_sequencecursor_init(&self->lastplaycursor);
	self->lastplaycursor.linecache = psy_INDEX_INVALID;	
	self->currplaying = FALSE;	
}

void hostsequencertime_set_play_cursor(HostSequencerTime* self,
	psy_audio_SequenceCursor cursor)
{
	self->currplaycursor = cursor;
}

void hostsequencertime_update_last_play_cursor(HostSequencerTime* self)
{
	self->lastplaycursor = self->currplaycursor;
}

/* Workspace */

/* audio */
static void workspace_initplayer(Workspace*);
static void workspace_initaudio(Workspace*);
static void workspace_initplugincatcherandmachinefactory(Workspace*);
static void workspace_initsignals(Workspace*);
static void workspace_disposesignals(Workspace*);
static void workspace_wait_for_driver_configure_load(Workspace*);
static void workspace_update_play_status(Workspace*);
/* config */
static void workspace_configvisual(Workspace*);
static void workspace_set_song(Workspace*, psy_audio_Song*, int flag);
static void workspace_on_load_progress(Workspace*, psy_audio_Song*, int progress);
static void workspace_on_scan_progress(Workspace*, psy_audio_PluginCatcher*,
	int progress);
/* configure actions */
static void workspace_on_default_control_skin(Workspace*);
static void workspace_on_load_control_skin(Workspace*);
static void workspace_on_add_event_driver(Workspace*);
static void workspace_on_remove_event_driver(Workspace*);
static void workspace_on_edit_event_driver_configuration(Workspace*);
static void workspace_set_default_font(Workspace*, psy_Property*);
static void workspace_set_app_theme(Workspace*);
static void workspace_updatesavepoint(Workspace*);
/* machinecallback */
static psy_audio_MachineFactory* workspace_on_machinefactory(Workspace*);
static bool workspace_on_machine_file_select_load(Workspace*, char filter[],
	char inoutName[]);
static bool workspace_on_machine_file_select_save(Workspace*, char filter[],
	char inoutName[]);
static void workspace_on_machine_file_select_directory(Workspace*);
static void workspace_on_machine_terminal_output(Workspace*, const char* text);
static bool workspace_on_machine_edit_resize(Workspace*, psy_audio_Machine* sender,
	intptr_t w, intptr_t h);
static void workspace_on_machine_bus_changed(Workspace*, psy_audio_Machine* sender);
static const char* workspace_on_machine_language(Workspace*);
/* terminal */
static void workspace_on_terminal_warning(Workspace*,
	psy_audio_SongFile* sender, const char* text);
static void workspace_on_terminal_output(Workspace*,
	psy_audio_SongFile* sender, const char* text);
static void workspace_on_terminal_error(Workspace*,
	psy_audio_SongFile* sender, const char* text);
/* pluginscan */
static void workspace_on_scan_start(Workspace*, psy_audio_PluginCatcher* sender);
static void workspace_on_scan_end(Workspace*, psy_audio_PluginCatcher* sender);
static void workspace_on_scan_file(Workspace*, psy_audio_PluginCatcher* sender,
	const char* path, int type);
static void workspace_on_scan_task_start(Workspace*,
	psy_audio_PluginCatcher* sender, psy_audio_PluginScanTask*);
static void workspace_on_plugin_cache_changed(Workspace*,
	psy_audio_PluginCatcher* sender);
/* MachineCallback VTable */
static psy_audio_MachineCallbackVtable machinecallback_vtable;
static bool machinecallback_vtable_initialized = FALSE;

static void psy_audio_machinecallbackvtable_init(Workspace* self)
{
	assert(self);

	if (!machinecallback_vtable_initialized) {
		machinecallback_vtable = *self->machinecallback.vtable;
		machinecallback_vtable.machinefactory =
			(fp_mcb_machinefactory)
			workspace_on_machinefactory;
		machinecallback_vtable.fileselect_load =
			(fp_mcb_fileselect_load)
			workspace_on_machine_file_select_load;
		machinecallback_vtable.fileselect_save =
			(fp_mcb_fileselect_save)
			workspace_on_machine_file_select_save;
		machinecallback_vtable.fileselect_directory =
			(fp_mcb_fileselect_directory)
			workspace_on_machine_file_select_directory;
		machinecallback_vtable.editresize =
			(fp_mcb_editresize)
			workspace_on_machine_edit_resize;
		machinecallback_vtable.buschanged =
			(fp_mcb_buschanged)
			workspace_on_machine_bus_changed;
		machinecallback_vtable.output =
			(fp_mcb_output)
			workspace_on_machine_terminal_output;
		machinecallback_vtable_initialized = TRUE;
	}
}

void workspace_init(Workspace* self, psy_ui_Component* main)
{
	assert(self);

	psy_audio_machinecallback_init(&self->machinecallback);
	psy_audio_machinecallbackvtable_init(self);
	self->machinecallback.vtable = &machinecallback_vtable;
	psy_lock_init(&self->pluginscanlock);
	psy_audio_init();
	self->record_tweaks = FALSE;	
	self->main = main;
	self->filename = psy_strdup(PSYCLE_UNTITLED);	
	self->undo_save_point = 0;
	self->gearvisible = FALSE;
	self->machines_undo_save_point = 0;	
	self->terminalstyleid = STYLE_TERM_BUTTON;
	self->restoreview = viewindex_make(VIEW_ID_MACHINEVIEW,
		SECTION_ID_MACHINEVIEW_WIRES, psy_INDEX_INVALID, psy_INDEX_INVALID);
	self->playrow = FALSE;
	self->restoreplaymode = psy_audio_SEQUENCERPLAYMODE_PLAYALL;
	self->restorenumplaybeats = 4.0;
	self->restoreloop = TRUE;
	self->driverconfigloading = FALSE;	
	self->modified_without_undo = FALSE;	
	self->song_has_file = FALSE;
	self->paramviews = NULL;
	self->terminal_output = NULL;
	hostsequencertime_init(&self->host_sequencer_time);
	psy_thread_init(&self->driverconfigloadthread);
	psy_thread_init(&self->pluginscanthread);
	viewhistory_init(&self->view_history);
	psy_playlist_init(&self->playlist);
	workspace_initplugincatcherandmachinefactory(self);
	psycleconfig_init(&self->config, &self->player, &self->machinefactory);
	psy_audio_plugincatcher_setdirectories(&self->plugincatcher,
		psycleconfig_directories(&self->config)->directories);
	psy_audio_plugincatcher_load(&self->plugincatcher);
	self->song = psy_audio_song_allocinit(&self->machinefactory);
	psy_audio_machinecallback_setsong(&self->machinecallback, self->song);	
	psy_audio_sequencepaste_init(&self->sequencepaste);
	psy_undoredo_init(&self->undoredo);
	workspace_initsignals(self);
	workspace_initplayer(self);	
	eventdriverconfig_registereventdrivers(&self->config.input);
	inputhandler_init(&self->inputhandler, &self->player, NULL, NULL);	
}

void workspace_initplugincatcherandmachinefactory(Workspace* self)
{
	assert(self);

	psy_audio_plugincatcher_init(&self->plugincatcher);
	psy_signal_connect(&self->plugincatcher.signal_changed, self,
		workspace_on_plugin_cache_changed);
	psy_signal_connect(&self->plugincatcher.signal_scanprogress, self,
		workspace_on_scan_progress);
	psy_signal_connect(&self->plugincatcher.signal_scanfile, self,
		workspace_on_scan_file);
	psy_signal_connect(&self->plugincatcher.signal_scanstart, self,
		workspace_on_scan_start);
		psy_signal_connect(&self->plugincatcher.signal_scanend, self,
		workspace_on_scan_end);
	psy_signal_connect(&self->plugincatcher.signal_taskstart, self,
		workspace_on_scan_task_start);
	psy_audio_machinefactory_init(&self->machinefactory,
		&self->machinecallback,
		&self->plugincatcher);
	self->filescanned = 0;
	self->scanstart = 0;
	self->scanend = 0;
	self->scantaskstart = 0;
	self->plugincachechanged = 0;
	self->scanprogresschanged = 0;
	self->scanfilename = NULL;
	self->scanplugintype = psy_audio_UNDEFINED;
}

void workspace_initsignals(Workspace* self)
{
	assert(self);

	psy_signal_init(&self->signal_octavechanged);
	psy_signal_init(&self->signal_songchanged);		
	psy_signal_init(&self->signal_play_line_changed);
	psy_signal_init(&self->signal_play_status_changed);
	psy_signal_init(&self->signal_gotocursor);	
	psy_signal_init(&self->signal_load_progress);
	psy_signal_init(&self->signal_scanprogress);
	psy_signal_init(&self->signal_scanfile);
	psy_signal_init(&self->signal_scanstart);
	psy_signal_init(&self->signal_scanend);
	psy_signal_init(&self->signal_scantaskstart);
	psy_signal_init(&self->signal_plugincachechanged);
	psy_signal_init(&self->signal_beforesavesong);	
	psy_signal_init(&self->signal_viewselected);	
	psy_signal_init(&self->signal_parametertweak);
	psy_signal_init(&self->signal_status_out);
	psy_signal_init(&self->signal_togglegear);
	psy_signal_init(&self->signal_machineeditresize);
	psy_signal_init(&self->signal_buschanged);
	psy_signal_init(&self->signal_gearselect);
}

void workspace_dispose(Workspace* self)
{
	assert(self);

	psy_thread_dispose(&self->driverconfigloadthread);
	psy_thread_dispose(&self->pluginscanthread);
	workspace_save_styleconfiguration(self);
	psy_audio_player_dispose(&self->player);
	psy_audio_song_deallocate(self->song);
	self->song = NULL;
	psycleconfig_dispose(&self->config);
	free(self->filename);
	self->filename = NULL;
	psy_audio_plugincatcher_save(&self->plugincatcher);
	psy_audio_plugincatcher_dispose(&self->plugincatcher);
	psy_audio_machinefactory_dispose(&self->machinefactory);
	psy_undoredo_dispose(&self->undoredo);
	viewhistory_dispose(&self->view_history);
	workspace_disposesignals(self);	
	psy_audio_sequencepaste_dispose(&self->sequencepaste);
	psy_playlist_dispose(&self->playlist);
	psy_audio_dispose();
	psy_lock_dispose(&self->pluginscanlock);	
	free(self->scanfilename);
	self->scanfilename = NULL;	
	inputhandler_dispose(&self->inputhandler);
}

void workspace_save_styleconfiguration(Workspace* self)
{
	psy_Path path;
	const psy_Property* styleconfig;

	assert(self);

	styleconfig = psy_ui_styles_configuration(&psy_ui_defaults()->styles);
	if (styleconfig && !psy_property_empty(styleconfig)) {
		psy_path_init(&path, NULL);
		psy_path_set_prefix(&path, dirconfig_config_dir(&self->config.directories));
		if (psy_ui_defaults()->styles.theme == psy_ui_DARKTHEME) {
			psy_path_set_name(&path, PSYCLE_DARKSTYLES_INI);
		} else {
			psy_path_set_name(&path, PSYCLE_LIGHTSTYLES_INI);
		}
		/* propertiesio_save(styleconfig, &path); */
		psy_path_dispose(&path);
	}
}

void workspace_disposesignals(Workspace* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_octavechanged);
	psy_signal_dispose(&self->signal_songchanged);		
	psy_signal_dispose(&self->signal_play_line_changed);
	psy_signal_dispose(&self->signal_play_status_changed);
	psy_signal_dispose(&self->signal_gotocursor);
	psy_signal_dispose(&self->signal_load_progress);
	psy_signal_dispose(&self->signal_scanprogress);
	psy_signal_dispose(&self->signal_scanfile);
	psy_signal_dispose(&self->signal_scanstart);
	psy_signal_dispose(&self->signal_scanend);
	psy_signal_dispose(&self->signal_scantaskstart);
	psy_signal_dispose(&self->signal_plugincachechanged);
	psy_signal_dispose(&self->signal_beforesavesong);	
	psy_signal_dispose(&self->signal_viewselected);	
	psy_signal_dispose(&self->signal_parametertweak);
	psy_signal_dispose(&self->signal_status_out);	
	psy_signal_dispose(&self->signal_togglegear);
	psy_signal_dispose(&self->signal_machineeditresize);
	psy_signal_dispose(&self->signal_buschanged);
	psy_signal_dispose(&self->signal_gearselect);
}

void workspace_clear_sequence_paste(Workspace* self)
{
	assert(self);

	psy_audio_sequencepaste_clear(&self->sequencepaste);
}

void workspace_initplayer(Workspace* self)
{
	assert(self);
		
	psy_audio_player_init(&self->player, self->song,
		/* mainwindow platform handle for directx driver */
		(self->main)
		? psy_ui_component_platform(self->main)
		: NULL);
	psy_audio_machinecallback_setplayer(&self->machinecallback, &self->player);
	psy_audio_eventdrivers_setcmds(&self->player.eventdrivers,
		cmdproperties_create());
	psy_audio_luabind_setplayer(&self->player);
	workspace_initaudio(self);
	metronomeconfig_onchanged(&self->config.metronome, NULL);
}

void workspace_initaudio(Workspace* self)
{
	audioconfig_driverconfigure_section(&self->config.audio);
	eventdriverconfig_update_active(&self->config.input);
	eventdriverconfig_show_active(&self->config.input, 0);	
}

void workspace_updatemetronome(Workspace* self)
{
	self->player.sequencer.metronome_event.note =
		metronomeconfig_note(&self->config.metronome);
	self->player.sequencer.metronome_event.mach =
		(uint8_t)
		metronomeconfig_machine(&self->config.metronome);
}

void workspace_configvisual(Workspace* self)
{
	psy_ui_Font font;
	psy_ui_FontInfo fontinfo;

	assert(self);

	psy_ui_fontinfo_init_string(&fontinfo,
		psycleconfig_defaultfontstr(&self->config));
	psy_ui_font_init(&font, &fontinfo);
	fontinfo = psy_ui_font_fontinfo(&font);	
	psy_ui_replacedefaultfont(self->main, &font);
	psy_ui_font_dispose(&font);
}

const char* workspace_driverpath(Workspace* self)
{
	return audioconfig_driverpath(&self->config.audio);
}

#if defined DIVERSALIS__OS__MICROSOFT
static unsigned int __stdcall pluginscanthread(void* context)
#else
static unsigned int pluginscanthread(void* context)
#endif
{
	Workspace* self;

	assert(context);

	self = (Workspace*)context;
	printf("Enter Pluginscan Thread function\n");
	psy_audio_plugincatcher_scan(&self->plugincatcher);
	self->filescanned = 0;
	self->scantaskstart = 0;
	free(self->scanfilename);
	self->scanfilename = NULL;
	return 0;
}

void workspace_scan_plugins(Workspace* self)
{
	assert(self);	

	if (!psy_audio_plugincatcher_scanning(&self->plugincatcher)) {
		free(self->scanfilename);
		self->scanfilename = NULL;
		self->filescanned = 0;
		self->plugincachechanged = 0;
		self->scanstart = 0;
		self->scantaskstart = 0;
		free(self->scanfilename);
		self->scanplugintype = psy_audio_UNDEFINED;		
		psy_thread_start(&self->pluginscanthread, self, pluginscanthread);
	}
}

void workspace_config_changed(Workspace* self, psy_Property* property,
	uintptr_t* rebuild_level)
{
	bool worked;
	assert(self && property);

	worked = TRUE;
	workspace_wait_for_driver_configure_load(self);
	switch (psy_property_id(property)) {	
	case PROPERTY_ID_REGENERATEPLUGINCACHE:
		workspace_scan_plugins(self);
		break;
	case PROPERTY_ID_LOADCONTROLSKIN:
		workspace_on_load_control_skin(self);
		break;
	case PROPERTY_ID_DEFAULTCONTROLSKIN:
		workspace_on_default_control_skin(self);
		break;
	case PROPERTY_ID_ADDEVENTDRIVER:
		workspace_on_add_event_driver(self);
		*rebuild_level = 1;
		break;
	case PROPERTY_ID_REMOVEEVENTDRIVER:
		workspace_on_remove_event_driver(self);
		*rebuild_level = 1;
		break;
	case PROPERTY_ID_EVENTDRIVERCONFIGDEFAULTS:
		eventdriverconfig_reset(&self->config.input);
		*rebuild_level = 1;
		break;
	case PROPERTY_ID_EVENTDRIVERCONFIGLOAD:
		eventdriverconfig_load(&self->config.input);
		*rebuild_level = 1;
		break;
	case PROPERTY_ID_EVENTDRIVERCONFIGKEYMAPSAVE:
		eventdriverconfig_save(&self->config.input);
		break;
	case PROPERTY_ID_DEFAULTFONT:
		workspace_set_default_font(self, property);
		break;
	case PROPERTY_ID_APPTHEME:
		workspace_set_app_theme(self);
		*rebuild_level = 1;
		break;
	case PROPERTY_ID_DEFAULTLINES:
		if (psy_property_item_int(property) > 0) {
			psy_audio_pattern_setdefaultlines((uintptr_t)
				psy_property_item_int(property));
		}
		break;
	case PROPERTY_ID_DRAWVUMETERS:
		if (psy_property_item_bool(property)) {
			psy_audio_player_set_vu_meter_mode(&self->player, VUMETER_RMS);
		} else {
			psy_audio_player_set_vu_meter_mode(&self->player, VUMETER_NONE);
		}
	case PROPERTY_ID_ADDCONTROLLERMAP: {
		psy_audio_MidiConfigGroup group;

		psy_audio_midiconfiggroup_init(&group, psy_audio_MIDICONFIG_GT_CUSTOM,
			1);
		psy_audio_midiconfig_addcontroller(
			&self->player.midiinput.midiconfig, group);
		midiviewconfig_make_controllers(
			psycleconfig_midi(&self->config));
		*rebuild_level = 1;
		break; }
	case PROPERTY_ID_REMOVECONTROLLERMAP: {
		psy_Property* group;
		intptr_t id;

		group = psy_property_parent(property);
		if (group) {
			id = psy_property_at_int(group, "id", -1);
			if (id != -1) {
				psy_audio_midiconfig_removecontroller(
					&self->player.midiinput.midiconfig, id);
				midiviewconfig_make_controller_save(
					psycleconfig_midi(&self->config));
				midiviewconfig_make_controllers(
					psycleconfig_midi(&self->config));
				*rebuild_level = 1;
			}
		}
		break; }
	case PROPERTY_ID_ACTIVEEVENTDRIVERS:
		eventdriverconfig_show_active(
			&self->config.input, psy_property_item_int(property));
		*rebuild_level = 1;
		break;
	case PROPERTY_ID_PATTERNDISPLAY:
		patternviewconfig_select_pattern_display(&self->config.patview,
			(PatternDisplayMode)psy_property_item_int(property));
		break;
	default: {				
		if (psy_property_in_section(property,
				self->config.audio.driverconfigure)) {
			audioconfig_on_edit_audio_driver_configuration(&self->config.audio,
				psycleconfig_audio_enabled(&self->config));
			audioconfig_driverconfigure_section(&self->config.audio);
			*rebuild_level = 0;
		} else if (psy_property_in_section(property,
				self->config.input.eventdriverconfigure)) {
			workspace_on_edit_event_driver_configuration(self);
		} else if (psy_property_in_section(property,
				self->config.midi.controllers)) {
			psy_audio_player_midi_configure(&self->player,
				self->config.midi.controllers, FALSE);
			midiviewconfig_make_controller_save(
				psycleconfig_midi(&self->config));
			*rebuild_level = 1;
		} else {
			worked = FALSE;
		}		
		break; }
	}
	if (!worked) {
		*rebuild_level = psycleconfig_notify_changed(&self->config, property);		
	}
}

void workspace_on_load_control_skin(Workspace* self)
{
	psy_ui_OpenDialog opendialog;

	psy_ui_opendialog_init_all(&opendialog, 0,
		"Load Dial Bitmap",
		"Control Skins|*.psc|Bitmaps|*.bmp", "psc",
		dirconfig_skins(&self->config.directories));
	if (psy_ui_opendialog_execute(&opendialog)) {
		machineparamconfig_set_dial_bpm(psycleconfig_macparam(&self->config),
			psy_path_full(psy_ui_opendialog_path(&opendialog)));		
	}
	psy_ui_opendialog_dispose(&opendialog);
}

void workspace_on_default_control_skin(Workspace* self)
{
	psycleconfig_reset_control_skin(&self->config);
}

void workspace_on_add_event_driver(Workspace* self)
{
	psy_Property* installeddriver;

	installeddriver = psy_property_at(self->config.input.eventinputs,
		"installeddriver", PSY_PROPERTY_TYPE_CHOICE);
	if (installeddriver) {
		psy_Property* choice;

		choice = psy_property_at_choice(installeddriver);
		if (choice) {
			psy_EventDriver* driver;
			psy_Property* activedrivers;

			driver = psy_audio_player_loadeventdriver(&self->player,
				psy_property_item_str(choice));
			if (driver) {
				psy_eventdriver_setcmddef(driver,
					self->player.eventdrivers.cmds);
			}
			eventdriverconfig_update_active(&self->config.input);
			activedrivers = psy_property_at(self->config.input.eventinputs,
				"activedrivers", PSY_PROPERTY_TYPE_CHOICE);
			if (activedrivers) {
				psy_property_setitem_int(activedrivers,
					psy_audio_player_numeventdrivers(&self->player) - 1);
				eventdriverconfig_show_active(
					&self->config.input,
					psy_property_item_int(activedrivers));
			}
		}
	}
}

void workspace_on_remove_event_driver(Workspace* self)
{
	psy_audio_player_remove_event_driver(&self->player,
		psy_property_item_int(self->config.input.activedrivers));
	eventdriverconfig_update_active(&self->config.input);
	if (psy_property_item_int(self->config.input.activedrivers) > 0) {
		psy_property_setitem_int(self->config.input.activedrivers,
			psy_property_item_int(self->config.input.activedrivers) - 1);
	}
	eventdriverconfig_show_active(&self->config.input,
		psy_property_item_int(self->config.input.activedrivers));
}

void workspace_on_edit_event_driver_configuration(Workspace* self)
{
	psy_Property* driversection;
	psy_EventDriver* driver;

	driver = psy_audio_player_eventdriver(&self->player,
	psy_property_item_int(self->config.input.activedrivers));
	driversection = psy_property_find(
		self->config.input.eventdriverconfigure,
		psy_property_key(psy_eventdriver_configuration(driver)),
		PSY_PROPERTY_TYPE_NONE);
	if (driversection) {
		psy_audio_player_restarteventdriver(&self->player,
			psy_property_item_int(self->config.input.activedrivers),
			driversection);
	}
}

void workspace_set_default_font(Workspace* self, psy_Property* property)
{
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_FONT) {
		psy_ui_Font font;
		psy_ui_FontInfo fontinfo;

		psy_ui_fontinfo_init_string(&fontinfo, psy_property_item_str(property));
		psy_ui_font_init(&font, &fontinfo);
		psy_ui_app_set_default_font(psy_ui_app(), &font);
		psy_ui_font_dispose(&font);
	}
}

void workspace_set_app_theme(Workspace* self)
{
	psy_Property* choice;

	assert(self);

	choice = psy_property_at_choice(self->config.apptheme);
	if (choice) {
		psy_ui_ThemeMode theme;

		theme = (psy_ui_ThemeMode)(psy_property_item_int(choice));		
		/* reset styles */		
		psy_ui_defaults_inittheme(psy_ui_appdefaults(), theme, TRUE);
		init_host_styles(&psy_ui_appdefaults()->styles, theme);
		machineviewconfig_load(&self->config.macview);
		machineparamconfig_update_styles(&self->config.macparam);
		patternviewconfig_write_styles(&self->config.patview);		
		psy_ui_defaults_load_theme(psy_ui_appdefaults(),
			dirconfig_config_dir(&self->config.directories),
			theme);		
		if (psy_ui_app()->imp) {
			psy_ui_app()->imp->vtable->dev_onappdefaultschange(
				psy_ui_app()->imp);
		}
		psy_ui_appzoom_update_base_fontsize(&psy_ui_app()->zoom,
			psy_ui_defaults_font(&psy_ui_app()->defaults));
		psy_ui_notify_style_update(psy_ui_app()->main);
	}
}

void workspace_newsong(Workspace* self)
{
	assert(self);
	
	psy_strreset(&self->filename, "Untitled.psy");	
	workspace_set_song(self, psy_audio_song_allocinit(&self->machinefactory),
		WORKSPACE_NEWSONG);
	workspace_select_view(self, viewindex_make(VIEW_ID_MACHINEVIEW, 0,
		psy_INDEX_INVALID, 0));
}

void workspace_load_song_fileselect(Workspace* self)
{
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

void workspace_load_song(Workspace* self, const char* filename, bool play)
{
	psy_audio_Song* song;

	assert(self);

	song = psy_audio_song_allocinit(&self->machinefactory);	
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
		psy_audio_machinecallback_setsong(&self->machinecallback, song);
		if (psy_audio_songfile_load(&songfile, filename) != PSY_OK) {
			psy_audio_song_deallocate(song);
			workspace_output_error(self, songfile.serr);
			workspace_output_error(self, "\n");			
			psy_audio_songfile_dispose(&songfile);
			play = FALSE;
		} else {
			psy_strreset(&self->filename, filename);
			workspace_set_song(self, song, WORKSPACE_LOADSONG);
			psy_audio_songfile_dispose(&songfile);
			if (generalconfig_saving_recent_songs(psycleconfig_general(
					workspace_conf(self)))) {
				psy_playlist_add(&self->playlist, filename);
			}
			psy_audio_songfile_dispose(&songfile);
		}
		workspace_clear_undo(self);
		workspace_output(self, "ready\n");
		if (play) {
			psy_audio_player_stop(&self->player);
			psy_audio_player_setposition(&self->player, 0.0);
			psy_audio_player_start(&self->player);
		}
	}
}

void workspace_on_load_progress(Workspace* self, psy_audio_Song* sender,
	int progress)
{
	assert(self);

	psy_signal_emit(&self->signal_load_progress, self, 1, progress);
}

void workspace_set_song(Workspace* self, psy_audio_Song* song, int flag)
{
	assert(self);

	if (self->song != song) {
		psy_audio_Song* old_song;
		ViewIndex view;	
		psy_audio_SequenceCursor cursor;

		old_song = self->song;		
		psy_audio_player_stop(&self->player);
		psy_audio_player_setemptysong(&self->player);		
		workspace_clear_sequence_paste(self);
		workspace_clear_undo(self);
		self->modified_without_undo = FALSE;		
		view = viewhistory_current(&self->view_history);
		viewhistory_clear(&self->view_history);
		viewhistory_add(&self->view_history, view);		
		psy_audio_exclusivelock_enter();
		psy_audio_machinecallback_setsong(&self->machinecallback, song);
		self->song = song;
		psy_audio_player_setsong(&self->player, self->song);					
		if (flag == WORKSPACE_NEWSONG) {
			self->song_has_file = FALSE;
			psy_audio_player_set_sampler_index(&self->player,
				seqeditconfig_machine(&self->config.seqedit));
		} else if (flag == WORKSPACE_LOADSONG) {
			self->song_has_file = TRUE;
		}
		psy_audio_exclusivelock_leave();
		psy_signal_emit(&self->signal_songchanged, self, 0);
		psy_signal_emit(&self->song->patterns.signal_numsongtrackschanged, self,
			1, self->song->patterns.songtracks);
		psy_audio_song_deallocate(old_song);
		psy_audio_sequenceselection_clear(&self->song->sequence.sequenceselection);
		psy_audio_sequenceselection_select_first(&self->song->sequence.sequenceselection,
			psy_audio_orderindex_make(0, 0));		
		psy_audio_sequencecursor_init(&cursor);
		psy_audio_sequence_set_cursor(&self->song->sequence, cursor);
	}
}

bool workspace_save_song_fileselect(Workspace* self)
{
	psy_ui_SaveDialog dialog;
	int success;

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
	psy_signal_emit(&self->signal_beforesavesong, self, 1, &songfile);
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
	psy_signal_emit(&self->signal_beforesavesong, self, 1, &songfile);

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
	psy_signal_emit(&self->signal_beforesavesong, self, 1, &songfile);
	if (psy_audio_songfile_export_ly_file(&songfile, path)) {
		workspace_output_error(self, songfile.serr);
	} else {
		workspace_updatesavepoint(self);
	}
	psy_audio_songfile_dispose(&songfile);
	workspace_output(self, "ready\n");
}

void workspace_save_song(Workspace* self, const char* path)
{
	if (psy_strlen(path) > 0) {
		psy_audio_SongFile songfile;

		assert(self);

		psy_audio_songfile_init_song(&songfile, self->song);
		psy_signal_emit(&self->signal_beforesavesong, self, 1, &songfile);
		if (psy_audio_songfile_save(&songfile, path)) {
			workspace_output_error(self, songfile.serr);
		} else {
			workspace_updatesavepoint(self);
			self->song_has_file = TRUE;
		}
		psy_audio_songfile_dispose(&songfile);
		workspace_output(self, "ready\n");
	}
}

void workspace_updatesavepoint(Workspace* self)
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
	driverkey = audioconfig_driverkey(psycleconfig_audio(&self->config));
	audioconfig_makeconfiguration_driverkey(psycleconfig_audio(&self->config),
		driverkey);	
	psy_propertyreader_load(&propertyreader);
	if (keyboardmiscconfig_patdefaultlines(
			&self->config.misc) > 0) {
		psy_audio_pattern_setdefaultlines(keyboardmiscconfig_patdefaultlines(
			&self->config.misc));
	}
	languageconfig_update_language(&self->config.language);
	{
		psy_Property* driversection = NULL;

		psy_audio_player_loaddriver(&self->player, audioconfig_driverpath(
			&self->config.audio), NULL /*no config*/,
			FALSE /* don't open yet*/);
		if (psy_audiodriver_configuration(self->player.driver)) {
			driversection = psy_property_find(
				self->config.audio.driverconfigurations,
				psy_property_key(
					psy_audiodriver_configuration(self->player.driver)),
				PSY_PROPERTY_TYPE_NONE);
		}
		if (psycleconfig_audio_enabled(&self->config)) {
			/* psy_audio_player_restartdriver(&self->player, driversection); */
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
	psy_audio_player_midi_configure(&self->player, self->config.midi.controllers,
		TRUE /* use controllerdata */);
	midiviewconfig_make_controllers(
		psycleconfig_midi(&self->config));	
	workspace_configvisual(self);
	if (compatconfig_loadnewblitz(psycleconfig_compat(
			workspace_conf(self)))) {
		psy_audio_machinefactory_loadnewgamefxandblitzifversionunknown(
			&self->machinefactory);
	} else {
		psy_audio_machinefactory_loadoldgamefxandblitzifversionunknown(
			&self->machinefactory);
	}
	workspace_set_default_font(self, self->config.defaultfont);
	workspace_set_app_theme(self);	
	psycleconfig_notifyall_changed(&self->config);
	psy_path_dispose(&path);
	psy_propertyreader_load(&propertyreader);
	workspace_postload_driver_configurations(self);
	psy_audio_player_set_sampler_index(&self->player,
		seqeditconfig_machine(&self->config.seqedit));
}

void workspace_start_audio(Workspace* self)
{
	if (self->player.driver) {
		psy_audiodriver_open(self->player.driver);
		psy_audio_player_stop_threads(&self->player);
		psy_audio_player_start_threads(&self->player,
			audioconfig_numthreads(&self->config.audio));
	}
}

void workspace_save_configuration(Workspace* self)
{
	psy_Path path;
	psy_PropertyWriter propertywriter;

	assert(self);

	workspace_wait_for_driver_configure_load(self);
	psy_path_init(&path, NULL);
	psy_path_set_prefix(&path, dirconfig_config_dir(&self->config.directories));
	psy_path_set_name(&path, PSYCLE_INI);
	eventdriverconfig_make(&self->config.input);
	midiviewconfig_make_controller_save(psycleconfig_midi(&self->config));
	printf("save psycle configuration: %s\n", psy_path_full(&path));
	psy_propertywriter_init(&propertywriter, &self->config.config,
		psy_path_full(&path));
	psy_propertywriter_save(&propertywriter);
	psy_propertywriter_dispose(&propertywriter);	
	psy_path_dispose(&path);
}

void workspace_wait_for_driver_configure_load(Workspace* self)
{
	while (self->driverconfigloading) {
#ifdef DIVERSALIS__OS__MICROSOFT
		Sleep(50);
#endif
#ifdef DIVERSALIS__OS__UNIX
		usleep(50000);
#endif
	}
}

#if defined DIVERSALIS__OS__MICROSOFT
static unsigned int __stdcall driverconfigloadthread(void* context)
#else
static unsigned int driverconfigloadthread(void* context)
#endif
{
	Workspace* self;
	psy_Path path;
	psy_PropertyReader propertyreader;

	assert(context);
	
	self = (Workspace*)context;
	psy_path_init(&path, NULL);
	psy_path_set_prefix(&path, dirconfig_config_dir(&self->config.directories));
	psy_path_set_name(&path, PSYCLE_INI);	
	audioconfig_make_driver_configurations(psycleconfig_audio(&self->config), TRUE);
	psy_propertyreader_init(&propertyreader, &self->config.config,
		psy_path_full(&path));	
	psy_propertyreader_load(&propertyreader);
	psy_propertyreader_dispose(&propertyreader);	
	psy_path_dispose(&path);
	self->driverconfigloading = FALSE;
	if (psycleconfig_audio_enabled(&self->config)) {
		workspace_start_audio(self);
		workspace_output_status(self, psy_ui_translate("msg.audiostarted"));
	}
	return 0;
}

void workspace_postload_driver_configurations(Workspace* self)
{
	assert(self);

	if (!self->driverconfigloading) {
		psy_Thread thread;

		self->driverconfigloading = TRUE;
		workspace_output_status(self, psy_ui_translate("msg.audiostarting"));
		psy_thread_start(&thread, self, driverconfigloadthread);
	}
}

psy_Property* workspace_recentsongs(Workspace* self)
{
	assert(self);

	return self->playlist.recentsongs;
}

void workspace_load_recentsongs(Workspace* self)
{
	assert(self);

	psy_playlist_load(&self->playlist);
}

void workspace_set_octave(Workspace* self, uint8_t octave)
{
	assert(self);

	psy_audio_player_set_octave(&self->player, octave);
	psy_signal_emit(&self->signal_octavechanged, self, 1, octave);
}

uint8_t workspace_octave(Workspace* self)
{
	assert(self);

	return psy_audio_player_octave(&self->player);
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

	return psy_audio_plugincatcher_hascache(&self->plugincatcher);
}

psy_audio_PluginCatcher* workspace_plugincatcher(Workspace* self)
{
	assert(self);

	return &self->plugincatcher;
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

	workspace_update_play_status(self);		
	if (self->scanstart) {
		psy_lock_enter(&self->pluginscanlock);
		psy_signal_emit(&self->signal_scanstart, self, 0);
		self->scanstart = 0;
		psy_lock_leave(&self->pluginscanlock);
	}
	if (self->scanend) {
		psy_lock_enter(&self->pluginscanlock);
		psy_signal_emit(&self->signal_scanend, self, 0);
		self->scanend = 0;
		psy_lock_leave(&self->pluginscanlock);
	}
	if (self->scantaskstart) {
		psy_lock_enter(&self->pluginscanlock);
		psy_signal_emit(&self->signal_scantaskstart, self, 1,
			&self->lastscantask);
		self->scantaskstart = 0;
		psy_lock_leave(&self->pluginscanlock);
	}
	if (self->filescanned) {
		psy_lock_enter(&self->pluginscanlock);
		psy_signal_emit(&self->signal_scanfile, self, 2, self->scanfilename,
			self->scanplugintype);
		self->filescanned = 0;
		free(self->scanfilename);
		self->scanfilename = NULL;
		psy_lock_leave(&self->pluginscanlock);
	}
	if (self->scanprogresschanged) {
		assert(self);
		psy_signal_emit(&self->signal_scanprogress, self, 1,
			self->scanprogress);
		self->scanprogresschanged = 0;
	}
	if (self->plugincachechanged) {
		psy_lock_enter(&self->pluginscanlock);
		psy_signal_emit(&self->signal_plugincachechanged, self, 0);
		self->plugincachechanged = 0;
		psy_lock_leave(&self->pluginscanlock);
	}
	psy_audio_player_idle(&self->player);
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
}

void workspace_update_play_status(Workspace* self)
{
	bool play_status_changed;
	
	assert(self);

	play_status_changed = psy_audio_player_playing(&self->player) !=
		hostsequencertime_playing(&self->host_sequencer_time);
	if (hostsequencertime_playing(&self->host_sequencer_time) || play_status_changed) {
		psy_audio_exclusivelock_enter();
		hostsequencertime_set_play_cursor(&self->host_sequencer_time,
			psy_audio_sequencer_play_cursor(&self->player.sequencer));
		if (play_status_changed) {
			self->host_sequencer_time.currplaying =
				psy_audio_player_playing(&self->player);
			if (hostsequencertime_playing(&self->host_sequencer_time)) {
				self->host_sequencer_time.lastplaycursor.linecache =
					psy_INDEX_INVALID;
			}
		}
		psy_audio_exclusivelock_leave();
		if (hostsequencertime_play_line_changed(&self->host_sequencer_time)) {			
			if (keyboardmiscconfig_following_song(&self->config.misc)) {
				bool restore_prevented;

				restore_prevented = viewhistory_prevented(&self->view_history);
				viewhistory_prevent(&self->view_history);
				psy_audio_sequence_set_cursor(psy_audio_song_sequence(self->song),
					self->host_sequencer_time.currplaycursor);
				if (!restore_prevented) {
					viewhistory_enable(&self->view_history);
				}
			}
			psy_signal_emit(&self->signal_play_line_changed, self, 0);
		}
		if (play_status_changed) {
			psy_signal_emit(&self->signal_play_status_changed, self, 0);
		}
		hostsequencertime_update_last_play_cursor(&self->host_sequencer_time);
	}
}

void workspace_show_parameters(Workspace* self, uintptr_t machineslot)
{
	assert(self);

	if (self->song) {
		paramviews_show(self->paramviews, machineslot);
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

void workspace_parameter_tweak(Workspace* self, int slot, uintptr_t tweak,
	float value)
{
	assert(self);

	psy_signal_emit(&self->signal_parametertweak, self, 3, slot, tweak,
		value);
}

void workspace_record_tweaks(Workspace* self)
{
	assert(self);

	self->record_tweaks = TRUE;
}

void workspace_stop_record_tweaks(Workspace* self)
{
	assert(self);

	self->record_tweaks = FALSE;
}

bool workspace_recording_tweaks(Workspace* self)
{
	assert(self);

	return self->record_tweaks;
}

void workspace_on_view_changed(Workspace* self, ViewIndex view)
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
		self->terminal_output->vtable->output(self->terminal_output->context, text);
	}
}

void workspace_output_warning(Workspace* self, const char* text)
{
	assert(self);

	if (self->terminal_output) {
		if (self->terminalstyleid == STYLE_TERM_BUTTON) {
			self->terminalstyleid = STYLE_TERM_BUTTON_WARNING;
		}
		self->terminal_output->vtable->warn(self->terminal_output->context, text);
	}
}

void workspace_output_error(Workspace* self, const char* text)
{
	assert(self);

	if (self->terminal_output) {
		self->terminalstyleid = STYLE_TERM_BUTTON_ERROR;
		self->terminal_output->vtable->error(self->terminal_output->context, text);
	}
}

void workspace_output_status(Workspace* self, const char* text)
{
	assert(self);

	psy_signal_emit(&self->signal_status_out, self, 1, text);
}

void workspace_toggle_gear(Workspace* self)
{
	assert(self);
	self->gearvisible = !self->gearvisible;
	psy_signal_emit(&self->signal_togglegear, self, 0);
}

bool workspace_gear_visible(const Workspace* self)
{
	assert(self);

	return self->gearvisible;
}

bool workspace_song_modified(const Workspace* self)
{
	assert(self);

	return self->modified_without_undo ||
		psy_list_size(self->undoredo.undo) != self->undo_save_point ||
		psy_list_size(self->song->machines.undoredo.undo) != self->machines_undo_save_point;
}

void workspace_mark_song_modified(Workspace* self)
{
	assert(self);

	self->modified_without_undo = TRUE;
}

void workspace_goto_cursor(Workspace* self, psy_audio_SequenceCursor cursor)
{
	psy_signal_emit(&self->signal_gotocursor, self, 1, &cursor);
}

/* Host specialization of machine callbacks */
void workspace_on_machine_terminal_output(Workspace* self, const char* text)
{
	assert(self);

	workspace_output_error(self, text);	
}

static bool workspace_on_machine_edit_resize(Workspace* self,
	psy_audio_Machine* sender, intptr_t w, intptr_t h)
{
	assert(self);

	psy_signal_emit(&self->signal_machineeditresize, self, 3, sender, w, h);
	return TRUE;
}

static void workspace_on_machine_bus_changed(Workspace* self, psy_audio_Machine* sender)
{
	assert(self);

	psy_signal_emit(&self->signal_buschanged, self, 1, sender);
}

static const char* workspace_on_machine_language(Workspace* self)
{
	return psy_translator_langid(psy_ui_translator());
}

bool workspace_on_machine_file_select_load(Workspace* self, char filter[], char inoutname[])
{
	bool success;
	psy_ui_OpenDialog dialog;

	assert(self);

	psy_ui_opendialog_init_all(&dialog, NULL, "file.plugin-fileload", filter,
		"", dirconfig_vsts32(psycleconfig_directories(&self->config)));
	success = psy_ui_opendialog_execute(&dialog);
	psy_snprintf(inoutname, _MAX_PATH, "%s",
		psy_path_full(psy_ui_opendialog_path(&dialog)));
	psy_ui_opendialog_dispose(&dialog);
	return success;
}

bool workspace_on_machine_file_select_save(Workspace* self, char filter[],
	char inoutname[])
{
	bool success;
	psy_ui_SaveDialog dialog;

	assert(self);

	psy_ui_savedialog_init_all(&dialog, NULL, "file.plugin-filesave", filter,
		"", dirconfig_vsts32(psycleconfig_directories(&self->config)));
	success = psy_ui_savedialog_execute(&dialog);
	psy_snprintf(inoutname, _MAX_PATH, "%s",
		psy_path_full(psy_ui_savedialog_path(&dialog)));
	psy_ui_savedialog_dispose(&dialog);
	return success;
}

void workspace_on_machine_file_select_directory(Workspace* self)
{
	assert(self);
	/* todo */
}

psy_audio_MachineFactory* workspace_on_machinefactory(Workspace* self)
{
	assert(self);

	return &self->machinefactory;
}

void workspace_multi_select_gear(Workspace* self, psy_List* machinelist)
{
	assert(self);

	psy_signal_emit(&self->signal_gearselect, self, 1, machinelist);
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
			viewindex_make(VIEW_ID_MACHINEVIEW,
			SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_APPEND, 0));
		break;
	case CMD_IMM_EDITMACHINE:
		if (workspace_current_view(self).id != VIEW_ID_MACHINEVIEW) {
			workspace_select_view(self, viewindex_make(VIEW_ID_MACHINEVIEW,
				psy_INDEX_INVALID, psy_INDEX_INVALID, 0));
		} else {
			if (workspace_current_view(self).section == SECTION_ID_MACHINEVIEW_WIRES) {
				workspace_select_view(self, viewindex_make(
					VIEW_ID_MACHINEVIEW,
					SECTION_ID_MACHINEVIEW_STACK,
					psy_INDEX_INVALID, 0));
			} else {
				workspace_select_view(self,
					viewindex_make(VIEW_ID_MACHINEVIEW,
					SECTION_ID_MACHINEVIEW_WIRES, psy_INDEX_INVALID, 0));
			}
		}
		break;
	case CMD_IMM_HELP:
		workspace_select_view(self, viewindex_make(
			VIEW_ID_HELPVIEW, SECTION_ID_HELPVIEW_HELP, 0,
			psy_INDEX_INVALID));
		break;
	case CMD_IMM_EDITPATTERN:
		workspace_select_view(self,
			viewindex_make(
				VIEW_ID_PATTERNVIEW, psy_INDEX_INVALID,
				psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_EDITINSTR:		
		workspace_select_view(self, viewindex_make(
			VIEW_ID_INSTRUMENTSVIEW, psy_INDEX_INVALID,
			psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_EDITSAMPLE:
		workspace_select_view(self, viewindex_make(
			VIEW_ID_SAMPLESVIEW, psy_INDEX_INVALID,
			psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_EDITWAVE:
		workspace_select_view(self, viewindex_make(
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
		psycleconfig_enableaudio(&self->config,
			!psycleconfig_audio_enabled(&self->config));
		break;
	case CMD_IMM_SETTINGS:
		workspace_select_view(self, viewindex_make(VIEW_ID_SETTINGSVIEW, 0, 0, 0));
		break;
	case CMD_IMM_LOADSONG:
		if (keyboardmiscconfig_savereminder(&self->config.misc) &&
				workspace_song_modified(self)) {
			workspace_select_view(self, viewindex_make(VIEW_ID_CHECKUNSAVED,
				0, CONFIRM_LOAD, psy_INDEX_INVALID));
		} else {
			workspace_load_song_fileselect(self);
		}
		break;
	case CMD_IMM_INFOMACHINE:
		if (self->song) {
			workspace_show_parameters(self,
				psy_audio_machines_selected(&self->song->machines));
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
	case CMD_IMM_PLAYROWTRACK: {
		/*psy_dsp_big_beat_t playposition = 0;
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
		break; */ }
	case CMD_IMM_PLAYROWPATTERN: {
		/*psy_dsp_big_beat_t playposition = 0;
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
		break; */ }
	case CMD_IMM_PLAYFROMPOS: {
		/*psy_dsp_big_beat_t playposition = 0;
		psy_audio_SequencePatternEntry* entry;

		entry = psy_audio_sequenceposition_entry(&self->workspace.sequenceselection.editposition);
		playposition = (entry ? entry->offset : 0) +
			(psy_dsp_big_beat_t)self->workspace.patterneditposition.offset;
		psy_audio_player_setposition(&self->workspace.player, playposition);
		psy_audio_player_start(&self->workspace.player);
		break; */}
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
		if (self->song && psy_audio_song_numsongtracks(self->song) >=
				(uintptr_t)(cmdid - CMD_COLUMN_0)) {
			self->song->sequence.cursor.track = (cmdid - CMD_COLUMN_0);			
			psy_audio_sequence_set_cursor(psy_audio_song_sequence(self->song),
				self->song->sequence.cursor);
		}
		break;
	case CMD_IMM_TAB1:
		workspace_select_view(self,
			viewindex_make(0, psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB2:
		workspace_select_view(self,
			viewindex_make(1, psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB3:
		workspace_select_view(self,
			viewindex_make(2, psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB4:
		workspace_select_view(self,
			viewindex_make(3, psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB5:
		workspace_select_view(self,
			viewindex_make(4, psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB6:
		workspace_select_view(self,
			viewindex_make(psy_INDEX_INVALID, 0, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB7:
		workspace_select_view(self,
			viewindex_make(psy_INDEX_INVALID, 1, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB8:
		workspace_select_view(self,
			viewindex_make(psy_INDEX_INVALID, 2, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	case CMD_IMM_TAB9:
		workspace_select_view(self,
			viewindex_make(psy_INDEX_INVALID, 3, psy_INDEX_INVALID, psy_INDEX_INVALID));
		break;
	default:
		break;
	}
}

void workspace_on_scan_progress(Workspace* self, psy_audio_PluginCatcher* sender,
	int progress)
{
	psy_lock_enter(&self->pluginscanlock);
	self->scanprogresschanged = 1;
	self->scanprogress = progress;
	psy_lock_leave(&self->pluginscanlock);
}

void workspace_on_scan_file(Workspace* self, psy_audio_PluginCatcher* sender,
	const char* path, int type)
{
	assert(self);

	psy_lock_enter(&self->pluginscanlock);
	self->filescanned = 1;
	psy_strreset(&self->scanfilename, path);
	self->scanplugintype = type;
	psy_lock_leave(&self->pluginscanlock);
}

void workspace_on_scan_start(Workspace* self, psy_audio_PluginCatcher* sender)
{
	assert(self);

	psy_lock_enter(&self->pluginscanlock);
	self->scanstart = 1;	
	psy_lock_leave(&self->pluginscanlock);
}

void workspace_on_scan_end(Workspace* self, psy_audio_PluginCatcher* sender)
{
	assert(self);

	psy_lock_enter(&self->pluginscanlock);
	self->scanend = 1;	
	psy_lock_leave(&self->pluginscanlock);
}

void workspace_on_scan_task_start(Workspace* self, psy_audio_PluginCatcher* sender,
	psy_audio_PluginScanTask* task)
{
	assert(self);
	assert(task);

	psy_lock_enter(&self->pluginscanlock);
	self->scantaskstart = 1;
	self->lastscantask = *task;
	psy_lock_leave(&self->pluginscanlock);
}

void workspace_on_plugin_cache_changed(Workspace* self,
	psy_audio_PluginCatcher* sender)
{
	assert(self);

	psy_lock_enter(&self->pluginscanlock);
	self->plugincachechanged = 1;
	psy_lock_leave(&self->pluginscanlock);
}

void workspace_app_title(Workspace* self, char* rv_title, uintptr_t max_len)
{
	psy_Path path;

	assert(self);

	rv_title[0] = '\n';
	psy_path_init(&path, self->filename);	
	psy_snprintf(rv_title, max_len,
		(workspace_song_modified(self))
		? "[%s.%s *] %s %s"
		: "[%s.%s] %s %s",		
		psy_path_name(&path), psy_path_ext(&path),
		"Psycle Modular Music Creation Studio",
		" (" PSYCLE__VERSION ")");	
	psy_path_dispose(&path);	
}

const char* workspace_song_title(const Workspace* self)
{
	assert(self);

	return ((self->song)
		? psy_audio_song_title(self->song)
		: "");	
}

void workspace_select_start_view(Workspace* self)
{	
	assert(self);
	
	workspace_select_view(self, generalconfig_start_view(
		psycleconfig_general(&self->config)));
}
