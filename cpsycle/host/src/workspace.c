// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/psyconf.h"

#include "workspace.h"
// local
#include "cmdproperties.h"
#include "cmdsgeneral.h"
#include "sequencehostcmds.h"
#include "styles.h"
#include "trackergridstate.h"
// file
#include <dir.h>
// dsp
#include <operations.h>
// audio
#include <exclusivelock.h>
#include <kbddriver.h>
#include <songio.h>
// ui
#include <uiopendialog.h>
#include <uisavedialog.h>
#ifdef DIVERSALIS__OS__MICROSOFT
// For directx drivers to get the win32 mainwindow handle
#include <imps/win32/uiwincomponentimp.h>
// thread for pluginscan
#include <windows.h>
#include <process.h>
#endif
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#endif

// audio
static void workspace_initplayer(Workspace*);
static void workspace_initaudio(Workspace*);
static void workspace_initplugincatcherandmachinefactory(Workspace*);
static void workspace_initsignals(Workspace*);
static void workspace_disposesignals(Workspace*);
// config
static void workspace_configvisual(Workspace*);
static void workspace_setsong(Workspace*, psy_audio_Song*, int flag);
static void workspace_onloadprogress(Workspace*, psy_audio_Song*, int progress);
static void workspace_onscanprogress(Workspace*, psy_audio_PluginCatcher*, int progress);
static void workspace_onpatternviewconfigurationchanged(Workspace*,
	PatternViewConfig* sender);
// configure actions
static void workspace_onloadskin(Workspace*);
static void workspace_ondefaultskin(Workspace*);
static void workspace_ondefaultcontrolskin(Workspace*);
static void workspace_onloadcontrolskin(Workspace*);
static void workspace_onaddeventdriver(Workspace*);
static void workspace_onremoveeventdriver(Workspace*);
static void workspace_onediteventdriverconfiguration(Workspace*);
static void workspace_setdefaultfont(Workspace*, psy_Property*);
static void workspace_setapptheme(Workspace*, psy_Property*);
static void workspace_updatemetronome(Workspace*);
/// Machinecallback
static psy_audio_MachineFactory* onmachinefactory(Workspace*);
static bool onmachinefileselectload(Workspace*, char filter[],
	char inoutName[]);
static bool onmachinefileselectsave(Workspace*, char filter[],
	char inoutName[]);
static void onmachinefileselectdirectory(Workspace*);
static void onmachineterminaloutput(Workspace*, const char* text);
static bool onmachineeditresize(Workspace*, psy_audio_Machine* sender,
	intptr_t w, intptr_t h);
static void onmachinebuschanged(Workspace*, psy_audio_Machine* sender);
static const char* onmachinelanguage(Workspace*);
/// terminal
static void workspace_onterminalwarning(Workspace*,
	psy_audio_SongFile* sender, const char* text);
static void workspace_onterminaloutput(Workspace*,
	psy_audio_SongFile* sender, const char* text);
static void workspace_onterminalerror(Workspace*,
	psy_audio_SongFile* sender, const char* text);
// EventDriver Input Handler
static void workspace_oneventdriverinput(Workspace*, psy_EventDriver* sender);
static void workspace_onscanfile(Workspace*, psy_audio_PluginCatcher* sender,
	const char* path, int type);
static void workspace_onscantaskstart(Workspace*, psy_audio_PluginCatcher* sender,
	psy_audio_PluginScanTask*);
static void workspace_onplugincachechanged(Workspace* self,
	psy_audio_PluginCatcher* sender);


// MachineCallback VTable
static psy_audio_MachineCallbackVtable machinecallback_vtable;
static bool machinecallback_vtable_initialized = FALSE;

static void psy_audio_machinecallbackvtable_init(Workspace* self)
{
	assert(self);

	if (!machinecallback_vtable_initialized) {
		machinecallback_vtable = *self->machinecallback.vtable;
		machinecallback_vtable.machinefactory =
			(fp_mcb_machinefactory)
			onmachinefactory;
		machinecallback_vtable.fileselect_load =
			(fp_mcb_fileselect_load)
			onmachinefileselectload;
		machinecallback_vtable.fileselect_save =
			(fp_mcb_fileselect_save)
			onmachinefileselectsave;
		machinecallback_vtable.fileselect_directory =
			(fp_mcb_fileselect_directory)
			onmachinefileselectdirectory;
		machinecallback_vtable.editresize =
			(fp_mcb_editresize)
			onmachineeditresize;
		machinecallback_vtable.buschanged =
			(fp_mcb_buschanged)
			onmachinebuschanged;
		machinecallback_vtable.output =
			(fp_mcb_output)
			onmachineterminaloutput;
		machinecallback_vtable_initialized = TRUE;
	}
}

void workspace_init(Workspace* self, void* mainhandle)
{
	assert(self);

	psy_audio_machinecallback_init(&self->machinecallback);
	psy_audio_machinecallbackvtable_init(self);
	self->machinecallback.vtable = &machinecallback_vtable;
	psy_audio_lock_init(&self->pluginscanlock);
	psy_audio_init();
	self->fontheight = 12;
	self->cursorstep = 1;
	self->followsong = 0;
	self->recordtweaks = 0;
	self->startpage = FALSE;
	self->mainhandle = mainhandle;
	self->filename = psy_strdup(PSYCLE_UNTITLED);
	self->lastentry = 0;
	self->undosavepoint = 0;
	self->gearvisible = FALSE;
	self->machines_undosavepoint = 0;
	self->navigating = FALSE;
	self->restoreview.id = VIEW_ID_MACHINEVIEW;
	self->restoreview.section = SECTION_ID_MACHINEVIEW_WIRES;
	self->patternsinglemode = TRUE;
	self->playrow = FALSE;
	self->restoreplaymode = psy_audio_SEQUENCERPLAYMODE_PLAYALL;
	self->restorenumplaybeats = 4.0;
	self->restoreloop = TRUE;
	self->errorstrs = NULL;
	viewhistory_init(&self->viewhistory);
	psy_playlist_init(&self->playlist);
	workspace_initplugincatcherandmachinefactory(self);
	psycleconfig_init(&self->config, &self->player,
		&self->machinefactory);
	psy_signal_connect(&self->config.patview.signal_changed,
		self, workspace_onpatternviewconfigurationchanged);
	psy_audio_plugincatcher_setdirectories(&self->plugincatcher,
		psycleconfig_directories(&self->config)->directories);
	psy_audio_plugincatcher_load(&self->plugincatcher);
	self->song = psy_audio_song_allocinit(&self->machinefactory);
	psy_audio_machinecallback_setsong(&self->machinecallback, self->song);
	psy_audio_sequenceselection_init(&self->sequenceselection);
	psy_audio_sequencepaste_init(&self->sequencepaste);
	psy_undoredo_init(&self->undoredo);
	workspace_initsignals(self);
	workspace_initplayer(self);
	eventdriverconfig_registereventdrivers(&self->config.input);
	psy_audio_patterncursor_init(&self->patterneditposition);
	psy_audio_pattern_init(&self->patternpaste);
	psy_signal_connect(&self->player.eventdrivers.signal_input, self,
		workspace_oneventdriverinput);
}

void workspace_initplugincatcherandmachinefactory(Workspace* self)
{
	assert(self);

	psy_audio_plugincatcher_init(&self->plugincatcher);
	psy_signal_connect(&self->plugincatcher.signal_changed, self,
		workspace_onplugincachechanged);
	psy_signal_connect(&self->plugincatcher.signal_scanprogress, self,
		workspace_onscanprogress);
	psy_signal_connect(&self->plugincatcher.signal_scanfile, self,
		workspace_onscanfile);
	psy_signal_connect(&self->plugincatcher.signal_taskstart, self,
		workspace_onscantaskstart);
	psy_audio_machinefactory_init(&self->machinefactory,
		&self->machinecallback,
		&self->plugincatcher);
	self->filescanned = 0;
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
	psy_signal_init(&self->signal_configchanged);
	psy_signal_init(&self->signal_changecontrolskin);
	psy_signal_init(&self->signal_patterncursorchanged);
	psy_signal_init(&self->signal_gotocursor);
	// psy_signal_init(&self->signal_sequenceselectionchanged);
	psy_signal_init(&self->signal_loadprogress);
	psy_signal_init(&self->signal_scanprogress);
	psy_signal_init(&self->signal_scanfile);
	psy_signal_init(&self->signal_scantaskstart);
	psy_signal_init(&self->signal_plugincachechanged);
	psy_signal_init(&self->signal_beforesavesong);
	psy_signal_init(&self->signal_showparameters);
	psy_signal_init(&self->signal_viewselected);
	psy_signal_init(&self->signal_focusview);
	psy_signal_init(&self->signal_parametertweak);
	psy_signal_init(&self->signal_terminal_error);
	psy_signal_init(&self->signal_terminal_out);
	psy_signal_init(&self->signal_terminal_warning);
	psy_signal_init(&self->signal_status_out);
	psy_signal_init(&self->signal_followsongchanged);
	psy_signal_init(&self->signal_togglegear);
	psy_signal_init(&self->signal_floatsection);
	psy_signal_init(&self->signal_docksection);
	psy_signal_init(&self->signal_machineeditresize);
	psy_signal_init(&self->signal_buschanged);
	psy_signal_init(&self->signal_gearselect);
}

void workspace_dispose(Workspace* self)
{
	assert(self);

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
	viewhistory_dispose(&self->viewhistory);
	workspace_disposesignals(self);
	psy_audio_pattern_dispose(&self->patternpaste);
	psy_audio_sequenceselection_dispose(&self->sequenceselection);
	psy_audio_sequencepaste_dispose(&self->sequencepaste);
	psy_playlist_dispose(&self->playlist);
	psy_audio_dispose();
	psy_audio_lock_dispose(&self->pluginscanlock);
	free(self->scanfilename);
	psy_list_deallocate(&self->errorstrs, NULL);
}

void workspace_save_styleconfiguration(Workspace* self)
{
	psy_Path path;
	const psy_Property* styleconfig;

	assert(self);

	styleconfig = psy_ui_styles_configuration(&psy_ui_defaults()->styles);
	if (styleconfig && !psy_property_empty(styleconfig)) {
		psy_path_init(&path, NULL);
		psy_path_setprefix(&path, dirconfig_config(&self->config.directories));
		if (psy_ui_defaults()->styles.theme == psy_ui_DARKTHEME) {
			psy_path_setname(&path, PSYCLE_DARKSTYLES_INI);
		} else {
			psy_path_setname(&path, PSYCLE_LIGHTSTYLES_INI);
		}
		//propertiesio_save(styleconfig, &path);
		psy_path_dispose(&path);
	}
}

void workspace_disposesignals(Workspace* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_octavechanged);
	psy_signal_dispose(&self->signal_songchanged);
	psy_signal_dispose(&self->signal_configchanged);
	psy_signal_dispose(&self->signal_changecontrolskin);
	psy_signal_dispose(&self->signal_patterncursorchanged);
	psy_signal_dispose(&self->signal_gotocursor);
	psy_signal_dispose(&self->signal_loadprogress);
	psy_signal_dispose(&self->signal_scanprogress);
	psy_signal_dispose(&self->signal_scanfile);
	psy_signal_dispose(&self->signal_scantaskstart);
	psy_signal_dispose(&self->signal_plugincachechanged);
	psy_signal_dispose(&self->signal_beforesavesong);
	psy_signal_dispose(&self->signal_showparameters);
	psy_signal_dispose(&self->signal_viewselected);
	psy_signal_dispose(&self->signal_focusview);
	psy_signal_dispose(&self->signal_parametertweak);
	psy_signal_dispose(&self->signal_terminal_error);
	psy_signal_dispose(&self->signal_terminal_out);
	psy_signal_dispose(&self->signal_terminal_warning);
	psy_signal_dispose(&self->signal_status_out);
	psy_signal_dispose(&self->signal_followsongchanged);
	psy_signal_dispose(&self->signal_togglegear);
	psy_signal_dispose(&self->signal_floatsection);
	psy_signal_dispose(&self->signal_docksection);
	psy_signal_dispose(&self->signal_machineeditresize);
	psy_signal_dispose(&self->signal_buschanged);
	psy_signal_dispose(&self->signal_gearselect);
}

void workspace_clearsequencepaste(Workspace* self)
{
	assert(self);

	psy_audio_sequencepaste_clear(&self->sequencepaste);
}

void workspace_initplayer(Workspace* self)
{
	assert(self);

#ifdef DIVERSALIS__OS__MICROSOFT
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
	psy_audio_player_init(&self->player, self->song,
		((psy_ui_win_ComponentImp*)(self->mainhandle->imp))->hwnd);
#else
	psy_audio_player_init(&self->player, self->song, 0);
#endif
#else
	psy_audio_player_init(&self->player, self->song, 0);
#endif
	psy_audio_machinecallback_setplayer(&self->machinecallback, &self->player);
	psy_audio_eventdrivers_setcmds(&self->player.eventdrivers,
		cmdproperties_create());
	workspace_initaudio(self);
}

void workspace_initaudio(Workspace* self)
{
	audioconfig_driverconfigure_section(&self->config.audio);
	eventdriverconfig_updateactiveeventdriverlist(&self->config.input);
	eventdriverconfig_showactiveeventdriverconfig(&self->config.input, 0);
	workspace_updatemetronome(self);
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
	self->fontheight = fontinfo.lfHeight;
	psy_ui_replacedefaultfont(self->mainhandle, &font);
	psy_ui_font_dispose(&font);
}

const char* workspace_driverpath(Workspace* self)
{
	return audioconfig_driverpath(&self->config.audio);
}

#ifdef DIVERSALIS__OS__MICROSOFT
static void pluginscanthread(void* context)
{
	Workspace* self;

	assert(context);

	self = context;
	psy_audio_plugincatcher_scan(&self->plugincatcher);
	self->filescanned = 0;
	self->scantaskstart = 0;
	free(self->scanfilename);
	self->scanfilename = NULL;
}
#endif

void workspace_scanplugins(Workspace* self)
{
	assert(self);

	if (!psy_audio_plugincatcher_scanning(&self->plugincatcher)) {
        free(self->scanfilename);
		self->scanfilename = NULL;
		self->filescanned = 0;
		self->plugincachechanged = 0;
		self->scantaskstart = 0;
		free(self->scanfilename);
		self->scanplugintype = psy_audio_UNDEFINED;
#ifdef DIVERSALIS__OS__MICROSOFT

		_beginthread(pluginscanthread, 0, self);
#else
		psy_audio_plugincatcher_scan(&self->plugincatcher);
		self->filescanned = 0;
		self->plugincachechanged = 0;
		self->scantaskstart = 0;
        psy_signal_emit(&self->signal_plugincachechanged, self, 0);
#endif
	}
}

void workspace_configurationchanged(Workspace* self, psy_Property* property,
	uintptr_t* rebuild_level)
{
	bool worked;
	assert(self && property);

	worked = TRUE;
	switch (psy_property_id(property)) {
	case PROPERTY_ID_REGENERATEPLUGINCACHE:
		workspace_scanplugins(self);
		break;
	case PROPERTY_ID_LOADSKIN:
		workspace_onloadskin(self);
		break;
	case PROPERTY_ID_DEFAULTSKIN:
		workspace_ondefaultskin(self);
		break;
	case PROPERTY_ID_LOADCONTROLSKIN:
		workspace_onloadcontrolskin(self);
		break;
	case PROPERTY_ID_DEFAULTCONTROLSKIN:
		workspace_ondefaultcontrolskin(self);
		break;
	case PROPERTY_ID_ADDEVENTDRIVER:
		workspace_onaddeventdriver(self);
		*rebuild_level = 1;
		break;
	case PROPERTY_ID_REMOVEEVENTDRIVER:
		workspace_onremoveeventdriver(self);
		*rebuild_level = 1;
		break;
	case PROPERTY_ID_EVENTDRIVERCONFIGDEFAULTS:
		eventdriverconfig_reseteventdriverconfiguration(&self->config.input);
		*rebuild_level = 1;
		break;
	case PROPERTY_ID_EVENTDRIVERCONFIGLOAD:
		eventdriverconfig_loadeventdriverconfiguration(&self->config.input);
		*rebuild_level = 1;
		break;
	case PROPERTY_ID_EVENTDRIVERCONFIGKEYMAPSAVE:
		eventdriverconfig_saveeventdriverconfiguration(&self->config.input);
		break;
	case PROPERTY_ID_DEFAULTFONT:
		workspace_setdefaultfont(self, property);
		break;
	case PROPERTY_ID_APPTHEME:
		workspace_setapptheme(self, property);
		break;
	case PROPERTY_ID_DEFAULTLINES:
		if (psy_property_item_int(property) > 0) {
			psy_audio_pattern_setdefaultlines((uintptr_t)psy_property_item_int(property));
		}
		break;
	case PROPERTY_ID_DRAWVUMETERS:
		if (psy_property_item_bool(property)) {
			psy_audio_player_setvumetermode(&self->player, VUMETER_RMS);
		} else {
			psy_audio_player_setvumetermode(&self->player, VUMETER_NONE);
		}
	case PROPERTY_ID_ADDCONTROLLERMAP: {
		psy_audio_MidiConfigGroup group;

		psy_audio_midiconfiggroup_init(&group, psy_audio_MIDICONFIG_GT_CUSTOM,
			1);
		psy_audio_midiconfig_addcontroller(
			&self->player.midiinput.midiconfig, group);
		midiviewconfig_makecontrollers(
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
				midiviewconfig_makecontrollersave(
					psycleconfig_midi(&self->config));
				midiviewconfig_makecontrollers(
					psycleconfig_midi(&self->config));
				*rebuild_level = 1;
			}
		}
		break; }
	default: {
			psy_Property* choice;

			choice = (psy_property_ischoiceitem(property))
				? psy_property_parent(property)
				: NULL;
			if (choice && psy_property_id(choice) == PROPERTY_ID_APPTHEME) {
				workspace_setapptheme(self, property);
				worked = TRUE;
			} else if (audioconfig_onpropertychanged(&self->config.audio, property, rebuild_level)) {
				worked = TRUE;
			} else if (languageconfig_onchanged(
				&self->config.language, property)) {
				worked = TRUE;
			} else if (choice && psy_property_id(choice) ==
				PROPERTY_ID_ACTIVEEVENTDRIVERS) {
					eventdriverconfig_showactiveeventdriverconfig(
						&self->config.input, psy_property_item_int(choice));
					*rebuild_level = 1;
			} else if (choice && (psy_property_id(choice) ==
					PROPERTY_ID_PATTERNDISPLAY)) {
				workspace_selectpatterndisplay(self,
					(PatternDisplayMode)psy_property_item_int(choice));
			} else if (psy_property_insection(property,
					self->config.audio.driverconfigure)) {
				audioconfig_oneditaudiodriverconfiguration(&self->config.audio,
					psycleconfig_audioenabled(&self->config));
				audioconfig_driverconfigure_section(&self->config.audio);
				*rebuild_level = 1;
			} else if (psy_property_insection(property,
					self->config.input.eventdriverconfigure)) {
				workspace_onediteventdriverconfiguration(self);
			} else if (psy_property_insection(property, self->config.midi.controllers)) {
				psy_audio_player_midiconfigure(&self->player,
					self->config.midi.controllers, FALSE);
				midiviewconfig_makecontrollersave(
					psycleconfig_midi(&self->config));
				*rebuild_level = 1;
			} else if (psy_property_insection(property, self->config.metronome.metronome)) {
				workspace_updatemetronome(self);
			} else {
				worked = FALSE;
			}
			break;
		}
	}
	if (!worked) {
		psycleconfig_notify_changed(&self->config, property);
		psy_signal_emit(&self->signal_configchanged, self, 1, property);
	}
}

void workspace_onpatternviewconfigurationchanged(Workspace* self, PatternViewConfig* sender)
{
	self->patternsinglemode = patternviewconfig_issinglepatterndisplay(
		&self->config.patview);
}

void workspace_onloadskin(Workspace* self)
{
	psy_ui_OpenDialog opendialog;

	psy_ui_opendialog_init_all(&opendialog, 0,
		"Load Theme",
		"Psycle Display Presets|*.psv", "PSV",
		dirconfig_skins(&self->config.directories));
	if (psy_ui_opendialog_execute(&opendialog)) {
		psycleconfig_loadskin(&self->config,
			psy_ui_opendialog_path(&opendialog));
	}
	psy_ui_opendialog_dispose(&opendialog);
}

void workspace_ondefaultskin(Workspace* self)
{
	psycleconfig_resetskin(&self->config);
}

void workspace_onloadcontrolskin(Workspace* self)
{
	psy_ui_OpenDialog opendialog;

	psy_ui_opendialog_init_all(&opendialog, 0,
		"Load Dial Bitmap",
		"Control Skins|*.psc|Bitmaps|*.bmp", "psc",
		dirconfig_skins(&self->config.directories));
	if (psy_ui_opendialog_execute(&opendialog)) {
		machineparamconfig_setdialbpm(psycleconfig_macparam(&self->config),
			psy_path_full(psy_ui_opendialog_path(&opendialog)));
		psy_signal_emit(&self->signal_changecontrolskin, self, 1,
			psy_path_full(psy_ui_opendialog_path(&opendialog)));
	}
	psy_ui_opendialog_dispose(&opendialog);
}

void workspace_ondefaultcontrolskin(Workspace* self)
{
	psycleconfig_resetcontrolskin(&self->config);
}

void workspace_onaddeventdriver(Workspace* self)
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
				psy_eventdriver_setcmddef(driver, self->player.eventdrivers.cmds);
			}
			eventdriverconfig_updateactiveeventdriverlist(&self->config.input);
			activedrivers = psy_property_at(self->config.input.eventinputs, "activedrivers",
				PSY_PROPERTY_TYPE_CHOICE);
			if (activedrivers) {
				psy_property_setitem_int(activedrivers,
					psy_audio_player_numeventdrivers(&self->player) - 1);
				eventdriverconfig_showactiveeventdriverconfig(
					&self->config.input,
					psy_property_item_int(activedrivers));
			}
		}
	}
}

void workspace_onremoveeventdriver(Workspace* self)
{
	psy_audio_player_removeeventdriver(&self->player,
		psy_property_item_int(self->config.input.activedrivers));
	eventdriverconfig_updateactiveeventdriverlist(&self->config.input);
	if (psy_property_item_int(self->config.input.activedrivers) > 0) {
		psy_property_setitem_int(self->config.input.activedrivers,
			psy_property_item_int(self->config.input.activedrivers) - 1);
	}
	eventdriverconfig_showactiveeventdriverconfig(&self->config.input,
		psy_property_item_int(self->config.input.activedrivers));
}

void workspace_onediteventdriverconfiguration(Workspace* self)
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

void workspace_setdefaultfont(Workspace* self, psy_Property* property)
{
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_FONT) {
		psy_ui_Font font;
		psy_ui_FontInfo fontinfo;

		psy_ui_fontinfo_init_string(&fontinfo, psy_property_item_str(property));
		psy_ui_font_init(&font, &fontinfo);
		psy_ui_app_setdefaultfont(psy_ui_app(), &font);
		psy_ui_font_dispose(&font);
	}
}

void workspace_setapptheme(Workspace* self, psy_Property* property)
{
	psy_Property* choice;

	assert(self);

	choice = psy_property_at_choice(self->config.apptheme);
	if (choice) {
		psy_ui_ThemeMode theme;

		switch (psy_property_item_int(choice)) {
		case psy_ui_LIGHTTHEME:
			theme = psy_ui_LIGHTTHEME;
			break;
		case psy_ui_DARKTHEME:
		default:
			theme = psy_ui_DARKTHEME;
			break;
		}
		// reset host styles
		inithoststyles(&psy_ui_appdefaults()->styles, theme);
		psy_ui_defaults_loadtheme(psy_ui_appdefaults(),
			dirconfig_config(&self->config.directories),
			theme);
		if (psy_ui_app()->imp) {
			psy_ui_app()->imp->vtable->dev_onappdefaultschange(psy_ui_app()->imp);
		}
		psy_ui_appzoom_updatebasefontsize(&psy_ui_app()->zoom,
			psy_ui_defaults_font(&psy_ui_app()->defaults));
		psy_ui_notifystyleupdate(psy_ui_app()->main);
	}
}

void workspace_newsong(Workspace* self)
{
	psy_audio_Song* song;

	assert(self);

	song = psy_audio_song_allocinit(&self->machinefactory);
	psy_strreset(&self->filename, "Untitled.psy");
	workspace_setsong(self, song, WORKSPACE_NEWSONG);
	workspace_selectview(self, VIEW_ID_MACHINEVIEW, 0, 0);
}

void workspace_loadsong_fileselect(Workspace* self)
{
	psy_ui_OpenDialog dialog;

	psy_ui_opendialog_init_all(&dialog, NULL,
		psy_ui_translate("file.loadsong"),
		psy_audio_songfile_loadfilter(),
		psy_audio_songfile_standardloadfilter(),
		dirconfig_songs(psycleconfig_directories(
			workspace_conf(self))));
	if (psy_ui_opendialog_execute(&dialog)) {
		workspace_loadsong(self,
			psy_path_full(psy_ui_opendialog_path(&dialog)),
			generalconfig_playsongafterload(psycleconfig_general(
				workspace_conf(self))));
	}
	psy_ui_opendialog_dispose(&dialog);
}

void workspace_loadsong(Workspace* self, const char* filename, bool play)
{
	psy_audio_Song* song;

	assert(self);

	song = psy_audio_song_allocinit(&self->machinefactory);
	if (song) {
		psy_audio_SongFile songfile;

		psy_signal_connect(&song->signal_loadprogress, self,
			workspace_onloadprogress);
		psy_signal_emit(&song->signal_loadprogress, self, 1, -1);
		psy_audio_songfile_init(&songfile);
		psy_signal_connect(&songfile.signal_warning, self,
			workspace_onterminalwarning);
		psy_signal_connect(&songfile.signal_output, self,
			workspace_onterminaloutput);
		songfile.song = song;
		songfile.file = 0;
		psy_audio_player_setemptysong(&self->player);
		psy_audio_machinecallback_setsong(&self->machinecallback, song);
		if (psy_audio_songfile_load(&songfile, filename) != PSY_OK) {
			psy_audio_song_deallocate(song);
			psy_signal_emit(&self->signal_terminal_error, self, 1,
				songfile.serr);
			psy_signal_emit(&self->signal_terminal_error, self, 1, "\n");
			psy_audio_songfile_dispose(&songfile);
			play = FALSE;
		} else {
			psy_strreset(&self->filename, filename);
			workspace_setsong(self, song, WORKSPACE_LOADSONG);
			psy_audio_songfile_dispose(&songfile);
			if (generalconfig_saverecentsongs(psycleconfig_general(
					workspace_conf(self)))) {
				psy_playlist_add(&self->playlist, filename);
			}
			psy_audio_songfile_dispose(&songfile);
		}
		workspace_clearundo(self);
		psy_signal_emit(&self->signal_terminal_out, self, 1, "ready\n");
		if (play) {
			psy_audio_player_start(&self->player);
		}
	}
}

void workspace_onloadprogress(Workspace* self, psy_audio_Song* sender,
	int progress)
{
	assert(self);

	psy_signal_emit(&self->signal_loadprogress, self, 1, progress);
}

void workspace_setsong(Workspace* self, psy_audio_Song* song, int flag)
{
	assert(self);

	if (self->song != song) {
		psy_audio_Song* oldsong;
		ViewHistoryEntry view;

		oldsong = self->song;
		psy_audio_player_stop(&self->player);
		psy_audio_player_setemptysong(&self->player);
		workspace_clearsequencepaste(self);
		workspace_clearundo(self);
		self->sequenceselection.editposition =
			psy_audio_orderindex_make(0, 0);
		view = viewhistory_currview(&self->viewhistory);
		viewhistory_clear(&self->viewhistory);
		viewhistory_add(&self->viewhistory, view);
		self->lastentry = 0;
		psy_audio_exclusivelock_enter();
		psy_audio_machinecallback_setsong(&self->machinecallback,
			song);
		self->song = song;
		psy_audio_player_setsong(&self->player, self->song);
		psy_audio_exclusivelock_leave();
		psy_signal_emit(&self->signal_songchanged, self, 2, flag, self->song);
		psy_signal_emit(&self->song->patterns.signal_numsongtrackschanged, self,  1,
			self->song->patterns.songtracks);
		psy_audio_song_deallocate(oldsong);
	}
}

bool workspace_savesong_fileselect(Workspace* self)
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
		workspace_savesong(self,
			psy_path_full(psy_ui_savedialog_path(&dialog)));
	}
	psy_ui_savedialog_dispose(&dialog);
	return success;
}

bool workspace_exportsong(Workspace* self)
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
		workspace_exportmodule(self,
			psy_path_full(psy_ui_savedialog_path(&dialog)));
	}
	psy_ui_savedialog_dispose(&dialog);
	return success;
}

void workspace_exportmodule(Workspace* self, const char* path)
{
	psy_audio_SongFile songfile;

	assert(self);

	psy_audio_songfile_init(&songfile);
	songfile.file = 0;
	songfile.song = self->song;
	psy_signal_emit(&self->signal_beforesavesong, self, 1, &songfile);
	if (psy_audio_songfile_exportmodule(&songfile, path)) {
		psy_signal_emit(&self->signal_terminal_error, self, 1,
			songfile.serr);
	} else {
		self->undosavepoint = psy_list_size(self->undoredo.undo);
		self->machines_undosavepoint = psy_list_size(self->undoredo.undo);
	}
	psy_audio_songfile_dispose(&songfile);
	workspace_output(self, "ready\n");
}

bool workspace_exportmidifile_fileselect(Workspace* self)
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
		workspace_exportmidifile(self,
			psy_path_full(psy_ui_savedialog_path(&dialog)));
	}
	psy_ui_savedialog_dispose(&dialog);
	return success;
}

void workspace_exportmidifile(Workspace* self, const char* path)
{
	psy_audio_SongFile songfile;

	assert(self);

	psy_audio_songfile_init(&songfile);
	songfile.file = 0;
	songfile.song = self->song;
	psy_signal_emit(&self->signal_beforesavesong, self, 1, &songfile);

	if (psy_audio_songfile_exportmidifile(&songfile, path)) {
		psy_signal_emit(&self->signal_terminal_error, self, 1,
			songfile.serr);
	} else {
		self->undosavepoint = psy_list_size(self->undoredo.undo);
		self->machines_undosavepoint = psy_list_size(self->undoredo.undo);
	}
	psy_audio_songfile_dispose(&songfile);
	workspace_output(self, "ready\n");
}

void workspace_savesong(Workspace* self, const char* path)
{
	psy_audio_SongFile songfile;

	assert(self);

	psy_audio_songfile_init(&songfile);
	songfile.file = 0;
	songfile.song = self->song;
	psy_signal_emit(&self->signal_beforesavesong, self, 1, &songfile);
	if (psy_audio_songfile_save(&songfile, path)) {
		psy_signal_emit(&self->signal_terminal_error, self, 1,
			songfile.serr);
	} else {
		self->undosavepoint = psy_list_size(self->undoredo.undo);
		self->machines_undosavepoint = psy_list_size(self->undoredo.undo);
	}
	psy_audio_songfile_dispose(&songfile);
	workspace_output(self, "ready\n");
}

psy_Playlist* workspace_playlist(Workspace* self)
{
	assert(self);

	return &self->playlist;
}

void workspace_load_configuration(Workspace* self)
{
	psy_Path path;

	assert(self);

	psy_path_init(&path, NULL);
	psy_path_setprefix(&path, dirconfig_config(&self->config.directories));
	psy_path_setname(&path, PSYCLE_INI);
	propertiesio_load(&self->config.config, &path, 0);
	if (keyboardmiscconfig_patdefaultlines(
			&self->config.misc) > 0) {
		psy_audio_pattern_setdefaultlines(keyboardmiscconfig_patdefaultlines(
			&self->config.misc));
	}
	languageconfig_updatelanguage(&self->config.language);
	{
		psy_Property* driversection = NULL;

		psy_audio_player_loaddriver(&self->player, audioconfig_driverpath(&self->config.audio),
			NULL /*no config*/, FALSE /*do not open yet*/);
		if (psy_audiodriver_configuration(self->player.driver)) {
			driversection = psy_property_find(self->config.audio.driverconfigurations,
				psy_property_key(
					psy_audiodriver_configuration(self->player.driver)),
				PSY_PROPERTY_TYPE_NONE);
		}
		if (psycleconfig_audioenabled(&self->config)) {
			psy_audio_player_restartdriver(&self->player, driversection);
		} else if (self->player.driver) {
			psy_audiodriver_configure(self->player.driver, driversection);
		}
		audioconfig_driverconfigure_section(&self->config.audio);
	}
	eventdriverconfig_configeventdrivers(&self->config.input);
	psy_audio_eventdrivers_restartall(&self->player.eventdrivers);
	eventdriverconfig_updateactiveeventdriverlist(&self->config.input);
	eventdriverconfig_makeeventdriverconfigurations(&self->config.input);
	propertiesio_load(&self->config.config, &path, 0);
	eventdriverconfig_readeventdriverconfigurations(&self->config.input);
	psy_audio_eventdrivers_restartall(&self->player.eventdrivers);
	eventdriverconfig_showactiveeventdriverconfig(&self->config.input,
		eventdriverconfig_curreventdriverconfiguration(&self->config.input));
	psy_audio_player_midiconfigure(&self->player, self->config.midi.controllers,
		TRUE /* use controllerdata */);
	midiviewconfig_makecontrollers(
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
	workspace_setdefaultfont(self, self->config.defaultfont);
	workspace_setapptheme(self, self->config.apptheme);
	psycleconfig_notifyall_changed(&self->config);
	psy_path_dispose(&path);
	self->patternsinglemode =
		patternviewconfig_issinglepatterndisplay(&self->config.patview);
	workspace_updatemetronome(self);
}

void workspace_save_configuration(Workspace* self)
{
	psy_Path path;

	assert(self);

	psy_path_init(&path, NULL);
	psy_path_setprefix(&path, dirconfig_config(&self->config.directories));
	psy_path_setname(&path, PSYCLE_INI);
	eventdriverconfig_makeeventdriverconfigurations(&self->config.input);
	midiviewconfig_makecontrollersave(
		psycleconfig_midi(&self->config));
	printf("save psycle configuration: %s\n", psy_path_full(&path));
	propertiesio_save(&self->config.config, psy_path_full(&path));
	psy_path_dispose(&path);
}

psy_Property* workspace_recentsongs(Workspace* self)
{
	assert(self);

	return self->playlist.recentsongs;
}

void workspace_load_recentsongs(Workspace* self)
{
	psy_playlist_load(&self->playlist);
}

void workspace_save_recentsongs(Workspace* self)
{
	psy_playlist_save(&self->playlist);
}

void workspace_clearrecentsongs(Workspace* self)
{
	psy_playlist_clear(&self->playlist);
}

void workspace_setoctave(Workspace* self, int octave)
{
	assert(self);

	psy_audio_player_setoctave(&self->player, octave);
	psy_signal_emit(&self->signal_octavechanged, self, 1, octave);
}

uintptr_t workspace_octave(Workspace* self)
{
	assert(self);

	return psy_audio_player_octave(&self->player);
}

void workspace_undo(Workspace* self)
{
	assert(self);

	if (workspace_currview(self).id == VIEW_ID_MACHINEVIEW) {
		psy_undoredo_undo(&self->song->machines.undoredo);
	} else {
		psy_undoredo_undo(&self->undoredo);
	}
}

void workspace_redo(Workspace* self)
{
	assert(self);

	if (workspace_currview(self).id == VIEW_ID_MACHINEVIEW) {
		psy_undoredo_redo(&self->song->machines.undoredo);
	} else {
		psy_undoredo_redo(&self->undoredo);
	}
}

bool workspace_currview_hasundo(Workspace* self)
{
	assert(self);

	if (workspace_currview(self).id == VIEW_ID_MACHINEVIEW) {
		return psy_list_size(self->song->machines.undoredo.undo) != 0;
	}
	return psy_list_size(self->undoredo.undo) != 0;
}

bool workspace_currview_hasredo(Workspace* self)
{
	assert(self);

	if (workspace_currview(self).id == VIEW_ID_MACHINEVIEW) {
		return psy_list_size(self->song->machines.undoredo.redo) != self->machines_undosavepoint;
	}
	return psy_list_size(self->undoredo.redo) != self->undosavepoint;
}

void workspace_clearundo(Workspace* self)
{
	psy_undoredo_dispose(&self->undoredo);
	psy_undoredo_init(&self->undoredo);
	self->undosavepoint = 0;
	if (self->song) {
		psy_undoredo_dispose(&self->song->machines.undoredo);
		psy_undoredo_init(&self->song->machines.undoredo);
	}
	self->machines_undosavepoint = 0;
}

void workspace_setpatterncursor(Workspace* self,
	psy_audio_PatternCursor editposition)
{
	assert(self);

	if (!self->patternsinglemode) {
		psy_audio_SequenceEntry* entry;

		entry = psy_audio_sequence_entry(&self->song->sequence,
			self->sequenceselection.editposition);
		if (entry) {
			editposition.seqoffset = entry->offset;
		}
	} else {
		editposition.seqoffset = 0.0;
	}
	self->patterneditposition = editposition;
	self->patterneditposition.line =
		(uintptr_t)(editposition.offset * psy_audio_player_lpb(&self->player));
	psy_signal_emit(&self->signal_patterncursorchanged, self, 0);
}

psy_audio_PatternCursor workspace_patterncursor(Workspace* self)
{
	assert(self);

	return self->patterneditposition;
}

void workspace_setsequenceeditposition(Workspace* self, psy_audio_OrderIndex index)
{
	if (workspace_song(self)) {
		if (index.track < psy_audio_sequence_width(&self->song->sequence)) {
			psy_audio_sequenceselection_seteditposition(&self->sequenceselection,
				index);
			if (!self->navigating) {
				viewhistory_addseqpos(&self->viewhistory,
					self->sequenceselection.editposition.order);
			}
		}
	}
}

psy_audio_OrderIndex workspace_sequenceeditposition(const Workspace* self)
{
	return self->sequenceselection.editposition;
}

ViewHistoryEntry workspace_currview(Workspace* self)
{
	assert(self);

	return viewhistory_currview(&self->viewhistory);
}

void workspace_setcursorstep(Workspace* self, int step)
{
	assert(self);

	self->cursorstep = step;
}

int workspace_cursorstep(Workspace* self)
{
	assert(self);

	return self->cursorstep;
}

int workspace_hasplugincache(const Workspace* self)
{
	assert(self);

	return psy_audio_plugincatcher_hascache(&self->plugincatcher);
}

psy_audio_PluginCatcher* workspace_plugincatcher(Workspace* self)
{
	assert(self);

	return &self->plugincatcher;
}

void workspace_editquantizechange(Workspace* self, int diff) // User Called (Hotkey)
{
	const int total = 17;
	const int nextsel = (total + workspace_cursorstep(self) + diff) % total;
	workspace_setcursorstep(self, nextsel);
}

psy_EventDriver* workspace_kbddriver(Workspace* self)
{
	assert(self);

	return psy_audio_player_kbddriver(&self->player);
}

int workspace_followingsong(Workspace* self)
{
	assert(self);

	return self->followsong;
}

void workspace_followsong(Workspace* self)
{
	assert(self);

	if (self->followsong != 1) {
		self->followsong = 1;
		psy_signal_emit(&self->signal_followsongchanged, self, 0);
	}
}

void workspace_stopfollowsong(Workspace* self)
{
	assert(self);

	if (self->followsong != 0) {
		self->followsong = 0;
		psy_signal_emit(&self->signal_followsongchanged, self, 0);
	}
}

void workspace_idle(Workspace* self)
{
	assert(self);

	if (self->followsong) {
		psy_audio_SequenceTrackIterator it;

		if (psy_audio_player_playing(&self->player)) {
			it = psy_audio_sequence_begin(&self->song->sequence,
				self->song->sequence.tracks,
				psy_audio_player_position(&self->player));
			if (it.sequencentrynode && self->lastentry != it.sequencentrynode->entry) {
				psy_audio_SequenceEntry* entry;
				bool prevented;

				entry = (psy_audio_SequenceEntry*)it.sequencentrynode->entry;
				self->lastentry = (psy_audio_SequenceEntry*)it.sequencentrynode->entry;
				prevented = viewhistory_prevented(&self->viewhistory);
				viewhistory_prevent(&self->viewhistory);
				workspace_setsequenceeditposition(self,
					psy_audio_orderindex_make(
						0, entry->row));
				if (!prevented) {
					viewhistory_enable(&self->viewhistory);
				}
			}
			if (self->lastentry) {
				self->patterneditposition.line = (int) (
					(psy_audio_player_position(&self->player) -
					self->lastentry->offset) * psy_audio_player_lpb(&self->player));
				self->patterneditposition.offset =
					psy_audio_player_position(&self->player) - self->lastentry->offset;
				self->patterneditposition.offset =
					self->patterneditposition.line /
					(psy_dsp_big_beat_t) psy_audio_player_lpb(&self->player);
				if (!self->patternsinglemode) {
					self->patterneditposition.seqoffset = self->lastentry->offset;
				}
				workspace_setpatterncursor(self,
					self->patterneditposition);
			}
		} else if (self->lastentry) {
			self->patterneditposition.line = (int) (
				(psy_audio_player_position(&self->player) -
				self->lastentry->offset) * psy_audio_player_lpb(&self->player));
			self->patterneditposition.offset =
				psy_audio_player_position(&self->player) - self->lastentry->offset;
			self->patterneditposition.offset =
				self->patterneditposition.line /
				(psy_dsp_big_beat_t)psy_audio_player_lpb(&self->player);
			workspace_setpatterncursor(self,
				self->patterneditposition);
			self->lastentry = 0;
		}
	}
#if PSYCLE_USE_TK != PSYCLE_TK_X11
	/* todo segfault X11 imp */			
	if (self->scanprogresschanged) {
		assert(self);
		psy_signal_emit(&self->signal_scanprogress, self, 1, self->scanprogress);
		self->scanprogresschanged = 0;
	}
	if (self->plugincachechanged) {
		psy_audio_lock_enter(&self->pluginscanlock);
		psy_signal_emit(&self->signal_plugincachechanged, self, 0);
		self->plugincachechanged = 0;
		psy_audio_lock_leave(&self->pluginscanlock);
	}
	if (self->scantaskstart) {
		psy_audio_lock_enter(&self->pluginscanlock);
		psy_signal_emit(&self->signal_scantaskstart, self, 1, &self->lastscantask);
		self->scantaskstart = 0;
		psy_audio_lock_leave(&self->pluginscanlock);
	}
	if (self->filescanned) {
		psy_audio_lock_enter(&self->pluginscanlock);
		psy_signal_emit(&self->signal_scanfile, self, 2, self->scanfilename,
			self->scanplugintype);
		self->filescanned = 0;
		free(self->scanfilename);
		self->scanfilename = NULL;
		psy_audio_lock_leave(&self->pluginscanlock);
	}
#endif
	psy_audio_player_idle(&self->player);
	if (self->playrow && !psy_audio_player_playing(&self->player)) {
		self->playrow = FALSE;
		psy_audio_sequencer_setplaymode(&self->player.sequencer,
			self->restoreplaymode);
		psy_audio_sequencer_setnumplaybeats(&self->player.sequencer,
			self->restorenumplaybeats);
		if (self->restoreloop) {
			psy_audio_sequencer_loop(&self->player.sequencer);
		} else {
			psy_audio_sequencer_stoploop(&self->player.sequencer);
		}
		self->player.sequencer.playtrack = psy_INDEX_INVALID;
	}
	if (self->errorstrs) {
		psy_List* p;
		psy_audio_exclusivelock_enter();
		for (p = self->errorstrs; p != NULL; p = p->next) {
			psy_signal_emit(&self->signal_terminal_error, self, 1,
				(const char*)p->entry);
		}
		psy_list_deallocate(&self->errorstrs, NULL);
		psy_audio_exclusivelock_leave();
	}
}

void workspace_showparameters(Workspace* self, uintptr_t machineslot)
{
	assert(self);

	psy_signal_emit(&self->signal_showparameters, self, 1, machineslot);
}

void workspace_saveview(Workspace* self)
{
	assert(self);

	self->restoreview = workspace_currview(self);
}

void workspace_restoreview(Workspace* self)
{
	assert(self);

	workspace_selectview(self, self->restoreview.id,
		self->restoreview.section, 0);
}

void workspace_selectview(Workspace* self, uintptr_t view, uintptr_t section,
	uintptr_t option)
{
	assert(self);

	if (view == VIEW_ID_CHECKUNSAVED &&
			workspace_currview(self).id != VIEW_ID_CHECKUNSAVED &&
			workspace_currview(self).id != VIEW_ID_CONFIRM) {
		workspace_saveview(self);
	}
	psy_signal_emit(&self->signal_viewselected, self, 3, view, section, option);
}

void workspace_focusview(Workspace* self)
{
	assert(self);

	psy_signal_emit(&self->signal_focusview, self, 0);
}

void workspace_parametertweak(Workspace* self, int slot, uintptr_t tweak,
	float value)
{
	assert(self);

	psy_signal_emit(&self->signal_parametertweak, self, 3, slot, tweak,
		value);
}

void workspace_recordtweaks(Workspace* self)
{
	assert(self);

	self->recordtweaks = 1;
}

void workspace_stoprecordtweaks(Workspace* self)
{
	assert(self);

	self->recordtweaks = 0;
}

int workspace_recordingtweaks(Workspace* self)
{
	assert(self);

	return self->recordtweaks;
}

void workspace_onviewchanged(Workspace* self, ViewHistoryEntry view)
{
	assert(self);

	viewhistory_add(&self->viewhistory, view);
}

void workspace_back(Workspace* self)
{
	ViewHistoryEntry view;

	assert(self);

	view = viewhistory_currview(&self->viewhistory);
	if (viewhistory_back(&self->viewhistory)) {
		if (!viewhistory_equal(&self->viewhistory, view)) {
			workspace_updatecurrview(self);
		}
	}
}

void workspace_forward(Workspace* self)
{
	ViewHistoryEntry view;

	assert(self);

	view = viewhistory_currview(&self->viewhistory);
	if (viewhistory_forward(&self->viewhistory)) {
		if (!viewhistory_equal(&self->viewhistory, view)) {
			workspace_updatecurrview(self);
		}
	}
}

void workspace_updatecurrview(Workspace* self)
{
	ViewHistoryEntry view;
	int prevented;

	assert(self);

	view = viewhistory_currview(&self->viewhistory);
	prevented = viewhistory_prevented(&self->viewhistory);
	viewhistory_prevent(&self->viewhistory);
	workspace_selectview(self, view.id, 0, 0);
	//if (view.seqpos != -1 &&
		//self->sequenceselection.editposition.trackposition.sequencentrynode) {
		//psy_audio_SequencePosition position;

		//position = psy_audio_sequence_positionfromid(&self->song->sequence,
			//view.seqpos);
		//psy_audio_sequenceselection_seteditposition(&self->sequenceselection,
			//position);
		//workspace_setsequenceselection(self, self->sequenceselection);
	//}
	if (!prevented) {
		viewhistory_enable(&self->viewhistory);
	}
}

void workspace_onterminalwarning(Workspace* self, psy_audio_SongFile* sender,
	const char* text)
{
	assert(self);

	workspace_outputwarning(self, text);
}

void workspace_onterminalerror(Workspace* self, psy_audio_SongFile* sender,
	const char* text)
{
	assert(self);

	workspace_outputerror(self, text);
}

void workspace_onterminaloutput(Workspace* self, psy_audio_SongFile* sender,
	const char* text)
{
	assert(self);

	workspace_output(self, text);
}

void workspace_outputwarning(Workspace* self, const char* text)
{
	assert(self);

	psy_signal_emit(&self->signal_terminal_warning, self, 1, text);
}

void workspace_outputerror(Workspace* self, const char* text)
{
	assert(self);

	psy_signal_emit(&self->signal_terminal_error, self, 1, text);
}

void workspace_output(Workspace* self, const char* text)
{
	assert(self);

	psy_signal_emit(&self->signal_terminal_out, self, 1, text);
}

void workspace_outputstatus(Workspace* self, const char* text)
{
	assert(self);

	psy_signal_emit(&self->signal_status_out, self, 1, text);
}

void workspace_connectasmixersend(Workspace* self)
{
	assert(self);

	psy_audio_machines_connectasmixersend(&self->song->machines);
}

void workspace_connectasmixerinput(Workspace* self)
{
	assert(self);

	psy_audio_machines_connectasmixerinput(&self->song->machines);
}

bool workspace_isconnectasmixersend(const Workspace* self)
{
	assert(self);

	return psy_audio_machines_isconnectasmixersend(&self->song->machines);
}

void workspace_togglegear(Workspace* self)
{
	assert(self);
	self->gearvisible = !self->gearvisible;
	psy_signal_emit(&self->signal_togglegear, self, 0);
}

bool workspace_gearvisible(const Workspace* self)
{
	return self->gearvisible;
}

bool workspace_songmodified(const Workspace* self)
{
	assert(self);

	return psy_list_size(self->undoredo.undo) != self->undosavepoint ||
		psy_list_size(self->song->machines.undoredo.undo) != self->machines_undosavepoint;
}

psy_dsp_NotesTabMode workspace_notetabmode(Workspace* self)
{
	assert(self);

	return (psy_property_at_bool(&self->config.config,
			"visual.patternview.notetab", 0))
		? psy_dsp_NOTESTAB_A440
		: psy_dsp_NOTESTAB_A220;
}

void workspace_songposdec(Workspace* self)
{
	if (self->song && self->sequenceselection.editposition.order > 0) {
		workspace_setsequenceeditposition(self,
			psy_audio_orderindex_make(
				self->sequenceselection.editposition.track,
				self->sequenceselection.editposition.order - 1
			));
	}
}

void workspace_songposinc(Workspace* self)
{
	if (self->song && self->sequenceselection.editposition.order + 1 <
			psy_audio_sequence_track_size(&self->song->sequence,
				self->sequenceselection.editposition.track)) {
		workspace_setsequenceeditposition(self,
			psy_audio_orderindex_make(
				self->sequenceselection.editposition.track,
				self->sequenceselection.editposition.order + 1));
	}
}

void workspace_gotocursor(Workspace* self, psy_audio_PatternCursor cursor)
{
	psy_signal_emit(&self->signal_gotocursor, self, 1, &cursor);
}

void workspace_playstart(Workspace* self)
{
	assert(self);

	if (self->song) {
		psy_audio_player_setposition(&self->player,
			psy_audio_sequence_offset(&self->song->sequence,
				self->sequenceselection.editposition));
		psy_audio_player_start(&self->player);
	}
}

PatternDisplayMode workspace_patterndisplaytype(Workspace* self)
{
	psy_Property* patterndisplay;
	psy_Property* choice;

	assert(self);

	patterndisplay = psy_property_at(&self->config.config,
		"visual.patternview.patterndisplay",
		PSY_PROPERTY_TYPE_CHOICE);
	if (patterndisplay &&
			psy_property_id(patterndisplay) == PROPERTY_ID_PATTERNDISPLAY) {
		choice = psy_property_at_choice(patterndisplay);
		if (choice) {
			PatternDisplayMode rv;

			switch (psy_property_id(choice)) {
				case PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER:
					rv = PATTERN_DISPLAYMODE_TRACKER;
					break;
				case PROPERTY_ID_PATTERN_DISPLAYMODE_PIANOROLL:
					rv = PATTERN_DISPLAYMODE_PIANOROLL;
					break;
				case PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL:
					rv = PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL;
					break;
				case PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL:
					rv = PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL;
					break;
				default:
					rv = PATTERN_DISPLAYMODE_TRACKER;
					break;
			}
			return rv;
		}
	}
	return PATTERN_DISPLAYMODE_TRACKER;
}

void workspace_selectpatterndisplay(Workspace* self, PatternDisplayMode
	display)
{
	psy_Property* patterndisplay;

	assert(self);

	patterndisplay = psy_property_at(&self->config.config,
		"visual.patternview.patterndisplay",
		PSY_PROPERTY_TYPE_CHOICE);
	if (patterndisplay) {
		psy_property_setitem_int(patterndisplay, display);
	}
	patternviewconfig_onchanged(&self->config.patview, patterndisplay);
}

void workspace_floatsection(Workspace* self, int view, uintptr_t section)
{
	assert(self);

	psy_signal_emit(&self->signal_floatsection, self, 2, view, section);
}

void workspace_docksection(Workspace* self, int view, uintptr_t section)
{
	assert(self);

	psy_signal_emit(&self->signal_docksection, self, 2, view, section);
}

// Host specialization of machine callbacks
void onmachineterminaloutput(Workspace* self, const char* text)
{
	assert(self);

	psy_audio_exclusivelock_enter();
	psy_list_append(&self->errorstrs, psy_strdup(text));
	psy_audio_exclusivelock_leave();
}

static bool onmachineeditresize(Workspace* self, psy_audio_Machine* sender, intptr_t w, intptr_t h)
{
	psy_signal_emit(&self->signal_machineeditresize, self, 3, sender, w, h);
	return TRUE;
}

static void onmachinebuschanged(Workspace* self, psy_audio_Machine* sender)
{
	psy_signal_emit(&self->signal_buschanged, self, 1, sender);
}

static const char* onmachinelanguage(Workspace* self)
{
	return psy_translator_langid(psy_ui_translator());
}

bool onmachinefileselectload(Workspace* self, char filter[], char inoutname[])
{
	bool success;
	psy_ui_OpenDialog dialog;

	assert(self);

	psy_ui_opendialog_init_all(&dialog, NULL,
		"file.plugin-fileload", filter, "",
		dirconfig_vsts32(
			psycleconfig_directories(&self->config)));
	success = psy_ui_opendialog_execute(&dialog);
	psy_snprintf(inoutname, _MAX_PATH, "%s",
		psy_path_full(psy_ui_opendialog_path(&dialog)));
	psy_ui_opendialog_dispose(&dialog);
	return success;
}

bool onmachinefileselectsave(Workspace* self, char filter[],
	char inoutname[])
{
	bool success;
	psy_ui_SaveDialog dialog;

	assert(self);

	psy_ui_savedialog_init_all(&dialog, NULL,
		"file.plugin-filesave", filter, "",
		dirconfig_vsts32(
			psycleconfig_directories(&self->config)));
	success = psy_ui_savedialog_execute(&dialog);
	psy_snprintf(inoutname, _MAX_PATH, "%s",
		psy_path_full(psy_ui_savedialog_path(&dialog)));
	psy_ui_savedialog_dispose(&dialog);
	return success;
}

void onmachinefileselectdirectory(Workspace* self)
{
	assert(self);
	// todo
}

psy_audio_MachineFactory* onmachinefactory(Workspace* self)
{
	assert(self);

	return &self->machinefactory;
}

void workspace_multiselectgear(Workspace* self, psy_List* machinelist)
{
	psy_signal_emit(&self->signal_gearselect, self, 1, machinelist);
}

void workspace_connectterminal(Workspace* self,
	void* context,
	fp_workspace_output out,
	fp_workspace_output warning,
	fp_workspace_output error)
{
	psy_signal_connect(&self->signal_terminal_out, context, out);
	psy_signal_connect(&self->signal_terminal_warning, context, warning);
	psy_signal_connect(&self->signal_terminal_error, context, error);
}

void workspace_connectstatus(Workspace* self,
	void* context, fp_workspace_output status)
{
	psy_signal_connect(&self->signal_status_out, context, status);
}

void workspace_connectloadprogress(Workspace* self, void* context,
	fp_workspace_songloadprogress fp)
{
	psy_signal_connect(&self->signal_loadprogress, context, fp);
}

void workspace_oneventdriverinput(Workspace* self, psy_EventDriver* sender)
{
	psy_EventDriverCmd cmd;

	cmd = psy_eventdriver_getcmd(sender, "general");
	switch (cmd.id) {
	case CMD_IMM_ADDMACHINE:
		workspace_selectview(self, VIEW_ID_MACHINEVIEW,
			SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_APPEND);
		break;
	case CMD_IMM_EDITMACHINE:
		if (workspace_currview(self).id != VIEW_ID_MACHINEVIEW) {
			workspace_selectview(self, VIEW_ID_MACHINEVIEW, psy_INDEX_INVALID,
				0);
		} else {
			if (workspace_currview(self).section == SECTION_ID_MACHINEVIEW_WIRES) {
				workspace_selectview(self, VIEW_ID_MACHINEVIEW,
					SECTION_ID_MACHINEVIEW_STACK, 0);
			} else {
				workspace_selectview(self, VIEW_ID_MACHINEVIEW,
					SECTION_ID_MACHINEVIEW_WIRES, 0);
			}
		}
		break;
	case CMD_IMM_HELP:
		workspace_selectview(self, VIEW_ID_HELPVIEW, 0, 0);
		break;
	case CMD_IMM_EDITPATTERN:
		workspace_selectview(self, VIEW_ID_PATTERNVIEW, psy_INDEX_INVALID,
			psy_INDEX_INVALID);
		break;
	case CMD_IMM_EDITINSTR:
		workspace_selectview(self, VIEW_ID_INSTRUMENTSVIEW, 0, 0);
		break;
	case CMD_IMM_EDITSAMPLE:
		workspace_selectview(self, VIEW_ID_SAMPLESVIEW, 0, 0);
		break;
	case CMD_IMM_EDITWAVE:
		workspace_selectview(self, VIEW_ID_SAMPLESVIEW, 2, 0);
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
			!psycleconfig_audioenabled(&self->config));
		break;
	case CMD_IMM_SETTINGS:
		workspace_selectview(self, VIEW_ID_SETTINGSVIEW, 0, 0);
		break;
	case CMD_IMM_LOADSONG:
		if (keyboardmiscconfig_savereminder(&self->config.misc) &&
				workspace_songmodified(self)) {
			workspace_selectview(self, VIEW_ID_CHECKUNSAVED, 0, CONFIRM_LOAD);
		} else {
			workspace_loadsong_fileselect(self);
		}
		break;
	case CMD_IMM_INFOMACHINE:
		if (self->song) {
			workspace_showparameters(self,
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
		workspace_songposdec(self);
		break;
	case CMD_IMM_SONGPOSINC:
		workspace_songposinc(self);
		break;
	case CMD_IMM_PLAYSONG:
		psy_audio_player_setposition(&self->player, 0);
		psy_audio_player_start(&self->player);
		break;
	case CMD_IMM_PLAYSTART:
		workspace_playstart(self);
		break;
	case CMD_IMM_PLAYSTOP:
		psy_audio_player_stop(&self->player);
		break;
	case CMD_IMM_PLAYROWTRACK: {
		/*psy_dsp_big_beat_t playposition = 0;
		psy_audio_SequenceEntry* entry;

		psy_audio_exclusivelock_enter();
		psy_audio_player_stop(&self->workspace.player);
		entry = psy_audio_sequenceposition_entry(&self->workspace.sequenceselection.editposition);
		playposition = (entry ? entry->offset : 0) +
			(psy_dsp_big_beat_t)self->workspace.patterneditposition.offset;
		self->restoreplaymode = psy_audio_sequencer_playmode(&self->workspace.player.sequencer);
		self->restorenumplaybeats = self->workspace.player.sequencer.numplaybeats;
		self->restoreloop = psy_audio_sequencer_looping(&self->workspace.player.sequencer);
		psy_audio_sequencer_stoploop(&self->workspace.player.sequencer);
		psy_audio_sequencer_setplaymode(&self->workspace.player.sequencer,
			psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS);
		psy_audio_sequencer_setnumplaybeats(&self->workspace.player.sequencer,
			psy_audio_player_bpl(&self->workspace.player));
		self->workspace.player.sequencer.playtrack = self->workspace.patterneditposition.track;
		psy_audio_player_setposition(&self->workspace.player, playposition);
		psy_audio_player_start(&self->workspace.player);
		self->playrow = TRUE;
		psy_audio_exclusivelock_leave();
		break; */ }
	case CMD_IMM_PLAYROWPATTERN: {
		/*psy_dsp_big_beat_t playposition = 0;
		psy_audio_SequenceEntry* entry;

		psy_audio_exclusivelock_enter();
		psy_audio_player_stop(&self->workspace.player);
		entry = psy_audio_sequenceposition_entry(&self->workspace.sequenceselection.editposition);
		playposition = (entry ? entry->offset : 0) +
			(psy_dsp_big_beat_t)self->workspace.patterneditposition.offset;
		self->restoreplaymode = psy_audio_sequencer_playmode(&self->workspace.player.sequencer);
		self->restorenumplaybeats = self->workspace.player.sequencer.numplaybeats;
		self->restoreloop = psy_audio_sequencer_looping(&self->workspace.player.sequencer);
		psy_audio_sequencer_stoploop(&self->workspace.player.sequencer);
		psy_audio_sequencer_setplaymode(&self->workspace.player.sequencer,
			psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS);
		psy_audio_sequencer_setnumplaybeats(&self->workspace.player.sequencer,
			psy_audio_player_bpl(&self->workspace.player));
		psy_audio_player_setposition(&self->workspace.player, playposition);
		psy_audio_player_start(&self->workspace.player);
		self->playrow = TRUE;
		psy_audio_exclusivelock_leave();
		break; */ }
	case CMD_IMM_PLAYFROMPOS: {
		/*psy_dsp_big_beat_t playposition = 0;
		psy_audio_SequenceEntry* entry;

		entry = psy_audio_sequenceposition_entry(&self->workspace.sequenceselection.editposition);
		playposition = (entry ? entry->offset : 0) +
			(psy_dsp_big_beat_t)self->workspace.patterneditposition.offset;
		psy_audio_player_setposition(&self->workspace.player, playposition);
		psy_audio_player_start(&self->workspace.player);
		break; */}
	case CMD_IMM_SAVESONG:
		workspace_savesong_fileselect(self);
		break;
	case CMD_IMM_FOLLOWSONG:
		if (workspace_followingsong(self)) {
			workspace_stopfollowsong(self);
		} else {
			workspace_followsong(self);
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
				(uintptr_t)(cmd.id - CMD_COLUMN_0)) {
			self->patterneditposition.track = (cmd.id - CMD_COLUMN_0);
			workspace_setpatterncursor(self, self->patterneditposition);
		}
		break;
	case CMD_IMM_TAB1:
		workspace_selectview(self, 0, psy_INDEX_INVALID, psy_INDEX_INVALID);
		break;
	case CMD_IMM_TAB2:
		workspace_selectview(self, 1, psy_INDEX_INVALID, psy_INDEX_INVALID);
		break;
	case CMD_IMM_TAB3:
		workspace_selectview(self, 2, psy_INDEX_INVALID, psy_INDEX_INVALID);
		break;
	case CMD_IMM_TAB4:
		workspace_selectview(self, 3, psy_INDEX_INVALID, psy_INDEX_INVALID);
		break;
	case CMD_IMM_TAB5:
		workspace_selectview(self, 4, psy_INDEX_INVALID, psy_INDEX_INVALID);
		break;
	case CMD_IMM_TAB6:
		workspace_selectview(self, psy_INDEX_INVALID, 0, psy_INDEX_INVALID);
		break;
	case CMD_IMM_TAB7:
		workspace_selectview(self, psy_INDEX_INVALID, 1, psy_INDEX_INVALID);
		break;
	case CMD_IMM_TAB8:
		workspace_selectview(self, psy_INDEX_INVALID, 2, psy_INDEX_INVALID);
		break;
	case CMD_IMM_TAB9:
		workspace_selectview(self, psy_INDEX_INVALID, 3, psy_INDEX_INVALID);
		break;
	default:
		break;
	}
}

void workspace_onscanprogress(Workspace* self, psy_audio_PluginCatcher* sender,
	int progress)
{
	psy_audio_lock_enter(&self->pluginscanlock);
	self->scanprogresschanged = 1;
	self->scanprogress = progress;
	psy_audio_lock_leave(&self->pluginscanlock);
}

void workspace_onscanfile(Workspace* self, psy_audio_PluginCatcher* sender,
	const char* path, int type)
{
	psy_audio_lock_enter(&self->pluginscanlock);
	self->filescanned = 1;
	psy_strreset(&self->scanfilename, path);
	self->scanplugintype = type;
	psy_audio_lock_leave(&self->pluginscanlock);
}

void workspace_onscantaskstart(Workspace* self, psy_audio_PluginCatcher* sender,
	psy_audio_PluginScanTask* task)
{
	assert(task);

	psy_audio_lock_enter(&self->pluginscanlock);
	self->scantaskstart = 1;
	self->lastscantask = *task;
	psy_audio_lock_leave(&self->pluginscanlock);
}

void workspace_onplugincachechanged(Workspace* self,
	psy_audio_PluginCatcher* sender)
{
	psy_audio_lock_enter(&self->pluginscanlock);
	self->plugincachechanged = 1;
	psy_audio_lock_leave(&self->pluginscanlock);
}

void workspace_apptitle(Workspace* self, char* rv_title, uintptr_t max_len)
{
	psy_Path path;

	rv_title[0] = '\n';
	psy_path_init(&path, self->filename);
	psy_snprintf(rv_title, max_len, "[%s.%s]  Psycle Modular Music Creation Studio ",
		psy_path_name(&path), psy_path_ext(&path));
	psy_path_dispose(&path);
}

const char* workspace_songtitle(const Workspace* self)
{
	if (self->song) {
		return psy_audio_song_title(self->song);
	}
	return "";
}

void workspace_setstartpage(Workspace* self)
{
	if (generalconfig_showaboutatstart(psycleconfig_general(
		workspace_conf(self)))) {
		workspace_selectview(self, VIEW_ID_HELPVIEW, 1, 0);
	} else {
		workspace_selectview(self, VIEW_ID_MACHINEVIEW, 0, 0);
	}
	self->startpage = TRUE;
}
