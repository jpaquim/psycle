// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/psyconf.h"

#include "workspace.h"
// local
#include "cmdproperties.h"
#include "skinio.h"
// file
#include <dir.h>
// dsp
#include <operations.h>
// audio
#include <exclusivelock.h>
#include <kbddriver.h>
// ui
#include <uiopendialog.h>
#include <uisavedialog.h>
#include <uiwincomponentimp.h>
// std
#include <stdlib.h>
#include <string.h>
#include <songio.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"
#include "../../detail/os.h"

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#endif

static void workspace_initplayer(Workspace*);
static void workspace_initplugincatcherandmachinefactory(Workspace*);
static void workspace_initsignals(Workspace*);
static void workspace_disposesignals(Workspace*);
static void workspace_makerecentsongs(Workspace*);
static void workspace_makeconfig(Workspace*);
static void workspace_makegeneral(Workspace*);
static void workspace_makelanguagelist(Workspace*, psy_Property*);
static int workspace_onaddlanguage(Workspace*, const char* path, int flag);
static void workspace_makenotes(Workspace*);
static void workspace_makevisual(Workspace*);
static void workspace_makepatternview(Workspace*, psy_Property*);
static void workspace_makepatternviewtheme(Workspace*, psy_Property*);
static void workspace_makemachineview(Workspace*, psy_Property*);
static void workspace_makemachineviewtheme(Workspace*, psy_Property*);
static void workspace_makeparamview(Workspace*, psy_Property*);
static void workspace_makeparamtheme(Workspace*, psy_Property*);
static void workspace_makekeyboard(Workspace*);
static void workspace_makedirectories(Workspace*);
static void workspace_makedefaultuserpresetpath(Workspace*);
static void workspace_makedirectory(Workspace*, const char* key,
	const char* label, const char* defaultdir);
static void workspace_makenotes(Workspace*);
// audiodriver
static void workspace_makeinputoutput(Workspace*);
static void workspace_makedriverlist(Workspace*);
static void workspace_makedriverconfigurations(Workspace*);
static void workspace_update_driverconfigure_section(Workspace*);
static const char* workspace_driverpath(Workspace*);
static const char* workspace_driverkey(Workspace*);
// eventdriver
static void workspace_makeeventinput(Workspace*);
static void workspace_makeeventinputdriverlist(Workspace*);
static void workspace_registerinputdrivers(Workspace*);
static void workspace_makeeventdriverconfigurations(Workspace*);
static void workspace_showactiveeventinputconfig(Workspace*, int deviceid);
static void workspace_updateactiveeventdriverlist(Workspace*);
static void workspace_configeventinput(Workspace*);

static void workspace_makecompatibility(Workspace*);
static const char* workspace_eventdriverpath(Workspace*);
static int workspace_curreventinputconfiguration(Workspace*);
const char* workspace_languagekey(Workspace*);
static void workspace_configvisual(Workspace*);
static void workspace_configlanguage(Workspace*);
static void workspace_setsong(Workspace*, psy_audio_Song*, int flag,
	psy_audio_SongFile*);
static void workspace_onloadprogress(Workspace*, psy_audio_Song*, int progress);
static void workspace_onscanprogress(Workspace*, psy_audio_PluginCatcher*, int progress);
static void workspace_onsequenceeditpositionchanged(Workspace*, psy_audio_SequenceSelection*);
static void workspace_updatenavigation(Workspace*);
/// Machinecallback
static psy_dsp_big_beat_t machinecallback_beatspertick(Workspace*);
static psy_audio_Samples* machinecallback_samples(Workspace*);
static psy_audio_Machines* machinecallback_machines(Workspace*);
static psy_audio_Instruments* machinecallback_instruments(Workspace*);
static psy_audio_MachineFactory* machinecallback_machinefactory(Workspace*);
static bool machinecallback_fileselect_load(Workspace*, char filter[], char inoutName[]);
static bool machinecallback_fileselect_save(Workspace*, char filter[], char inoutName[]);
static void machinecallback_fileselect_directory(Workspace*);
static void machinecallback_output(Workspace*, const char* text);
/// terminal
static void workspace_onterminalwarning(Workspace*,
	psy_audio_SongFile* sender, const char* text);
static void workspace_onterminaloutput(Workspace*,
	psy_audio_SongFile* sender, const char* text);
static void workspace_onterminalerror(Workspace*,
	psy_audio_SongFile* sender, const char* text);
// recent files
static void workspace_addrecentsong(Workspace*, const char* path);
static void workspace_onlanguagechanged(Workspace*, Translator* sender);
static int workspace_onchangelanguageenum(Workspace*, psy_Property*,
	int level);

void history_init(History* self)
{
	assert(self);

	self->container = NULL;
	self->prevented = FALSE;
}

void history_dispose(History* self)
{
	assert(self);

	psy_list_deallocate(&self->container, (psy_fp_disposefunc)NULL);
	self->prevented = FALSE;
}

void history_clear(History* self)
{
	assert(self);

	psy_list_deallocate(&self->container, (psy_fp_disposefunc)NULL);
	self->prevented = FALSE;
}

void history_add(History* self, int viewid, int sequenceentryid)
{
	HistoryEntry* entry;

	assert(self);

	entry = (HistoryEntry*) malloc(sizeof(HistoryEntry));
	if (entry) {
		entry->viewid = viewid;
		entry->sequenceentryid = sequenceentryid;
		psy_list_append(&self->container, entry);
	}
}

// MachineCallback VTable
static psy_audio_MachineCallbackVtable psy_audio_machinecallbackvtable_vtable;
static int psy_audio_machinecallbackvtable_initialized = 0;

static void psy_audio_machinecallbackvtable_init(Workspace* self)
{
	assert(self);

	if (!psy_audio_machinecallbackvtable_initialized) {
		psy_audio_machinecallbackvtable_vtable = *self->machinecallback.vtable;		
		psy_audio_machinecallbackvtable_vtable.beatspertick = (fp_mcb_beatspertick)
			machinecallback_beatspertick;		
		psy_audio_machinecallbackvtable_vtable.samples = (fp_mcb_samples)
			machinecallback_samples;
		psy_audio_machinecallbackvtable_vtable.machines = (fp_mcb_machines)
			machinecallback_machines;
		psy_audio_machinecallbackvtable_vtable.instruments = (fp_mcb_instruments)
			machinecallback_instruments;
		psy_audio_machinecallbackvtable_vtable.machinefactory = (fp_mcb_machinefactory)
			machinecallback_machinefactory;
		psy_audio_machinecallbackvtable_vtable.fileselect_load = (fp_mcb_fileselect_load)
			machinecallback_fileselect_load;
		psy_audio_machinecallbackvtable_vtable.fileselect_save = (fp_mcb_fileselect_save)
			machinecallback_fileselect_save;
		psy_audio_machinecallbackvtable_vtable.fileselect_directory =
			(fp_mcb_fileselect_directory)machinecallback_fileselect_directory;
		psy_audio_machinecallbackvtable_vtable.output = (fp_mcb_output)
			machinecallback_output;		
		psy_audio_machinecallbackvtable_initialized = 1;
	}
}

void workspace_init(Workspace* self, void* handle)
{	
	assert(self);

	psy_audio_exclusivelock_init();
	psy_audio_machinecallback_init(&self->machinecallback, &self->player);
	psy_audio_machinecallbackvtable_init(self);
	self->machinecallback.vtable = &psy_audio_machinecallbackvtable_vtable;
#ifdef PSYCLE_USE_SSE
	psy_dsp_sse2_init(&dsp);
#else
	psy_dsp_noopt_init(&dsp);
#endif
	self->fontheight = 12;
	self->octave = 4;	
	self->cursorstep = 1;
	self->followsong = 0;
	self->recordtweaks = 0;
	self->inputoutput = 0;
	self->eventinputs = 0;
	self->driverconfigurations = 0;
	self->mainhandle = handle;
	self->filename = strdup("Untitled.psy");
	self->lastentry = 0;
	self->maximizeview.maximized = 0;
	self->maximizeview.row0 = 1;
	self->maximizeview.row1 = 1;
	self->maximizeview.row2 = 1;
	self->currnavigation = 0;
	self->currview = 0;
	self->dialbitmappath = 0;
	translator_init(&self->translator);
	self->undosavepoint = 0;
	self->machines_undosavepoint = 0;
	history_init(&self->history);
	workspace_makerecentsongs(self);
	workspace_makeconfig(self);	
	workspace_initplugincatcherandmachinefactory(self);
	self->song = psy_audio_song_allocinit(&self->machinefactory);
	self->songcbk = self->song;
	psy_audio_sequenceselection_init(&self->sequenceselection, &self->song->sequence);
	psy_audio_sequence_setplayselection(&self->song->sequence, &self->sequenceselection);
	psy_signal_connect(&self->sequenceselection.signal_editpositionchanged, self,
		workspace_onsequenceeditpositionchanged);
	psy_undoredo_init(&self->undoredo);	
	self->navigating = 0;
	workspace_initsignals(self);	
	workspace_initplayer(self);
	workspace_registerinputdrivers(self);
	psy_audio_patterneditposition_init(&self->patterneditposition);
	psy_audio_pattern_init(&self->patternpaste);
	self->sequencepaste = 0;
	psy_signal_connect(&self->signal_languagechanged, self,
		workspace_onlanguagechanged);
}

void workspace_initplugincatcherandmachinefactory(Workspace* self)
{
	assert(self);

	plugincatcher_init(&self->plugincatcher, self->directories);
	psy_signal_connect(&self->plugincatcher.signal_scanprogress, self,
		workspace_onscanprogress);
	self->hasplugincache = plugincatcher_load(&self->plugincatcher);
	psy_audio_machinefactory_init(&self->machinefactory, &self->machinecallback, 
		&self->plugincatcher);
}

void workspace_initsignals(Workspace* self)
{
	assert(self);

	psy_signal_init(&self->signal_octavechanged);
	psy_signal_init(&self->signal_songchanged);
	psy_signal_init(&self->signal_configchanged);
	psy_signal_init(&self->signal_skinchanged);
	psy_signal_init(&self->signal_changecontrolskin);
	psy_signal_init(&self->signal_patterneditpositionchanged);
	psy_signal_init(&self->signal_sequenceselectionchanged);
	psy_signal_init(&self->signal_loadprogress);
	psy_signal_init(&self->signal_scanprogress);
	psy_signal_init(&self->signal_beforesavesong);
	psy_signal_init(&self->signal_showparameters);
	psy_signal_init(&self->signal_viewselected);
	psy_signal_init(&self->signal_parametertweak);
	psy_signal_init(&self->signal_terminal_error);
	psy_signal_init(&self->signal_terminal_out);
	psy_signal_init(&self->signal_terminal_warning);
	psy_signal_init(&self->signal_followsongchanged);
	psy_signal_init(&self->signal_dockview);
	psy_signal_init(&self->signal_defaultfontchanged);
	psy_signal_init(&self->signal_showgear);
	psy_signal_init(&self->signal_languagechanged);
}

void workspace_dispose(Workspace* self)
{	
	assert(self);

	psy_audio_player_dispose(&self->player);
	psy_audio_song_deallocate(self->song);	
	self->song = NULL;	
	self->songcbk = NULL;
	psy_property_dispose(&self->config);
	translator_dispose(&self->translator);	
	free(self->filename);
	self->filename = NULL;
	plugincatcher_dispose(&self->plugincatcher);
	psy_audio_machinefactory_dispose(&self->machinefactory);
	psy_undoredo_dispose(&self->undoredo);	
	history_dispose(&self->history);
	workspace_disposesignals(self);
	psy_audio_pattern_dispose(&self->patternpaste);
	workspace_disposesequencepaste(self);
	psy_property_deallocate(self->cmds);
	self->cmds = NULL;
	psy_audio_sequenceselection_dispose(&self->sequenceselection);
	free(self->dialbitmappath);
	psy_property_deallocate(self->recentsongs);
	self->recentsongs = NULL;
	psy_audio_exclusivelock_dispose();
}

void workspace_disposesignals(Workspace* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_octavechanged);
	psy_signal_dispose(&self->signal_songchanged);	
	psy_signal_dispose(&self->signal_configchanged);
	psy_signal_dispose(&self->signal_skinchanged);
	psy_signal_dispose(&self->signal_changecontrolskin);
	psy_signal_dispose(&self->signal_patterneditpositionchanged);
	psy_signal_dispose(&self->signal_sequenceselectionchanged);
	psy_signal_dispose(&self->signal_loadprogress);
	psy_signal_dispose(&self->signal_scanprogress);
	psy_signal_dispose(&self->signal_beforesavesong);
	psy_signal_dispose(&self->signal_showparameters);
	psy_signal_dispose(&self->signal_viewselected);
	psy_signal_dispose(&self->signal_parametertweak);
	psy_signal_dispose(&self->signal_terminal_error);
	psy_signal_dispose(&self->signal_terminal_out);
	psy_signal_dispose(&self->signal_terminal_warning);
	psy_signal_dispose(&self->signal_followsongchanged);
	psy_signal_dispose(&self->signal_dockview);
	psy_signal_dispose(&self->signal_defaultfontchanged);
	psy_signal_dispose(&self->signal_showgear);
	psy_signal_dispose(&self->signal_languagechanged);
}

void workspace_disposesequencepaste(Workspace* self)
{
	assert(self);

	psy_list_deallocate(&self->sequencepaste, (psy_fp_disposefunc)NULL);	
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
	self->cmds = cmdproperties_create();
	psy_audio_eventdrivers_setcmds(&self->player.eventdrivers, self->cmds);
	workspace_update_driverconfigure_section(self);
	workspace_updateactiveeventdriverlist(self);
	workspace_showactiveeventinputconfig(self, 0);
}

void workspace_configlanguage(Workspace* self)
{	
	psy_Property* langchoice;

	assert(self);

	langchoice = psy_property_at_choice(self->language);
	if (langchoice) {
		if (translator_load(&self->translator,
			psy_property_item_str(langchoice))) {
			psy_signal_emit(&self->signal_languagechanged, &self->translator,
				0);
			psy_ui_updatealign(self->mainhandle, NULL);
		}
	}	
}

void workspace_configvisual(Workspace* self)
{	
	psy_Property* visual;

	assert(self);

	visual = psy_property_find(&self->config, "visual", PSY_PROPERTY_TYPE_SECTION);
	if (visual) {
		psy_ui_Font font;
		psy_ui_FontInfo fontinfo;
		
		psy_ui_fontinfo_init_string(&fontinfo, 
			psy_property_at_str(visual, "defaultfont",
			PSYCLE_DEFAULT_FONT));
		psy_ui_font_init(&font, &fontinfo);
		fontinfo = psy_ui_font_fontinfo(&font);
		self->fontheight = fontinfo.lfHeight;
		psy_ui_replacedefaultfont(self->mainhandle, &font);
		psy_ui_font_dispose(&font);
	}
}

const char* workspace_driverpath(Workspace* self)
{
	psy_Property* p;
	const char* rv = 0;

	assert(self);

	if (p = psy_property_at(self->inputoutput, "driver", PSY_PROPERTY_TYPE_NONE)) {
		if (p = psy_property_at_choice(p)) {		
			rv = psy_property_item_str(p);
		}
	}
	return rv;
}

int workspace_curreventinputconfiguration(Workspace* self)
{
	psy_Property* p;

	assert(self);

	if (p = psy_property_at(self->eventinputs, "activedrivers", PSY_PROPERTY_TYPE_NONE)) {
		return psy_property_item_int(p);			
	}
	return 0;
}

const char* workspace_languagekey(Workspace* self)
{
	psy_Property* p;
	const char* rv = 0;

	assert(self);

	if (p = psy_property_at(self->general, "lang", PSY_PROPERTY_TYPE_NONE)) {
		if (p = psy_property_at_choice(p)) {
			rv = psy_property_item_str(p);
		}
	}
	return rv;
}

const char* workspace_driverkey(Workspace* self)
{
	psy_Property* p;
	const char* rv = 0;

	assert(self);

	if (p = psy_property_at(self->inputoutput, "driver",
			PSY_PROPERTY_TYPE_NONE)) {
		if (p = psy_property_at_choice(p)) {		
			rv = psy_property_key(p);
		}
	}
	return rv;
}

const char* workspace_eventdriverpath(Workspace* self)
{
	psy_Property* activedrivers;

	assert(self);

	activedrivers = psy_property_at(self->eventinputs, "activedrivers",
		PSY_PROPERTY_TYPE_NONE);
	if (activedrivers) {
		psy_Property* choice;
		
		choice = psy_property_at_choice(activedrivers);
		if (choice) {
			return psy_property_item_str(choice);
		}		
	}
	return NULL;
}

void workspace_update_driverconfigure_section(Workspace* self)
{
	assert(self);

	if (self->driverconfigure) {
		psy_Property* driversection;

		psy_property_clear(self->driverconfigure);
		if (psy_audiodriver_configuration(self->player.driver)) {
			psy_property_append_property(self->driverconfigure,
				psy_property_clone(
					psy_audiodriver_configuration(self->player.driver)));
		}
		if (psy_audiodriver_configuration(self->player.driver)) {
			driversection = psy_property_find(self->driverconfigurations,
				psy_property_key(
					psy_audiodriver_configuration(self->player.driver)),
				PSY_PROPERTY_TYPE_NONE);
			if (driversection) {
				psy_property_remove(self->driverconfigurations,
					driversection);
				psy_property_append_property(self->driverconfigurations,
					psy_property_clone(psy_audiodriver_configuration(
						self->player.driver)));
			}
		}
	}
}

void workspace_showactiveeventinputconfig(Workspace* self, int driverid)
{	
	psy_EventDriver* eventdriver;
	
	assert(self);

	eventdriver = psy_audio_player_eventdriver(&self->player, driverid);
	if (eventdriver) {
		self->midiconfigure->item.save = FALSE;
		self->midiconfigure->item.disposechildren = FALSE;
		self->midiconfigure->children = eventdriver->properties ?
			eventdriver->properties->children : 0;
	} else {
		self->midiconfigure->children = 0;	
	}
}

void workspace_scanplugins(Workspace* self)
{		
	assert(self);

	plugincatcher_scan(&self->plugincatcher);	
	plugincatcher_save(&self->plugincatcher);
}

void workspace_onscanprogress(Workspace* self, psy_audio_PluginCatcher* sender, int progress)
{
	assert(self);

	psy_signal_emit(&self->signal_scanprogress, self, 1, progress);
}

void workspace_makerecentsongs(Workspace* self)
{
	assert(self);
	self->recentsongs = psy_property_setcomment(
		psy_property_allocinit_key(NULL),
		"Psycle Recent Song History created by\r\n; " PSYCLE__BUILD__IDENTIFIER("\r\n; "));
	self->recentfiles = psy_property_settext(
		psy_property_append_section(self->recentsongs, "files"),
		"Recent Songs");
}

void workspace_makeconfig(Workspace* self)
{	
	assert(self);

	workspace_makegeneral(self);
	workspace_makevisual(self);	
	workspace_makekeyboard(self);
	workspace_makedirectories(self);
	workspace_makeinputoutput(self);
	workspace_makeeventinput(self);
	workspace_makecompatibility(self);
	workspace_onlanguagechanged(self, &self->translator);
}

void workspace_makegeneral(Workspace* self)
{	
	assert(self);

	psy_property_init(&self->config);
	psy_property_setcomment(&self->config,
		"Psycle Configuration File created by\r\n; " PSYCLE__BUILD__IDENTIFIER("\r\n; "));
	self->general = psy_property_settext(
		psy_property_append_section(&self->config, "general"),
		"settingsview.general");
	psy_property_sethint(psy_property_settext(
		psy_property_append_string(self->general, "version", "alpha"),
		"settingsview.version"),
		PSY_PROPERTY_HINT_HIDE);
	workspace_makelanguagelist(self, self->general);
	psy_property_settext(
		psy_property_append_bool(self->general, "showaboutatstart", TRUE),
		"settingsview.show-about-at-startup");
	psy_property_settext(
		psy_property_append_bool(self->general, "showsonginfoonload", TRUE),
		"settingsview.show-song-info-on-load");
	psy_property_settext(
		psy_property_append_bool(self->general, "showmaximizedatstart", TRUE),
		"settingsview.show-maximized-at-startup");
	psy_property_settext(
		psy_property_append_bool(self->general, "showplaylisteditor", FALSE),
		"settingsview.show-playlist-editor");
	psy_property_settext(
		psy_property_append_bool(self->general, "showstepsequencer", TRUE),
		"settingsview.show-sequencestepbar");
	psy_property_settext(
		psy_property_append_bool(self->general, "saverecentsongs", TRUE),
		"settingsview.save-recent-songs");
	psy_property_settext(
		psy_property_append_bool(self->general, "playsongafterload", TRUE),
		"settingsview.play-song-after-load");
}

void workspace_makelanguagelist(Workspace* self, psy_Property* parent)
{
	char currdir[4096];
	
	assert(self);

	self->language = psy_property_settext(
		psy_property_append_choice(self->general, "lang", 0),
		"settingsview.language");
	if (workdir(currdir)) {
		psy_dir_enumerate(self, currdir, "*.ini", 0, (psy_fp_findfile)
			workspace_onaddlanguage);
	}	
}

int workspace_onaddlanguage(Workspace* self, const char* path, int flag)
{
	char languageid[256];

	assert(self);

	if (translator_test(&self->translator, path, languageid)) {
		psy_property_settext(
			psy_property_append_string(self->language, languageid, path),
			languageid);
	}	
	return TRUE;
}

void workspace_makevisual(Workspace* self)
{
	psy_Property* visual;
	
	assert(self);

	visual = psy_property_settext(
		psy_property_append_section(&self->config, "visual"),
		"settingsview.visual");
	psy_property_settext(
		psy_property_append_action(visual, "loadskin"),
		"settingsview.load-skin");
	psy_property_settext(
		psy_property_append_action(visual, "defaultskin"),
		"settingsview.default-skin");
	psy_property_settext(
		psy_property_append_font(visual, "defaultfont", PSYCLE_DEFAULT_FONT),
		"settingsview.default-font");	
	workspace_makepatternview(self, visual);
	workspace_makemachineview(self, visual);
	workspace_makeparamview(self, visual);
}

void workspace_makepatternview(Workspace* self, psy_Property* visual)
{
	psy_Property* pvc;	
	
	assert(self);

	pvc = psy_property_append_section(visual, "patternview");
	psy_property_settext(pvc,
		"settingsview.patternview");
	psy_property_settext(
		psy_property_append_font(pvc, "font", PSYCLE_DEFAULT_FONT),
		"settingsview.font");
	psy_property_settext(
		psy_property_append_bool(pvc, "drawemptydata", 0),
		"settingsview.draw-empty-data");	
	psy_property_settext(
		psy_property_append_bool(pvc, "griddefaults", 1),
		"settingsview.default-entries");
	psy_property_settext(
		psy_property_append_bool(pvc, "linenumbers", 1),
		"settingsview.line-numbers");
	psy_property_settext(
		psy_property_append_bool(pvc, "beatoffset", 1),
		"settingsview.beat-offset");
	psy_property_settext(
		psy_property_append_bool(pvc, "linenumberscursor", 1),
		"settingsview.line-numbers-cursor");
	psy_property_settext(
		psy_property_append_bool(pvc, "linenumbersinhex", 1),
		"settingsview.line-numbers-in-hex");
	psy_property_settext(
		psy_property_append_bool(pvc, "wideinstcolumn", 0),
		"settingsview.wide-instrument-column");
	psy_property_settext(
		psy_property_append_bool(pvc, "trackscopes", 1),
		"settingsview.pattern-track-scopes");
	psy_property_settext(
		psy_property_append_bool(pvc, "wraparound", 1),
		"settingsview.wrap-around");
	psy_property_settext(
		psy_property_append_bool(pvc, "centercursoronscreen", 0),
		"settingsview.center-cursor-on-screen");	
	psy_property_settext(
		psy_property_append_int(pvc, "beatsperbar", 4, 1, 16),
		"settingsview.bar-highlighting");
	psy_property_settext(
		psy_property_append_bool(pvc, "notetab", 1),
		"settingsview.a4-440hz");
	psy_property_settext(
		psy_property_append_bool(pvc, "movecursorwhenpaste", 1),
		"settingsview.move-cursor-when-paste");	
	workspace_makepatternviewtheme(self, pvc);
}

void workspace_makepatternviewtheme(Workspace* self, psy_Property* view)
{
	assert(self);

	self->patternviewtheme = psy_property_settext(
		psy_property_append_section(view, "theme"),
		"settingsview.theme");
	psy_property_append_string(self->patternviewtheme,
		"pattern_fontface", "Tahoma");
	psy_property_append_int(self->patternviewtheme, "pattern_font_point", 0x00000050, 0, 0),
		psy_property_append_int(self->patternviewtheme, "pattern_font_flags", 0x00000001, 0, 0);
	psy_property_settext(
		psy_property_append_int(self->patternviewtheme,
		"pattern_font_x", 0x00000009, 0, 0),
		"Point X");
	psy_property_settext(
		psy_property_append_int(self->patternviewtheme,
		"pattern_font_y", 0x0000000B, 0, 0),
		"Point Y");
	psy_property_append_string(self->patternviewtheme, "pattern_header_skin", "");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_separator", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Separator Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_separator2", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Separator Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_background", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"BackGnd Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_background2", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"BackGnd Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_font", 0x00CACACA, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Font Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_font2", 0x00CACACA, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
		"Font Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_fontCur", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Font Cur Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_fontCur2", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Font Cur Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_fontSel", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Font Sel Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_fontSel2", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Font Sel Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_fontPlay", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Font Play Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_fontPlay2", 0x00FFFFFF, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Font Play Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_row", 0x003E3E3E, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Row Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_row2", 0x003E3E3E, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Row Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_rowbeat", 0x00363636, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Beat Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_rowbeat2", 0x00363636, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Beat Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_row4beat", 0x00595959, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Bar Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_row4beat2", 0x00595959, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Bar Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_selection", 0x009B7800, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Selection Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_selection2", 0x009B7800, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Selection Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_playbar", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Playbar Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_playbar2", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Playbar Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_cursor", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Cursor Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_cursor2", 0x009F7B00, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Cursor Right");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_midline", 0x007D6100, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Midline Left");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->patternviewtheme,
			"pvc_midline", 0x007D6100, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"Midline Right");	
}

void workspace_makemachineview(Workspace* self, psy_Property* visual)
{
	psy_Property* mvc;
	
	assert(self);

	mvc = psy_property_settext(
		psy_property_append_section(visual, "machineview"),
			"settingsview.machineview");
	psy_property_settext(
		psy_property_append_bool(mvc, "drawmachineindexes", TRUE),
		"settingsview.draw-machine-indexes");
	psy_property_settext(
		psy_property_append_bool(mvc, "drawvumeters", TRUE),
		"settingsview.draw-vu-meters");
	psy_property_settext(
		psy_property_append_bool(mvc, "drawwirehover", FALSE),
		"settingsview.draw-wire-hover");
	workspace_makemachineviewtheme(self, mvc);
}

void workspace_makemachineviewtheme(Workspace* self, psy_Property* view)
{
	assert(self);

	self->machineviewtheme = psy_property_settext(
		psy_property_append_section(view, "theme"),
			"settingsview.theme");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"vu2", 0x00403731, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.vu-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"vu1", 0x0080FF80, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.vu-bar");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"vu3", 0x00262bd7, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.onclip");
	psy_property_settext(
		psy_property_append_string(self->machineviewtheme,
		"generator_fontface", "Tahoma"),
		"settingsview.generators-font-face");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"generator_font_point", 0x00000050, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.generators-font-point");
	psy_property_settext(psy_property_sethint(
		psy_property_append_int(self->machineviewtheme,
			"generator_font_flags", 0x00000000, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.generator_font_flags");
	psy_property_settext(psy_property_append_string(self->machineviewtheme,
		"effect_fontface", "Tahoma"),
		"settingsview.effect_fontface");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"effect_font_point", 0x00000050, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.effect_font_point");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"effect_font_flags", 0x00000000, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.effect_font_flags");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"mv_colour", 0x00232323, 0, 0), //
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"mv_wirecolour", 0x005F5F5F, 0, 0),//
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.wirecolour");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"mv_wirecolour2", 0x005F5F5F, 0, 0),//
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.wirecolour2");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"mv_polycolour", 0x00B1C8B0, 0, 0),//
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.polygons");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"mv_generator_fontcolour", 0x00B1C8B0, 0, 0),//
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.generators-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"mv_effect_fontcolour", 0x00D1C5B6, 0, 0),//
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.effects-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"mv_wirewidth", 0x00000001, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.wire-width");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"mv_wireaa", 0x01, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.antialias-halo");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"machine_background", 0, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.machine-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->machineviewtheme,
			"mv_triangle_size", 0x0A, 0, 0),//
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.polygon-size");
	psy_property_append_string(self->machineviewtheme, "machine_skin", "");//		
}

void workspace_makeparamview(Workspace* self, psy_Property* visual)
{	
	psy_Property* paramview;

	assert(self);

	paramview = psy_property_settext(
		psy_property_append_section(visual, "paramview"),
		"settingsview.native-machine-parameter-window");
	psy_property_settext(
		psy_property_append_font(paramview, "font", PSYCLE_DEFAULT_FONT),
		"settingsview.font");
	psy_property_settext(
		psy_property_append_action(paramview, "loadcontrolskin"),
		"settingsview.load-dial-bitmap");
	psy_property_settext(
		psy_property_append_bool(paramview, "showaswindow", 1),
		"settingsview.show-as-window");
	workspace_makeparamtheme(self, paramview);
}

void workspace_makeparamtheme(Workspace* self, psy_Property* view)
{
	assert(self);

	self->paramtheme = psy_property_settext(
		psy_property_append_section(view, "theme"),
		"theme");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguititlecolor", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.title-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguititlefontcolor", 0x00B4B4B4, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.title-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguitopcolor", 0x00555555, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.param-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguifonttopcolor", 0x00CDCDCD, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.param-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguibottomcolor", 0x00444444, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.value-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguifontbottomcolor", 0x00E7BD18, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.value-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguihtopcolor", 0x00555555, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.selparam-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguihfonttopcolor", 0x00CDCDCD, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.selparam-font");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguihbottomcolor", 0x00292929, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.selvalue-background");
	psy_property_settext(
		psy_property_sethint(psy_property_append_int(self->paramtheme,
			"machineguihfontbottomcolor", 0x00E7BD18, 0, 0),
			PSY_PROPERTY_HINT_EDITCOLOR),
			"settingsview.selvalue-font");	
}

void workspace_makekeyboard(Workspace* self)
{	
	assert(self);

	self->keyboard = psy_property_settext(
		psy_property_append_section(&self->config, "keyboard"),
		"settingsview.keyboard-and-misc");
	psy_property_settext(
		psy_property_append_bool(self->keyboard, 
		"recordtweaksastws", 0),
		"settingsview.record-tws");
	psy_property_settext(
		psy_property_append_bool(self->keyboard, 
		"advancelineonrecordtweak", 0),
		"settingsview.advance-line-on-record");
}

void workspace_makedirectories(Workspace* self)
{	
	const char* home;
#if defined(DIVERSALIS__OS__MICROSOFT)		
#else
	char path[4096];
#endif

	assert(self);

	home = psy_dir_home();
	self->directories = psy_property_settext(
		psy_property_append_section(&self->config, "directories"),
			"settingsview.directories");
	psy_property_sethint(
		psy_property_settext(
			psy_property_append_string(self->directories, "app", PSYCLE_APP_DIR),
			"App directory"),
		PSY_PROPERTY_HINT_HIDE);
#if defined(DIVERSALIS__OS__MICROSOFT)		
	workspace_makedirectory(self, "songs", "settingsview.song-directory",
		PSYCLE_SONGS_DEFAULT_DIR);	
#else
	psy_snprintf(path, 4096, "%s", home);
	printf("path %s\n", path);
	workspace_makedirectory(self, "songs", "settingsview.song-directory",
		path);	
#endif		
	workspace_makedirectory(self, "samples", "settingsview.samples-directory",
		PSYCLE_SAMPLES_DEFAULT_DIR);	
	workspace_makedirectory(self, "plugins", "settingsview.plug-in-directory",
		PSYCLE_PLUGINS_DEFAULT_DIR);
	workspace_makedirectory(self, "luascripts", "settingsview.lua-scripts-directory",
		PSYCLE_LUASCRIPTS_DEFAULT_DIR);
	workspace_makedirectory(self, "vsts32", "settingsview.vst-directories",
		PSYCLE_VSTS32_DEFAULT_DIR);
	workspace_makedirectory(self, "vsts64", "settingsview.vst64-directories",
		PSYCLE_VSTS64_DEFAULT_DIR);
	workspace_makedirectory(self, "ladspas", "settingsview.ladspa-directories",
		PSYCLE_LADSPAS_DEFAULT_DIR);
	workspace_makedirectory(self, "skin", "settingsview.skin-directory",
		"C:\\Programme\\Psycle\\Skins");
	workspace_makedefaultuserpresetpath(self);
}

void workspace_makedefaultuserpresetpath(Workspace* self)
{
	assert(self);

	psy_Path defaultuserpresetpath;

	psy_path_init(&defaultuserpresetpath, psy_dir_home());
	psy_path_setname(&defaultuserpresetpath, "Presets");
	workspace_makedirectory(self, "presets", "User Presets directory",
		psy_path_path(&defaultuserpresetpath));
	psy_path_dispose(&defaultuserpresetpath);
}

void workspace_makecompatibility(Workspace* self)
{
	assert(self);

	self->compatibility = psy_property_settext(
		psy_property_append_section(&self->config, "compatibility"),
			"settingsview.compatibility");
	psy_property_settext(
		psy_property_append_bool(self->compatibility, "loadnewgamefxblitz", 0),
		"settingsview.jme-version-unknown");
}

void workspace_makedirectory(Workspace* self, const char* key,
	const char* label, const char* defaultdir)
{
	assert(self);

	psy_property_sethint(
		psy_property_settext(
			psy_property_append_string(self->directories, key, defaultdir),
			label),
		PSY_PROPERTY_HINT_EDITDIR);
}

void workspace_makeinputoutput(Workspace* self)
{		
	assert(self);

	self->inputoutput = psy_property_settext(
		psy_property_append_section(&self->config, "inputoutput"),
		"Input/Output");
	workspace_makedriverlist(self);
	self->driverconfigure = psy_property_settext(
		psy_property_append_section(self->inputoutput, "configure"),
			"settingsview.configure");
	self->driverconfigure->item.save = 0;
	workspace_makedriverconfigurations(self);
}

void workspace_makedriverlist(Workspace* self)
{
	psy_Property* drivers;

	assert(self);

	// change number to set startup driver, if no psycle.ini found
#if defined(DIVERSALIS__OS__MICROSOFT)	
	// 2 : directx
	drivers = psy_property_settext(psy_property_append_choice(
		self->inputoutput, "driver", 2),
		"settingsview.audio-drivers");
#elif defined(DIVERSALIS__OS__LINUX)
	// 1 : alsa
	drivers = psy_property_settext(psy_property_append_choice(
		self->inputoutput, "driver", 1),
		"settingsview.audio-drivers");
#endif		
	psy_property_append_string(drivers, "silent", "silentdriver");
#if defined(DIVERSALIS__OS__MICROSOFT)
	// output target for the audio driver dlls is {solutiondir}/Debug or 
	// {solutiondir}/Release
	// if they aren't found, check if direcories fit and if
	// dlls are compiled
	psy_property_append_string(drivers, "mme", ".\\mme.dll");
	psy_property_append_string(drivers, "directx", ".\\directx.dll");
	psy_property_append_string(drivers, "wasapi", ".\\wasapi.dll");
	psy_property_append_string(drivers, "asio", ".\\asiodriver.dll");
#elif defined(DIVERSALIS__OS__LINUX)
	psy_property_append_string(drivers, "alsa",
		"../../driver/alsa/libpsyalsa.so");
#endif
}

void workspace_makedriverconfigurations(Workspace* self)
{
	psy_Property* drivers;
	
	assert(self);

	self->driverconfigurations = psy_property_sethint(
		psy_property_append_section(self->inputoutput, "configurations"),
		PSY_PROPERTY_HINT_HIDE);	
	drivers = psy_property_at(self->inputoutput, "driver", PSY_PROPERTY_TYPE_NONE);
	if (drivers) {
		psy_List* p;

		for (p = psy_property_children(drivers); p != NULL; psy_list_next(&p)) {
			psy_Property* property;

			property = (psy_Property*)psy_list_entry(p);
			if (psy_property_type(property) == PSY_PROPERTY_TYPE_STRING) {
				const char* path;

				path = psy_property_item_str(property);
				if (path) {
					psy_Library library;

					psy_library_init(&library);
					psy_library_load(&library, path);
					printf("driverconfigurations %s\n", path);
					if (library.module) {
						pfndriver_create fpdrivercreate;

						fpdrivercreate = (pfndriver_create)
							psy_library_functionpointer(&library,
								"driver_create");
						if (fpdrivercreate) {
							psy_AudioDriver* driver;							

							driver = fpdrivercreate();
							printf("driverconfigurations create \n");
							if (driver &&
									psy_audiodriver_configuration(driver) &&
									!psy_property_empty(psy_audiodriver_configuration(driver))) {
								printf("driversection create\n");															
									psy_property_append_property(
										self->driverconfigurations,
										psy_property_clone(
											psy_audiodriver_configuration(driver)));
								psy_audiodriver_dispose(driver);								
							}
						}
					}
					psy_library_dispose(&library);
				}
			}
		}
	}
}

void workspace_makeeventinput(Workspace* self)
{
	assert(self);

	self->eventinputs = psy_property_settext(
		psy_property_append_section(&self->config, "eventinput"),
		"Event Input");
	workspace_makeeventinputdriverlist(self);
	self->eventdriverconfigurations = psy_property_sethint(
		psy_property_append_section(self->eventinputs, "configurations"),
		PSY_PROPERTY_HINT_HIDE);
}

void workspace_makeeventinputdriverlist(Workspace* self)
{		
	psy_Property* installed;
	psy_Property* active;
		
	assert(self);

	psy_property_settext(self->eventinputs, "settingsview.event-input");
	// change number to set startup driver, if no psycle.ini found
	installed = psy_property_append_choice(self->eventinputs, "installeddriver", 0);
	psy_property_settext(installed, "Input Drivers");
	psy_property_append_string(installed, "kbd", "kbd");	
#if defined(DIVERSALIS__OS__MICROSOFT)
	psy_property_append_string(installed, "mmemidi", ".\\mmemidi.dll");
	psy_property_append_string(installed, "dxjoystick", ".\\dxjoystick.dll");
#endif	
	psy_property_settext(
		psy_property_append_action(self->eventinputs, "addeventdriver"),
		"Add to active drivers");
	active = psy_property_settext(
		psy_property_append_choice(self->eventinputs, "activedrivers", 0),
		"Active Drivers");
	psy_property_settext(
		psy_property_append_action(self->eventinputs, "removeeventdriver"),
			"Remove active driver");
	self->midiconfigure = psy_property_settext(
		psy_property_append_section(self->eventinputs,
			"configure"),
			"Configure");
}

void workspace_registerinputdrivers(Workspace* self)
{
	psy_Property* drivers;	

	assert(self);

	drivers = psy_property_at(self->eventinputs, "installeddriver",
		PSY_PROPERTY_TYPE_NONE);
	if (drivers) {
		psy_List* p;

		for (p = psy_property_children(drivers); p != NULL; psy_list_next(&p)) {
			psy_Property* property;
			int guid;

			property = (psy_Property*)psy_list_entry(p);
			if (psy_property_type(property) == PSY_PROPERTY_TYPE_STRING) {
				guid = psy_audio_eventdrivers_guid(&self->player.eventdrivers,
					psy_property_item_str(property));
				if (guid != -1) {
					psy_audio_eventdrivers_register(&self->player.eventdrivers,
						guid, psy_property_item_str(property));
				}
			}
		}
	}
}

void workspace_configeventinput(Workspace* self)
{
	psy_Property* activedrivers;

	assert(self);

	activedrivers = psy_property_at(self->eventinputs, "activedrivers",
		PSY_PROPERTY_TYPE_NONE);	
	if (activedrivers) {
		psy_List* p;

		for (p = psy_property_children(activedrivers); p != NULL;
				psy_list_next(&p)) {
			psy_Property* property;

			property = (psy_Property*)psy_list_entry(p);
			if (psy_property_type(property) == PSY_PROPERTY_TYPE_INTEGER) {
				psy_EventDriver* driver;

				driver = psy_audio_eventdrivers_loadbyguid(
					&self->player.eventdrivers,
					psy_property_item_int(property));
				if (driver) {
					psy_eventdriver_setcmddef(driver, self->cmds);
				}
			}
		}
	}
}

void workspace_updateactiveeventdriverlist(Workspace* self)
{	
	psy_Property* drivers;
	
	assert(self);

	drivers = psy_property_at(self->eventinputs, "activedrivers",
		PSY_PROPERTY_TYPE_NONE);
	if (drivers) {
		uintptr_t numdrivers;
		uintptr_t i;

		psy_property_clear(drivers);	
		numdrivers = psy_audio_player_numeventdrivers(&self->player);
		for (i = 0; i < numdrivers; ++i) {
			psy_audio_EventDriverEntry* entry;

			entry = psy_audio_eventdrivers_entry(&self->player.eventdrivers, i);
			if (entry && entry->eventdriver) {
				const psy_EventDriverInfo* driverinfo;				
								
				driverinfo = psy_eventdriver_info(entry->eventdriver);
				if (driverinfo) {
					char key[40];

					psy_snprintf(key, 40, "guid-%d", (int)i);
					psy_property_settext(
						psy_property_append_int(drivers, key,
							driverinfo->guid, 0, 0),
						driverinfo->Name);
				}
			}
		}
	}
}

void workspace_makeeventdriverconfigurations(Workspace* self)
{	
	int numdrivers;
	int i;

	assert(self);

	psy_property_clear(self->eventdriverconfigurations);
	numdrivers = psy_audio_player_numeventdrivers(&self->player);
	for (i = 0; i < numdrivers; ++i) {
		psy_audio_EventDriverEntry* entry;

		entry = psy_audio_eventdrivers_entry(&self->player.eventdrivers, i);
		if (entry) {
			psy_EventDriver* driver;

			driver = entry->eventdriver;
			if (driver && driver->properties) {								
				psy_property_append_property(self->eventdriverconfigurations,
					psy_property_clone(driver->properties));				
			}
		}
	}
}

void workspace_readeventdriverconfigurations(Workspace* self)
{		
	assert(self);

	if (self->eventdriverconfigurations) {
		psy_List* p;
		
		for (p = psy_property_children(self->eventdriverconfigurations);
				p != NULL; psy_list_next(&p)) {
			psy_Property* configuration;

			configuration = (psy_Property*)p->entry;
			if (!psy_property_empty(configuration)) {
				int guid;

				guid = psy_property_at_int(configuration, "guid", -1);
				if (guid != -1) {
					uintptr_t numdrivers;
					uintptr_t i;

					numdrivers = psy_audio_eventdrivers_size(&self->player.eventdrivers);
					for (i = 0; i < numdrivers; ++i) {
						psy_EventDriver* driver;

						driver = psy_audio_eventdrivers_driver(&self->player.eventdrivers, i);
						if (driver) {
							const psy_EventDriverInfo* driverinfo;

							driverinfo = psy_eventdriver_info(driver);
							if (driverinfo) {
								if (driverinfo->guid == guid) {
									psy_eventdriver_configure(driver,
										configuration);
									break;
								}
							}
						}
					}
				}
			}
		}							
	}
}

const char* workspace_translate(Workspace* self, const char* key)
{			
	assert(self);

	return translator_translate(&self->translator, key);
}

void workspace_configchanged(Workspace* self, psy_Property* property, 
	psy_Property* choice)
{
	assert(self);

	if (psy_property_type(property) == PSY_PROPERTY_TYPE_ACTION) {
		if (strcmp(psy_property_key(property), "loadskin") == 0) {
			psy_ui_OpenDialog opendialog;

			psy_ui_opendialog_init_all(&opendialog, 0,
				"Load Theme",
				"Psycle Display psy_audio_Presets|*.psv", "PSV",
				workspace_skins_directory(self));
			if (psy_ui_opendialog_execute(&opendialog)) {			
				workspace_loadskin(self, psy_ui_opendialog_filename(
					&opendialog));
			}
			psy_ui_opendialog_dispose(&opendialog);
		} else if (strcmp(psy_property_key(property), "defaultskin") == 0) {
			psy_Property* view;
			psy_Property* theme;
			
			view = psy_property_findsection(&self->config, "visual.patternview");
			theme = psy_property_findsection(view, "theme");
			if (theme) {
				psy_property_remove(view, theme);
			}
			workspace_makepatternviewtheme(self, view);
			view = psy_property_findsection(&self->config, "visual.machineview");
			theme = psy_property_findsection(view, "theme");
			if (theme) {
				psy_property_remove(view, theme);
			}
			workspace_makemachineviewtheme(self, view);
			view = psy_property_findsection(&self->config, "visual.paramview");
			theme = psy_property_findsection(view, "theme");
			if (theme) {
				psy_property_remove(view, theme);
			}
			workspace_makeparamtheme(self, view);
			psy_signal_emit(&self->signal_skinchanged, self, 0);
		} else if (strcmp(psy_property_key(property), "loadcontrolskin") == 0) {
			psy_ui_OpenDialog opendialog;

			psy_ui_opendialog_init_all(&opendialog, 0,
				"Load Dial Bitmap",
				"Control Skins|*.psc|Bitmaps|*.bmp", "psc",
				workspace_skins_directory(self));
			if (psy_ui_opendialog_execute(&opendialog)) {
				workspace_loadcontrolskin(self, psy_ui_opendialog_filename(
					&opendialog));
			}
			psy_ui_opendialog_dispose(&opendialog);
		} else if (strcmp(psy_property_key(property), "addeventdriver") == 0) {
			psy_Property* installeddriver;			

			installeddriver = psy_property_at(self->eventinputs, "installeddriver",
				PSY_PROPERTY_TYPE_CHOICE);
			if (installeddriver) {
				psy_Property* choice;

				choice = psy_property_at_choice(installeddriver);
				if (choice) {
					psy_EventDriver* driver;
					psy_Property* activedrivers;					 

					driver = psy_audio_player_loadeventdriver(&self->player,
						psy_property_item_str(choice));
					if (driver) {
						psy_eventdriver_setcmddef(driver, self->cmds);
					}
					workspace_updateactiveeventdriverlist(self);
					activedrivers = psy_property_at(self->eventinputs, "activedrivers",
						PSY_PROPERTY_TYPE_CHOICE);
					if (activedrivers) {
						psy_property_setitem_int(activedrivers,						
							psy_audio_player_numeventdrivers(&self->player) - 1);
						workspace_showactiveeventinputconfig(self,
							psy_property_item_int(activedrivers));
					}
				}
			}			
		} else if (strcmp(psy_property_key(property), "removeeventdriver") == 0) {
			psy_Property* activedrivers;

			activedrivers = psy_property_at(self->eventinputs, "activedrivers",
				PSY_PROPERTY_TYPE_CHOICE);
			if (activedrivers) {				
				psy_audio_player_removeeventdriver(&self->player,
					psy_property_item_int(activedrivers));
				workspace_updateactiveeventdriverlist(self);
				if (psy_property_item_int(activedrivers) > 0) {
					psy_property_setitem_int(activedrivers,
						psy_property_item_int(activedrivers) - 1);
				}				
				workspace_showactiveeventinputconfig(self,
					psy_property_item_int(activedrivers));
			}
		}
	} else if (psy_property_insection(property, self->driverconfigure)) {
		if (psy_audiodriver_configuration(self->player.driver) != NULL) {
			psy_Property* driversection;
						
			driversection = psy_property_find(self->driverconfigure,
				psy_property_key(psy_audiodriver_configuration(
					self->player.driver)),
				PSY_PROPERTY_TYPE_NONE);
			if (driversection) {
				psy_audio_player_restartdriver(&self->player, driversection);
				workspace_update_driverconfigure_section(self);				
			}
		}
		return;
	} else if (choice && psy_property_insection(property, self->eventinputs)) {
		if (strcmp(psy_property_key(choice), "activedrivers") == 0) {
			workspace_showactiveeventinputconfig(self, psy_property_item_int(choice));	
		} else {		
			psy_Property* p;

			p = psy_property_find(self->eventinputs, "activedrivers", PSY_PROPERTY_TYPE_SECTION);
			if (p) {
				psy_audio_player_restarteventdriver(&self->player, psy_property_item_int(p));
			}
		}
	} else if (psy_property_insection(property, self->eventinputs)) {
		psy_Property* p;

		p = psy_property_at(self->eventinputs, "activedrivers", PSY_PROPERTY_TYPE_NONE);
		if (p) {
			psy_audio_player_restarteventdriver(&self->player, psy_property_item_int(p));
		}		
	} else if (choice && (strcmp(psy_property_key(choice), "driver") == 0)) {		
		psy_Property* driversection = NULL;

		psy_audio_player_unloaddriver(&self->player);
		psy_audio_player_loaddriver(&self->player, workspace_driverpath(self),
			NULL /*no config*/, FALSE /*do not open yet*/);
		if (psy_audiodriver_configuration(self->player.driver)) {
			driversection = psy_property_find(self->driverconfigurations,
				psy_property_key(psy_audiodriver_configuration(
					self->player.driver)),
				PSY_PROPERTY_TYPE_NONE);
		}
		psy_audio_player_restartdriver(&self->player, driversection);
		psy_property_clear(self->driverconfigure);
		if (psy_audiodriver_configuration(self->player.driver)) {
			psy_property_append_property(self->driverconfigure,
				psy_property_clone(psy_audiodriver_configuration(
					self->player.driver)));
		}				
	} else if (choice && (strcmp(psy_property_key(choice), "lang") == 0)) {
		workspace_configlanguage(self);
	} else if (strcmp(psy_property_key(property), "defaultfont") == 0) {
		psy_ui_Font font;
		psy_ui_FontInfo fontinfo;
		
		psy_ui_fontinfo_init_string(&fontinfo, 
			psy_property_item_str(property));
		psy_ui_font_init(&font, &fontinfo);
		psy_ui_replacedefaultfont(self->mainhandle, &font);
		psy_ui_component_invalidate(self->mainhandle);
		psy_signal_emit(&self->signal_defaultfontchanged, self, 0);
	} else if (strcmp(psy_property_key(property), "drawvumeters") == 0) {
		if (psy_property_item_str(property)) {
			psy_audio_player_setvumetermode(&self->player, VUMETER_RMS);
		} else {
			psy_audio_player_setvumetermode(&self->player, VUMETER_NONE);
		}
	}
	psy_signal_emit(&self->signal_configchanged, self, 1, property);
}

int workspace_showsonginfoonload(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(&self->config, "general.showsonginfoonload", 1);
}

int workspace_showaboutatstart(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(&self->config, "general.showaboutatstart", 1);	
}

int workspace_showmaximizedatstart(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(&self->config, "general.showmaximizedatstart", 1);	
}

int workspace_saverecentsongs(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "saverecentsongs", 1);
}

int workspace_playsongafterload(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "playsongafterload", 1);
}

int workspace_showplaylisteditor(Workspace* self)
{	
	assert(self);

	return psy_property_at_bool(&self->config, "general.showplaylisteditor",
		0);
}

int workspace_showstepsequencer(Workspace* self)
{	
	assert(self);

	return psy_property_at_bool(&self->config, "general.showstepsequencer", 0);
}

int workspace_showgriddefaults(Workspace* self)
{	
	assert(self);

	return psy_property_at_bool(&self->config,
		"visual.patternview.griddefaults", 1);	
}

int workspace_showlinenumbers(Workspace* self)
{	
	assert(self);

	return psy_property_at_bool(&self->config,
		"visual.patternview.linenumbers", 1);
}

int workspace_showbeatoffset(Workspace* self)
{	
	assert(self);

	return psy_property_at_bool(&self->config,
		"visual.patternview.beatoffset", 1);	
}

int workspace_showlinenumbercursor(Workspace* self)
{	
	assert(self);

	return psy_property_at_bool(&self->config,
		"visual.patternview.linenumberscursor", 1);
}

int workspace_showlinenumbersinhex(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(&self->config,
		"visual.patternview.linenumbersinhex", 1);
}

int workspace_showwideinstcolumn(Workspace* self)
{	
	assert(self);

	return psy_property_at_bool(&self->config,
		"visual.patternview.wideinstcolumn", 1);
}

int workspace_showtrackscopes(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(&self->config,
		"visual.patternview.trackscopes", 1);
}

int workspace_wraparound(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(&self->config,
		"visual.patternview.wraparound", 1);
}

int workspace_ismovecursorwhenpaste(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(&self->config,
		"visual.patternview.movecursorwhenpaste", 1);
}

void workspace_movecursorwhenpaste(Workspace* self, bool on)
{
	assert(self);

	psy_property_set_bool(&self->config,
		"visual.patternview.movecursorwhenpaste", on);
}

int workspace_showmachineindexes(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(&self->config,
		"visual.machineview.drawmachineindexes", 1);
}

int workspace_showwirehover(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(&self->config,
		"visual.machineview.drawwirehover", 1);
}

int workspace_showparamviewaswindow(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(&self->config,
		"visual.paramview.showaswindow", 1);
}

void workspace_newsong(Workspace* self)
{			
	psy_audio_Song* song;	
	
	assert(self);

	song = psy_audio_song_allocinit(&self->machinefactory);
	free(self->filename);
	self->filename = strdup("Untitled.psy");	
	workspace_setsong(self, song, WORKSPACE_NEWSONG, 0);
	workspace_selectview(self, TABPAGE_MACHINEVIEW, 0, 0);
}

void workspace_loadsong(Workspace* self, const char* path, bool play)
{	
	psy_audio_Song* song;
	psy_audio_SongFile songfile;

	assert(self);

	song = psy_audio_song_allocinit(&self->machinefactory);
	if (song) {
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
		self->songcbk = song;
		if (psy_audio_songfile_load(&songfile, path) != PSY_OK) {
			self->songcbk = self->song;			
			psy_audio_song_deallocate(song);
			psy_signal_emit(&self->signal_terminal_error, self, 1,
				songfile.serr);
			psy_audio_songfile_dispose(&songfile);
			play = FALSE;
		} else {
			free(self->filename);
			self->filename = strdup(path);
			workspace_setsong(self, song, WORKSPACE_LOADSONG, &songfile);
			psy_audio_songfile_dispose(&songfile);
			workspace_addrecentsong(self, path);
			psy_audio_songfile_dispose(&songfile);
		}
		psy_signal_emit(&self->signal_terminal_out, self, 1,
			"ready\n");
		if (play) {
			psy_audio_player_start(&self->player);
		}
	}	
}

void workspace_addrecentsong(Workspace* self, const char* filename)
{
	assert(self);

	if (workspace_saverecentsongs(self) &&
			!psy_property_find(self->recentfiles, filename,
				PSY_PROPERTY_TYPE_NONE)) {
		psy_Path path;		

		psy_path_init(&path, filename);
		psy_property_settext(psy_property_setreadonly(
			psy_property_append_string(self->recentfiles,
				filename, ""), TRUE), psy_path_name(&path));
		workspace_save_recentsongs(self);
		psy_path_dispose(&path);
	}
}

void workspace_onloadprogress(Workspace* self, psy_audio_Song* sender, int progress)
{
	assert(self);

	psy_signal_emit(&self->signal_loadprogress, self, 1, progress);
}

void workspace_setsong(Workspace* self, psy_audio_Song* song, int flag, psy_audio_SongFile* songfile)
{
	psy_audio_Song* oldsong;
	
	assert(self);

	oldsong = self->song;
	psy_audio_player_stop(&self->player);
	psy_audio_player_setemptysong(&self->player);	
	workspace_disposesequencepaste(self);
	psy_audio_sequenceselection_setsequence(&self->sequenceselection
		, &song->sequence);
	history_clear(&self->history);
	workspace_addhistory(self);
	self->lastentry = 0;
	psy_audio_exclusivelock_enter();
	self->song = song;
	self->songcbk = song;
	psy_audio_player_setsong(&self->player, self->song);	
	psy_audio_exclusivelock_leave();
	psy_signal_emit(&self->signal_songchanged, self, 2, flag, songfile);			
	psy_audio_song_deallocate(oldsong);	
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
	psy_signal_emit(&self->signal_terminal_out, self, 1,
		"ready\n");
}

void workspace_loadskin(Workspace* self, const char* path)
{
	psy_Property* skin;
		
	assert(self);

	skin = psy_property_allocinit_key(NULL);
	skin_load(skin, path);	
	psy_property_sync(self->paramtheme, skin);
	psy_property_sync(self->machineviewtheme, skin);
	psy_property_sync(self->patternviewtheme, skin);
	psy_signal_emit(&self->signal_skinchanged, self, 0);
	psy_property_deallocate(skin);
}

void workspace_loadcontrolskin(Workspace* self, const char* path)
{
	assert(self);

	free(self->dialbitmappath);
	self->dialbitmappath = strdup(path);
	psy_signal_emit(&self->signal_changecontrolskin, self, 1, path);
}

psy_Property* workspace_pluginlist(Workspace* self)
{
	assert(self);

	return self->plugincatcher.plugins;
}

psy_Property* workspace_recentsongs(Workspace* self)
{
	assert(self);

	return self->recentsongs;
}

void workspace_load_configuration(Workspace* self)
{
	psy_Path path;

	assert(self);

	psy_path_init(&path, NULL);
	psy_path_setprefix(&path, workspace_config_directory(self));
	psy_path_setname(&path, PSYCLE_INI);		
	propertiesio_load(&self->config, psy_path_path(&path), 0);	
	workspace_configlanguage(self);
	{
		psy_Property* driversection = NULL;

		psy_audio_player_loaddriver(&self->player, workspace_driverpath(self),
			NULL /*no config*/, FALSE /*do not open yet*/);
		if (psy_audiodriver_configuration(self->player.driver)) {
			driversection = psy_property_find(self->driverconfigurations,
				psy_property_key(
					psy_audiodriver_configuration(self->player.driver)),
				PSY_PROPERTY_TYPE_NONE);			
		}
		psy_audio_player_restartdriver(&self->player, driversection);
		workspace_update_driverconfigure_section(self);
	}
	workspace_configeventinput(self);
	psy_audio_eventdrivers_restartall(&self->player.eventdrivers);
	workspace_updateactiveeventdriverlist(self);
	workspace_makeeventdriverconfigurations(self);
	propertiesio_load(&self->config, psy_path_path(&path), 0);
	workspace_readeventdriverconfigurations(self);
	workspace_showactiveeventinputconfig(self,
		workspace_curreventinputconfiguration(self));
	workspace_configvisual(self);
	if (workspace_loadnewblitz(self)) {
		psy_audio_machinefactory_loadnewgamefxandblitzifversionunknown(
			&self->machinefactory);
	} else {
		psy_audio_machinefactory_loadoldgamefxandblitzifversionunknown(
			&self->machinefactory);
	}
	psy_signal_emit(&self->signal_configchanged, self, 1, self->config);
	psy_signal_emit(&self->signal_skinchanged, self, 0);
	psy_path_dispose(&path);
}

void workspace_save_configuration(Workspace* self)
{
	psy_Path path;

	assert(self);

	psy_path_init(&path, NULL);
	psy_path_setprefix(&path, workspace_config_directory(self));
	psy_path_setname(&path, PSYCLE_INI);
	workspace_makeeventdriverconfigurations(self);
	propertiesio_save(&self->config, psy_path_path(&path));
	psy_path_dispose(&path);
}

void workspace_load_recentsongs(Workspace* self)
{	
	psy_List* p;
	psy_Path path;

	assert(self);

	psy_path_init(&path, NULL);
	psy_path_setprefix(&path, workspace_config_directory(self));
	psy_path_setname(&path, PSYCLE_RECENT_SONG_INI);	
	propertiesio_load(self->recentsongs, psy_path_path(&path), 1);
	psy_path_dispose(&path);
	if (self->recentfiles) {
		for (p = psy_property_children(self->recentfiles); p != NULL;
				psy_list_next(&p)) {
			psy_Property* property;

			property = (psy_Property*)psy_list_entry(p);
			psy_path_init(&path, psy_property_key(property));
			psy_property_settext(property, psy_path_name(&path));
			psy_property_setreadonly(property, TRUE);
			psy_path_dispose(&path);
		}
	}
}

void workspace_save_recentsongs(Workspace* self)
{
	psy_Path path;

	assert(self);

	psy_path_init(&path, NULL);
	psy_path_setprefix(&path, workspace_config_directory(self));
	psy_path_setname(&path, PSYCLE_RECENT_SONG_INI);	
	propertiesio_save(self->recentsongs, psy_path_path(&path));
	psy_path_dispose(&path);
}

void workspace_clearrecentsongs(Workspace* self)
{
	psy_Path path;
	
	assert(self);

	psy_path_init(&path, NULL);
	psy_path_setprefix(&path, workspace_config_directory(self));
	psy_path_setname(&path, PSYCLE_RECENT_SONG_INI);
	psy_property_clear(self->recentfiles);
	propertiesio_save(self->recentsongs, psy_path_path(&path));
	psy_path_dispose(&path);
}

void workspace_setoctave(Workspace* self, int octave)
{
	assert(self);

	self->octave = octave;
	psy_signal_emit(&self->signal_octavechanged, self, 1, octave);
}

int workspace_octave(Workspace* self)
{
	assert(self);

	return self->octave;
}

void workspace_undo(Workspace* self)
{
	assert(self);

	if (workspace_currview(self) == TABPAGE_PATTERNVIEW) {
		psy_undoredo_undo(&self->undoredo);
	} else if (workspace_currview(self) == TABPAGE_MACHINEVIEW) {
		psy_undoredo_undo(&self->song->machines.undoredo);
	}
}

void workspace_redo(Workspace* self)
{
	assert(self);

	if (workspace_currview(self) == TABPAGE_PATTERNVIEW) {
		psy_undoredo_redo(&self->undoredo);
	} else if (workspace_currview(self) == TABPAGE_MACHINEVIEW) {
		psy_undoredo_redo(&self->song->machines.undoredo);
	}
}

bool workspace_currview_hasundo(Workspace* self)
{
	assert(self);

	if (workspace_currview(self) == TABPAGE_PATTERNVIEW) {
		return psy_list_size(self->undoredo.undo) != 0;
	} else if (workspace_currview(self) == TABPAGE_MACHINEVIEW) {
		return psy_list_size(self->song->machines.undoredo.undo) != 0;
	}
	return FALSE;
}

bool workspace_currview_hasredo(Workspace* self)
{
	assert(self);

	if (workspace_currview(self) == TABPAGE_PATTERNVIEW) {
		return psy_list_size(self->undoredo.redo) != 0;
	} else if (workspace_currview(self) == TABPAGE_MACHINEVIEW) {
		return psy_list_size(self->song->machines.undoredo.redo) != 0;
	}
	return FALSE;
}

void workspace_changedefaultfontsize(Workspace* self, int size)
{
	psy_Property* visual;

	assert(self);

	visual = psy_property_find(&self->config, "visual",
		PSY_PROPERTY_TYPE_SECTION);
	if (visual) {
		psy_ui_FontInfo fontinfo;
		psy_ui_Font font;
			
		psy_ui_fontinfo_init_string(&fontinfo, 
			psy_property_at_str(visual, "defaultfont",
			PSYCLE_DEFAULT_FONT));
		fontinfo.lfHeight = size;	
		psy_ui_font_init(&font, &fontinfo);
		psy_ui_replacedefaultfont(self->mainhandle, &font);
		psy_ui_component_invalidate(self->mainhandle);
		psy_signal_emit(&self->signal_defaultfontchanged, self, 0);
	}
}

void workspace_setpatterneditposition(Workspace* self,
	psy_audio_PatternEditPosition editposition)
{	
	assert(self);

	self->patterneditposition = editposition;
	self->patterneditposition.line = 
		(int) (editposition.offset * psy_audio_player_lpb(&self->player));
	psy_signal_emit(&self->signal_patterneditpositionchanged, self, 0);
}

psy_audio_PatternEditPosition workspace_patterneditposition(Workspace* self)
{
	assert(self);

	return self->patterneditposition;
}

void workspace_setsequenceselection(Workspace* self,
		psy_audio_SequenceSelection selection)
{	
	assert(self);

	self->sequenceselection = selection;	
	psy_audio_sequence_setplayselection(&self->song->sequence, &selection);
	psy_signal_emit(&self->signal_sequenceselectionchanged, self, 0);
	workspace_addhistory(self);	
}

void workspace_addhistory(Workspace* self)
{
	assert(self);

	if (self->currview != TABPAGE_CHECKUNSAVED && !self->navigating &&
			!self->history.prevented) {
		int sequencentryid = -1;

		if (self->sequenceselection.editposition.trackposition.tracknode) {
				psy_audio_SequenceEntry* entry;			
				entry = (psy_audio_SequenceEntry*)
					self->sequenceselection.editposition.trackposition.tracknode->entry;
				sequencentryid = entry->id;
		}
		history_add(&self->history, self->currview, sequencentryid);
		self->currnavigation = self->history.container->tail;
	}
}

void workspace_updatecurrview(Workspace* self)
{	
	assert(self);

	if (self->currnavigation) {
		HistoryEntry* entry;

		entry = (HistoryEntry*)(self->currnavigation->entry);
		self->navigating = 1;
		workspace_selectview(self, entry->viewid, 0, 0);
		if (entry->sequenceentryid != -1 &&
			self->sequenceselection.editposition.trackposition.tracknode) {
			psy_audio_SequencePosition position;

			position = psy_audio_sequence_positionfromid(&self->song->sequence,
				entry->sequenceentryid);
			psy_audio_sequenceselection_seteditposition(&self->sequenceselection,
				position);
			workspace_setsequenceselection(self, self->sequenceselection);
		}
		self->navigating = 0;
	}
}

int workspace_currview(Workspace* self)
{
	assert(self);

	if (self->currnavigation) {
		HistoryEntry* entry;

		entry = (HistoryEntry*)(self->currnavigation->entry);		
		return entry->viewid;		
	}
	return 0;
}

psy_audio_SequenceSelection workspace_sequenceselection(Workspace* self)
{
	assert(self);

	return self->sequenceselection;
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

int workspace_hasplugincache(Workspace* self)
{
	assert(self);

	return self->hasplugincache;
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

void workspace_onsequenceeditpositionchanged(Workspace* self,
	psy_audio_SequenceSelection* selection)
{
	psy_audio_PatternEditPosition position;
	psy_audio_SequenceEntry* entry;

	assert(self);

	if (selection->editposition.trackposition.tracknode) {
		entry = (psy_audio_SequenceEntry*)
			selection->editposition.trackposition.tracknode->entry;
		position.pattern = entry->patternslot;
		position.column = 0;
		position.digit = 0;
		position.line = 0;
		position.offset = 0;		
		position.track = 0;
		workspace_setpatterneditposition(self, position);		
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
			if (it.tracknode && self->lastentry != it.tracknode->entry) {
				psy_audio_sequenceselection_seteditposition(&self->sequenceselection, 
						psy_audio_sequence_makeposition(&self->song->sequence,
							self->song->sequence.tracks, it.tracknode));
				self->history.prevented = 1;
				workspace_setsequenceselection(self, self->sequenceselection);
				self->history.prevented = 0;
				self->lastentry = (psy_audio_SequenceEntry*) it.tracknode->entry;		
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
				workspace_setpatterneditposition(self, 
					self->patterneditposition);				
			}
		} else
		if (self->lastentry) {				
			self->patterneditposition.line = (int) (
				(psy_audio_player_position(&self->player) -
				self->lastentry->offset) * psy_audio_player_lpb(&self->player));
			self->patterneditposition.offset = 
				psy_audio_player_position(&self->player) - self->lastentry->offset;
			self->patterneditposition.offset = 
				self->patterneditposition.line / 
				(psy_dsp_big_beat_t)psy_audio_player_lpb(&self->player);
			workspace_setpatterneditposition(self, 
				self->patterneditposition);
			self->lastentry = 0;
		}		
	}
	psy_audio_eventdrivers_idle(&self->player.eventdrivers);
}

void workspace_showparameters(Workspace* self, uintptr_t machineslot)
{
	assert(self);

	psy_signal_emit(&self->signal_showparameters, self, 1, machineslot);
}

void workspace_selectview(Workspace* self, int view, uintptr_t section,
	int option)
{
	assert(self);

	self->currview = view;
	psy_signal_emit(&self->signal_viewselected, self, 3, view, section, option);
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

int workspace_recordtweaksastws(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard, 
		"recordtweaksastws", 0);
}

int workspace_advancelineonrecordtweak(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard, 
		"advancelineonrecordtweak", 0);
}

void workspace_onviewchanged(Workspace* self, int view)
{
	assert(self);

	self->currview = view;
	if (view != TABPAGE_CHECKUNSAVED) {
		workspace_addhistory(self);
	}
}

void workspace_back(Workspace* self)
{
	assert(self);

	if (self->currnavigation->prev) {		
		self->currnavigation = self->currnavigation->prev;
		workspace_updatenavigation(self);
	}
}

void workspace_forward(Workspace* self)
{
	assert(self);

	if (self->currnavigation->next) {						
		self->currnavigation = self->currnavigation->next;	
		workspace_updatenavigation(self);
	}
}

void workspace_updatenavigation(Workspace* self)
{
	HistoryEntry* entry;

	assert(self);

	entry = (HistoryEntry*) (self->currnavigation->entry);
	self->navigating = 1;
	if (self->currview != entry->viewid) {
		workspace_selectview(self, entry->viewid, 0, 0);
	}
	if (entry->sequenceentryid != -1 &&
			self->sequenceselection.editposition.trackposition.tracknode) {
		psy_audio_SequencePosition position;

		position = psy_audio_sequence_positionfromid(&self->song->sequence,
			entry->sequenceentryid);
		psy_audio_sequenceselection_seteditposition(&self->sequenceselection,
			position);
		workspace_setsequenceselection(self, self->sequenceselection);
	}
	self->navigating = 0;
}

const char* workspace_songs_directory(Workspace* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "songs",
		PSYCLE_SONGS_DEFAULT_DIR);
}

const char* workspace_samples_directory(Workspace* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "samples",
		PSYCLE_SAMPLES_DEFAULT_DIR);
}

const char* workspace_plugins_directory(Workspace* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "plugins",
		PSYCLE_PLUGINS_DEFAULT_DIR);
}

const char* workspace_luascripts_directory(Workspace* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "luascripts",
		PSYCLE_LUASCRIPTS_DEFAULT_DIR);
}

const char* workspace_vsts32_directory(Workspace* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "vsts32",
		PSYCLE_VSTS32_DEFAULT_DIR);
}

const char* workspace_vsts64_directory(Workspace* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "vsts64",
		PSYCLE_VSTS64_DEFAULT_DIR);
}

const char* workspace_ladspa_directory(Workspace* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "ladspas",
		PSYCLE_LADSPAS_DEFAULT_DIR);
}

const char* workspace_skins_directory(Workspace* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "skins",
		PSYCLE_SKINS_DEFAULT_DIR);
}

const char* workspace_doc_directory(Workspace* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "skins",
		PSYCLE_DOC_DEFAULT_DIR);
}

const char* workspace_config_directory(Workspace* self)
{
	assert(self);

	return psy_dir_config();
}

const char* workspace_userpresets_directory(Workspace* self)
{
	assert(self);

	return psy_property_at_str(self->directories, "presets",
		PSYCLE_USERPRESETS_DEFAULT_DIR);
}

bool machinecallback_fileselect_load(Workspace* self, char filter[], char inoutName[])
{
	bool rv;
	psy_ui_OpenDialog dialog;

	assert(self);

	psy_ui_opendialog_init_all(&dialog, 0,
		"Plugin File Load",
		filter,
		"",
		workspace_vsts32_directory(self));
	rv = psy_ui_opendialog_execute(&dialog);
	psy_snprintf(inoutName, _MAX_PATH, "%s",
		psy_ui_opendialog_filename(&dialog));
	psy_ui_opendialog_dispose(&dialog);
	return rv;
}

bool machinecallback_fileselect_save(Workspace* self, char filter[],
	char inoutName[])
{	
	bool rv;
	psy_ui_SaveDialog dialog;

	assert(self);

	psy_ui_savedialog_init_all(&dialog, 0,
		"Plugin File Save",
		filter,
		"",
		workspace_vsts32_directory(self));
	rv = psy_ui_savedialog_execute(&dialog);	
	psy_snprintf(inoutName, _MAX_PATH, "%s",
		psy_ui_savedialog_filename(&dialog));
	psy_ui_savedialog_dispose(&dialog);
	return rv;
}

void machinecallback_fileselect_directory(Workspace* self)
{
	assert(self);
}

psy_audio_Samples* machinecallback_samples(Workspace* self)
{
	assert(self);

	return (self->songcbk)
		? &self->songcbk->samples
		: NULL;
}


psy_dsp_big_beat_t machinecallback_beatspertick(Workspace* self)
{
	assert(self);

	return (self->songcbk)
		? 1 / (psy_dsp_big_beat_t) self->songcbk->properties.tpb
		: 1 / (psy_dsp_big_beat_t) 24.f;
}

psy_audio_Machines* machinecallback_machines(Workspace* self)
{
	assert(self);

	return (self->songcbk) ? &self->song->machines : 0;
}

psy_audio_Instruments* machinecallback_instruments(Workspace* self)
{
	assert(self);

	return (self->songcbk) ? &self->song->instruments : 0;
}

psy_audio_MachineFactory* machinecallback_machinefactory(Workspace* self)
{
	assert(self);

	return &self->machinefactory;
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

void machinecallback_output(Workspace* self, const char* text)
{
	assert(self);

	psy_signal_emit(&self->signal_terminal_out, self, 1, text);
}

const char* workspace_dialbitmap_path(Workspace* self)
{
	assert(self);

	return self->dialbitmappath;
}

void workspace_dockview(Workspace* self, psy_ui_Component* view)
{
	assert(self);

	psy_signal_emit(&self->signal_dockview, self, 1, view);
}

void workspace_setloadnewblitz(Workspace* self, int mode)
{
	assert(self);

	psy_property_set_bool(self->compatibility,
		"loadnewgamefxblitz", mode != 0);
	if (mode == 1) {
		psy_audio_machinefactory_loadnewgamefxandblitzifversionunknown(&self->machinefactory);
	} else {
		psy_audio_machinefactory_loadoldgamefxandblitzifversionunknown(&self->machinefactory);
	}
}

int workspace_loadnewblitz(Workspace* self)
{
	assert(self);

	return psy_property_at_bool(self->compatibility,
		"loadnewgamefxblitz", 0);
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

void workspace_showgear(Workspace* self)
{
	assert(self);

	psy_signal_emit(&self->signal_showgear, self, 0);
}

void workspace_onlanguagechanged(Workspace* self, Translator* sender)
{
	assert(self);

	if (!psy_property_empty(&self->config)) {
		psy_property_enumerate(&self->config, self,
			(psy_PropertyCallback)workspace_onchangelanguageenum);
	}
}

int workspace_onchangelanguageenum(Workspace* self,
	psy_Property* property, int level)
{	
	assert(self);

	psy_property_settranslation(property,
		translator_translate(&self->translator,
		psy_property_text(property)));	
	return TRUE;
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

	return (psy_property_at_bool(&self->config,
			"visual.patternview.notetab", 0))
		? psy_dsp_NOTESTAB_A440
		: psy_dsp_NOTESTAB_A220;
}
